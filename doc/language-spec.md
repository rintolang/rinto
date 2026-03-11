# Rinto Language Specification

This document provides a formal specification of the Rinto programming
language syntax using BNF-like notation. It covers all currently
implemented features.

## 1. Lexical Elements

### 1.1 Comments

```
line_comment    ::= "//" <any character except newline>* newline
block_comment   ::= "/*" <any character>* "*/"
```

Comments are ignored by the scanner. Line comments extend to end of line.
Block comments may span multiple lines.

### 1.2 Identifiers

```
identifier      ::= letter ( letter | digit | "_" )*
letter          ::= "a"..."z" | "A"..."Z"
digit           ::= "0"..."9"
```

Identifiers are case-sensitive. They must begin with a letter and may
contain letters, digits, and underscores.

### 1.3 Keywords (Reserved Identifiers)

```
keyword         ::= "float" | "int" | "bool" | "string" | "var"
                   | "if" | "else" | "for" | "while"
                   | "fn" | "return"
                   | "break" | "continue"
                   | "switch"
                   | "true" | "false"
```

Keywords cannot be used as identifiers.

### 1.4 Literals

#### Integer Literals

```
integer_literal ::= digit+
hex_literal     ::= "0x" hex_digit+
hex_digit       ::= digit | "a"..."f" | "A"..."F"
```

#### Float Literals

```
float_literal   ::= digit+ "." digit+
                   | digit+ "f"
                   | digit+ "." digit+ "f"
```

The `f` suffix explicitly marks a numeric literal as floating-point.

### 1.5 Operators

#### Arithmetic Operators

| Operator | Name | Precedence |
|----------|------|------------|
| `+`  | Addition       | 4 |
| `-`  | Subtraction    | 4 |
| `*`  | Multiplication | 5 |
| `/`  | Division       | 5 |
| `%`  | Remainder      | 5 |

#### Unary Operators

| Operator | Name |
|----------|------|
| `-` (prefix) | Negation |
| `!`  | Logical NOT |
| `~`  | Bitwise NOT |
| `++` | Increment (postfix) |
| `--` | Decrement (postfix) |

#### Comparison Operators

| Operator | Name | Precedence |
|----------|------|------------|
| `==` | Equal          | 2 |
| `!=` | Not equal      | 2 |
| `<`  | Less than      | 3 |
| `>`  | Greater than   | 3 |
| `<=` | Less or equal  | 3 |
| `>=` | Greater or equal | 3 |

#### Logical Operators

| Operator | Name | Precedence |
|----------|------|------------|
| `&&` | Logical AND | 1 |
| `\|\|` | Logical OR | 0 |

#### Bitwise Operators

| Operator | Name | Precedence |
|----------|------|------------|
| `&`  | Bitwise AND   | 2 |
| `\|` | Bitwise OR    | 0 |
| `^`  | Bitwise XOR   | 1 |
| `<<` | Left shift    | 3 |
| `>>` | Right shift   | 3 |

#### Assignment Operators

| Operator | Name |
|----------|------|
| `=`  | Assignment |
| `+=` | Add-assign |
| `-=` | Subtract-assign |
| `*=` | Multiply-assign |
| `/=` | Divide-assign |

#### Paired Symbols

| Open | Close | Name |
|------|-------|------|
| `(` | `)` | Parentheses |
| `[` | `]` | Brackets |
| `{` | `}` | Braces |

#### Other

| Symbol | Name |
|--------|------|
| `;` | Semicolon (statement terminator, or use newline) |

### 1.6 Statement Terminators

Statements are terminated by either a newline (EOL) or a semicolon (`;`).
Both are equivalent.

## 2. Types

### 2.1 Primitive Types

```
type_specifier  ::= "float" | "int" | "bool" | "string" | "var"
```

| Type | Description |
|------|-------------|
| `float` | Floating-point number (default precision) |
| `int` | Integer |
| `bool` | Boolean (`true` / `false`) |
| `string` | String (reserved, not yet fully implemented) |
| `var` | Type-inferred variable (reserved) |

## 3. Expressions

```
expression      ::= unary_expression
                   | binary_expression
                   | literal_expression
                   | var_reference
                   | call_expression
                   | "(" expression ")"

unary_expression    ::= "-" expression
                      | "!" expression
                      | "~" expression
                      | var_reference "++"
                      | var_reference "--"

binary_expression   ::= expression operator expression

literal_expression  ::= integer_literal
                       | float_literal

var_reference       ::= identifier

call_expression     ::= identifier "(" argument_list? ")"
argument_list       ::= expression ( "," expression )*
```

### 3.1 Operator Precedence

Operators are parsed using the Shunting Yard algorithm. Precedence
(highest to lowest):

| Level | Operators |
|-------|-----------|
| 6 | `++` `--` `~` (unary) `-` (unary negation) |
| 5 | `*` `/` `%` |
| 4 | `+` `-` |
| 3 | `<` `>` `<=` `>=` `<<` `>>` |
| 2 | `==` `!=` `&` |
| 1 | `&&` `^` |
| 0 | `\|\|` |

All binary operators are left-associative.

## 4. Statements

### 4.1 Variable Declaration

```
var_declaration ::= type_specifier identifier
                  | type_specifier identifier "=" expression
```

Examples:

```
float x
float y = 3.14f
int count = 0
```

### 4.2 Assignment

```
assignment      ::= identifier "=" expression
                  | identifier "+=" expression
                  | identifier "-=" expression
                  | identifier "*=" expression
                  | identifier "/=" expression
```

The left-hand side must be a previously declared variable.

Examples:

```
x = 42.0f
x += 1.0f
```

### 4.3 Increment / Decrement

```
inc_dec_stmt    ::= identifier "++"
                  | identifier "--"
```

Examples:

```
x++
count--
```

### 4.4 If Statement

```
if_statement    ::= "if" expression "{" statement_list "}"
                  | "if" expression "{" statement_list "}" "else" "{" statement_list "}"
                  | "if" expression "{" statement_list "}" "else" if_statement
```

The opening brace MUST be on the same line as the condition.
No parentheses are required around the condition.

Examples:

```
if x > 0.0f {
    y = x
}

if x > 0.0f {
    y = x
} else {
    y = 0.0f
}

if x > 10.0f {
    y = 10.0f
} else if x > 5.0f {
    y = 5.0f
} else {
    y = 0.0f
}
```

### 4.5 For Loop

```
for_statement   ::= "for" for_init ";" for_cond ";" for_inc "{" statement_list "}"

for_init        ::= var_declaration | assignment | epsilon
for_cond        ::= expression | epsilon
for_inc         ::= inc_dec_stmt | assignment | epsilon
```

Go-style syntax: no parentheses around the clause, opening brace on
the same line. Semicolons separate the three clauses.

Examples:

```
for float i = 0.0f; i < 10.0f; i++ {
    x += 1.0f
}
```

### 4.6 While Loop

```
while_statement ::= "while" expression "{" statement_list "}"
```

Equivalent to a for-loop with no init or increment.

Examples:

```
while count < 5.0f {
    count++
}
```

### 4.7 Break and Continue

```
break_stmt      ::= "break"
continue_stmt   ::= "continue"
```

`break` exits the innermost loop. `continue` skips to the next iteration.

### 4.8 Function Declaration

```
function_decl   ::= "fn" identifier "(" param_list? ")" "{" statement_list "}"
param_list      ::= identifier ( "," identifier )*
```

Parameters are untyped in the current implementation.

Examples:

```
fn add(a, b) {
    return a + b
}

fn main() {
    float result = 0.0f
}
```

### 4.9 Return Statement

```
return_stmt     ::= "return" expression?
```

Returns a value from the current function. The expression is optional
for void returns.

### 4.10 Function Call

```
call_stmt       ::= identifier "(" argument_list? ")"
argument_list   ::= expression ( "," expression )*
```

Examples:

```
add(x, 2.0f)
```

### 4.11 Expression Statement

Any expression can stand alone as a statement when it has side effects
(e.g., function calls).

## 5. Program Structure

```
program         ::= statement_list
statement_list  ::= ( statement terminator )*
terminator      ::= newline | ";"
```

A Rinto program is a sequence of top-level statements. Currently, all
code runs in an implicit `main()` function when compiled through the
GCC backend.

## 6. Scope Rules

- Variables are lexically scoped.
- Inner scopes (if/for/while/fn bodies) can access variables from
  enclosing scopes.
- Variable lookup walks the parent scope chain until found or the
  supercontext is reached.
- Redefinition of a variable in the same scope is an error.
- Referencing an undefined variable is an error.
