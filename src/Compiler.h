#pragma once

// =============================================================================
//  Compiler.h
// =============================================================================

#include "AST.h"
#include "Chunk.h"
#include <vector>
#include <string>

namespace cvm {

struct Local {
    std::string name;
    int         depth; // -1 if not initialized
};

class Compiler {
public:
    Chunk compile(const std::vector<StmtPtr>& program);

private:
    void compileStmt(const Stmt& s);
    void compileExpr(const Expr& e);
    void compileBlock(const std::vector<StmtPtr>& stmts);

    std::size_t emitJump(OpCode op, int line);
    void        patchJump(std::size_t addr);
    void        emitLoop(std::size_t loopStart, int line);

    // Local variable management
    int  resolveLocal(const std::string& name);
    void addLocal(const std::string& name);
    void beginScope();
    void endScope();

    Chunk*               chunk_       = nullptr;
    int                  currentLine_ = 1;
    int                  scopeDepth_  = 0;
    std::vector<Local>   locals_;
};

} // namespace cvm
