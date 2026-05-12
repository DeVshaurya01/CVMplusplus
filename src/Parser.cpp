// =============================================================================
//  Parser.cpp
//
//  Recursive descent matching docs/GRAMMAR.md. Each grammar rule is one
//  method. Expression precedence is encoded directly in the call chain:
//  equality -> comparison -> term -> factor -> unary -> primary.
//
//  Assignment is folded into the expression layer (right-associative,
//  lowest precedence) so `x = y = 1` parses naturally and `x = 1` can
//  appear as an exprStmt.
// =============================================================================

#include "Parser.h"
#include "Error.h"

#include <utility>

namespace cvm {

namespace {

ExprPtr makeExpr(LiteralExpr e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(VarExpr e)     { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(UnaryExpr e)   { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(BinaryExpr e)  { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(AssignExpr e)  { return std::make_unique<Expr>(Expr{std::move(e)}); }

StmtPtr makeStmt(LetStmt s)   { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(PrintStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(InputStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(IfStmt s)    { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(WhileStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(BlockStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(ExprStmt s)  { return std::make_unique<Stmt>(Stmt{std::move(s)}); }

} // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

// ---------- helpers ----------

const Token& Parser::peek() const     { return tokens_[pos_]; }
const Token& Parser::previous() const { return tokens_[pos_ - 1]; }
bool         Parser::isAtEnd() const  { return peek().type == TokenType::END_OF_FILE; }
bool         Parser::check(TokenType t) const { return !isAtEnd() && peek().type == t; }

const Token& Parser::advance() {
    if (!isAtEnd()) ++pos_;
    return previous();
}

bool Parser::match(TokenType t) {
    if (!check(t)) return false;
    advance();
    return true;
}

const Token& Parser::consume(TokenType t, const char* msg) {
    if (check(t)) return advance();
    throw CompileError(peek().line,
                       std::string(msg) + " (got '" + peek().lexeme + "')");
}

// ---------- top level ----------

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}

// ---------- statements ----------

StmtPtr Parser::statement() {
    if (match(TokenType::LET))    return letStmt();
    if (match(TokenType::PRINT))  return printStmt();
    if (match(TokenType::INPUT))  return inputStmt();
    if (match(TokenType::IF))     return ifStmt();
    if (match(TokenType::WHILE))  return whileStmt();
    if (match(TokenType::LBRACE)) return blockStmt();
    return exprOrAssignStmt();
}

StmtPtr Parser::letStmt() {
    const Token& name = consume(TokenType::IDENT, "expected variable name after 'let'");
    consume(TokenType::EQ, "expected '=' after variable name");
    ExprPtr value = expression();
    consume(TokenType::SEMI, "expected ';' after let initializer");
    return makeStmt(LetStmt{name.lexeme, std::move(value), name.line});
}

StmtPtr Parser::printStmt() {
    ExprPtr value = expression();
    consume(TokenType::SEMI, "expected ';' after print expression");
    return makeStmt(PrintStmt{std::move(value)});
}

StmtPtr Parser::inputStmt() {
    const Token& name = consume(TokenType::IDENT, "expected variable name after 'input'");
    consume(TokenType::SEMI, "expected ';' after input target");
    return makeStmt(InputStmt{name.lexeme, name.line});
}

StmtPtr Parser::ifStmt() {
    consume(TokenType::LPAREN, "expected '(' after 'if'");
    ExprPtr cond = expression();
    consume(TokenType::RPAREN, "expected ')' after if condition");
    consume(TokenType::LBRACE, "expected '{' to start if body");
    auto thenBlk = block();

    std::vector<StmtPtr> elseBlk;
    if (match(TokenType::ELSE)) {
        consume(TokenType::LBRACE, "expected '{' to start else body");
        elseBlk = block();
    }
    return makeStmt(IfStmt{std::move(cond), std::move(thenBlk), std::move(elseBlk)});
}

StmtPtr Parser::whileStmt() {
    consume(TokenType::LPAREN, "expected '(' after 'while'");
    ExprPtr cond = expression();
    consume(TokenType::RPAREN, "expected ')' after while condition");
    consume(TokenType::LBRACE, "expected '{' to start while body");
    auto body = block();
    return makeStmt(WhileStmt{std::move(cond), std::move(body)});
}

StmtPtr Parser::blockStmt() {
    auto stmts = block();
    return makeStmt(BlockStmt{std::move(stmts)});
}

// Caller must have already consumed the opening '{'. Reads statements
// up to and consuming the matching '}'.
std::vector<StmtPtr> Parser::block() {
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        stmts.push_back(statement());
    }
    consume(TokenType::RBRACE, "expected '}' to close block");
    return stmts;
}

StmtPtr Parser::exprOrAssignStmt() {
    ExprPtr e = expression();
    consume(TokenType::SEMI, "expected ';' after expression");
    return makeStmt(ExprStmt{std::move(e)});
}

// ---------- expressions ----------

// expression -> assignment
// assignment -> IDENT "=" assignment | equality
ExprPtr Parser::expression() {
    ExprPtr lhs = equality();

    if (match(TokenType::EQ)) {
        const Token& eq = previous();
        ExprPtr rhs = expression();   // right-assoc

        // Only IDENT on the LHS is a valid assignment target.
        if (auto* var = std::get_if<VarExpr>(&lhs->node)) {
            return makeExpr(AssignExpr{var->name, std::move(rhs), var->line});
        }
        throw CompileError(eq.line, "invalid assignment target");
    }
    return lhs;
}

ExprPtr Parser::equality() {
    ExprPtr lhs = comparison();
    while (match(TokenType::EQ_EQ)) {
        ExprPtr rhs = comparison();
        lhs = makeExpr(BinaryExpr{BinaryExpr::Op::Eq, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::comparison() {
    ExprPtr lhs = term();
    while (match(TokenType::LT)) {
        ExprPtr rhs = term();
        lhs = makeExpr(BinaryExpr{BinaryExpr::Op::Lt, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::term() {
    ExprPtr lhs = factor();
    while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
        TokenType op = advance().type;
        ExprPtr rhs  = factor();
        BinaryExpr::Op bop = (op == TokenType::PLUS) ? BinaryExpr::Op::Add
                                                     : BinaryExpr::Op::Sub;
        lhs = makeExpr(BinaryExpr{bop, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::factor() {
    ExprPtr lhs = unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        TokenType op = advance().type;
        ExprPtr rhs  = unary();
        BinaryExpr::Op bop = (op == TokenType::STAR) ? BinaryExpr::Op::Mul
                                                     : BinaryExpr::Op::Div;
        lhs = makeExpr(BinaryExpr{bop, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::unary() {
    if (match(TokenType::MINUS)) {
        ExprPtr operand = unary();
        return makeExpr(UnaryExpr{UnaryExpr::Op::Neg, std::move(operand)});
    }
    return primary();
}

ExprPtr Parser::primary() {
    if (match(TokenType::NUMBER)) {
        return makeExpr(LiteralExpr{previous().intValue, false});
    }
    if (match(TokenType::TRUE)) {
        return makeExpr(LiteralExpr{1, true});
    }
    if (match(TokenType::FALSE)) {
        return makeExpr(LiteralExpr{0, true});
    }
    if (match(TokenType::IDENT)) {
        return makeExpr(VarExpr{previous().lexeme, previous().line});
    }
    if (match(TokenType::LPAREN)) {
        ExprPtr e = expression();
        consume(TokenType::RPAREN, "expected ')' after expression");
        return e;
    }
    throw CompileError(peek().line,
                       "expected expression (got '" + peek().lexeme + "')");
}

} // namespace cvm
