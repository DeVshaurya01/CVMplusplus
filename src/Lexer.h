#pragma once

// =============================================================================
//  Lexer.h
// =============================================================================

#include "Token.h"
#include <string>
#include <vector>

namespace cvm {

class Lexer {
public:
    explicit Lexer(std::string source);
    std::vector<Token> scanAll();

private:
    bool isAtEnd() const;
    char advance();
    char peek() const;
    char peekNext() const;
    bool match(char expected);

    void skipWhitespaceAndComments();
    void scanNumber(std::vector<Token>& out, std::size_t start);
    void scanIdentifier(std::vector<Token>& out, std::size_t start);
    void scanString(std::vector<Token>& out);
    void scanToken(std::vector<Token>& out);

    std::string source_;
    std::size_t pos_  = 0;
    int         line_ = 1;
};

} // namespace cvm
