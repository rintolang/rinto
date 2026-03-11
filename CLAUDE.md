# CLAUDE.md - Rinto Language Compiler

## Project Overview

Rinto is an in-development, open-source programming language based on C/C++ that aims to simplify
and modernize C-family features and syntax. It compiles through GCC's infrastructure.

See `.claude/purpose.md` for the full project vision.

---

## Build TOC

### Directory Structure

```
rinto/
├── src/
│   ├── frontend/                 # Language frontend (scanner, parser, AST)
│   │   ├── scanner.hpp/cc        # Lexical analyzer - tokenization
│   │   ├── parser.hpp/cc         # Syntactic analyzer - AST construction
│   │   ├── expressions.hpp/cc    # Expression AST node types
│   │   ├── statements.hpp/cc     # Statement AST node types
│   │   ├── backend.hpp           # Abstract backend interface + Scope/Named_object
│   │   ├── operators.hpp/cc      # Operator enum, precedence, utilities
│   │   ├── diagnostic.hpp/cc     # Error/warning reporting (GCC-style)
│   │   └── file.hpp/cc           # File I/O and location tracking
│   ├── gcc/                      # GCC backend implementation
│   │   ├── gcc-backend.hpp/cc    # Gcc_backend class - tree generation
│   │   ├── rin-system.hpp        # GCC system includes and macros
│   │   ├── rin1.cc               # GCC language hooks (entry point)
│   │   ├── gcc-diagnostics.cc    # Backend diagnostic bridge
│   │   ├── lang-specs.h          # GCC language spec registration
│   │   ├── Make-lang.in          # GCC build integration
│   │   └── config-lang.in        # Language configuration for GCC
│   └── debug-tools/              # Standalone testing executables
│       ├── scanner.cc            # Debug scanner driver
│       ├── parser.cc             # Debug parser driver
│       ├── debug-diagnostic.cc   # Mock diagnostic backend
│       ├── rin-system.hpp        # Mock system definitions (non-GCC)
│       └── README.md             # Debug tools usage
├── doc/                          # Logo assets
├── .github/workflows/c-cpp.yml  # CI: build debug-scanner + debug-parser
├── Makefile                      # Build targets: debug-scanner, debug-parser
├── CONTRIBUTING.md               # Contribution guidelines and code style
├── README.md                     # Project README
└── LICENSE.md                    # GPL 3.0
```

### Compilation Pipeline

```
.rin source → Scanner (tokens) → Parser (AST) → Backend (GCC trees) → GCC → machine code
```

### Build Commands

```bash
make debug-scanner    # Builds build/debug-scanner.out
make debug-parser     # Builds build/debug-parser.out
```

### Dependencies

- **g++** with C++14 support
- **libmpfr-dev** - Arbitrary precision floating-point
- **libgmp-dev** - GNU multiprecision arithmetic (mpfr dependency)

### Key Architectural Patterns

| Pattern | Where | Description |
|---------|-------|-------------|
| Factory methods | `Token::make_*`, `Expression::make_*`, `Statement::make_*` | Static constructors |
| Type classification enums | All AST nodes | `_classification` field + `convert<>()` template |
| Abstract backend | `backend.hpp` | Frontend decoupled from code generation |
| Shunting Yard | `parser.cc:parse_expression()` | Infix → postfix → AST for expressions |
| Scope chain | `backend.hpp:Scope` | Nested scope with parent-chain variable lookup |
| Token buffering | `scanner.cc` | `std::deque` with peek/lookahead support |
| Paired symbol tracking | `scanner.cc:ExpectMatch` | Stack-based bracket/paren/brace matching |

---

## Style Guide

**All code contributions MUST follow these conventions. Do not deviate.**

### Naming

| Element | Convention | Example |
|---------|-----------|---------|
| Local variables | camelCase | `lineCount`, `expectMatches` |
| Function names | camelCase or snake_case matching existing pattern | `scan_token()`, `peek_token()` |
| Class names | PascalCase (underscores allowed for multi-word) | `Scanner`, `Binary_expression`, `Gcc_backend` |
| Private attributes | Underscore prefix | `_classification`, `_location`, `_scanner` |
| Enums | UPPER_CASE with category prefix | `TOKEN_EOF`, `OPER_ADD`, `STATEMENT_IF` |
| Macros | UPPER_CASE | `RIN_ASSERT`, `RIN_UNREACHABLE` |
| Enum type names | UPPER_CASE or PascalCase | `RIN_OPERATOR`, `Classification` |

### Indentation & Formatting

- **Tabs for indentation** (NOT spaces). This is non-negotiable.
- **Opening braces on new line** for classes and structs:
  ```cpp
  class MyClass
  {
  public:
          void myMethod();
  };
  ```
- **Opening braces on same line** for functions, if/for/while:
  ```cpp
  void myFunction() {
          // body
  }
  ```
- **Short methods** may be on a single line:
  ```cpp
  int classification() const
  { return _classification; }
  ```
- **No trailing whitespace**.
- **Single blank line** between method implementations.

### Headers

- Include guards: `#ifndef RIN_FILENAME_HPP` / `#define RIN_FILENAME_HPP` / `#endif`
- Class declarations in `.hpp`, implementations in `.cc`
- Include corresponding `.hpp` first in `.cc` files

### Error Handling

- Use `RIN_ASSERT(condition)` for internal invariants
- Use `RIN_UNREACHABLE()` for unreachable code paths
- Use `rin_error_at(location, format, ...)` for user-facing errors
- Return `*::make_invalid(location)` on parse errors
- Never throw exceptions

### Memory Management

- Manual `new`/`delete` (no smart pointers currently)
- Destructors must recursively delete owned children
- Factory methods (`make_*`) allocate and return via `new`

### Comments

- `//` for single-line comments
- `/* */` for multi-line comments
- Explain non-obvious logic; do not restate what code does
- Mark future work with `// TODO:`

### C++ Standard

- **C++14** (`-std=c++14` or `-std=gnu++14` for GCC backend)
- **Warnings**: `-Wall` must compile clean
- Do not use features from C++17 or later

---

## Self-Learning Log

> When working on this codebase, append new findings here that are not obvious from reading
> the code. This section is a living document.

### Learnings

1. **For-loop syntax** uses Go-style: `for init; cond; inc { body }` — no parentheses around
   the clause, braces required, and the opening brace MUST be on the same line as the condition.

2. **If-statement syntax** similarly requires the opening brace on the same line as the condition.

3. **Float suffixes**: The scanner recognizes `f` suffix on numeric literals (e.g., `3.14f`).

4. **Expression parsing** uses a two-phase approach: Shunting Yard algorithm produces
   `Expression_node` trees (internal to parser), then `__parse_ast_node()` converts them
   to the public `Expression` AST types.

5. **The debug-tools mock backend** (`src/debug-tools/rin-system.hpp`) provides stub
   implementations of `Bexpression`, `Bstatement`, `Bvariable` so the frontend can be tested
   without GCC. The macros `BE_ASSERT` and `BE_UNREACHABLE` map to the non-GCC diagnostic path.

6. **MPFR precision** is set to 256 bits in `rin_langhook_init()` (rin1.cc).

7. **Location is 0-based** in the File class but converted to 1-based for GCC integration.

8. **Scanner buffer** uses `std::deque<Token>` for arbitrary lookahead via `peek_nth_token(n)`.

9. **Scope variable lookup** walks the parent chain — `Scope::lookup()` checks the current
   `ident_map` then recurses to `_parent`.

10. **GCC backend variable caching**: `Gcc_backend` maintains `_var_map` mapping `Named_object*`
    to `Bvariable*` so the same variable isn't recreated.

11. **Else/else-if syntax**: `if cond { } else { }` and `if cond { } else if cond2 { } else { }`.
    The else keyword is parsed immediately after the closing `}` of the if block.

12. **While-loop** reuses `For_statement` with NULL induction and NULL increment. No new AST or
    backend node types are needed.

13. **Unary negation** uses internal `OPER_NEG` operator (not scanned) to distinguish from binary
    `OPER_SUB`. Detection happens in the Shunting Yard algorithm when `-` appears at expression
    start or after another operator.

14. **Compound assignments** (`+=`, `-=`, `*=`, `/=`) are desugared in the parser to
    `x = x op rhs`, requiring no backend changes.

15. **Windows line endings**: The repository uses CRLF. Git's `autocrlf` handles conversion, but
    tools like `sed` may produce LF-only files. Git warnings about CRLF replacement are expected.

---

## Mixture-of-Experts Code Review Panel

Before finalizing any non-trivial code change, present it to each of the following 10 experts.
Apply their feedback before committing.

### Expert 1: Security Engineer

**Name**: SecureGuard
**Focus**: Vulnerability analysis and safe coding practices

**Review Prompt**:
> You are a Security Engineer reviewing C++ compiler code. Examine the code for:
> - Buffer overflows, out-of-bounds access, and use-after-free
> - Unsafe string operations (sprintf, strcpy — prefer snprintf, strncpy)
> - Integer overflow/underflow in size calculations
> - Null pointer dereferences (especially from failed allocations or `convert<>()` returning NULL)
> - Format string vulnerabilities in diagnostic functions
> - Input validation on source file content (malformed UTF-8, extremely long lines, binary data)
> - Memory leaks from error paths that skip cleanup
> - Stack overflow potential from deep recursion (deeply nested expressions/scopes)
>
> Flag anything that could crash the compiler or be exploited by malicious input files.

### Expert 2: Compiler Architect

**Name**: CompilerSage
**Focus**: Compiler design correctness and theory

**Review Prompt**:
> You are a Compiler Architect with deep knowledge of language implementation. Review for:
> - Correctness of the lexer → parser → AST → codegen pipeline
> - Proper operator precedence and associativity handling
> - Sound scope semantics (variable shadowing, lifetime, lookup correctness)
> - AST node completeness (can every valid program be represented?)
> - Backend abstraction leaks (does frontend code depend on GCC specifics?)
> - Semantic analysis gaps (type checking, undeclared variables, redeclaration)
> - Edge cases in the Shunting Yard algorithm (empty expressions, single operand)
> - Correct GCC tree construction (proper tree codes, types, locations)
>
> Ensure the compiler is theoretically sound and handles all edge cases.

### Expert 3: Performance Engineer

**Name**: PerfHawk
**Focus**: Runtime and compile-time performance

**Review Prompt**:
> You are a Performance Engineer. Review the compiler code for:
> - Unnecessary allocations (can stack allocation replace heap?)
> - Redundant copies (pass by const reference where possible)
> - Algorithmic complexity (O(n^2) lookups in hot paths, linear scope chain walks)
> - Scanner efficiency (unnecessary re-scanning, buffer management)
> - Parser backtracking or re-parsing overhead
> - MPFR operation efficiency (initialization, cleanup, precision settings)
> - Backend tree construction overhead (redundant fold_build calls)
> - Memory allocation patterns (frequent small allocations vs. arena allocation)
>
> Focus on patterns that would degrade compile times for large source files.

### Expert 4: C++ Standards Expert

**Name**: CppGuru
**Focus**: Modern C++ idioms, standards compliance, and best practices

**Review Prompt**:
> You are a C++ Standards Expert. Review for:
> - C++14 compliance (no C++17/20 features)
> - Correct use of `const`, `noexcept`, `override`, `final` where appropriate
> - Proper rule-of-three/five (copy/move constructors, assignment, destructor)
> - Undefined behavior (signed overflow, uninitialized reads, aliasing violations)
> - Correct use of unions (active member tracking, no UB access)
> - Template correctness and safety
> - Header include hygiene (minimal includes, forward declarations)
> - Enum class vs. plain enum consistency
>
> The codebase uses manual memory management — verify delete/free pairs are correct.

### Expert 5: Technical Lead

**Name**: TechLead
**Focus**: Code quality, maintainability, and architecture

**Review Prompt**:
> You are a Technical Lead responsible for long-term codebase health. Review for:
> - Separation of concerns (is each file/class doing one thing well?)
> - Consistent naming conventions (per the style guide above)
> - Code duplication that should be extracted
> - Dead code or unused parameters
> - Appropriate abstraction level (not too much, not too little)
> - Clear control flow (no deeply nested conditionals, no goto)
> - Error handling consistency (same pattern everywhere)
> - File organization (right file for the right code)
>
> Would a new contributor understand this code? Could it be maintained for years?

### Expert 6: QA / Test Engineer

**Name**: TestMaster
**Focus**: Testability and coverage

**Review Prompt**:
> You are a QA Engineer. Review for:
> - Can this code be tested in isolation? Are dependencies injectable?
> - What test cases would exercise this code? List edge cases.
> - Are error paths tested? What happens with malformed input?
> - Does the debug-tools mock backend need updating for this change?
> - Are there regression risks? What existing behavior might break?
> - Can the CI pipeline catch failures from this change?
> - Are diagnostic messages clear enough for users to understand errors?
>
> Propose specific test cases that should exist before this code is merged.

### Expert 7: GCC Integration Specialist

**Name**: GccExpert
**Focus**: Correct GCC plugin/frontend integration

**Review Prompt**:
> You are a GCC Integration Specialist. Review for:
> - Correct use of GCC tree API (tree codes, types, attributes)
> - Proper location_t handling (linemap integration)
> - Memory management with GCC's garbage collector (ggc_alloc vs malloc)
> - Correct language hook implementation
> - Compatibility with GCC versions (API stability)
> - Proper use of DECL, EXPR, and STMT tree nodes
> - build_decl / build1_loc / build2_loc correctness
> - MPFR to REAL_VALUE_TYPE conversion accuracy
>
> Ensure GCC integration follows the patterns established in other GCC frontends (Go, D, Rust).

### Expert 8: Product Manager

**Name**: ProductOwner
**Focus**: User-facing behavior and language design

**Review Prompt**:
> You are a Product Manager for the Rinto language. Review for:
> - Does this change make the language more intuitive for C/C++ developers?
> - Are error messages clear, actionable, and helpful?
> - Does the syntax choice align with "simplify and modernize C-family"?
> - Is there a migration path for users when syntax changes?
> - Does this feature have parity with what users expect from the website/docs?
> - Are there naming conflicts with C/C++ keywords that could confuse users?
> - Is the feature complete enough to ship, or is it a footgun in half-finished state?
>
> The goal is a language that feels familiar but cleaner. Does this change serve that?

### Expert 9: DevOps / Build Engineer

**Name**: BuildMaster
**Focus**: Build system, CI/CD, and deployment

**Review Prompt**:
> You are a Build Engineer. Review for:
> - Does the Makefile correctly compile all new/changed files?
> - Are new dependencies declared (libraries, headers, tools)?
> - Does the CI pipeline (`.github/workflows/c-cpp.yml`) cover this change?
> - Are there new build warnings with `-Wall`?
> - Is the GCC backend build (`Make-lang.in`) updated if needed?
> - Are include paths correct for both debug-tools and GCC builds?
> - Would this change break cross-platform compilation?
> - Are build artifacts properly gitignored?
>
> The build must be reproducible, fast, and catch regressions.

### Expert 10: End User / Language Learner

**Name**: NewUser
**Focus**: First-time experience and usability

**Review Prompt**:
> You are a developer trying Rinto for the first time, coming from C/C++. Review for:
> - Is the syntax intuitive? Could you guess what it does without docs?
> - Are error messages understandable without knowledge of compiler internals?
> - Does the feature work as you'd expect from C-family experience?
> - Are there surprising behaviors or gotchas?
> - If you wrote this code wrong, would the compiler help you fix it?
> - Is the feature documented or at least self-explanatory?
> - Would you actually use this feature in a real program?
>
> Report confusion, surprise, and friction from a newcomer's perspective.

### How to Apply Expert Reviews

For every non-trivial code change:

1. **Present** the diff or new code to each expert (mentally or in writing)
2. **Collect** feedback — flag any CRITICAL or HIGH issues
3. **Resolve** all CRITICAL issues before proceeding
4. **Document** any accepted trade-offs (HIGH issues deferred with justification)
5. **Verify** the fix doesn't introduce new issues flagged by other experts

---

## Task Management

Tasks are tracked in `.claude/tasks/tasks.md` using `X.Y.Z` numbering.

When a task is completed:
1. Mark it `[x]` in `tasks.md`
2. Create `.claude/tasks/completed/X.Y.Z-COMPLETED.md` documenting:
   - What was done
   - Files modified
   - Key decisions made
   - Any follow-up work identified
