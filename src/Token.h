#pragma once

// =============================================================================
//  Token.h
// =============================================================================

#include <string>

namespace cvm {

enum class TokenType {
    // Literals
    NUMBER,
    STRING,
    IDENT,
    TRUE,
    FALSE,
    NULL_KW,

    // Keywords
    LET,
    FN,
    RETURN,
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
    BANG_EQ,   // !=
    BANG,      // !
    LT,        // <

    // Punctuation
    LPAREN,    // (
    RPAREN,    // )
    LBRACE,    // {
    RBRACE,    // }
    LBRACKET,  // [
    RBRACKET,  // ]
    COLON,     // :
    COMMA,     // ,
    LEN,       // len
    HAS,       // has
    SEMI,      // ;

    END_OF_FILE,
};

struct Token {
    TokenType   type;
    std::string lexeme;
    int         line     = 0;
    int         intValue = 0;
};

} // namespace cvm
