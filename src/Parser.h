#pragma once

// =============================================================================
//  Parser.h
// =============================================================================

#include "Token.h"
#include "AST.h"
#include <vector>
#include <memory>

namespace cvm {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::vector<StmtPtr> parseProgram();

private:
    StmtPtr statement();
    StmtPtr letStmt();
    StmtPtr fnStmt();
    StmtPtr returnStmt();
    StmtPtr printStmt();
    StmtPtr inputStmt();
    StmtPtr ifStmt();
    StmtPtr whileStmt();
    StmtPtr blockStmt();
    StmtPtr exprOrAssignStmt();

    std::vector<StmtPtr> block();

    ExprPtr expression();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr postfix(ExprPtr lhs);
    ExprPtr primary();

    ExprPtr arrayLiteral(int line);
    ExprPtr mapLiteral(int line);

    // Helpers
    const Token& peek() const;
    const Token& previous() const;
    bool         isAtEnd() const;
    bool         check(TokenType t) const;
    const Token& advance();
    bool         match(TokenType t);
    const Token& consume(TokenType t, const char* msg);

    std::vector<Token> tokens_;
    std::size_t        pos_ = 0;
};

} // namespace cvm
