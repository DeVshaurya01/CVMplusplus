#pragma once

// =============================================================================
//  AST.h
// =============================================================================

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace cvm {

struct Expr;
struct Stmt;
using ExprPtr = std::unique_ptr<Expr>;
using StmtPtr = std::unique_ptr<Stmt>;

// ----- Expression nodes -----
struct LiteralExpr {
    enum class Kind { Int, Bool, String, Null };
    Kind        kind;
    long long   intValue    = 0;
    bool        boolValue   = false;
    std::string stringValue;
};

struct VarExpr {
    std::string name;
    int         line;
};

struct UnaryExpr {
    enum class Op { Neg, Not };
    Op      op;
    ExprPtr operand;
};

struct BinaryExpr {
    enum class Op { Add, Sub, Mul, Div, Eq, Ne, Lt };
    Op      op;
    ExprPtr lhs;
    ExprPtr rhs;
};

struct AssignExpr {
    std::string name;
    ExprPtr     value;
    int         line;
};

struct ArrayExpr {
    std::vector<ExprPtr> elements;
    int                  line;
};

struct MapExpr {
    std::vector<ExprPtr> keys;
    std::vector<ExprPtr> values;
    int                  line;
};

struct IndexGetExpr {
    ExprPtr collection;
    ExprPtr index;
    int     line;
};

struct IndexSetExpr {
    ExprPtr collection;
    ExprPtr index;
    ExprPtr value;
    int     line;
};

struct LenExpr {
    ExprPtr collection;
    int     line;
};

struct HasExpr {
    ExprPtr collection;
    ExprPtr key;
    int     line;
};

struct CallExpr {
    ExprPtr              callee;
    std::vector<ExprPtr> args;
    int                  line;
};

struct Expr {
    std::variant<
        LiteralExpr,
        VarExpr,
        UnaryExpr,
        BinaryExpr,
        AssignExpr,
        ArrayExpr,
        MapExpr,
        IndexGetExpr,
        IndexSetExpr,
        LenExpr,
        HasExpr,
        CallExpr
    > node;
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
    std::vector<StmtPtr> elseBlk;
};

struct WhileStmt {
    ExprPtr              cond;
    std::vector<StmtPtr> body;
};

struct BlockStmt {
    std::vector<StmtPtr> stmts;
};

struct FnStmt {
    std::string              name;
    std::vector<std::string> params;
    std::vector<StmtPtr>     body;
    int                      line;
};

struct ReturnStmt {
    ExprPtr value; // can be nullptr for 'return;'
    int     line;
};

struct ExprStmt {
    ExprPtr expr;
};

struct Stmt {
    std::variant<
        LetStmt, PrintStmt, InputStmt, IfStmt, WhileStmt, 
        BlockStmt, FnStmt, ReturnStmt, ExprStmt
    > node;
};

} // namespace cvm
