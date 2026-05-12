// =============================================================================
//  Debug.cpp
//
//  Diagnostic dumpers for --debug. Three free functions, one per stage.
// =============================================================================

#include "Debug.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <string>
#include <variant>

namespace cvm::debug {

namespace {

const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:      return "NUMBER";
        case TokenType::IDENT:       return "IDENT";
        case TokenType::TRUE:        return "TRUE";
        case TokenType::FALSE:       return "FALSE";
        case TokenType::LET:         return "LET";
        case TokenType::IF:          return "IF";
        case TokenType::ELSE:        return "ELSE";
        case TokenType::WHILE:       return "WHILE";
        case TokenType::PRINT:       return "PRINT";
        case TokenType::INPUT:       return "INPUT";
        case TokenType::PLUS:        return "PLUS";
        case TokenType::MINUS:       return "MINUS";
        case TokenType::STAR:        return "STAR";
        case TokenType::SLASH:       return "SLASH";
        case TokenType::EQ:          return "EQ";
        case TokenType::EQ_EQ:       return "EQ_EQ";
        case TokenType::LT:          return "LT";
        case TokenType::LPAREN:      return "LPAREN";
        case TokenType::RPAREN:      return "RPAREN";
        case TokenType::LBRACE:      return "LBRACE";
        case TokenType::RBRACE:      return "RBRACE";
        case TokenType::SEMI:        return "SEMI";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "?";
}

const char* binOpName(BinaryExpr::Op op) {
    switch (op) {
        case BinaryExpr::Op::Add: return "+";
        case BinaryExpr::Op::Sub: return "-";
        case BinaryExpr::Op::Mul: return "*";
        case BinaryExpr::Op::Div: return "/";
        case BinaryExpr::Op::Eq:  return "==";
        case BinaryExpr::Op::Lt:  return "<";
    }
    return "?";
}

template <class... Ts>
struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

void indent(std::ostream& os, int n) { for (int i = 0; i < n; ++i) os << "  "; }

void printExpr(const Expr& e, std::ostream& os, int depth);
void printStmt(const Stmt& s, std::ostream& os, int depth);

void printExpr(const Expr& e, std::ostream& os, int depth) {
    indent(os, depth);
    std::visit(Overloaded{
        [&](const LiteralExpr& x) {
            os << (x.isBool ? (x.value ? "Literal(true)" : "Literal(false)")
                            : "Literal(" + std::to_string(x.value) + ")") << "\n";
        },
        [&](const VarExpr& x)    { os << "Var(" << x.name << ")\n"; },
        [&](const UnaryExpr& x)  {
            os << "Unary(-)\n";
            printExpr(*x.operand, os, depth + 1);
        },
        [&](const BinaryExpr& x) {
            os << "Binary(" << binOpName(x.op) << ")\n";
            printExpr(*x.lhs, os, depth + 1);
            printExpr(*x.rhs, os, depth + 1);
        },
        [&](const AssignExpr& x) {
            os << "Assign(" << x.name << ")\n";
            printExpr(*x.value, os, depth + 1);
        },
    }, e.node);
}

void printStmt(const Stmt& s, std::ostream& os, int depth) {
    indent(os, depth);
    std::visit(Overloaded{
        [&](const LetStmt& x) {
            os << "Let(" << x.name << ")\n";
            printExpr(*x.value, os, depth + 1);
        },
        [&](const PrintStmt& x) {
            os << "Print\n";
            printExpr(*x.value, os, depth + 1);
        },
        [&](const InputStmt& x) {
            os << "Input(" << x.name << ")\n";
        },
        [&](const IfStmt& x) {
            os << "If\n";
            indent(os, depth + 1); os << "cond:\n";
            printExpr(*x.cond, os, depth + 2);
            indent(os, depth + 1); os << "then:\n";
            for (const auto& st : x.thenBlk) printStmt(*st, os, depth + 2);
            if (!x.elseBlk.empty()) {
                indent(os, depth + 1); os << "else:\n";
                for (const auto& st : x.elseBlk) printStmt(*st, os, depth + 2);
            }
        },
        [&](const WhileStmt& x) {
            os << "While\n";
            indent(os, depth + 1); os << "cond:\n";
            printExpr(*x.cond, os, depth + 2);
            indent(os, depth + 1); os << "body:\n";
            for (const auto& st : x.body) printStmt(*st, os, depth + 2);
        },
        [&](const BlockStmt& x) {
            os << "Block\n";
            for (const auto& st : x.stmts) printStmt(*st, os, depth + 1);
        },
        [&](const ExprStmt& x) {
            os << "ExprStmt\n";
            printExpr(*x.expr, os, depth + 1);
        },
    }, s.node);
}

// Disassemble one instruction. Returns the offset of the next instruction.
std::size_t disasmOne(const Chunk& chunk, std::size_t offset, std::ostream& os) {
    os << std::right << std::setw(4) << std::setfill('0') << offset << "  ";
    os << std::setfill(' ');

    auto simple = [&](const char* name) -> std::size_t {
        os << name << "\n";
        return offset + 1;
    };
    auto withByte = [&](const char* name) -> std::size_t {
        std::uint8_t idx = chunk.code[offset + 1];
        os << std::left << std::setw(18) << name << static_cast<int>(idx);
        if (idx < chunk.names.size()) os << "    ; '" << chunk.names[idx] << "'";
        os << "\n";
        return offset + 2;
    };
    auto withInt = [&](const char* name) -> std::size_t {
        std::uint32_t b0 = chunk.code[offset + 1];
        std::uint32_t b1 = chunk.code[offset + 2];
        std::uint32_t b2 = chunk.code[offset + 3];
        std::uint32_t b3 = chunk.code[offset + 4];
        std::int32_t v = static_cast<std::int32_t>(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
        os << std::left << std::setw(18) << name << v << "\n";
        return offset + 5;
    };
    auto jumpInstr = [&](const char* name, int sign) -> std::size_t {
        std::uint16_t lo = chunk.code[offset + 1];
        std::uint16_t hi = chunk.code[offset + 2];
        std::uint16_t off = static_cast<std::uint16_t>(lo | (hi << 8));
        std::size_t target = (sign > 0)
            ? (offset + 3 + off)
            : (offset + 3 - off);
        os << std::left << std::setw(18) << name
           << std::right << std::setw(4) << std::setfill('0') << target
           << std::setfill(' ') << "\n";
        return offset + 3;
    };

    OpCode op = static_cast<OpCode>(chunk.code[offset]);
    switch (op) {
        case OP_CONST_INT:        return withInt("OP_CONST_INT");
        case OP_TRUE:             return simple("OP_TRUE");
        case OP_FALSE:            return simple("OP_FALSE");
        case OP_POP:              return simple("OP_POP");
        case OP_ADD:              return simple("OP_ADD");
        case OP_SUB:              return simple("OP_SUB");
        case OP_MUL:              return simple("OP_MUL");
        case OP_DIV:              return simple("OP_DIV");
        case OP_NEG:              return simple("OP_NEG");
        case OP_EQ:               return simple("OP_EQ");
        case OP_LT:               return simple("OP_LT");
        case OP_DEFINE_GLOBAL:    return withByte("OP_DEFINE_GLOBAL");
        case OP_GET_GLOBAL:       return withByte("OP_GET_GLOBAL");
        case OP_SET_GLOBAL:       return withByte("OP_SET_GLOBAL");
        case OP_JUMP:             return jumpInstr("OP_JUMP", +1);
        case OP_JUMP_IF_FALSE:    return jumpInstr("OP_JUMP_IF_FALSE", +1);
        case OP_LOOP:             return jumpInstr("OP_LOOP", -1);
        case OP_PRINT:            return simple("OP_PRINT");
        case OP_INPUT:            return simple("OP_INPUT");
        case OP_HALT:             return simple("OP_HALT");
    }
    os << "??? (" << static_cast<int>(op) << ")\n";
    return offset + 1;
}

} // namespace

void printTokens(const std::vector<Token>& tokens, std::ostream& os) {
    os << "== tokens ==\n";
    for (const auto& t : tokens) {
        os << "  line " << std::right << std::setw(3) << t.line << "  "
           << std::left << std::setw(12) << tokenTypeName(t.type)
           << "  '" << t.lexeme << "'";
        if (t.type == TokenType::NUMBER) os << "  (" << t.intValue << ")";
        os << "\n";
    }
}

void printAst(const std::vector<StmtPtr>& program, std::ostream& os) {
    os << "== ast ==\n";
    for (const auto& s : program) printStmt(*s, os, 0);
}

void disassemble(const Chunk& chunk, std::ostream& os, const char* label) {
    os << "== bytecode: " << label << " ==\n";
    std::size_t offset = 0;
    while (offset < chunk.code.size()) {
        offset = disasmOne(chunk, offset, os);
    }
}

} // namespace cvm::debug
