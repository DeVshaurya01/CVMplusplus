// =============================================================================
//  Compiler.cpp  —  implementation goes here.
//
//  Owner: <fill in>
//  Branch: feat/compiler
//
//  Day 1 target: literals, binary ops (+ - * /), unary -, print stmt, halt.
//  Day 2: globals (OP_DEFINE_GLOBAL/GET/SET), ==, <, if/else, while, input.
//
//  Key pattern for binary ops:
//      compileExpr(lhs);
//      compileExpr(rhs);
//      emit(OP_ADD);  // or whatever
//
//  Key pattern for jumps (see docs/ISA.md):
//      auto patch = emitJump(OP_JUMP_IF_FALSE);
//      compileStmt(thenBlk);
//      patchJump(patch);
// =============================================================================

#include "Compiler.h"

namespace cvm {

Chunk Compiler::compile(const std::vector<StmtPtr>& program) {
    Chunk chunk;
    // TODO(compiler):
    //   chunk_ = &chunk;
    //   for (const auto& s : program) compileStmt(*s);
    //   chunk.code.push_back(OP_HALT);
    (void)program;
    return chunk;
}

} // namespace cvm
