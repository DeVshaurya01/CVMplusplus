// =============================================================================
//  Lexer.cpp
//
//  Scans raw .cvm source into a flat token vector. Single forward pass,
//  one char of lookahead (two for `//` comments).
// =============================================================================

#include "Lexer.h"
#include "Error.h"

#include <cctype>
#include <string>
#include <unordered_map>

namespace cvm {

namespace {

const std::unordered_map<std::string, TokenType>& keywords() {
    static const std::unordered_map<std::string, TokenType> kw{
        {"let",   TokenType::LET},
        {"if",    TokenType::IF},
        {"else",  TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"print", TokenType::PRINT},
        {"input", TokenType::INPUT},
        {"true",  TokenType::TRUE},
        {"false", TokenType::FALSE},
    };
    return kw;
}

bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool isAlphaNum(char c) {
    return isAlpha(c) || (c >= '0' && c <= '9');
}

} // namespace

Lexer::Lexer(std::string source) : source_(std::move(source)) {}

bool Lexer::isAtEnd() const { return pos_ >= source_.size(); }
char Lexer::advance()       { return source_[pos_++]; }
char Lexer::peek() const    { return isAtEnd() ? '\0' : source_[pos_]; }
char Lexer::peekNext() const {
    return (pos_ + 1 >= source_.size()) ? '\0' : source_[pos_ + 1];
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[pos_] != expected) return false;
    ++pos_;
    return true;
}

void Lexer::skipWhitespaceAndComments() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                ++pos_;
                break;
            case '\n':
                ++line_;
                ++pos_;
                break;
            case '/':
                if (peekNext() == '/') {
                    while (!isAtEnd() && peek() != '\n') ++pos_;
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::scanNumber(std::vector<Token>& out, std::size_t start) {
    while (!isAtEnd() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
    std::string lex = source_.substr(start, pos_ - start);
    Token t;
    t.type     = TokenType::NUMBER;
    t.lexeme   = lex;
    t.line     = line_;
    t.intValue = std::stoi(lex);
    out.push_back(std::move(t));
}

void Lexer::scanIdentifier(std::vector<Token>& out, std::size_t start) {
    while (!isAtEnd() && isAlphaNum(peek())) advance();
    std::string lex = source_.substr(start, pos_ - start);

    TokenType type = TokenType::IDENT;
    auto it = keywords().find(lex);
    if (it != keywords().end()) type = it->second;

    out.push_back({type, std::move(lex), line_, 0});
}

void Lexer::scanToken(std::vector<Token>& out) {
    std::size_t start = pos_;
    char c = advance();

    auto push = [&](TokenType t, std::string lex) {
        out.push_back({t, std::move(lex), line_, 0});
    };

    switch (c) {
        case '(': push(TokenType::LPAREN, "("); return;
        case ')': push(TokenType::RPAREN, ")"); return;
        case '{': push(TokenType::LBRACE, "{"); return;
        case '}': push(TokenType::RBRACE, "}"); return;
        case ';': push(TokenType::SEMI,   ";"); return;
        case '+': push(TokenType::PLUS,   "+"); return;
        case '-': push(TokenType::MINUS,  "-"); return;
        case '*': push(TokenType::STAR,   "*"); return;
        case '/': push(TokenType::SLASH,  "/"); return;
        case '<': push(TokenType::LT,     "<"); return;
        case '=':
            if (match('=')) push(TokenType::EQ_EQ, "==");
            else            push(TokenType::EQ,    "=");
            return;
        default: break;
    }

    if (std::isdigit(static_cast<unsigned char>(c))) {
        scanNumber(out, start);
        return;
    }

    if (isAlpha(c)) {
        scanIdentifier(out, start);
        return;
    }

    throw CompileError(line_, std::string("unexpected character '") + c + "'");
}

std::vector<Token> Lexer::scanAll() {
    std::vector<Token> tokens;
    while (true) {
        skipWhitespaceAndComments();
        if (isAtEnd()) break;
        scanToken(tokens);
    }
    tokens.push_back({TokenType::END_OF_FILE, "", line_, 0});
    return tokens;
}

} // namespace cvm
