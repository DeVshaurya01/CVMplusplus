#pragma once

// =============================================================================
//  Lexer.h
//
//  Consumes a source string and produces a vector of Tokens.
//  Throws CompileError on unknown characters.
// =============================================================================

#include "Token.h"
#include <string>
#include <vector>

namespace cvm {

class Lexer {
public:
    explicit Lexer(std::string source);

    // Run the scanner to completion and return all tokens, ending with EOF.
    std::vector<Token> scanAll();

private:
    // TODO(lexer): private helpers
    //   bool   isAtEnd() const;
    //   char   advance();
    //   char   peek() const;
    //   bool   match(char expected);
    //   void   scanToken(std::vector<Token>& out);
    //   void   scanNumber(std::vector<Token>& out);
    //   void   scanIdentifier(std::vector<Token>& out);
    //   TokenType keywordOrIdent(const std::string& lexeme) const;

    std::string source_;
    std::size_t pos_  = 0;
    int         line_ = 1;
};

} // namespace cvm
