# CVM++ Architecture

## Pipeline

```
┌─────────┐   ┌───────┐   ┌─────────┐   ┌───────┐   ┌──────────┐   ┌──────────┐   ┌────┐
│ source  │ → │ Lexer │ → │ Tokens  │ → │Parser │ → │   AST    │ → │ Compiler │ → │ VM │
│  .cvm   │   │       │   │         │   │       │   │ (variant │   │          │   │    │
└─────────┘   └───────┘   └─────────┘   └───────┘   │  nodes)  │   │ Bytecode │   │ stdout │
                                                    └──────────┘   └──────────┘   └────┘
```

Every stage is a pure transformation: it owns its input/output types and
exposes a single entry function. No global state.

---

## Module responsibilities

| Module | Owns | Entry point | Output |
|---|---|---|---|
| `Lexer` | scanning source chars → tokens | `Lexer(src).scanAll()` | `std::vector<Token>` |
| `Parser` | tokens → AST | `Parser(toks).parseProgram()` | `std::vector<StmtPtr>` |
| `Compiler` | AST → bytecode | `Compiler().compile(program)` | `Chunk` |
| `VM` | execute bytecode | `VM().run(chunk)` | side effects (stdout) |
| `Debug` | dump tokens / AST / bytecode | `Debug::*` | strings to stderr |
| `main` | CLI + REPL glue | — | — |

---

## Key design choices

**Stack-based VM, not register-based.** Compiler backend is trivial: post-order
AST traversal maps 1:1 to push/op sequences. Register allocation is a
post-sprint problem.

**`std::variant` for AST nodes.** Avoids virtual dispatch, keeps nodes
cache-friendly, plays nicely with `std::visit` (or `std::get_if` chains).
Tradeoff: adding a new node type touches every visitor. Acceptable —
we won't add many.

**Integers and booleans share a representation.** `Value` is just `int64_t`
for v1: `true` = 1, `false` = 0, comparisons return 0/1. Cuts type-checking
overhead. If we add strings later, `Value` upgrades to a tagged union.

**Globals via `unordered_map<string, Value>`.** Slower than indexed slots but
trivially correct, and REPL state across submissions is automatic — just
keep the map alive across `run()` calls.

**No constant pool for integers.** Inline 4 bytes after `OP_CONST_INT`. Saves
us building a constant-table indexing system on Day 1. Names get a tiny
parallel table because they're variable-length strings.

**Switch dispatch in the VM.** Computed gotos are a 20–40% win at millions
of ops/sec — irrelevant for demo workloads. Switch keeps the code portable
and debuggable.

---

## Error handling

- **Lexer errors** (unknown character) → throw `CompileError` with line number.
- **Parser errors** (unexpected token) → throw `CompileError` with line + token.
- **Compiler errors** (undefined variable, etc.) → throw `CompileError`.
- **VM runtime errors** (stack underflow, divide-by-zero) → throw `RuntimeError`.

`main.cpp` catches both at the top level. In REPL mode, exceptions reset
the loop without killing the process; globals survive.

---

## Build & file layout

Each module has a `.h` declaring its public interface and a `.cpp` with the
implementation. `main.cpp` orchestrates. Single executable, single
`CMakeLists.txt`.
