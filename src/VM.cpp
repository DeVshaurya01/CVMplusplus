// =============================================================================
//  VM.cpp
// =============================================================================

#include "VM.h"
#include "Error.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace cvm {

namespace {

std::uint8_t readByte(CallFrame& f) {
    return f.function->chunk->code[f.ip++];
}

std::uint16_t readShort(CallFrame& f) {
    std::uint16_t lo = f.function->chunk->code[f.ip++];
    std::uint16_t hi = f.function->chunk->code[f.ip++];
    return static_cast<std::uint16_t>(lo | (hi << 8));
}

std::int32_t readInt32(CallFrame& f) {
    std::uint32_t b0 = f.function->chunk->code[f.ip++];
    std::uint32_t b1 = f.function->chunk->code[f.ip++];
    std::uint32_t b2 = f.function->chunk->code[f.ip++];
    std::uint32_t b3 = f.function->chunk->code[f.ip++];
    return static_cast<std::int32_t>(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

std::int64_t asInt(const Value& v, const char* ctx) {
    if (v.kind != ValueKind::Int)
        throw RuntimeError(std::string(ctx) + ": expected integer, got " + valueToString(v));
    return v.intVal;
}

ObjArray* asArray(const Value& v, const char* ctx) {
    if (v.kind != ValueKind::Obj || !v.obj || v.obj->kind != ObjKind::Array)
        throw RuntimeError(std::string(ctx) + ": value is not an array");
    return static_cast<ObjArray*>(v.obj.get());
}

ObjMap* asMap(const Value& v, const char* ctx) {
    if (v.kind != ValueKind::Obj || !v.obj || v.obj->kind != ObjKind::Map)
        throw RuntimeError(std::string(ctx) + ": value is not a map");
    return static_cast<ObjMap*>(v.obj.get());
}

} // namespace

void VM::resetStack() { stack_.clear(); frames_.clear(); }

void VM::run(const Chunk& mainChunk) {
    // 1. Setup the 'main' function wrapper
    auto mainFunc = std::make_shared<ObjFunction>("main", 0, std::make_shared<Chunk>(mainChunk));
    
    frames_.clear();
    frames_.push_back({mainFunc, 0, 0});
    
    // We push a dummy value at index 0 because that's where locals_[0] points
    // (In a function call, the function object itself sits at slots[0]).
    push(makeNull()); 

    while (!frames_.empty()) {
        CallFrame& frame = frames_.back();
        Chunk& chunk = *frame.function->chunk;
        
        OpCode op = static_cast<OpCode>(readByte(frame));
        switch (op) {

            case OP_CONST_INT: push(makeInt(readInt32(frame))); break;
            case OP_CONST_STR: push(makeString(chunk.names[readByte(frame)])); break;
            case OP_TRUE:      push(makeBool(true));  break;
            case OP_FALSE:     push(makeBool(false)); break;
            case OP_NULL:      push(makeNull());      break;
            case OP_POP:       pop();                 break;

            case OP_ADD: { Value b = pop(); Value a = pop(); push(makeInt(asInt(a, "ADD") + asInt(b, "ADD"))); break; }
            case OP_SUB: { Value b = pop(); Value a = pop(); push(makeInt(asInt(a, "SUB") - asInt(b, "SUB"))); break; }
            case OP_MUL: { Value b = pop(); Value a = pop(); push(makeInt(asInt(a, "MUL") * asInt(b, "MUL"))); break; }
            case OP_DIV: {
                Value b = pop(); Value a = pop();
                std::int64_t bi = asInt(b, "DIV");
                if (bi == 0) throw RuntimeError("division by zero");
                push(makeInt(asInt(a, "DIV") / bi));
                break;
            }
            case OP_NEG: push(makeInt(-asInt(pop(), "NEG"))); break;
            case OP_NOT: push(makeBool(!pop().isTruthy())); break;

            case OP_EQ: { Value b = pop(); Value a = pop(); push(makeBool(a == b)); break; }
            case OP_NE: { Value b = pop(); Value a = pop(); push(makeBool(a != b)); break; }
            case OP_LT: { Value b = pop(); Value a = pop(); push(makeBool(a < b));  break; }

            case OP_DEFINE_GLOBAL: globals_[chunk.names[readByte(frame)]] = pop(); break;
            case OP_GET_GLOBAL: {
                const std::string& name = chunk.names[readByte(frame)];
                auto it = globals_.find(name);
                if (it == globals_.end()) throw RuntimeError("undefined variable '" + name + "'");
                push(it->second);
                break;
            }
            case OP_SET_GLOBAL: {
                const std::string& name = chunk.names[readByte(frame)];
                if (globals_.find(name) == globals_.end()) throw RuntimeError("assignment to undefined variable '" + name + "'");
                globals_[name] = stack_.back();
                break;
            }

            case OP_GET_LOCAL: {
                std::uint8_t slot = readByte(frame);
                push(stack_[frame.stackBase + slot]);
                break;
            }
            case OP_SET_LOCAL: {
                std::uint8_t slot = readByte(frame);
                stack_[frame.stackBase + slot] = stack_.back();
                break;
            }

            case OP_JUMP:          frame.ip += readShort(frame); break;
            case OP_JUMP_IF_FALSE: { std::uint16_t off = readShort(frame); if (!pop().isTruthy()) frame.ip += off; break; }
            case OP_LOOP:          frame.ip -= readShort(frame); break;

            case OP_PRINT: std::cout << valueToString(pop()) << "\n"; break;
            case OP_INPUT: {
                std::string line; if (!std::getline(std::cin, line)) throw RuntimeError("input: end of stream");
                try { push(makeInt(std::stoll(line))); } catch (...) { throw RuntimeError("input: invalid integer"); }
                break;
            }

            case OP_BUILD_ARRAY: {
                int count = readByte(frame);
                auto arr = std::make_shared<ObjArray>(); arr->elements.resize(count);
                for (int i = count - 1; i >= 0; --i) arr->elements[i] = pop();
                push(makeObj(std::move(arr)));
                break;
            }
            case OP_BUILD_MAP: {
                int count = readByte(frame);
                auto m = std::make_shared<ObjMap>();
                for (int i = 0; i < count; ++i) { Value val = pop(); Value key = pop(); m->pairs[key] = val; }
                push(makeObj(std::move(m)));
                break;
            }
            case OP_INDEX_GET: {
                Value idx = pop(); Value coll = pop();
                if (coll.kind == ValueKind::Obj && coll.obj) {
                    if (coll.obj->kind == ObjKind::Array) {
                        auto* arr = asArray(coll, "INDEX_GET");
                        std::int64_t i = asInt(idx, "INDEX_GET");
                        if (i < 0 || (std::size_t)i >= arr->elements.size()) throw RuntimeError("index out of bounds");
                        push(arr->elements[(std::size_t)i]);
                    } else if (coll.obj->kind == ObjKind::Map) {
                        auto* m = asMap(coll, "INDEX_GET");
                        auto it = m->pairs.find(idx);
                        if (it == m->pairs.end()) throw RuntimeError("map key not found: " + valueToString(idx));
                        push(it->second);
                    }
                } else throw RuntimeError("indexing non-collection");
                break;
            }
            case OP_INDEX_SET: {
                Value val = pop(); Value idx = pop(); Value coll = pop();
                if (coll.kind == ValueKind::Obj && coll.obj) {
                    if (coll.obj->kind == ObjKind::Array) {
                        auto* arr = asArray(coll, "INDEX_SET");
                        std::int64_t i = asInt(idx, "INDEX_SET");
                        if (i < 0 || (std::size_t)i >= arr->elements.size()) throw RuntimeError("index out of bounds");
                        arr->elements[(std::size_t)i] = val;
                    } else if (coll.obj->kind == ObjKind::Map) {
                        asMap(coll, "INDEX_SET")->pairs[idx] = val;
                    }
                } else throw RuntimeError("indexing non-collection");
                push(std::move(val));
                break;
            }
            case OP_LEN: {
                Value v = pop();
                if (v.kind == ValueKind::Obj && v.obj) {
                    if (v.obj->kind == ObjKind::Array) push(makeInt(asArray(v, "LEN")->elements.size()));
                    else if (v.obj->kind == ObjKind::Map) push(makeInt(asMap(v, "LEN")->pairs.size()));
                    else throw RuntimeError("no length");
                } else throw RuntimeError("no length");
                break;
            }
            case OP_HAS_KEY: {
                Value k = pop(); Value m = pop();
                if (m.kind == ValueKind::Obj && m.obj && m.obj->kind == ObjKind::Map)
                    push(makeBool(asMap(m, "HAS")->pairs.count(k)));
                else throw RuntimeError("expected map");
                break;
            }

            case OP_CLOSURE: {
                push(makeObj(chunk.constants[readByte(frame)]));
                break;
            }

            case OP_CALL: {
                int argCount = readByte(frame);
                Value callee = stack_[stack_.size() - argCount - 1];
                if (callee.kind == ValueKind::Obj && callee.obj && callee.obj->kind == ObjKind::Function) {
                    auto func = std::static_pointer_cast<ObjFunction>(callee.obj);
                    if (argCount != func->arity) throw RuntimeError("expected " + std::to_string(func->arity) + " args");
                    if (frames_.size() >= 128) throw RuntimeError("stack overflow");
                    frames_.push_back({func, 0, stack_.size() - argCount - 1});
                } else throw RuntimeError("can only call functions");
                break;
            }

            case OP_RETURN: {
                Value result = pop();
                std::size_t base = frames_.back().stackBase;
                frames_.pop_back();
                if (frames_.empty()) return; // returned from top level
                stack_.resize(base); // clear locals/params/callee
                push(std::move(result));
                break;
            }

            case OP_HALT: return;
            default: throw RuntimeError("unknown opcode " + std::to_string(op));
        }
    }
}

} // namespace cvm
