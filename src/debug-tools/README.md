# DEBUG-TOOLS
The debug-tools implement a mock backend to serve the frontend scanner and parser as isolated executables. Their purpose was to test preliminary features before committing them to the source code. Given a source file, the tools will print relevant scanner/parser metadata.

* [Build](#build)
* [Usage](#usage)
* [Interpreting Results](#interpreting-results)
* [Files](#files)

## Build
The tools can be built by invoking the `debug-scanner` and/or `debug-parser` targets from the Makefile in the <b><ins>root directory</ins></b>. To build the scanner tool:
```
make debug-scanner
```

To build the parser tool:
```
make debug-parser
```

## Usage
To use either tool, call the executable along with a source file:
```
build/debug-scanner.out myfile.rin
```

```
build/debug-parser.out myfile.rin
```

## Interpreting Results
With respect to the following source file, with name `example.rin`:
```c++
float x = 30f
float y = 10f + x
for ; x > y; x++ {
    y = x + 5f
}

// Unclosed parenthesis:
(
```
### Scanner
Running the scanner tool produces:
```
 ---- DEBUG TOOL: SCANNER ----

example.rin:0:5: float keyword 	 float keyword
example.rin:0:7: identifier 	 x
example.rin:0:9: '=' operator 	 '=' operator
example.rin:0:13: float literal 	 30f
example.rin:1:0: EOL 	 EOL
example.rin:1:5: float keyword 	 float keyword
example.rin:1:7: identifier 	 y
example.rin:1:9: '=' operator 	 '=' operator
example.rin:1:13: float literal 	 10f
example.rin:1:15: '+' operator 	 '+' operator
example.rin:1:17: identifier 	 x
example.rin:2:0: EOL 	 EOL
example.rin:2:3: for keyword 	 for keyword
example.rin:2:5: ';' operator 	 ';' operator
...
example.rin:7:1: EOF 	 EOF


SCANNER ERRORS:
[DEBUG ERROR] example.rin:7:1: Unmatched parenthesis operator.

 ---- END DEBUG TOOL ----

```

Each line specifies the file, line, and column of each rinto token in the following format: `FILE:LINE:COLUMN` - with lines and columns both starting at 0 (this is not the case in the GCC implementation). Then, the program prints the token's classification, a tab character, and then the token's string value (usually as it appears on file).

Because some scanner errors require that the scanner finishes reading the entire source file, they are only printed at the end of the scanner tools output. In the example above, the tool issues an `Unmatched parenthesis operator` error.

### Parser
Similarly, running the parser tool produces:
```
---- DEBUG TOOL: PARSER ----

[DEBUG INFORM] example.rin:0:7: CREATED VAR OBJECT: 'x'
[DEBUG INFORM] example.rin:0:7: CREATED VARIABLE DECLARATION STMT FOR VAR: x

[DEBUG INFORM] example.rin:0:7: CREATED VAR OBJECT: 'x'
[DEBUG INFORM] example.rin:0:7: CREATED VAR REF WITH IDENT: x
[DEBUG INFORM] example.rin:0:13: CREATED FLOAT EXPRESSION WITH VAL: 3000000000000000
[DEBUG INFORM] example.rin:0:7: CREATED ASSIGNMENT STATEMENT

[DEBUG INFORM] example.rin:0:7: CREATED COMPOUND STATEMENT

...

[DEBUG ERROR] example.rin:7:1: Unmatched parenthesis operator.

 ---- END DEBUG TOOL ----
```

Statements with separate scopes (such as a for loop statement) will process each statement inside their scope first. In the case of a for-loop statement, this would be the induction statement (i.e the counter variable), followed by every statement within the for-loop's '{}' braces. The result is that constituent statements, for ex:
```
"y = x + 5f"  will print :

[DEBUG INFORM] example.rin:3:5: CREATED VAR OBJECT: 'y'
[DEBUG INFORM] example.rin:3:5: CREATED VAR REF WITH IDENT: y
[DEBUG INFORM] example.rin:3:9: CREATED VAR OBJECT: 'x'
[DEBUG INFORM] example.rin:3:9: CREATED VAR REF WITH IDENT: x
[DEBUG INFORM] example.rin:3:14: CREATED FLOAT EXPRESSION WITH VAL: 5000000000000000
[DEBUG INFORM] example.rin:3:11: CREATED BINARY EXPRESSION WITH: '+' operator
[DEBUG INFORM] example.rin:3:5: CREATED ASSIGNMENT STATEMENT
```

Will print <ins>before</ins> the parent statement:
```
"for ; x > y; x++ {" will print :

[DEBUG INFORM] example.rin:2:7: CREATED VAR OBJECT: 'x'
[DEBUG INFORM] example.rin:2:7: CREATED VAR REF WITH IDENT: x
[DEBUG INFORM] example.rin:2:11: CREATED VAR OBJECT: 'y'
[DEBUG INFORM] example.rin:2:11: CREATED VAR REF WITH IDENT: y
[DEBUG INFORM] example.rin:2:9: CREATED BINARY EXPRESSION WITH: '>' operator
[DEBUG INFORM] example.rin:2:9: CREATED COND EXPRESSION
[DEBUG INFORM] example.rin:2:9: CREATED EXPRESSION STATEMENT

[DEBUG INFORM] example.rin:2:14: CREATED VAR OBJECT: 'x'
[DEBUG INFORM] example.rin:2:14: CREATED VAR REF WITH IDENT: x
[DEBUG INFORM] example.rin:2:16: CREATED UNARY EXPRESSION WITH OP: '++' operator
[DEBUG INFORM] unknown:-1:-1: CREATED INC STMT

[DEBUG INFORM] example.rin:2:3: CREATED FOR STATEMENT
```

## Files
- debug-diagnostic.cc : implements `frontend/diagnostic.hpp`, which defines how to output errors, fatal errors, warnings, and meta information to the user.
- parser.cc : the debug-parser executable and a minimal/mock backend.
- rin-system.hpp : defines `BE_UNREACHABLE` and `BE_ASSERT` macros, which are required by the frontend.
- scanner.cc : the debug-scanner executable (backend not needed in this case).
