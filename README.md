# CVM++

A lightweight scripting language and stack-based virtual machine, built from scratch in C++.

CVM++ takes raw `.cvm` source code, tokenizes it, parses it into an AST, compiles that AST into a custom bytecode, and executes the bytecode on a stack-based virtual machine — demystifying every stage of the compilation pipeline that production languages like Python, Java, and JavaScript hide behind layers of abstraction.

---

## Pipeline

```
source.cvm  →  Lexer  →  Tokens  →  Parser  →  AST  →  Compiler  →  Bytecode  →  VM  →  Output
```

Each stage is a standalone module under `src/`, swappable and independently testable.

---

## Language features

- **Types:** integers, booleans
- **Operators:** `+`, `-`, `*`, `/`, `==`, `<`
- **Variables:** `let x = 10;`, reassignment `x = 20;`
- **Control flow:** `if / else`, `while`
- **I/O:** `print expr;`, `input` keyword

See [`docs/GRAMMAR.md`](docs/GRAMMAR.md) for the full BNF and [`docs/ISA.md`](docs/ISA.md) for the opcode reference.

---

## Build

Requires CMake ≥ 3.15 and a C++17 compiler.

```bash
git clone <repo-url> cvm-plus-plus
cd cvm-plus-plus
cmake -B build
cmake --build build
```

Binary lands at `build/cvm`.

---

## Usage

**Run a script:**
```bash
./build/cvm examples/arithmetic.cvm
```

**Interactive REPL:**
```bash
./build/cvm
> let x = 10;
> print x * 2;
20
```

**Debug mode** — dumps tokens, AST, and disassembled bytecode before execution:
```bash
./build/cvm --debug examples/factorial.cvm
```

---

## Example

```
let n = 10;
let sum = 0;
let i = 1;
while (i < n + 1) {
    sum = sum + i;
    i = i + 1;
}
print sum;
```

Output: `55`

More samples in [`examples/`](examples/).

---

## Project structure

```
cvm-plus-plus/
├── src/             # Lexer, Parser, Compiler, VM, Debug, main
├── docs/            # Grammar, ISA, architecture notes
├── examples/        # Sample .cvm scripts
├── tests/           # Output-diff test harness
└── CMakeLists.txt
```

---

## Team workflow

We work on feature branches and merge to `main` via PR. Suggested branches:

| Branch | Owner | Scope |
|---|---|---|
| `feat/lexer` | — | `Token.h`, `Lexer.h/.cpp` |
| `feat/parser` | — | `AST.h`, `Parser.h/.cpp` |
| `feat/compiler` | — | `Chunk.h`, `Value.h`, `Compiler.h/.cpp` |
| `feat/vm` | — | `VM.h/.cpp` |
| `feat/debug-cli` | — | `Debug.h/.cpp`, `main.cpp`, REPL |

Fill in owners after Day 0 kickoff. Use `feat/<area>-<short-desc>` for sub-tasks.

**Commit style:** `lexer: handle multi-char operators` / `vm: fix stack underflow on OP_POP`.

---

## Status

🚧 Under active development (3-day sprint).

- [ ] Day 1: vertical slice (`print 1 + 2;`)
- [ ] Day 2: variables, control flow, input
- [ ] Day 3: REPL, debug flag, demo scripts, README polish

---

## References

- [_Crafting Interpreters_](https://craftinginterpreters.com/) by Robert Nystrom — Part III is the architectural reference for this project.
- [LLVM Kaleidoscope tutorial](https://llvm.org/docs/tutorial/) — for C++ parser/AST patterns.

---

## Mentor

Raman — 7977779056

## License

MIT — see [`LICENSE`](LICENSE).
