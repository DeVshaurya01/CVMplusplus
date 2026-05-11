#pragma once

// =============================================================================
//  Debug.h
//
//  Diagnostic dumpers used by the --debug CLI flag and during development.
//  Three free functions, one per pipeline stage:
//
//    printTokens(tokens)  → human-readable token stream
//    printAst(program)    → indented tree
//    disassemble(chunk)   → assembly-style bytecode dump
//
//  All write to std::ostream so we can redirect (default: std::cerr).
// =============================================================================

#include "AST.h"
#include "Chunk.h"
#include "Token.h"
#include <iosfwd>
#include <vector>

namespace cvm::debug {

void printTokens(const std::vector<Token>& tokens, std::ostream& os);
void printAst(const std::vector<StmtPtr>& program, std::ostream& os);
void disassemble(const Chunk& chunk, std::ostream& os, const char* label = "chunk");

} // namespace cvm::debug
