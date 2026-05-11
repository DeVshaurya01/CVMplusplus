// =============================================================================
//  Parser.cpp  —  implementation goes here.
//
//  Owner: <fill in>
//  Branch: feat/parser
//
//  Day 1 target: parse expressions + `print expr ;`. Just enough that
//                `print 1 + 2 * 3;` produces a valid AST.
//
//  Day 2: add letStmt, assignStmt, ifStmt, whileStmt, inputStmt, blocks,
//         == and < operators.
// =============================================================================

#include "Parser.h"

namespace cvm {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> stmts;
    // TODO(parser): loop until END_OF_FILE, parsing one statement at a time.
    //   while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}

} // namespace cvm
