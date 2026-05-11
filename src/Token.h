#pragma once

// =============================================================================
//  Token.h
//
//  Token types and the Token struct emitted by the Lexer and consumed by the
//  Parser. Add new TokenType values as the lexer grows.
// =============================================================================

#include <string>

namespace cvm {

enum class TokenType {
    // TODO(lexer): fill in
    //   Literals:   NUMBER, IDENT, TRUE, FALSE
    //   Keywords:   LET, IF, ELSE, WHILE, PRINT, INPUT
    //   Operators:  PLUS, MINUS, STAR, SLASH, EQ, EQ_EQ, LT
    //   Punct:      LPAREN, RPAREN, LBRACE, RBRACE, SEMI
    //   Misc:       END_OF_FILE
};

struct Token {
    // TODO(lexer):
    //   TokenType   type;
    //   std::string lexeme;
    //   int         line;
    //   int         intValue;   // valid when type == NUMBER
};

} // namespace cvm
