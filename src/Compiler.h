#pragma once

// =============================================================================
//  Compiler.h
//
//  Walks the AST and emits bytecode into a Chunk.
// =============================================================================

#include "AST.h"
#include "Chunk.h"
#include <vector>

namespace cvm {

class Compiler {
public:
    // Compile a full program (vector of top-level statements) into a Chunk.
    Chunk compile(const std::vector<StmtPtr>& program);

private:
    // TODO(compiler): visitor methods
    //   void compileStmt(const Stmt& s);
    //   void compileExpr(const Expr& e);
    //
    //   // jump backpatching helpers
    //   std::size_t emitJump(OpCode op);        // emits op + 2 placeholder bytes; returns patch addr
    //   void        patchJump(std::size_t addr); // writes (current - addr - 2) into placeholder
    //   void        emitLoop(std::size_t loopStart);
    //
    //   Chunk* chunk_; // current chunk being built
};

} // namespace cvm
