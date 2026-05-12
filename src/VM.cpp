// =============================================================================
//  VM.cpp
//
//  Stack-based dispatch loop. `switch` inside `while(true)`; clean and
//  portable. Globals map persists across run() calls so the REPL builds state.
// =============================================================================

#include "VM.h"
#include "Error.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace cvm {

namespace {

std::uint8_t readByte(const Chunk& c, std::size_t& ip) {
    return c.code[ip++];
}

std::uint16_t readShort(const Chunk& c, std::size_t& ip) {
    std::uint16_t lo = c.code[ip++];
    std::uint16_t hi = c.code[ip++];
    return static_cast<std::uint16_t>(lo | (hi << 8));
}

std::int32_t readInt32(const Chunk& c, std::size_t& ip) {
    std::uint32_t b0 = c.code[ip++];
    std::uint32_t b1 = c.code[ip++];
    std::uint32_t b2 = c.code[ip++];
    std::uint32_t b3 = c.code[ip++];
    std::uint32_t u  = b0 | (b1 << 8) | (b2 << 16) | (b3 << 24);
    return static_cast<std::int32_t>(u);
}

} // namespace

void VM::resetStack() { stack_.clear(); }

void VM::run(const Chunk& chunk) {
    std::size_t ip = 0;

    auto pop = [&]() -> Value {
        if (stack_.empty()) throw RuntimeError("stack underflow");
        Value v = stack_.back();
        stack_.pop_back();
        return v;
    };
    auto push = [&](Value v) { stack_.push_back(v); };

    while (ip < chunk.code.size()) {
        OpCode op = static_cast<OpCode>(readByte(chunk, ip));
        switch (op) {
            case OP_CONST_INT: {
                std::int32_t v = readInt32(chunk, ip);
                push(static_cast<Value>(v));
                break;
            }
            case OP_TRUE:  push(1); break;
            case OP_FALSE: push(0); break;
            case OP_POP:   pop();   break;

            case OP_ADD: { Value b = pop(); Value a = pop(); push(a + b); break; }
            case OP_SUB: { Value b = pop(); Value a = pop(); push(a - b); break; }
            case OP_MUL: { Value b = pop(); Value a = pop(); push(a * b); break; }
            case OP_DIV: {
                Value b = pop(); Value a = pop();
                if (b == 0) throw RuntimeError("division by zero");
                push(a / b);
                break;
            }
            case OP_NEG: { Value a = pop(); push(-a); break; }

            case OP_EQ: { Value b = pop(); Value a = pop(); push(a == b ? 1 : 0); break; }
            case OP_LT: { Value b = pop(); Value a = pop(); push(a <  b ? 1 : 0); break; }

            case OP_DEFINE_GLOBAL: {
                std::uint8_t idx = readByte(chunk, ip);
                Value v = pop();
                globals_[chunk.names[idx]] = v;
                break;
            }
            case OP_GET_GLOBAL: {
                std::uint8_t idx = readByte(chunk, ip);
                const std::string& name = chunk.names[idx];
                auto it = globals_.find(name);
                if (it == globals_.end()) {
                    throw RuntimeError("undefined variable '" + name + "'");
                }
                push(it->second);
                break;
            }
            case OP_SET_GLOBAL: {
                std::uint8_t idx = readByte(chunk, ip);
                const std::string& name = chunk.names[idx];
                if (globals_.find(name) == globals_.end()) {
                    throw RuntimeError("assignment to undefined variable '" + name + "'");
                }
                globals_[name] = stack_.back();   // leave value on stack
                break;
            }

            case OP_JUMP: {
                std::uint16_t off = readShort(chunk, ip);
                ip += off;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                std::uint16_t off = readShort(chunk, ip);
                Value v = pop();
                if (v == 0) ip += off;
                break;
            }
            case OP_LOOP: {
                std::uint16_t off = readShort(chunk, ip);
                ip -= off;
                break;
            }

            case OP_PRINT: {
                Value v = pop();
                std::cout << v << "\n";
                break;
            }
            case OP_INPUT: {
                std::string line;
                if (!std::getline(std::cin, line)) {
                    throw RuntimeError("input: unexpected end of stream");
                }
                try {
                    push(static_cast<Value>(std::stoll(line)));
                } catch (const std::exception&) {
                    throw RuntimeError("input: not a valid integer");
                }
                break;
            }
            case OP_HALT:
                return;

            default:
                throw RuntimeError("unknown opcode " +
                                   std::to_string(static_cast<int>(op)));
        }
    }
}

} // namespace cvm
