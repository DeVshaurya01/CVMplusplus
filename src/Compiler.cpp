// =============================================================================
//  Compiler.cpp
// =============================================================================

#include "Compiler.h"
#include "Error.h"
#include "Value.h"

#include <cstdint>
#include <limits>
#include <variant>

namespace cvm {

namespace {
template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;
}

Chunk Compiler::compile(const std::vector<StmtPtr>& program) {
    Chunk chunk;
    chunk_ = &chunk;
    currentLine_ = 1;
    scopeDepth_ = 0;
    locals_.clear();

    // The 'main' script is essentially a function at depth 0.
    // We add a dummy local at index 0 to align with how function calls work.
    addLocal(""); 
    locals_.back().depth = 0;

    for (const auto& s : program) compileStmt(*s);
    chunk_->writeByte(OP_HALT, currentLine_);

    chunk_ = nullptr;
    return chunk;
}

void Compiler::beginScope() { scopeDepth_++; }
void Compiler::endScope() {
    scopeDepth_--;
    while (!locals_.empty() && locals_.back().depth > scopeDepth_) {
        chunk_->writeByte(OP_POP, currentLine_);
        locals_.pop_back();
    }
}

void Compiler::addLocal(const std::string& name) {
    if (locals_.size() >= 255) throw CompileError(currentLine_, "Too many local variables");
    locals_.push_back({name, -1});
}

int Compiler::resolveLocal(const std::string& name) {
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; --i) {
        if (locals_[i].name == name) {
            if (locals_[i].depth == -1) throw CompileError(currentLine_, "Cannot read local variable in its own initializer");
            return i;
        }
    }
    return -1;
}

void Compiler::compileBlock(const std::vector<StmtPtr>& stmts) {
    beginScope();
    for (const auto& s : stmts) compileStmt(*s);
    endScope();
}

std::size_t Compiler::emitJump(OpCode op, int line) {
    chunk_->writeByte(op, line);
    std::size_t addr = chunk_->size();
    chunk_->writeByte(0xFF, line);
    chunk_->writeByte(0xFF, line);
    return addr;
}

void Compiler::patchJump(std::size_t addr) {
    std::size_t offset = chunk_->size() - addr - 2;
    if (offset > 0xFFFF) throw CompileError(currentLine_, "Jump offset too large");
    chunk_->code[addr]     = static_cast<std::uint8_t>(offset & 0xFF);
    chunk_->code[addr + 1] = static_cast<std::uint8_t>((offset >> 8) & 0xFF);
}

void Compiler::emitLoop(std::size_t loopStart, int line) {
    chunk_->writeByte(OP_LOOP, line);
    std::size_t offset = chunk_->size() + 2 - loopStart;
    if (offset > 0xFFFF) throw CompileError(line, "Loop body too large");
    chunk_->writeShort(static_cast<std::uint16_t>(offset), line);
}

void Compiler::compileExpr(const Expr& e) {
    std::visit(Overloaded{
        [&](const LiteralExpr& x) {
            switch (x.kind) {
                case LiteralExpr::Kind::Int:
                    chunk_->writeByte(OP_CONST_INT, currentLine_);
                    chunk_->writeInt32(static_cast<std::int32_t>(x.intValue), currentLine_);
                    break;
                case LiteralExpr::Kind::Bool:
                    chunk_->writeByte(x.boolValue ? OP_TRUE : OP_FALSE, currentLine_);
                    break;
                case LiteralExpr::Kind::String: {
                    std::uint8_t idx = chunk_->addName(x.stringValue);
                    chunk_->writeByte(OP_CONST_STR, currentLine_);
                    chunk_->writeByte(idx, currentLine_);
                    break;
                }
                case LiteralExpr::Kind::Null:
                    chunk_->writeByte(OP_NULL, currentLine_);
                    break;
            }
        },
        [&](const VarExpr& x) {
            currentLine_ = x.line;
            int loc = resolveLocal(x.name);
            if (loc != -1) {
                chunk_->writeByte(OP_GET_LOCAL, x.line);
                chunk_->writeByte(static_cast<std::uint8_t>(loc), x.line);
            } else {
                std::uint8_t idx = chunk_->addName(x.name);
                chunk_->writeByte(OP_GET_GLOBAL, x.line);
                chunk_->writeByte(idx, x.line);
            }
        },
        [&](const UnaryExpr& x) {
            compileExpr(*x.operand);
            if (x.op == UnaryExpr::Op::Neg) {
                chunk_->writeByte(OP_NEG, currentLine_);
            } else {
                chunk_->writeByte(OP_NOT, currentLine_);
            }
        },
        [&](const BinaryExpr& x) {
            compileExpr(*x.lhs);
            compileExpr(*x.rhs);
            switch (x.op) {
                case BinaryExpr::Op::Add: chunk_->writeByte(OP_ADD, currentLine_); break;
                case BinaryExpr::Op::Sub: chunk_->writeByte(OP_SUB, currentLine_); break;
                case BinaryExpr::Op::Mul: chunk_->writeByte(OP_MUL, currentLine_); break;
                case BinaryExpr::Op::Div: chunk_->writeByte(OP_DIV, currentLine_); break;
                case BinaryExpr::Op::Eq:  chunk_->writeByte(OP_EQ,  currentLine_); break;
                case BinaryExpr::Op::Ne:  chunk_->writeByte(OP_NE,  currentLine_); break;
                case BinaryExpr::Op::Lt:  chunk_->writeByte(OP_LT,  currentLine_); break;
            }
        },
        [&](const AssignExpr& x) {
            currentLine_ = x.line;
            compileExpr(*x.value);
            int loc = resolveLocal(x.name);
            if (loc != -1) {
                chunk_->writeByte(OP_SET_LOCAL, x.line);
                chunk_->writeByte(static_cast<std::uint8_t>(loc), x.line);
            } else {
                std::uint8_t idx = chunk_->addName(x.name);
                chunk_->writeByte(OP_SET_GLOBAL, x.line);
                chunk_->writeByte(idx, x.line);
            }
        },
        [&](const ArrayExpr& x) {
            for (const auto& elem : x.elements) compileExpr(*elem);
            chunk_->writeByte(OP_BUILD_ARRAY, x.line);
            chunk_->writeByte(static_cast<std::uint8_t>(x.elements.size()), x.line);
        },
        [&](const MapExpr& x) {
            for (std::size_t i = 0; i < x.keys.size(); ++i) {
                compileExpr(*x.keys[i]);
                compileExpr(*x.values[i]);
            }
            chunk_->writeByte(OP_BUILD_MAP, x.line);
            chunk_->writeByte(static_cast<std::uint8_t>(x.keys.size()), x.line);
        },
        [&](const IndexGetExpr& x) {
            compileExpr(*x.collection);
            compileExpr(*x.index);
            chunk_->writeByte(OP_INDEX_GET, x.line);
        },
        [&](const IndexSetExpr& x) {
            compileExpr(*x.collection);
            compileExpr(*x.index);
            compileExpr(*x.value);
            chunk_->writeByte(OP_INDEX_SET, x.line);
        },
        [&](const LenExpr& x) {
            compileExpr(*x.collection);
            chunk_->writeByte(OP_LEN, x.line);
        },
        [&](const HasExpr& x) {
            compileExpr(*x.collection);
            compileExpr(*x.key);
            chunk_->writeByte(OP_HAS_KEY, x.line);
        },
        [&](const CallExpr& x) {
            compileExpr(*x.callee);
            for (const auto& arg : x.args) compileExpr(*arg);
            chunk_->writeByte(OP_CALL, x.line);
            chunk_->writeByte(static_cast<std::uint8_t>(x.args.size()), x.line);
        },
    }, e.node);
}

void Compiler::compileStmt(const Stmt& s) {
    std::visit(Overloaded{
        [&](const LetStmt& x) {
            currentLine_ = x.line;
            compileExpr(*x.value);
            if (scopeDepth_ > 0) {
                addLocal(x.name);
                locals_.back().depth = scopeDepth_;
            } else {
                std::uint8_t idx = chunk_->addName(x.name);
                chunk_->writeByte(OP_DEFINE_GLOBAL, x.line);
                chunk_->writeByte(idx, x.line);
            }
        },
        [&](const PrintStmt& x) {
            compileExpr(*x.value);
            chunk_->writeByte(OP_PRINT, currentLine_);
        },
        [&](const InputStmt& x) {
            chunk_->writeByte(OP_INPUT, x.line);
            std::uint8_t idx = chunk_->addName(x.name);
            chunk_->writeByte(OP_DEFINE_GLOBAL, x.line);
            chunk_->writeByte(idx, x.line);
        },
        [&](const IfStmt& x) {
            compileExpr(*x.cond);
            std::size_t thenJmp = emitJump(OP_JUMP_IF_FALSE, currentLine_);
            compileBlock(x.thenBlk);
            if (!x.elseBlk.empty()) {
                std::size_t elseJmp = emitJump(OP_JUMP, currentLine_);
                patchJump(thenJmp);
                compileBlock(x.elseBlk);
                patchJump(elseJmp);
            } else { patchJump(thenJmp); }
        },
        [&](const WhileStmt& x) {
            std::size_t loopStart = chunk_->size();
            compileExpr(*x.cond);
            std::size_t exitJmp = emitJump(OP_JUMP_IF_FALSE, currentLine_);
            compileBlock(x.body);
            emitLoop(loopStart, currentLine_);
            patchJump(exitJmp);
        },
        [&](const BlockStmt& x) { compileBlock(x.stmts); },
        [&](const FnStmt& x) {
            // Save current state
            Chunk* prevChunk = chunk_;
            std::vector<Local> prevLocals = std::move(locals_);
            int prevScope = scopeDepth_;

            // Setup function compiler state
            Chunk fnChunk;
            chunk_ = &fnChunk;
            scopeDepth_ = 1;
            locals_.clear();

            // Name slot at 0 (internal convention)
            addLocal(x.name); locals_.back().depth = 0;
            // Parameters are locals
            for (const auto& p : x.params) {
                addLocal(p); locals_.back().depth = 1;
            }

            for (const auto& s : x.body) compileStmt(*s);
            // Implicit return null
            chunk_->writeByte(OP_NULL, x.line);
            chunk_->writeByte(OP_RETURN, x.line);

            // Restore state and store function as a global in the original chunk
            auto fnObj = std::make_shared<ObjFunction>(x.name, (int)x.params.size(), std::make_shared<Chunk>(std::move(fnChunk)));
            
            chunk_ = prevChunk;
            locals_ = std::move(prevLocals);
            scopeDepth_ = prevScope;

            std::uint8_t constIdx = chunk_->addConstant(std::move(fnObj));
            chunk_->writeByte(OP_CLOSURE, x.line);
            chunk_->writeByte(constIdx, x.line);

            std::uint8_t nameIdx = chunk_->addName(x.name);
            chunk_->writeByte(OP_DEFINE_GLOBAL, x.line);
            chunk_->writeByte(nameIdx, x.line);
        },
        [&](const ReturnStmt& x) {
            if (x.value) compileExpr(*x.value);
            else chunk_->writeByte(OP_NULL, x.line);
            chunk_->writeByte(OP_RETURN, x.line);
        },
        [&](const ExprStmt& x) {
            compileExpr(*x.expr);
            chunk_->writeByte(OP_POP, currentLine_);
        },
    }, s.node);
}

} // namespace cvm
