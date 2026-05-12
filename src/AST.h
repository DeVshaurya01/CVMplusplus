#pragma once

// =============================================================================
//  AST.h
//
//  Abstract Syntax Tree node definitions. We use std::variant for the
//  algebraic data type so traversal is std::visit-friendly and nodes are
//  cache-friendly.
//
//  Two top-level variants: Expr and Stmt. Children are held via
//  std::unique_ptr<Expr> / std::unique_ptr<Stmt>.
// =============================================================================

#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace cvm {

// Forward-declare so unique_ptr<Expr>/unique_ptr<Stmt> can appear in
// node bodies before the variant aliases are defined.
struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// ----- Expression nodes -----
struct LiteralExpr {
    long long value;     // int literal value, or 1/0 for bool
    bool      isBool;
};

struct VarExpr {
    std::string name;
    int         line;
};

struct UnaryExpr {
    enum class Op { Neg };
    Op      op;
    ExprPtr operand;
};

struct BinaryExpr {
    enum class Op { Add, Sub, Mul, Div, Eq, Lt };
    Op      op;
    ExprPtr lhs;
    ExprPtr rhs;
};

struct AssignExpr {
    std::string name;
    ExprPtr     value;
    int         line;
};

struct Expr {
    std::variant<LiteralExpr, VarExpr, UnaryExpr, BinaryExpr, AssignExpr> node;
};

// ----- Statement nodes -----
struct LetStmt {
    std::string name;
    ExprPtr     value;
    int         line;
};

struct PrintStmt {
    ExprPtr value;
};

struct InputStmt {
    std::string name;
    int         line;
};

struct IfStmt {
    ExprPtr              cond;
    std::vector<StmtPtr> thenBlk;
    std::vector<StmtPtr> elseBlk;   // empty if no else
};

struct WhileStmt {
    ExprPtr              cond;
    std::vector<StmtPtr> body;
};

struct BlockStmt {
    std::vector<StmtPtr> stmts;
};

struct ExprStmt {
    ExprPtr expr;
};

struct Stmt {
    std::variant<LetStmt, PrintStmt, InputStmt, IfStmt, WhileStmt, BlockStmt, ExprStmt> node;
};

} // namespace cvm
