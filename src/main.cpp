// =============================================================================
//  main.cpp  —  CLI entry point.
//
//  Owner: <fill in>
//  Branch: feat/debug-cli
//
//  Two modes:
//      cvm                    → REPL
//      cvm path/to/script.cvm → file runner
//
//  Optional flags:
//      --debug                → print tokens, AST, bytecode before running
//
//  Pipeline glue:
//      std::string src = readFile(path);    // or one line from stdin in REPL
//      Lexer lx(src);
//      auto tokens = lx.scanAll();
//      Parser p(std::move(tokens));
//      auto program = p.parseProgram();
//      Compiler c;
//      Chunk chunk = c.compile(program);
//      vm.run(chunk);                       // vm outlives the REPL loop
//
//  In REPL mode, wrap each iteration in try/catch so a parse/runtime error
//  doesn't kill the process. Globals survive because the VM instance does.
// =============================================================================

#include "Compiler.h"
#include "Debug.h"
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

[[maybe_unused]]
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

void runRepl([[maybe_unused]] bool debug) {
    std::cout << "CVM++ REPL — type Ctrl-D to exit.\n";
    // TODO(debug-cli):
    //   cvm::VM vm;
    //   std::string line;
    //   while (true) {
    //       std::cout << "> " << std::flush;
    //       if (!std::getline(std::cin, line)) break;
    //       if (line.empty()) continue;
    //       try {
    //           cvm::Lexer lx(line);
    //           auto toks = lx.scanAll();
    //           cvm::Parser p(std::move(toks));
    //           auto prog = p.parseProgram();
    //           cvm::Compiler c;
    //           auto chunk = c.compile(prog);
    //           if (debug) cvm::debug::disassemble(chunk, std::cerr, "<repl>");
    //           vm.run(chunk);
    //       } catch (const std::exception& e) {
    //           std::cerr << "error: " << e.what() << "\n";
    //       }
    //   }
}

void runFile([[maybe_unused]] const std::string& path, [[maybe_unused]] bool debug) {
    // TODO(debug-cli):
    //   auto src = readFile(path);
    //   cvm::Lexer lx(src);
    //   auto toks = lx.scanAll();
    //   if (debug) cvm::debug::printTokens(toks, std::cerr);
    //   cvm::Parser p(std::move(toks));
    //   auto prog = p.parseProgram();
    //   if (debug) cvm::debug::printAst(prog, std::cerr);
    //   cvm::Compiler c;
    //   auto chunk = c.compile(prog);
    //   if (debug) cvm::debug::disassemble(chunk, std::cerr, path.c_str());
    //   cvm::VM vm;
    //   vm.run(chunk);
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
    } catch (const std::exception& e) {
        std::cerr << "fatal: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
