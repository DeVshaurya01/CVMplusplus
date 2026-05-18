# CVM++ 🚀

A small, high-performance scripting language and stack-based virtual machine, written completely from scratch in modern C++17.

CVM++ takes raw `.cvm` source code, tokenizes it, parses it into an Abstract Syntax Tree (AST), compiles that AST into custom bytecode, and executes it on a stack-based VM. Every stage of the pipeline is a standalone module under [`src/`](src/) — no compiler magic, nothing hidden behind layers of abstraction.

```
source.cvm  →  Lexer  →  Tokens  →  Parser  →  AST  →  Compiler  →  Bytecode  →  VM  →  Output
```

---

## What it does

- **Run scripts** — `./build_cvm.exe script.cvm` executes a `.cvm` file end-to-end.
- **Interactive REPL** — `./build_cvm.exe` (no args) drops you into an interactive session where every line is lexed, parsed, compiled, and run; global variables and defined functions persist across lines.
- **Inspect the pipeline** — the `--debug` or `-d` flag dumps the token stream, the AST structure, and the disassembled bytecode (with opcode names, offsets, and resolved identifier comments) to stderr before executing.
- **Rich Data Abstractions (New in v2!)** — Support for strings, dynamic heterogeneous arrays, key-value maps (hashtables), recursion, and deep mutability makes the engine capable of running complex data structures and algorithms.

---

## Language Features

CVM++ has evolved from a basic calculator into a robust, dynamic, and clean scripting language.

| Feature | Syntax & Details |
|---|---|
| **Data Types** | `int` (64-bit signed), `bool` (`true` / `false`), `string` (`"hello"`), `array` (dynamic collection `[1, 2]`), `map` (associative hashtable `{ "key": 42 }`), and `null` (representing empty / uninitialized references) |
| **Arithmetic & Logical** | `+` `-` `*` `/` (standard arithmetic), unary `-`, logical NOT `!`, and comparison `==` `!=` `<` |
| **Membership Operator** | `map has key` (evaluates to `true` or `false` indicating key presence) |
| **Variables** | `let x = 10;` (declaration), `x = 20;` (reassignment), and **deep index assignment**: `matrix[0][1] = 42;` |
| **Control Flow** | `if (cond) { ... } else { ... }` and `while (cond) { ... }` |
| **Functions & Scope** | `fn myFunc(a, b) { return a + b; }` (first-class definitions), supporting arguments, local scoping, and full recursion |
| **Built-in Functions** | `len(container)` (returns length of string, array, or map) |
| **I/O** | `print expr;` (prints any value type), `input x;` (reads one integer from stdin) |
| **Comments** | `// single-line comments` |

Full grammar in [`docs/GRAMMAR.md`](docs/GRAMMAR.md). Bytecode reference in [`docs/ISA.md`](docs/ISA.md). Architecture notes in [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

---

## Quick Taste of CVM++ v2

Here are a few examples showcasing the dynamic type system and advanced features in CVM++:

### 1. Recursion (Fibonacci & Factorial)
```javascript
fn factorial(n) {
    if (n == 0) {
        return 1;
    }
    return n * factorial(n - 1);
}

fn fib(n) {
    if (n < 2) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

print "--- Recursion Test ---";
print factorial(5); // 120
print fib(7);       // 13
```

### 2. Nested Data Structures & Deep Mutation
```javascript
let database = {
    "users": [
        { "id": 1, "name": "Alice", "tags": ["admin"] },
        { "id": 2, "name": "Bob",   "tags": ["user"] }
    ],
    "config": { "version": 1 }
};

// Deep access and in-place update
database["users"][1]["tags"] = ["user", "beta"];
print database["users"][1]["tags"]; // ["user", "beta"]
```

### 3. Tree Structures & Object-Pointer Behavior
Maps can be used to represent dynamic node objects, while `null` represents pointers to child nodes:
```javascript
fn createNode(val, next) {
    return { "val": val, "next": next };
}

// Build list: 10 -> 20 -> null
let list = createNode(10, createNode(20, null));

// Traverse list
let curr = list;
while (curr != null) {
    print curr["val"];
    curr = curr["next"];
}
```

---

## Build Instructions

Requirements: A C++17 compiler and CMake ≥ 3.15.

```bash
cmake -B build
cmake --build build
```
The compiled executable lands at `build/cvm` (`build/cvm.exe` on Windows).

> 💡 **Precompiled Binary:** A ready-to-run Windows executable `build_cvm.exe` is already compiled and available in the root directory!

### Windows Build Note (for MinGW Users)
The bundled MinGW.org GCC 6.3 is too old (lacks `<variant>`). Install [MSYS2](https://www.msys2.org/) and setup the UCRT64 toolchain:
```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake make
```
Then build from the **MSYS2 UCRT64** shell using:
```bash
cmake -B build -G "MinGW Makefiles" && cmake --build build
```

---

## Usage

### Run a Script
```bash
./build_cvm.exe examples/recursion.cvm
```

### Run in Interactive REPL
```bash
./build_cvm.exe
CVM++ REPL - Ctrl-D (or Ctrl-Z on Windows) to exit.
> fn square(x) { return x * x; }
> print square(9);
81
```

### Introspect via Debug Mode
Use the `--debug` or `-d` flag to see how CVM++ handles your code under the hood. It compiles the source, disassembles the instructions, prints the AST, and then executes:
```bash
./build_cvm.exe --debug examples/test.cvm
```
*Sample Bytecode Dump:*
```nasm
0000  OP_BUILD_ARRAY    3
0002  OP_DEFINE_GLOBAL  0    ; 'list'
0004  OP_GET_GLOBAL     0    ; 'list'
0006  OP_CONST_INT      1
0011  OP_INDEX_GET
0012  OP_PRINT
0013  OP_GET_GLOBAL     0    ; 'list'
0015  OP_CONST_INT      1
0020  OP_CONST_INT      50
0025  OP_INDEX_SET
0026  OP_HALT
```

---

## Project layout

```
cvm++/
├── src/
│   ├── Token.h, Lexer.h/.cpp        # Source scanner (added strings, null, keywords)
│   ├── AST.h, Parser.h/.cpp         # Recursive descent parser (arrays, maps, fns)
│   ├── Chunk.h, Value.h/.cpp        # Bytecode, tagged-union Value & Obj hierarchy
│   ├── Compiler.h/.cpp              # AST → Bytecode (local variables, call offsets)
│   ├── VM.h/.cpp                    # Stack-based VM with activation frames
│   ├── Debug.h/.cpp                 # Disassembler & AST visualizer
│   ├── Error.h                      # CompileError and RuntimeError definitions
│   └── main.cpp                     # CLI, REPL, and argument handler
├── docs/
│   ├── GRAMMAR.md                   # Formal EBNF grammar
│   ├── ISA.md                       # Bytecode instruction set architecture
│   └── ARCHITECTURE.md              # Pipeline architecture documentation
├── examples/                        # Collection of CVM++ examples & scripts
├── tests/run_tests.sh               # Diff-based test runner
├── build_cvm.exe                    # Precompiled Windows binary
└── CMakeLists.txt
```

---

## Core Design Choices (Upgraded for v2)

- **Tagged Union Runtime `Value`**: To support strings, arrays, maps, and functions, `Value` was upgraded from a bare `int64_t` to a tagged struct that manages standard types and heap-allocated objects (`Obj`).
- **Reference-Counted GC**: Leverages C++ `std::shared_ptr` for automatic reference-counted memory management of complex heap objects (`ObjString`, `ObjArray`, `ObjMap`, `ObjFunction`). No memory leaks on objects or lists.
- **Activation Records & Call Frames**: The virtual machine maintains a `CallFrame` call stack. Each frame tracks its instruction pointer (`ip`), current executing function, and base slot index in the VM value stack, facilitating clean lexical scope, parameters, and recursion.
- **Deep Index Assignment**: High-level AST translation of l-values allows complex recursive assignments like `arr[i][j] = val;` to map to stack evaluation followed by `OP_INDEX_SET`.
- **Switch-Dispatch Stack Machine**: High readability, debuggability, and predictability during development.

---

## All Examples

Every example under [`examples/`](examples/) represents a verified script checked by the automated test harness:

| File | Type | Feature Focus |
|---|---|---|
| [`recursion.cvm`](examples/recursion.cvm) | **Functions** | Recursive Factorial & Fibonacci, function parameter scoping |
| [`linked_list.cvm`](examples/linked_list.cvm) | **Objects** | Singly-linked list traversal, `null` references, dynamic maps |
| [`tree.cvm`](examples/tree.cvm) | **Objects** | Tree node structures, object-pointer mutation behavior |
| [`bst.cvm`](examples/bst.cvm) | **Algorithms** | Building a Binary Search Tree (BST) from scratch |
| [`bst_target.cvm`](examples/bst_target.cvm) | **Algorithms** | Recursive search and lookup query on a binary search tree |
| [`data_structures.cvm`](examples/data_structures.cvm) | **Complex** | Nested dynamic arrays inside maps, deep index mutability |
| [`longest_subarray.cvm`](examples/longest_subarray.cvm) | **Algorithms** | Prefix sum map hashing using `has` and `len()` built-ins |
| [`longest_subarray_test2.cvm`](examples/longest_subarray_test2.cvm) | **Algorithms** | Alternate benchmark test case for prefix hashing subarray length |
| [`test.cvm`](examples/test.cvm) | **Core** | Minimal test verifying array element mutation and map lookup |
| [`arithmetic.cvm`](examples/arithmetic.cvm) | **Core** | Operator precedence, parentheses, unary operators |
| [`variables.cvm`](examples/variables.cvm) | **Core** | Declarations (`let`), scoping, and reassignment |
| [`factorial.cvm`](examples/factorial.cvm) | **Core** | Iterative while-loop factorial accumulator |
| [`fizzbuzz.cvm`](examples/fizzbuzz.cvm) | **Core** | Nested conditionals, standard fizzbuzz output loop |
| [`fibonacci.cvm`](examples/fibonacci.cvm) | **Core** | Iterative O(N) Fibonacci sequence generation |
| [`gcd.cvm`](examples/gcd.cvm) | **Core** | Euclidean GCD using synthesized modulo operator |
| [`is_prime.cvm`](examples/is_prime.cvm) | **Core** | Trial division checking integer primality |
| [`count_primes.cvm`](examples/count_primes.cvm) | **Core** | Nested loops to count prime numbers in a range |
| [`nth_prime.cvm`](examples/nth_prime.cvm) | **Core** | Finding the N-th prime number iteratively |
| [`power.cvm`](examples/power.cvm) | **Core** | Fast binary exponentiation algorithm |
| [`digit_sum.cvm`](examples/digit_sum.cvm) | **Core** | Sum of base-10 digits of an integer |
| [`reverse_digits.cvm`](examples/reverse_digits.cvm) | **Core** | Base-10 integer digit reversing |
| [`palindrome_number.cvm`](examples/palindrome_number.cvm) | **Core** | Numeric palindrome verification |
| [`collatz.cvm`](examples/collatz.cvm) | **Core** | Collatz conjecture sequence walk length |
| [`guess.cvm`](examples/guess.cvm) | **Interactive** | Number guessing game leveraging standard `input` statement |

---

## Status

- [x] **Phase 1**: Tokenizer, recursive-descent parser, AST, compiler, stack VM, REPL, debug disassembler.
- [x] **Phase 2 (v2)**: Support for heap objects (strings, dynamic heterogeneous arrays, key-value maps), recursive functions, parameter passing, deep indexing & assignments, and rich operators (`!`, `!=`, `has`, `len()`).
- [ ] **Phase 3**: Dynamic closures, mark-and-sweep or generational garbage collector, standard utility libraries.

---

## References

- [_Crafting Interpreters_](https://craftinginterpreters.com/) by Robert Nystrom — Part III is the core architectural inspiration for this engine.
- [LLVM Kaleidoscope Tutorial](https://llvm.org/docs/tutorial/) — for C++ parser, AST, and lexical scanner design.

---

## License

MIT — see [`LICENSE`](LICENSE).
