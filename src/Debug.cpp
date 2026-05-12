// =============================================================================
//  Debug.cpp
// =============================================================================

#include "Debug.h"
#include "Chunk.h"
#include "Token.h"
#include "AST.h"

#include <iomanip>
#include <iostream>
#include <variant>

namespace cvm {
namespace debug {

namespace {

template <class... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template <class... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

std::string tokenTypeName(TokenType t) {
    switch (t) {
        case TokenType::NUMBER:      return "NUMBER";
        case TokenType::STRING:      return "STRING";
        case TokenType::IDENT:       return "IDENT";
        case TokenType::TRUE:        return "TRUE";
        case TokenType::FALSE:       return "FALSE";
        case TokenType::NULL_KW:     return "NULL_KW";
        case TokenType::LET:         return "LET";
        case TokenType::FN:          return "FN";
        case TokenType::RETURN:      return "RETURN";
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
        case TokenType::BANG_EQ:     return "BANG_EQ";
        case TokenType::BANG:        return "BANG";
        case TokenType::LT:          return "LT";
        case TokenType::LPAREN:      return "LPAREN";
        case TokenType::RPAREN:      return "RPAREN";
        case TokenType::LBRACE:      return "LBRACE";
        case TokenType::RBRACE:      return "RBRACE";
        case TokenType::LBRACKET:    return "LBRACKET";
        case TokenType::RBRACKET:    return "RBRACKET";
        case TokenType::COLON:       return "COLON";
        case TokenType::COMMA:       return "COMMA";
        case TokenType::LEN:         return "LEN";
        case TokenType::HAS:         return "HAS";
        case TokenType::SEMI:        return "SEMI";
        case TokenType::END_OF_FILE: return "EOF";
    }
    return "???";
}

void indent(std::ostream& os, int depth) {
    for (int i = 0; i < depth; ++i) os << "  ";
}

void printExpr(const Expr& e, std::ostream& os, int depth) {
    indent(os, depth);
    std::visit(Overloaded{
        [&](const LiteralExpr& x) {
            os << "Literal(";
            switch (x.kind) {
                case LiteralExpr::Kind::Int:    os << x.intValue; break;
                case LiteralExpr::Kind::Bool:   os << (x.boolValue ? "true" : "false"); break;
                case LiteralExpr::Kind::String: os << "\"" << x.stringValue << "\""; break;
                case LiteralExpr::Kind::Null:   os << "null"; break;
            }
            os << ")\n";
        },
        [&](const VarExpr& x) { os << "Var(" << x.name << ")\n"; },
        [&](const UnaryExpr& x) {
            const char* s = (x.op == UnaryExpr::Op::Neg ? "-" : "!");
            os << "Unary(" << s << ")\n";
            printExpr(*x.operand, os, depth + 1);
        },
        [&](const BinaryExpr& x) {
            const char* s = "?";
            switch (x.op) {
                case BinaryExpr::Op::Add: s = "+";  break;
                case BinaryExpr::Op::Sub: s = "-";  break;
                case BinaryExpr::Op::Mul: s = "*";  break;
                case BinaryExpr::Op::Div: s = "/";  break;
                case BinaryExpr::Op::Eq:  s = "=="; break;
                case BinaryExpr::Op::Ne:  s = "!="; break;
                case BinaryExpr::Op::Lt:  s = "<";  break;
            }
            os << "Binary(" << s << ")\n";
            printExpr(*x.lhs, os, depth + 1);
            printExpr(*x.rhs, os, depth + 1);
        },
        [&](const AssignExpr& x) {
            os << "Assign(" << x.name << ")\n";
            printExpr(*x.value, os, depth + 1);
        },
        [&](const ArrayExpr& x) {
            os << "Array[" << x.elements.size() << "]\n";
            for (const auto& elem : x.elements) printExpr(*elem, os, depth + 1);
        },
        [&](const MapExpr& x) {
            os << "Map{" << x.keys.size() << "}\n";
            for (std::size_t i = 0; i < x.keys.size(); ++i) {
                indent(os, depth + 1); os << "key:\n"; printExpr(*x.keys[i], os, depth + 2);
                indent(os, depth + 1); os << "val:\n"; printExpr(*x.values[i], os, depth + 2);
            }
        },
        [&](const IndexGetExpr& x) {
            os << "IndexGet\n";
            indent(os, depth + 1); os << "coll:\n"; printExpr(*x.collection, os, depth + 2);
            indent(os, depth + 1); os << "idx:\n"; printExpr(*x.index, os, depth + 2);
        },
        [&](const IndexSetExpr& x) {
            os << "IndexSet\n";
            indent(os, depth + 1); os << "coll:\n"; printExpr(*x.collection, os, depth + 2);
            indent(os, depth + 1); os << "idx:\n"; printExpr(*x.index, os, depth + 2);
            indent(os, depth + 1); os << "val:\n"; printExpr(*x.value, os, depth + 2);
        },
        [&](const LenExpr& x) { os << "Len\n"; printExpr(*x.collection, os, depth + 1); },
        [&](const HasExpr& x) {
            os << "Has\n";
            indent(os, depth + 1); os << "coll:\n"; printExpr(*x.collection, os, depth + 2);
            indent(os, depth + 1); os << "key:\n"; printExpr(*x.key, os, depth + 2);
        },
        [&](const CallExpr& x) {
            os << "Call\n";
            indent(os, depth + 1); os << "callee:\n"; printExpr(*x.callee, os, depth + 2);
            indent(os, depth + 1); os << "args:\n";
            for (const auto& arg : x.args) printExpr(*arg, os, depth + 2);
        }
    }, e.node);
}

} // namespace

void printTokens(const std::vector<Token>& tokens, std::ostream& os) {
    os << "== tokens ==\n";
    for (const auto& t : tokens) {
        os << "  line " << std::setw(3) << t.line << "  "
           << std::setw(15) << std::left << tokenTypeName(t.type)
           << " '" << t.lexeme << "'";
        if (t.type == TokenType::NUMBER) os << "  (" << t.intValue << ")";
        os << "\n";
    }
}

void printAst(const std::vector<StmtPtr>& program, std::ostream& os) {
    os << "== ast ==\n";
    for (const auto& s : program) {
        std::visit(Overloaded{
            [&](const LetStmt& x) { os << "Let(" << x.name << ")\n"; printExpr(*x.value, os, 1); },
            [&](const PrintStmt& x) { os << "Print\n"; printExpr(*x.value, os, 1); },
            [&](const InputStmt& x) { os << "Input(" << x.name << ")\n"; },
            [&](const IfStmt& x) {
                os << "If\n"; printExpr(*x.cond, os, 1);
                indent(os, 1); os << "Then:\n"; // simplified
            },
            [&](const WhileStmt& x) { os << "While\n"; printExpr(*x.cond, os, 1); },
            [&](const BlockStmt&) { os << "Block\n"; },
            [&](const FnStmt& x) { os << "Fn(" << x.name << ")\n"; },
            [&](const ReturnStmt&) { os << "Return\n"; },
            [&](const ExprStmt& x) { os << "ExprStmt\n"; printExpr(*x.expr, os, 1); }
        }, s->node);
    }
}

std::size_t disasmOne(const Chunk& chunk, std::size_t ip, std::ostream& os) {
    os << std::setw(4) << std::setfill('0') << ip << "  ";
    if (ip > 0 && chunk.lines[ip] == chunk.lines[ip - 1]) os << "   |  ";
    else os << std::setw(4) << std::setfill(' ') << chunk.lines[ip] << "  ";

    OpCode op = static_cast<OpCode>(chunk.code[ip]);
    auto simple = [&](const char* name) { os << name << "\n"; return ip + 1; };
    auto withName = [&](const char* name) {
        std::uint8_t b = chunk.code[ip + 1];
        os << std::left << std::setw(16) << name << " " << (int)b << " ; '" << chunk.names[b] << "'\n";
        return ip + 2;
    };
    auto withShort = [&](const char* name) {
        std::uint16_t val = chunk.code[ip + 1] | (chunk.code[ip + 2] << 8);
        os << std::left << std::setw(16) << name << " " << val << "\n";
        return ip + 3;
    };
    auto withInt = [&](const char* name) {
        std::int32_t val = chunk.code[ip + 1] | (chunk.code[ip + 2] << 8) | (chunk.code[ip + 3] << 16) | (chunk.code[ip + 4] << 24);
        os << std::left << std::setw(16) << name << " " << val << "\n";
        return ip + 5;
    };
    auto withByte = [&](const char* name) {
        os << std::left << std::setw(16) << name << " " << (int)chunk.code[ip + 1] << "\n";
        return ip + 2;
    };

    switch (op) {
        case OP_CONST_INT:        return withInt  ("OP_CONST_INT");
        case OP_CONST_STR:        return withName ("OP_CONST_STR");
        case OP_TRUE:             return simple   ("OP_TRUE");
        case OP_FALSE:            return simple   ("OP_FALSE");
        case OP_NULL:             return simple   ("OP_NULL");
        case OP_POP:              return simple   ("OP_POP");
        case OP_ADD:              return simple   ("OP_ADD");
        case OP_SUB:              return simple   ("OP_SUB");
        case OP_MUL:              return simple   ("OP_MUL");
        case OP_DIV:              return simple   ("OP_DIV");
        case OP_NEG:              return simple   ("OP_NEG");
        case OP_NOT:              return simple   ("OP_NOT");
        case OP_EQ:               return simple   ("OP_EQ");
        case OP_NE:               return simple   ("OP_NE");
        case OP_LT:               return simple   ("OP_LT");
        case OP_DEFINE_GLOBAL:    return withName ("OP_DEFINE_GLOBAL");
        case OP_GET_GLOBAL:       return withName ("OP_GET_GLOBAL");
        case OP_SET_GLOBAL:       return withName ("OP_SET_GLOBAL");
        case OP_GET_LOCAL:        return withByte ("OP_GET_LOCAL");
        case OP_SET_LOCAL:        return withByte ("OP_SET_LOCAL");
        case OP_JUMP:             return withShort("OP_JUMP");
        case OP_JUMP_IF_FALSE:    return withShort("OP_JUMP_IF_FALSE");
        case OP_LOOP:             return withShort("OP_LOOP");
        case OP_PRINT:            return simple   ("OP_PRINT");
        case OP_INPUT:            return simple   ("OP_INPUT");
        case OP_BUILD_ARRAY:      return withByte ("OP_BUILD_ARRAY");
        case OP_BUILD_MAP:        return withByte ("OP_BUILD_MAP");
        case OP_INDEX_GET:        return simple   ("OP_INDEX_GET");
        case OP_INDEX_SET:        return simple   ("OP_INDEX_SET");
        case OP_LEN:              return simple   ("OP_LEN");
        case OP_HAS_KEY:          return simple   ("OP_HAS_KEY");
        case OP_CLOSURE:          return withByte ("OP_CLOSURE");
        case OP_CALL:             return withByte ("OP_CALL");
        case OP_RETURN:           return simple   ("OP_RETURN");
        case OP_HALT:             return simple   ("OP_HALT");
    }
    os << "??? (" << (int)op << ")\n"; return ip + 1;
}

void disassemble(const Chunk& chunk, std::ostream& os, const char* label) {
    os << "== bytecode: " << label << " ==\n";
    for (std::size_t ip = 0; ip < chunk.code.size(); ) ip = disasmOne(chunk, ip, os);
}

} // namespace debug
} // namespace cvm
