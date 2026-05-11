# CVM++ Grammar

Formal grammar for the CVM++ language, in EBNF-style notation.
`*` = zero or more, `?` = optional, `|` = alternation.

---

## Program

```
program     → statement* EOF
```

## Statements

```
statement   → letStmt
            | assignStmt
            | printStmt
            | inputStmt
            | ifStmt
            | whileStmt
            | block
            | exprStmt

letStmt     → "let" IDENT "=" expression ";"
assignStmt  → IDENT "=" expression ";"
printStmt   → "print" expression ";"
inputStmt   → "input" IDENT ";"
ifStmt      → "if" "(" expression ")" block ( "else" block )?
whileStmt   → "while" "(" expression ")" block
block       → "{" statement* "}"
exprStmt    → expression ";"
```

## Expressions (lowest → highest precedence)

```
expression  → equality
equality    → comparison ( "==" comparison )*
comparison  → term       ( "<"  term       )*
term        → factor     ( ("+" | "-") factor )*
factor      → unary      ( ("*" | "/") unary  )*
unary       → ( "-" ) unary | primary
primary     → NUMBER
            | "true" | "false"
            | IDENT
            | "(" expression ")"
```

## Lexical tokens

| Token | Pattern |
|---|---|
| `NUMBER` | `[0-9]+` |
| `IDENT` | `[a-zA-Z_][a-zA-Z0-9_]*` (excluding keywords) |
| Keywords | `let`, `if`, `else`, `while`, `print`, `input`, `true`, `false` |
| Operators | `+`, `-`, `*`, `/`, `==`, `=`, `<` |
| Punctuation | `(`, `)`, `{`, `}`, `;` |
| Comments | `// ...` to end of line |
| Whitespace | ignored |

---

## Example programs

**Sum 1..10:**
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

**Guess the number:**
```
let secret = 42;
let guess = 0;
input guess;
if (guess == secret) {
    print 1;
} else {
    print 0;
}
```
