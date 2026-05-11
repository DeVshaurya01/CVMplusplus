# Contributing

We're a 2–3 person team working a 3-day sprint. Here's how we keep merges
clean.

---

## Workflow

1. **Pick a branch from the table in the README** (`feat/lexer`, `feat/parser`, etc.) or create `feat/<area>-<short-desc>` for a sub-task.
2. **Pull `main` before starting:** `git pull --rebase origin main`.
3. **Commit often, in small chunks.** One concept per commit.
4. **Open a PR** to `main` when your slice compiles and runs.
5. **Self-review** the diff before requesting a teammate.
6. **One approval, then merge.** Squash if the commit history is messy.

---

## Commit messages

`<area>: <imperative summary>`

Examples:
- `lexer: handle multi-char operators (== and <)`
- `parser: add if/else with optional else branch`
- `vm: fix stack underflow on OP_POP`
- `docs: clarify backpatching in ISA.md`

---

## Code style

- **C++17.** No newer features (some old GCCs in lab environments).
- **No `using namespace std;`** anywhere except inside `.cpp` function bodies if absolutely needed.
- **All cvm code lives in `namespace cvm`.** Debug helpers in `cvm::debug`.
- **Header guards:** `#pragma once` (we're not supporting weird compilers).
- **Includes:** local headers first (`"Foo.h"`), then standard headers (`<string>`), alphabetized within each group.
- **Indent:** 4 spaces, no tabs.
- **Brace style:** opening brace on same line as function/control statement.
- **Pointers:** prefer `std::unique_ptr` for owning, raw `T*` for non-owning, `const T&` for params unless you need to move.

---

## Dividing the work (suggested for 3 people)

| Person | Day 1 | Day 2 | Day 3 |
|---|---|---|---|
| A — Frontend | Lexer, all tokens | Polish errors | Help with REPL |
| B — Middle | Parser + AST | Compiler: globals, control flow | Demo scripts |
| C — Backend | Compiler (Day 1 vertical slice) + VM | VM: jumps, input | REPL + debug flag + README polish |

Person C carries the integration risk on Day 1, which is fine because they
own the runtime end-to-end. Person A and B unblock C by Day 1 noon.

For 2 people: A does lexer + parser + debug; B does compiler + VM + main.
Pair on the Day 1 integration.

---

## Definition of "done" per stage

- **Lexer:** all sample `.cvm` files in `examples/` tokenize without throwing. Print the token stream to verify.
- **Parser:** all sample files produce an AST. `printAst` output is sensible.
- **Compiler:** all sample files compile to bytecode. `disassemble` output is sensible.
- **VM:** `tests/run_tests.sh` passes (excluding `guess.cvm` which needs stdin).
- **REPL:** persists state across lines; survives parse errors.

---

## Before opening a PR

```bash
cmake --build build           # compiles cleanly with -Wall -Wextra
./tests/run_tests.sh          # all green (or only the stdin one skipped)
```

If your PR adds a new feature, add or update an example in `examples/`.
