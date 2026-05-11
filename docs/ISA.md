# CVM++ Instruction Set Architecture

The compiler emits a flat `std::vector<uint8_t>` of bytecode. Each instruction
is a 1-byte opcode optionally followed by inline operands.

The VM uses a single value stack (`std::vector<Value>`) and a globals map
(`std::unordered_map<std::string, Value>`).

---

## Opcode table

| Opcode | Bytes | Operand format | Stack effect | Description |
|---|---|---|---|---|
| `OP_CONST_INT` | 5 | 4-byte little-endian `int32_t` | `→ value` | Push integer literal. |
| `OP_TRUE` | 1 | — | `→ true` | Push boolean `true` (as int 1). |
| `OP_FALSE` | 1 | — | `→ false` | Push boolean `false` (as int 0). |
| `OP_POP` | 1 | — | `value →` | Discard top of stack. |
| `OP_ADD` | 1 | — | `a, b → a+b` | Integer add. |
| `OP_SUB` | 1 | — | `a, b → a-b` | Integer subtract. |
| `OP_MUL` | 1 | — | `a, b → a*b` | Integer multiply. |
| `OP_DIV` | 1 | — | `a, b → a/b` | Integer divide. |
| `OP_NEG` | 1 | — | `a → -a` | Unary negate. |
| `OP_EQ` | 1 | — | `a, b → (a==b)` | Equality. Pushes 1 or 0. |
| `OP_LT` | 1 | — | `a, b → (a<b)` | Less-than. Pushes 1 or 0. |
| `OP_DEFINE_GLOBAL` | 2 | 1-byte name index | `value →` | Pop and bind to global name. |
| `OP_GET_GLOBAL` | 2 | 1-byte name index | `→ value` | Look up and push global. |
| `OP_SET_GLOBAL` | 2 | 1-byte name index | `value → value` | Assign without pop (expr value stays). |
| `OP_JUMP` | 3 | 2-byte LE unsigned offset | — | `ip += offset`. |
| `OP_JUMP_IF_FALSE` | 3 | 2-byte LE unsigned offset | `value →` | Pop; if 0, `ip += offset`. |
| `OP_LOOP` | 3 | 2-byte LE unsigned offset | — | `ip -= offset` (backward jump). |
| `OP_PRINT` | 1 | — | `value →` | Pop and print to stdout. |
| `OP_INPUT` | 1 | — | `→ value` | Read int from stdin and push. |
| `OP_HALT` | 1 | — | — | Stop execution. |

---

## Constant / name table

To keep the bytecode array byte-clean, identifier names live in a parallel
`std::vector<std::string> names` on the `Chunk`. Global-variable opcodes take
a 1-byte index into this table (max 256 distinct global names — fine for v1).

For v1, integer literals are inlined as 4 bytes directly in the bytecode
(via `OP_CONST_INT`). If we add strings later we'll introduce a real
constant pool.

---

## Compilation patterns

### Binary expression `a + b`
```
[compile a]
[compile b]
OP_ADD
```

### `if (cond) thenBlock else elseBlock`
```
[compile cond]
OP_JUMP_IF_FALSE  <patch1>     ; jump to else if cond false
[compile thenBlock]
OP_JUMP           <patch2>     ; skip else
patch1 →
[compile elseBlock]
patch2 →
```

### `while (cond) body`
```
loopStart →
[compile cond]
OP_JUMP_IF_FALSE  <patchExit>
[compile body]
OP_LOOP           (ip - loopStart)
patchExit →
```

`OP_LOOP` is just `OP_JUMP` with the offset subtracted instead of added —
keeps loop offsets unsigned and the math obvious.

---

## Disassembly format

The `Debug::disassemble(chunk)` helper prints one instruction per line:

```
0000  OP_CONST_INT     10
0005  OP_DEFINE_GLOBAL 0    ; 'n'
0007  OP_GET_GLOBAL    0    ; 'n'
0009  OP_CONST_INT     1
0014  OP_ADD
0015  OP_LT
0016  OP_JUMP_IF_FALSE 0042
...
```

Column 1 = byte offset. Column 2 = mnemonic. Column 3 = operand(s). Trailing
`;` = annotation (e.g. resolved name).
