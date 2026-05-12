#pragma once

// =============================================================================
//  Error.h
//
//  Two exception types thrown across the pipeline:
//    - CompileError: lexer, parser, or compiler detected an invalid program.
//    - RuntimeError: VM hit an illegal state during execution.
//
//  main.cpp catches both at the top level; the REPL loop catches per-iteration.
// =============================================================================

#include <stdexcept>
#include <string>

namespace cvm {

class CompileError : public std::runtime_error {
public:
    CompileError(int line, const std::string& msg)
        : std::runtime_error("[line " + std::to_string(line) + "] " + msg),
          line_(line) {}

    int line() const { return line_; }

private:
    int line_;
};

class RuntimeError : public std::runtime_error {
public:
    explicit RuntimeError(const std::string& msg)
        : std::runtime_error(msg) {}
};

} // namespace cvm
