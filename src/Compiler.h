#pragma once

// =============================================================================
//  Compiler.h
//
//  Walks the AST and emits bytecode into a Chunk.
// =============================================================================

#include "AST.h"
#include "Chunk.h"
#include <cstddef>
#include <vector>

namespace cvm {

class Compiler {
public:
    // Compile a full program (vector of top-level statements) into a Chunk.
    Chunk compile(const std::vector<StmtPtr>& program);

private:
    void compileStmt(const Stmt& s);
    void compileExpr(const Expr& e);

    void compileBlock(const std::vector<StmtPtr>& stmts);

    // Emit a jump opcode + 2 placeholder bytes; returns address of the
    // placeholder. Patch with patchJump() once the target is known.
    std::size_t emitJump(OpCode op, int line);
    void        patchJump(std::size_t addr);
    void        emitLoop(std::size_t loopStart, int line);

    Chunk* chunk_ = nullptr;
    int    currentLine_ = 1;
};

} // namespace cvm
