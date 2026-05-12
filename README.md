# CVM++

A small scripting language and stack-based virtual machine, written from scratch in C++17.

CVM++ takes raw `.cvm` source code, tokenizes it, parses it into an Abstract Syntax Tree (AST), compiles that AST into a custom bytecode, and executes the bytecode on a stack VM. Every stage of the pipeline is a standalone module under [`src/`](src/) — no compiler magic, nothing hidden behind layers of abstraction.

```
source.cvm  →  Lexer  →  Tokens  →  Parser  →  AST  →  Compiler  →  Bytecode  →  VM  →  Output
```

---

## What it does

- **Run scripts** — `cvm script.cvm` executes a `.cvm` file end-to-end.
- **Interactive REPL** — `cvm` (no args) drops you into a prompt where every line is lexed, parsed, compiled, and run; globals persist across lines.
- **Inspect the pipeline** — the `--debug` flag dumps the token stream, the AST, and the disassembled bytecode to stderr before the program runs. Useful for understanding what your code becomes at each stage.

It is **not** a production language. There are no strings, no arrays, no functions. The point is to expose the mechanism — to see exactly how `let x = 10; print x * 2;` becomes a sequence of bytes that a stack machine consumes one instruction at a time.

---

## Language

| | |
|---|---|
| **Types** | `int` (64-bit signed), `bool` (stored as `1` / `0`) |
| **Operators** | `+` `-` `*` `/`, comparison `==` `<`, unary `-` |
| **Variables** | `let x = 10;` declare, `x = 20;` reassign |
| **Control flow** | `if (cond) { ... } else { ... }`, `while (cond) { ... }` |
| **I/O** | `print expr;`, `input x;` (reads one int from stdin) |
| **Comments** | `// to end of line` |

Full grammar in [`docs/GRAMMAR.md`](docs/GRAMMAR.md). Bytecode reference in [`docs/ISA.md`](docs/ISA.md). Architecture notes in [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

### Quick taste

```
let n = 10;
let sum = 0;
let i = 1;
while (i < n + 1) {
    sum = sum + i;
    i = i + 1;
}
print sum;          // 55
```

---

## Build

Requirements: a C++17 compiler and CMake ≥ 3.15.

```bash
cmake -B build
cmake --build build
```

The executable lands at `build/cvm` (`build/cvm.exe` on Windows).

**Windows note:** the bundled MinGW.org GCC 6.3 is too old (no `<variant>`). Use [MSYS2](https://www.msys2.org/) and install the UCRT64 toolchain:
```
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake make
```
Then build from the **MSYS2 UCRT64** shell with `cmake -B build -G "MinGW Makefiles" && cmake --build build`.

---

## Use

### Run a script
```bash
./build/cvm examples/factorial.cvm
# 720
```

### REPL
```bash
./build/cvm
CVM++ REPL - Ctrl-D (or Ctrl-Z on Windows) to exit.
> let x = 10;
> print x * x;
100
> let i = 0; while (i < 3) { print i; i = i + 1; }
0
1
2
```

### Debug mode
Dumps everything the compiler sees — tokens, AST, bytecode — before running the program:
```bash
./build/cvm --debug examples/factorial.cvm
```
Sample bytecode dump (offset, mnemonic, operand, resolved-name comment):
```
0021  OP_GET_GLOBAL     2    ; 'i'
0023  OP_GET_GLOBAL     0    ; 'n'
0025  OP_CONST_INT      1
0030  OP_ADD
0031  OP_LT
0032  OP_JUMP_IF_FALSE  0057
...
0054  OP_LOOP           0021
0057  OP_GET_GLOBAL     1    ; 'result'
0059  OP_PRINT
0060  OP_HALT
```

### Run the test suite
```bash
bash ./tests/run_tests.sh
```
Diffs each `examples/*.cvm` output against its `.expected` file. Auto-detects `cvm.exe` on Windows and normalizes CRLF.

---

## Examples

Each `.cvm` file under [`examples/`](examples/) ships with a matching `.expected` that the test harness diffs against.

| File | What it shows |
|---|---|
| [`arithmetic.cvm`](examples/arithmetic.cvm) | precedence, parentheses, unary `-` |
| [`variables.cvm`](examples/variables.cvm) | `let`, reassignment, expressions over vars |
| [`factorial.cvm`](examples/factorial.cvm) | `while` loop, accumulator |
| [`fizzbuzz.cvm`](examples/fizzbuzz.cvm) | nested `if`, integer-only output (no strings yet) |
| [`fibonacci.cvm`](examples/fibonacci.cvm) | Fibonacci by two-variable iteration |
| [`gcd.cvm`](examples/gcd.cvm) | Euclidean algorithm; mod synthesized as `n - (n/b)*b` |
| [`is_prime.cvm`](examples/is_prime.cvm) | trial division up to √n |
| [`count_primes.cvm`](examples/count_primes.cvm) | nested loops, count primes ≤ N |
| [`nth_prime.cvm`](examples/nth_prime.cvm) | walk integers, find 10th prime |
| [`power.cvm`](examples/power.cvm) | binary exponentiation, O(log b) |
| [`digit_sum.cvm`](examples/digit_sum.cvm) | sum decimal digits |
| [`reverse_digits.cvm`](examples/reverse_digits.cvm) | reverse digits via build-up |
| [`palindrome_number.cvm`](examples/palindrome_number.cvm) | reverse and compare |
| [`collatz.cvm`](examples/collatz.cvm) | 3n+1 sequence length from 27 (= 111) |
| [`guess.cvm`](examples/guess.cvm) | `input` keyword (interactive — not in test harness) |

> **No `%` operator yet** — modulo is written as `n - (n/b)*b`. This is the most common workaround you'll see across these examples.

---

## Project layout

```
cvm++/
├── src/
│   ├── Token.h, Lexer.h/.cpp        # source text → tokens
│   ├── AST.h, Parser.h/.cpp         # tokens → AST (std::variant nodes)
│   ├── Chunk.h, Value.h             # bytecode container, runtime value type
│   ├── Compiler.h/.cpp              # AST → bytecode (with jump backpatching)
│   ├── VM.h/.cpp                    # stack-based dispatch loop
│   ├── Debug.h/.cpp                 # token / AST / disassembly dumpers
│   ├── Error.h                      # CompileError, RuntimeError
│   └── main.cpp                     # CLI + REPL
├── docs/
│   ├── GRAMMAR.md                   # full EBNF grammar
│   ├── ISA.md                       # opcode reference
│   └── ARCHITECTURE.md              # design notes & rationale
├── examples/                        # .cvm scripts + .expected outputs
├── tests/run_tests.sh               # diff-based test harness
└── CMakeLists.txt
```

---

## Design choices

A few key decisions, briefly:

- **Stack-based VM.** Compiler backend is trivial: post-order AST traversal maps 1:1 to push/op sequences. Register allocation is a post-sprint problem.
- **`std::variant` AST nodes.** Cache-friendly, no virtual dispatch, plays well with `std::visit`.
- **Switch dispatch in the VM.** Computed gotos are a 20–40% win at millions of ops/sec but are demo-irrelevant here. Switch is portable and debuggable.
- **Globals via `unordered_map<string, Value>`.** Slower than indexed slots but trivially correct, and REPL state across submissions is automatic — just keep the map alive.
- **No constant pool for integers.** Inline 4 bytes after `OP_CONST_INT`. Names get a tiny parallel table because they're variable-length.

See [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) for the full reasoning.

---

## Status

✅ Day 1 — lexer, parser, AST, compiler, VM, REPL, --debug, all examples pass
🚧 v2 — strings, functions / closures, mark-and-sweep GC

---

## References

- [_Crafting Interpreters_](https://craftinginterpreters.com/) by Robert Nystrom — Part III is the architectural reference for this project.
- [LLVM Kaleidoscope tutorial](https://llvm.org/docs/tutorial/) — for C++ parser / AST patterns.

---

## License

MIT — see [`LICENSE`](LICENSE).
