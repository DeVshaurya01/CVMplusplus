// =============================================================================
//  Debug.cpp  —  implementation goes here.
//
//  Owner: <fill in>
//  Branch: feat/debug-cli
//
//  Write this incrementally alongside the other stages — it's your single
//  best debugging tool. Disassembler especially: when the VM produces a
//  wrong answer, you read the bytecode dump and the bug is usually obvious.
//
//  Disassembly format (see docs/ISA.md):
//      0000  OP_CONST_INT     10
//      0005  OP_DEFINE_GLOBAL 0    ; 'n'
// =============================================================================

#include "Debug.h"
#include <ostream>

namespace cvm::debug {

void printTokens(const std::vector<Token>& tokens, std::ostream& os) {
    // TODO(debug-cli): for each token print  type | lexeme | line
    (void)tokens;
    (void)os;
}

void printAst(const std::vector<StmtPtr>& program, std::ostream& os) {
    // TODO(debug-cli): recursive printer with indent level.
    //   Use std::visit on Stmt::node and Expr::node.
    (void)program;
    (void)os;
}

void disassemble(const Chunk& chunk, std::ostream& os, const char* label) {
    // TODO(debug-cli): walk chunk.code byte by byte, decode each opcode,
    //                  print  offset (4-digit hex)  mnemonic  operand(s).
    (void)chunk;
    (void)os;
    (void)label;
}

} // namespace cvm::debug
