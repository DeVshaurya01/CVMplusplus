// =============================================================================
//  Parser.cpp
// =============================================================================

#include "Parser.h"
#include "Error.h"

#include <utility>

namespace cvm {

namespace {

ExprPtr makeExpr(LiteralExpr  e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(VarExpr      e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(UnaryExpr    e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(BinaryExpr   e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(AssignExpr   e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(ArrayExpr    e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(MapExpr      e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(IndexGetExpr e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(IndexSetExpr e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(LenExpr      e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(HasExpr      e) { return std::make_unique<Expr>(Expr{std::move(e)}); }
ExprPtr makeExpr(CallExpr     e) { return std::make_unique<Expr>(Expr{std::move(e)}); }

StmtPtr makeStmt(LetStmt   s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(PrintStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(InputStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(IfStmt    s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(WhileStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(BlockStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(FnStmt    s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(ReturnStmt s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }
StmtPtr makeStmt(ExprStmt  s) { return std::make_unique<Stmt>(Stmt{std::move(s)}); }

} // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token& Parser::peek()     const { return tokens_[pos_]; }
const Token& Parser::previous() const { return tokens_[pos_ - 1]; }
bool         Parser::isAtEnd()  const { return peek().type == TokenType::END_OF_FILE; }
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
    throw CompileError(peek().line, std::string(msg) + " (got '" + peek().lexeme + "')");
}

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) stmts.push_back(statement());
    return stmts;
}

StmtPtr Parser::statement() {
    if (match(TokenType::LET))    return letStmt();
    if (match(TokenType::FN))     return fnStmt();
    if (match(TokenType::RETURN)) return returnStmt();
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

StmtPtr Parser::fnStmt() {
    const Token& name = consume(TokenType::IDENT, "expected function name after 'fn'");
    consume(TokenType::LPAREN, "expected '(' after function name");
    std::vector<std::string> params;
    if (!check(TokenType::RPAREN)) {
        do {
            params.push_back(consume(TokenType::IDENT, "expected parameter name").lexeme);
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "expected ')' after parameters");
    consume(TokenType::LBRACE, "expected '{' before function body");
    auto body = block();
    return makeStmt(FnStmt{name.lexeme, std::move(params), std::move(body), name.line});
}

StmtPtr Parser::returnStmt() {
    int line = previous().line;
    ExprPtr val = nullptr;
    if (!check(TokenType::SEMI)) {
        val = expression();
    }
    consume(TokenType::SEMI, "expected ';' after return value");
    return makeStmt(ReturnStmt{std::move(val), line});
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

ExprPtr Parser::expression() {
    ExprPtr lhs = equality();
    if (match(TokenType::HAS)) {
        int line = previous().line;
        ExprPtr key = equality();
        lhs = makeExpr(HasExpr{std::move(lhs), std::move(key), line});
    }
    if (match(TokenType::EQ)) {
        const Token& eq = previous();
        ExprPtr rhs = expression();
        if (auto* var = std::get_if<VarExpr>(&lhs->node)) {
            return makeExpr(AssignExpr{var->name, std::move(rhs), var->line});
        }
        if (auto* idx = std::get_if<IndexGetExpr>(&lhs->node)) {
            return makeExpr(IndexSetExpr{std::move(idx->collection), std::move(idx->index), std::move(rhs), idx->line});
        }
        throw CompileError(eq.line, "invalid assignment target");
    }
    return lhs;
}

ExprPtr Parser::equality() {
    ExprPtr lhs = comparison();
    while (match(TokenType::EQ_EQ) || match(TokenType::BANG_EQ)) {
        TokenType opType = previous().type;
        ExprPtr rhs = comparison();
        BinaryExpr::Op op = (opType == TokenType::EQ_EQ) ? BinaryExpr::Op::Eq : BinaryExpr::Op::Ne;
        lhs = makeExpr(BinaryExpr{op, std::move(lhs), std::move(rhs)});
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
        BinaryExpr::Op bop = (op == TokenType::PLUS) ? BinaryExpr::Op::Add : BinaryExpr::Op::Sub;
        lhs = makeExpr(BinaryExpr{bop, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::factor() {
    ExprPtr lhs = unary();
    while (check(TokenType::STAR) || check(TokenType::SLASH)) {
        TokenType op = advance().type;
        ExprPtr rhs  = unary();
        BinaryExpr::Op bop = (op == TokenType::STAR) ? BinaryExpr::Op::Mul : BinaryExpr::Op::Div;
        lhs = makeExpr(BinaryExpr{bop, std::move(lhs), std::move(rhs)});
    }
    return lhs;
}

ExprPtr Parser::unary() {
    if (match(TokenType::MINUS)) {
        ExprPtr operand = unary();
        return makeExpr(UnaryExpr{UnaryExpr::Op::Neg, std::move(operand)});
    }
    if (match(TokenType::BANG)) {
        ExprPtr operand = unary();
        return makeExpr(UnaryExpr{UnaryExpr::Op::Not, std::move(operand)});
    }
    return postfix(primary());
}

ExprPtr Parser::postfix(ExprPtr lhs) {
    while (true) {
        if (check(TokenType::LBRACKET)) {
            int line = peek().line;
            advance();
            ExprPtr idx = expression();
            consume(TokenType::RBRACKET, "expected ']' after index expression");
            lhs = makeExpr(IndexGetExpr{std::move(lhs), std::move(idx), line});
        } else if (match(TokenType::LPAREN)) {
            int line = previous().line;
            std::vector<ExprPtr> args;
            if (!check(TokenType::RPAREN)) {
                do {
                    args.push_back(expression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "expected ')' after call arguments");
            lhs = makeExpr(CallExpr{std::move(lhs), std::move(args), line});
        } else {
            break;
        }
    }
    return lhs;
}

ExprPtr Parser::primary() {
    if (match(TokenType::NUMBER)) {
        LiteralExpr e; e.kind = LiteralExpr::Kind::Int; e.intValue = previous().intValue;
        return makeExpr(e);
    }
    if (match(TokenType::STRING)) {
        LiteralExpr e; e.kind = LiteralExpr::Kind::String; e.stringValue = previous().lexeme;
        return makeExpr(e);
    }
    if (match(TokenType::TRUE)) {
        LiteralExpr e; e.kind = LiteralExpr::Kind::Bool; e.boolValue = true;
        return makeExpr(e);
    }
    if (match(TokenType::FALSE)) {
        LiteralExpr e; e.kind = LiteralExpr::Kind::Bool; e.boolValue = false;
        return makeExpr(e);
    }
    if (match(TokenType::NULL_KW)) {
        LiteralExpr e; e.kind = LiteralExpr::Kind::Null;
        return makeExpr(e);
    }
    if (match(TokenType::IDENT)) {
        return makeExpr(VarExpr{previous().lexeme, previous().line});
    }
    if (match(TokenType::LEN)) {
        int line = previous().line;
        consume(TokenType::LPAREN, "expected '(' after 'len'");
        ExprPtr coll = expression();
        consume(TokenType::RPAREN, "expected ')' after 'len' argument");
        return makeExpr(LenExpr{std::move(coll), line});
    }
    if (match(TokenType::LPAREN)) {
        ExprPtr e = expression();
        consume(TokenType::RPAREN, "expected ')' after expression");
        return e;
    }
    if (check(TokenType::LBRACKET)) {
        int line = peek().line; advance();
        return arrayLiteral(line);
    }
    if (check(TokenType::LBRACE)) {
        int line = peek().line; advance();
        return mapLiteral(line);
    }
    throw CompileError(peek().line, "expected expression (got '" + peek().lexeme + "')");
}

ExprPtr Parser::arrayLiteral(int line) {
    std::vector<ExprPtr> elements;
    if (!check(TokenType::RBRACKET)) {
        do { elements.push_back(expression()); } while (match(TokenType::COMMA));
    }
    consume(TokenType::RBRACKET, "expected ']' to close array literal");
    return makeExpr(ArrayExpr{std::move(elements), line});
}

ExprPtr Parser::mapLiteral(int line) {
    std::vector<ExprPtr> keys, values;
    if (!check(TokenType::RBRACE)) {
        do {
            ExprPtr k = expression();
            consume(TokenType::COLON, "expected ':'");
            ExprPtr v = expression();
            keys.push_back(std::move(k));
            values.push_back(std::move(v));
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RBRACE, "expected '}' to close map literal");
    return makeExpr(MapExpr{std::move(keys), std::move(values), line});
}

} // namespace cvm
