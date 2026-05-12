#pragma once

// =============================================================================
//  Parser.h
//
//  Recursive descent parser. Consumes the lexer's token vector and produces
//  a vector of top-level statements (the program).
//
//  Grammar lives in docs/GRAMMAR.md.
// =============================================================================

#include "AST.h"
#include "Token.h"
#include <vector>

namespace cvm {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    std::vector<StmtPtr> parseProgram();

private:
    // Statement rules
    StmtPtr statement();
    StmtPtr letStmt();
    StmtPtr printStmt();
    StmtPtr inputStmt();
    StmtPtr ifStmt();
    StmtPtr whileStmt();
    StmtPtr blockStmt();
    std::vector<StmtPtr> block();
    StmtPtr exprOrAssignStmt();

    // Expression rules (lowest -> highest precedence)
    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr primary();

    // Helpers
    const Token& peek() const;
    const Token& previous() const;
    bool         isAtEnd() const;
    bool         check(TokenType t) const;
    bool         match(TokenType t);
    const Token& consume(TokenType t, const char* msg);
    const Token& advance();

    std::vector<Token> tokens_;
    std::size_t        pos_ = 0;
};

} // namespace cvm
