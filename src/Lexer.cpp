// =============================================================================
//  Lexer.cpp  —  implementation goes here.
//
//  Owner: <fill in>
//  Branch: feat/lexer
//
//  Day 1 target: recognize NUMBER, PLUS/MINUS/STAR/SLASH, LPAREN/RPAREN,
//                SEMI, PRINT keyword, EOF. Enough for `print 1 + 2;`.
//
//  Day 2: add identifiers, keywords (let, if, else, while, input,
//         true, false), EQ, EQ_EQ, LT, LBRACE/RBRACE.
// =============================================================================

#include "Lexer.h"

namespace cvm {

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

std::vector<Token> Lexer::scanAll() {
    std::vector<Token> tokens;
    // TODO(lexer): main scanning loop.
    //   while (!isAtEnd()) { skipWhitespaceAndComments(); scanToken(tokens); }
    //   tokens.push_back({ TokenType::END_OF_FILE, "", line_, 0 });
    return tokens;
}

} // namespace cvm
