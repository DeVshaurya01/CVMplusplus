// =============================================================================
//  Compiler.cpp
//
//  AST -> bytecode. Each Stmt/Expr variant has a visitor branch. Binary ops
//  use the canonical post-order pattern (compile lhs, compile rhs, emit op).
//  Control flow uses the backpatching pattern from docs/ISA.md.
// =============================================================================

#include "Compiler.h"
#include "Error.h"

#include <cstdint>
#include <limits>
#include <variant>

namespace cvm {

namespace {

template <class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

} // namespace

Chunk Compiler::compile(const std::vector<StmtPtr>& program) {
    Chunk chunk;
    chunk_ = &chunk;
    currentLine_ = 1;

    for (const auto& s : program) compileStmt(*s);
    chunk_->writeByte(OP_HALT, currentLine_);

    chunk_ = nullptr;
    return chunk;
}

void Compiler::compileBlock(const std::vector<StmtPtr>& stmts) {
    for (const auto& s : stmts) compileStmt(*s);
}

std::size_t Compiler::emitJump(OpCode op, int line) {
    chunk_->writeByte(op, line);
    std::size_t addr = chunk_->size();
    chunk_->writeByte(0xFF, line);
    chunk_->writeByte(0xFF, line);
    return addr;
}

void Compiler::patchJump(std::size_t addr) {
    // Offset = bytes from end of jump operand to current position.
    std::size_t offset = chunk_->size() - addr - 2;
    if (offset > std::numeric_limits<std::uint16_t>::max()) {
        throw CompileError(currentLine_, "jump offset too large");
    }
    chunk_->code[addr]     = static_cast<std::uint8_t>(offset & 0xFF);
    chunk_->code[addr + 1] = static_cast<std::uint8_t>((offset >> 8) & 0xFF);
}

void Compiler::emitLoop(std::size_t loopStart, int line) {
    chunk_->writeByte(OP_LOOP, line);
    // We're about to write 2 more bytes; offset measured from after them.
    std::size_t offset = chunk_->size() + 2 - loopStart;
    if (offset > std::numeric_limits<std::uint16_t>::max()) {
        throw CompileError(line, "loop body too large");
    }
    chunk_->writeShort(static_cast<std::uint16_t>(offset), line);
}

void Compiler::compileExpr(const Expr& e) {
    std::visit(Overloaded{
        [&](const LiteralExpr& x) {
            if (x.isBool) {
                chunk_->writeByte(x.value ? OP_TRUE : OP_FALSE, currentLine_);
            } else {
                chunk_->writeByte(OP_CONST_INT, currentLine_);
                chunk_->writeInt32(static_cast<std::int32_t>(x.value), currentLine_);
            }
        },
        [&](const VarExpr& x) {
            currentLine_ = x.line;
            std::uint8_t idx = chunk_->addName(x.name);
            chunk_->writeByte(OP_GET_GLOBAL, x.line);
            chunk_->writeByte(idx, x.line);
        },
        [&](const UnaryExpr& x) {
            compileExpr(*x.operand);
            chunk_->writeByte(OP_NEG, currentLine_);
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
                case BinaryExpr::Op::Lt:  chunk_->writeByte(OP_LT,  currentLine_); break;
            }
        },
        [&](const AssignExpr& x) {
            currentLine_ = x.line;
            compileExpr(*x.value);
            std::uint8_t idx = chunk_->addName(x.name);
            chunk_->writeByte(OP_SET_GLOBAL, x.line);
            chunk_->writeByte(idx, x.line);
        },
    }, e.node);
}

void Compiler::compileStmt(const Stmt& s) {
    std::visit(Overloaded{
        [&](const LetStmt& x) {
            currentLine_ = x.line;
            compileExpr(*x.value);
            std::uint8_t idx = chunk_->addName(x.name);
            chunk_->writeByte(OP_DEFINE_GLOBAL, x.line);
            chunk_->writeByte(idx, x.line);
        },
        [&](const PrintStmt& x) {
            compileExpr(*x.value);
            chunk_->writeByte(OP_PRINT, currentLine_);
        },
        [&](const InputStmt& x) {
            currentLine_ = x.line;
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
            } else {
                patchJump(thenJmp);
            }
        },
        [&](const WhileStmt& x) {
            std::size_t loopStart = chunk_->size();
            compileExpr(*x.cond);
            std::size_t exitJmp = emitJump(OP_JUMP_IF_FALSE, currentLine_);
            compileBlock(x.body);
            emitLoop(loopStart, currentLine_);
            patchJump(exitJmp);
        },
        [&](const BlockStmt& x) {
            compileBlock(x.stmts);
        },
        [&](const ExprStmt& x) {
            compileExpr(*x.expr);
            // Pop the expression's result; statements leave no residue.
            chunk_->writeByte(OP_POP, currentLine_);
        },
    }, s.node);
}

} // namespace cvm
