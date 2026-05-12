// =============================================================================
//  main.cpp  —  CLI entry point.
//
//  Modes:
//      cvm                    -> REPL
//      cvm path/to/script.cvm -> file runner
//
//  Optional flags:
//      --debug / -d           -> print tokens, AST, bytecode before running
// =============================================================================

#include "Compiler.h"
#include "Debug.h"
#include "Error.h"
#include "Lexer.h"
#include "Parser.h"
#include "VM.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Args {
    bool        debug = false;
    bool        hasPath = false;
    std::string path;
};

Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s == "--debug" || s == "-d") {
            a.debug = true;
        } else if (s == "--help" || s == "-h") {
            std::cout << "Usage: cvm [--debug] [script.cvm]\n";
            std::exit(0);
        } else {
            a.hasPath = true;
            a.path    = s;
        }
    }
    return a;
}

std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) {
        std::cerr << "cvm: cannot open '" << path << "'\n";
        std::exit(1);
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Compile and run a source string against an existing VM. Throws on error.
void execute(const std::string& src, cvm::VM& vm, bool debug, const char* label) {
    cvm::Lexer lx(src);
    auto toks = lx.scanAll();
    if (debug) cvm::debug::printTokens(toks, std::cerr);

    cvm::Parser p(std::move(toks));
    auto prog = p.parseProgram();
    if (debug) cvm::debug::printAst(prog, std::cerr);

    cvm::Compiler c;
    auto chunk = c.compile(prog);
    if (debug) cvm::debug::disassemble(chunk, std::cerr, label);

    vm.run(chunk);
}

void runFile(const std::string& path, bool debug) {
    std::string src = readFile(path);
    cvm::VM vm;
    execute(src, vm, debug, path.c_str());
}

void runRepl(bool debug) {
    std::cout << "CVM++ REPL - Ctrl-D (or Ctrl-Z on Windows) to exit.\n";
    cvm::VM vm;
    std::string line;
    while (true) {
        std::cout << "> " << std::flush;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        try {
            execute(line, vm, debug, "<repl>");
        } catch (const cvm::CompileError& e) {
            std::cerr << "compile error: " << e.what() << "\n";
            vm.resetStack();
        } catch (const cvm::RuntimeError& e) {
            std::cerr << "runtime error: " << e.what() << "\n";
            vm.resetStack();
        } catch (const std::exception& e) {
            std::cerr << "error: " << e.what() << "\n";
            vm.resetStack();
        }
    }
}

} // namespace

int main(int argc, char** argv) {
    auto args = parseArgs(argc, argv);
    try {
        if (args.hasPath) {
            runFile(args.path, args.debug);
        } else {
            runRepl(args.debug);
        }
    } catch (const cvm::CompileError& e) {
        std::cerr << "compile error: " << e.what() << "\n";
        return 1;
    } catch (const cvm::RuntimeError& e) {
        std::cerr << "runtime error: " << e.what() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
