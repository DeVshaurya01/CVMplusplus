#pragma once

// =============================================================================
//  Token.h
//
//  Token types and the Token struct emitted by the Lexer and consumed by the
//  Parser.
// =============================================================================

#include <string>

namespace cvm {

enum class TokenType {
    // Literals
    NUMBER,
    IDENT,
    TRUE,
    FALSE,

    // Keywords
    LET,
    IF,
    ELSE,
    WHILE,
    PRINT,
    INPUT,

    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    EQ,        // =
    EQ_EQ,     // ==
    LT,        // <

    // Punctuation
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    SEMI,

    END_OF_FILE,
};

struct Token {
    TokenType   type;
    std::string lexeme;
    int         line     = 0;
    int         intValue = 0;   // valid when type == NUMBER
};

} // namespace cvm
