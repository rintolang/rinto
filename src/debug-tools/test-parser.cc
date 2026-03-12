// test-parser.cc - Comprehensive unit tests for the Rinto parser
#include <backend.hpp>
#include <parser.hpp>
#include <fstream>
#include <cstdlib>

class Bexpression {};
class Bstatement  {};

class Bvariable
{
public:
	std::string identifier()
	{ return this->_identifier; }

	Location location()
	{ return this->_location; }

	void set_identifier(const std::string& ident)
	{ this->_identifier = ident; }

	void set_location(Location loc)
	{ this->_location = loc; }

	bool has_identifier()
	{ return (this->_identifier != ""); }

private:
	std::string _identifier = "";
	Location    _location   = File::unknown_location();
};

void delete_stmt(Bstatement* stmt)
{ delete stmt; }

/*
 * Silent mock backend. Tracks whether errors occurred
 * via invalid_expression() / invalid_statement() calls.
 */
class Test_backend : public Backend
{
public:
	Test_backend() : _had_error(false) {}
	bool had_error() const { return _had_error; }
	void reset_error() { _had_error = false; }

	Bvariable* variable(Named_object* obj) {
		Bvariable* var = new Bvariable;
		if (obj) { var->set_identifier(obj->identifier()); var->set_location(obj->location()); }
		return var;
	}

	Bexpression* invalid_expression()    { _had_error = true; return new Bexpression; }
	Bstatement*  invalid_statement()     { _had_error = true; return new Bstatement; }

	Bexpression* var_reference(Bvariable* v, Location)             { delete v; return new Bexpression; }
	Bexpression* conditional_expression(Bexpression* c, Location)  { delete c; return new Bexpression; }
	Bexpression* unary_expression(RIN_OPERATOR, Bexpression* e, Location) { delete e; return new Bexpression; }
	Bexpression* float_expression(const mpfr_t*, Location)         { return new Bexpression; }
	Bexpression* integer_expression(const mpfr_t*, Location)       { return new Bexpression; }
	Bexpression* binary_expression(RIN_OPERATOR, Bexpression* l, Bexpression* r, Location) { delete l; delete r; return new Bexpression; }
	Bexpression* call_expression(const std::string&, const std::vector<Bexpression*>& a, Location) {
		for (auto i = a.begin(); i != a.end(); ++i) delete *i;
		return new Bexpression;
	}

	Bstatement* var_dec_statement(Bvariable* v)              { delete v; return new Bstatement; }
	Bstatement* inc_statement(Bexpression* e, Location)      { delete e; return new Bstatement; }
	Bstatement* dec_statement(Bexpression* e, Location)      { delete e; return new Bstatement; }
	Bstatement* expression_statement(Bexpression* e, Location) { delete e; return new Bstatement; }
	Bstatement* compound_statement(Bstatement* a, Bstatement* b, Location) { delete a; delete b; return new Bstatement; }
	Bstatement* assignment_statement(Bexpression* l, Bexpression* r, Location) { delete l; delete r; return new Bstatement; }
	Bstatement* return_statement(Bexpression* e, Location)   { delete e; return new Bstatement; }
	Bstatement* break_statement(Location)    { return new Bstatement; }
	Bstatement* continue_statement(Location) { return new Bstatement; }

	// Scopes are owned by their statements; do NOT delete here.
	Bstatement* if_statement(Bexpression* e, Scope*, Scope*, Location) { delete e; return new Bstatement; }
	Bstatement* for_statement(Bstatement* a, Bstatement* b, Bstatement* c, Scope*, Location) { delete a; delete b; delete c; return new Bstatement; }
	Bstatement* function_statement(const std::string&, const std::vector<std::string>&, Scope*, Location) { return new Bstatement; }

private:
	bool _had_error;
};

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static const char* TEMP_FILE = "rin_test_parser_tmp.rin";

static std::string write_temp(const std::string& content) {
	std::ofstream out(TEMP_FILE, std::ios::trunc | std::ios::binary);
	out << content;
	out.flush();
	out.close();
	return std::string(TEMP_FILE);
}
static void cleanup() { std::remove(TEMP_FILE); }

#define BEGIN_TEST(name) \
	tests_run++; \
	printf("  [%02d] %-55s ", tests_run, name); \
	fflush(stdout);

#define PASS() do { tests_passed++; printf("PASS\n"); return; } while(0)
#define FAIL(msg) do { tests_failed++; printf("FAIL: %s\n", msg); return; } while(0)

/* Helper: parse content and return whether errors occurred */
static bool parses_ok(const std::string& content) {
	std::string path = write_temp(content);
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
	return !be->had_error();
}

// ==== VARIABLE DECLARATION TESTS ====

static void test_float_decl() {
	BEGIN_TEST("float x");
	if (!parses_ok("float x\n")) FAIL("parse error");
	PASS();
}

static void test_float_decl_with_init() {
	BEGIN_TEST("float x = 3.14f");
	if (!parses_ok("float x = 3.14f\n")) FAIL("parse error");
	PASS();
}

static void test_float_decl_with_expr() {
	BEGIN_TEST("float x = 1.0f + 2.0f");
	if (!parses_ok("float x = 1.0f + 2.0f\n")) FAIL("parse error");
	PASS();
}

static void test_int_decl() {
	BEGIN_TEST("int y");
	if (!parses_ok("int y\n")) FAIL("parse error");
	PASS();
}

static void test_int_decl_with_init() {
	BEGIN_TEST("int y = 42");
	if (!parses_ok("int y = 42\n")) FAIL("parse error");
	PASS();
}

static void test_var_decl() {
	BEGIN_TEST("var z = 1.0f");
	if (!parses_ok("var z = 1.0f\n")) FAIL("parse error");
	PASS();
}

static void test_multiple_decls() {
	BEGIN_TEST("Multiple variable declarations");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat c = 3.0f\n")) FAIL("parse error");
	PASS();
}

// ==== ASSIGNMENT TESTS ====

static void test_simple_assignment() {
	BEGIN_TEST("x = 5.0f");
	if (!parses_ok("float x\nx = 5.0f\n")) FAIL("parse error");
	PASS();
}

static void test_assignment_with_expr() {
	BEGIN_TEST("x = a + b");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat x\nx = a + b\n")) FAIL("parse error");
	PASS();
}

static void test_compound_add_assign() {
	BEGIN_TEST("x += 1.0f");
	if (!parses_ok("float x = 0.0f\nx += 1.0f\n")) FAIL("parse error");
	PASS();
}

static void test_compound_sub_assign() {
	BEGIN_TEST("x -= 1.0f");
	if (!parses_ok("float x = 5.0f\nx -= 1.0f\n")) FAIL("parse error");
	PASS();
}

static void test_compound_mul_assign() {
	BEGIN_TEST("x *= 2.0f");
	if (!parses_ok("float x = 3.0f\nx *= 2.0f\n")) FAIL("parse error");
	PASS();
}

static void test_compound_quo_assign() {
	BEGIN_TEST("x /= 2.0f");
	if (!parses_ok("float x = 6.0f\nx /= 2.0f\n")) FAIL("parse error");
	PASS();
}

// ==== INCREMENT / DECREMENT TESTS ====

static void test_increment() {
	BEGIN_TEST("x++");
	if (!parses_ok("float x = 0.0f\nx++\n")) FAIL("parse error");
	PASS();
}

static void test_decrement() {
	BEGIN_TEST("x--");
	if (!parses_ok("float x = 5.0f\nx--\n")) FAIL("parse error");
	PASS();
}

// ==== EXPRESSION TESTS ====

static void test_binary_add() {
	BEGIN_TEST("Binary expression: a + b");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat c = a + b\n")) FAIL("parse error");
	PASS();
}

static void test_binary_chain() {
	BEGIN_TEST("Chained binary: a + b * c - d");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat c = 3.0f\nfloat d = 4.0f\nfloat r = a + b * c - d\n")) FAIL("parse error");
	PASS();
}

static void test_parenthesized_expr() {
	BEGIN_TEST("Parenthesized: (a + b) * c");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat c = 3.0f\nfloat r = (a + b) * c\n")) FAIL("parse error");
	PASS();
}

static void test_nested_parens() {
	BEGIN_TEST("Nested parens: ((a + b))");
	if (!parses_ok("float a = 1.0f\nfloat b = 2.0f\nfloat r = ((a + b))\n")) FAIL("parse error");
	PASS();
}

static void test_unary_minus() {
	BEGIN_TEST("Unary minus: -x");
	if (!parses_ok("float x = 5.0f\nfloat y = -x\n")) FAIL("parse error");
	PASS();
}

static void test_unary_not() {
	BEGIN_TEST("Unary not: !x");
	if (!parses_ok("float x = 1.0f\nfloat y = !x\n")) FAIL("parse error");
	PASS();
}

static void test_comparison_operators() {
	BEGIN_TEST("Comparison: <, >, <=, >=, ==, !=");
	if (!parses_ok(
		"float a = 1.0f\nfloat b = 2.0f\n"
		"float r1 = a < b\nfloat r2 = a > b\n"
		"float r3 = a <= b\nfloat r4 = a >= b\n"
		"float r5 = a == b\nfloat r6 = a != b\n"
	)) FAIL("parse error");
	PASS();
}

static void test_logical_operators() {
	BEGIN_TEST("Logical: &&, ||");
	if (!parses_ok(
		"float a = 1.0f\nfloat b = 0.0f\n"
		"float r1 = a && b\nfloat r2 = a || b\n"
	)) FAIL("parse error");
	PASS();
}

static void test_arithmetic_all() {
	BEGIN_TEST("Arithmetic: +, -, *, /, %%");
	if (!parses_ok(
		"float a = 10.0f\nfloat b = 3.0f\n"
		"float r1 = a + b\nfloat r2 = a - b\n"
		"float r3 = a * b\nfloat r4 = a / b\n"
		"float r5 = a % b\n"
	)) FAIL("parse error");
	PASS();
}

// ==== IF / ELSE TESTS ====

static void test_if_simple() {
	BEGIN_TEST("if x > 0 { ... }");
	if (!parses_ok("float x = 1.0f\nif x > 0.0f {\nfloat y = x\n}\n")) FAIL("parse error");
	PASS();
}

static void test_if_else() {
	BEGIN_TEST("if ... { } else { }");
	if (!parses_ok(
		"float x = 1.0f\n"
		"if x > 0.0f {\nfloat y = x\n} else {\nfloat y = 0.0f\n}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_if_else_if_else() {
	BEGIN_TEST("if ... { } else if ... { } else { }");
	if (!parses_ok(
		"float x = 5.0f\n"
		"if x > 10.0f {\nfloat y = 1.0f\n"
		"} else if x > 0.0f {\nfloat y = 2.0f\n"
		"} else {\nfloat y = 3.0f\n}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_if_empty_body() {
	BEGIN_TEST("if x > 0 { } (empty body)");
	if (!parses_ok("float x = 1.0f\nif x > 0.0f {\n}\n")) FAIL("parse error");
	PASS();
}

// ==== FOR LOOP TESTS ====

static void test_for_loop_full() {
	BEGIN_TEST("for float i = 0; i < 10; i++ { ... }");
	if (!parses_ok(
		"for float i = 0.0f; i < 10.0f; i++ {\nfloat x = i\n}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_for_loop_dec() {
	BEGIN_TEST("for with decrement: i--");
	if (!parses_ok(
		"for float i = 10.0f; i > 0.0f; i-- {\nfloat x = i\n}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_for_loop_empty_body() {
	BEGIN_TEST("for ... { } (empty body)");
	if (!parses_ok("for float i = 0.0f; i < 10.0f; i++ {\n}\n")) FAIL("parse error");
	PASS();
}

// ==== WHILE LOOP TESTS ====

static void test_while_loop() {
	BEGIN_TEST("while x < 10 { x++ }");
	if (!parses_ok("float x = 0.0f\nwhile x < 10.0f {\nx++\n}\n")) FAIL("parse error");
	PASS();
}

static void test_while_empty_body() {
	BEGIN_TEST("while true { } (empty body)");
	if (!parses_ok("float x = 1.0f\nwhile x {\n}\n")) FAIL("parse error");
	PASS();
}

// ==== BREAK / CONTINUE TESTS ====

static void test_break_in_loop() {
	BEGIN_TEST("break inside for loop");
	if (!parses_ok("for float i = 0.0f; i < 10.0f; i++ {\nbreak\n}\n")) FAIL("parse error");
	PASS();
}

static void test_continue_in_loop() {
	BEGIN_TEST("continue inside while loop");
	if (!parses_ok("float x = 0.0f\nwhile x < 10.0f {\nx++\ncontinue\n}\n")) FAIL("parse error");
	PASS();
}

// ==== FUNCTION TESTS ====

static void test_fn_no_params() {
	BEGIN_TEST("fn noParams() { ... }");
	if (!parses_ok("fn noParams() {\nfloat x = 1.0f\n}\n")) FAIL("parse error");
	PASS();
}

static void test_fn_one_param() {
	BEGIN_TEST("fn oneParam(a) { ... }");
	if (!parses_ok("fn oneParam(a) {\nfloat x = a\n}\n")) FAIL("parse error");
	PASS();
}

static void test_fn_two_params() {
	BEGIN_TEST("fn twoParams(a, b) { ... }");
	if (!parses_ok("fn twoParams(a, b) {\nfloat x = a + b\n}\n")) FAIL("parse error");
	PASS();
}

static void test_fn_with_return() {
	BEGIN_TEST("fn with return statement");
	if (!parses_ok("fn add(a, b) {\nreturn a + b\n}\n")) FAIL("parse error");
	PASS();
}

static void test_fn_void_return() {
	BEGIN_TEST("fn with void return");
	if (!parses_ok("fn doNothing() {\nreturn\n}\n")) FAIL("parse error");
	PASS();
}

static void test_fn_call_no_args() {
	BEGIN_TEST("Function call: foo()");
	if (!parses_ok("fn foo() {\nfloat x = 1.0f\n}\nfoo()\n")) FAIL("parse error");
	PASS();
}

static void test_fn_call_with_args() {
	BEGIN_TEST("Function call: add(1.0f, 2.0f)");
	if (!parses_ok("fn add(a, b) {\nreturn a + b\n}\nadd(1.0f, 2.0f)\n")) FAIL("parse error");
	PASS();
}

// ==== BRACE PLACEMENT TESTS ====

static void test_brace_next_line_if() {
	BEGIN_TEST("Brace on next line for if");
	if (!parses_ok("float x = 1.0f\nif x > 0.0f\n{\nfloat y = x\n}\n")) FAIL("parse error");
	PASS();
}

static void test_brace_next_line_for() {
	BEGIN_TEST("Brace on next line for for-loop");
	if (!parses_ok("for float i = 0.0f; i < 10.0f; i++\n{\nfloat x = i\n}\n")) FAIL("parse error");
	PASS();
}

static void test_brace_next_line_while() {
	BEGIN_TEST("Brace on next line for while");
	if (!parses_ok("float x = 0.0f\nwhile x < 10.0f\n{\nx++\n}\n")) FAIL("parse error");
	PASS();
}

// ==== NESTED SCOPE TESTS ====

static void test_nested_if_in_for() {
	BEGIN_TEST("Nested if inside for loop");
	if (!parses_ok(
		"for float i = 0.0f; i < 10.0f; i++ {\n"
		"if i > 5.0f {\nfloat x = i\n}\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_nested_for_in_while() {
	BEGIN_TEST("Nested for inside while");
	if (!parses_ok(
		"float n = 0.0f\n"
		"while n < 3.0f {\n"
		"for float i = 0.0f; i < 5.0f; i++ {\nfloat x = i\n}\n"
		"n++\n}\n"
	)) FAIL("parse error");
	PASS();
}

// ==== SEMICOLON / EOL TESTS ====

static void test_semicolon_separator() {
	BEGIN_TEST("Semicolon as statement separator");
	if (!parses_ok("float x = 1.0f; float y = 2.0f\n")) FAIL("parse error");
	PASS();
}

static void test_eol_as_separator() {
	BEGIN_TEST("EOL as statement separator");
	if (!parses_ok("float x = 1.0f\nfloat y = 2.0f\n")) FAIL("parse error");
	PASS();
}

// ==== CRLF TESTS ====

static void test_crlf_basic() {
	BEGIN_TEST("CRLF: basic variable declaration");
	if (!parses_ok("float x = 1.0f\r\n")) FAIL("parse error");
	PASS();
}

static void test_crlf_if_else() {
	BEGIN_TEST("CRLF: if/else block");
	if (!parses_ok(
		"float x = 1.0f\r\n"
		"if x > 0.0f {\r\n"
		"float y = x\r\n"
		"} else {\r\n"
		"float y = 0.0f\r\n"
		"}\r\n"
	)) FAIL("parse error");
	PASS();
}

static void test_crlf_for_loop() {
	BEGIN_TEST("CRLF: for loop");
	if (!parses_ok(
		"for float i = 0.0f; i < 10.0f; i++ {\r\n"
		"float x = i\r\n"
		"}\r\n"
	)) FAIL("parse error");
	PASS();
}

// ==== COMPLEX / INTEGRATION TESTS ====

static void test_complex_program() {
	BEGIN_TEST("Complex: multi-statement program");
	if (!parses_ok(
		"float x = 10.0f\n"
		"float y = 20.0f\n"
		"float sum = x + y\n"
		"if sum > 25.0f {\n"
		"  float diff = y - x\n"
		"  diff += 1.0f\n"
		"}\n"
		"for float i = 0.0f; i < 5.0f; i++ {\n"
		"  x += 1.0f\n"
		"}\n"
		"while y > 0.0f {\n"
		"  y -= 1.0f\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_fn_with_body() {
	BEGIN_TEST("Complex: function with body statements");
	if (!parses_ok(
		"fn compute(a, b) {\n"
		"  float c = a + b\n"
		"  c *= 2.0f\n"
		"  return c\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

// ==== EDGE-CASE / REGRESSION TESTS ====

/* Helper: parse content and verify it doesn't crash (returns true always) */
static bool parses_without_crash(const std::string& content) {
	std::string path = write_temp(content);
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
	// We don't care about errors, just that it didn't crash/hang
	return true;
}

static void test_empty_program() {
	BEGIN_TEST("Empty program parses OK");
	if (!parses_without_crash("")) FAIL("crashed");
	PASS();
}

static void test_comment_only_program() {
	BEGIN_TEST("Comment-only program parses OK");
	if (!parses_without_crash("// hello\n")) FAIL("crashed");
	PASS();
}

static void test_undefined_variable_no_crash() {
	BEGIN_TEST("Undefined variable: no crash");
	if (!parses_without_crash("x = 5.0f\n")) FAIL("crashed");
	PASS();
}

static void test_redefinition_no_crash() {
	BEGIN_TEST("Redefinition: no crash");
	if (!parses_without_crash("float x\nfloat x\n")) FAIL("crashed");
	PASS();
}

static void test_shadowing_nested_scope_no_crash() {
	BEGIN_TEST("Variable shadowing in nested scope: no crash");
	if (!parses_without_crash(
		"float x = 1.0f\n"
		"if x > 0.0f {\n"
		"float x = 2.0f\n"
		"}\n"
	)) FAIL("crashed");
	PASS();
}

static void test_multiple_functions() {
	BEGIN_TEST("Multiple function declarations");
	if (!parses_ok(
		"fn first() {\n"
		"float a = 1.0f\n"
		"}\n"
		"fn second() {\n"
		"float b = 2.0f\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_fn_three_params() {
	BEGIN_TEST("Function with 3 params");
	if (!parses_ok(
		"fn f(a, b, c) {\n"
		"float x = a + b + c\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_while_with_break() {
	BEGIN_TEST("While with break inside");
	if (!parses_ok(
		"float x = 0.0f\n"
		"while x < 10.0f {\n"
		"break\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_for_with_continue() {
	BEGIN_TEST("For with continue inside");
	if (!parses_ok(
		"for float i = 0.0f; i < 10.0f; i++ {\n"
		"continue\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_deeply_nested_blocks() {
	BEGIN_TEST("Deeply nested: if in for in while");
	if (!parses_ok(
		"float x = 0.0f\n"
		"while x < 10.0f {\n"
		"for float i = 0.0f; i < 5.0f; i++ {\n"
		"if i > 2.0f {\n"
		"float y = i\n"
		"}\n"
		"}\n"
		"x++\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_integer_literal_in_assign() {
	BEGIN_TEST("Integer literal in assignment: float x = 42");
	if (!parses_ok("float x = 42\n")) FAIL("parse error");
	PASS();
}

static void test_var_reference_in_assign() {
	BEGIN_TEST("Variable reference: float b = a");
	if (!parses_ok("float a = 1.0f\nfloat b = a\n")) FAIL("parse error");
	PASS();
}

static void test_negative_literal_in_assign() {
	BEGIN_TEST("Negative literal: float x = -1.0f");
	if (!parses_ok("float x = -1.0f\n")) FAIL("parse error");
	PASS();
}

static void test_multiple_else_if_chains() {
	BEGIN_TEST("Multiple else-if chains");
	if (!parses_ok(
		"float x = 5.0f\n"
		"if x > 10.0f {\n"
		"float y = 1.0f\n"
		"} else if x > 5.0f {\n"
		"float y = 2.0f\n"
		"} else if x > 0.0f {\n"
		"float y = 3.0f\n"
		"} else {\n"
		"float y = 4.0f\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_mixed_int_and_float() {
	BEGIN_TEST("Mixed int and float declarations");
	if (!parses_ok("int a = 1\nfloat b = 2.0f\n")) FAIL("parse error");
	PASS();
}

static void test_semicolons_multiple_on_line() {
	BEGIN_TEST("Semicolon-separated: float a; float b; float c");
	if (!parses_ok("float a; float b; float c\n")) FAIL("parse error");
	PASS();
}

static void test_crlf_with_functions() {
	BEGIN_TEST("CRLF: function declaration");
	if (!parses_ok(
		"fn myFunc(a) {\r\n"
		"float x = a\r\n"
		"return x\r\n"
		"}\r\n"
	)) FAIL("parse error");
	PASS();
}

static void test_crlf_with_else_if() {
	BEGIN_TEST("CRLF: full if/else-if/else");
	if (!parses_ok(
		"float x = 5.0f\r\n"
		"if x > 10.0f {\r\n"
		"float y = 1.0f\r\n"
		"} else if x > 0.0f {\r\n"
		"float y = 2.0f\r\n"
		"} else {\r\n"
		"float y = 3.0f\r\n"
		"}\r\n"
	)) FAIL("parse error");
	PASS();
}

static void test_nested_function_scopes() {
	BEGIN_TEST("Function with if inside");
	if (!parses_ok(
		"fn check(val) {\n"
		"if val > 0.0f {\n"
		"return val\n"
		"}\n"
		"return 0.0f\n"
		"}\n"
	)) FAIL("parse error");
	PASS();
}

static void test_expression_many_operators() {
	BEGIN_TEST("Expression with many operators: a+b-c*d/e");
	if (!parses_ok(
		"float a = 1.0f\nfloat b = 2.0f\nfloat c = 3.0f\n"
		"float d = 4.0f\nfloat e = 5.0f\n"
		"float r = a + b - c * d / e\n"
	)) FAIL("parse error");
	PASS();
}

// ==== ENTRY POINT ====

typedef void (*TestFn)();

int main() {
	atexit(cleanup);
	printf("\n ---- TEST: PARSER ---- \n\n");

	TestFn tests[] = {
		// Variable declarations
		test_float_decl, test_float_decl_with_init, test_float_decl_with_expr,
		test_int_decl, test_int_decl_with_init, test_var_decl, test_multiple_decls,
		// Assignments
		test_simple_assignment, test_assignment_with_expr,
		test_compound_add_assign, test_compound_sub_assign,
		test_compound_mul_assign, test_compound_quo_assign,
		// Increment / decrement
		test_increment, test_decrement,
		// Expressions
		test_binary_add, test_binary_chain, test_parenthesized_expr,
		test_nested_parens, test_unary_minus, test_unary_not,
		test_comparison_operators, test_logical_operators, test_arithmetic_all,
		// If / else
		test_if_simple, test_if_else, test_if_else_if_else, test_if_empty_body,
		// For loop
		test_for_loop_full, test_for_loop_dec, test_for_loop_empty_body,
		// While loop
		test_while_loop, test_while_empty_body,
		// Break / continue
		test_break_in_loop, test_continue_in_loop,
		// Functions
		test_fn_no_params, test_fn_one_param, test_fn_two_params,
		test_fn_with_return, test_fn_void_return,
		test_fn_call_no_args, test_fn_call_with_args,
		// Brace placement
		test_brace_next_line_if, test_brace_next_line_for, test_brace_next_line_while,
		// Nested scopes
		test_nested_if_in_for, test_nested_for_in_while,
		// Semicolons / EOL
		test_semicolon_separator, test_eol_as_separator,
		// CRLF
		test_crlf_basic, test_crlf_if_else, test_crlf_for_loop,
		// Complex / integration
		test_complex_program, test_fn_with_body,
		// Edge-case / regression tests
		test_empty_program, test_comment_only_program,
		test_undefined_variable_no_crash, test_redefinition_no_crash,
		test_shadowing_nested_scope_no_crash,
		test_multiple_functions, test_fn_three_params,
		test_while_with_break, test_for_with_continue,
		test_deeply_nested_blocks,
		test_integer_literal_in_assign, test_var_reference_in_assign,
		test_negative_literal_in_assign, test_multiple_else_if_chains,
		test_mixed_int_and_float, test_semicolons_multiple_on_line,
		test_crlf_with_functions, test_crlf_with_else_if,
		test_nested_function_scopes, test_expression_many_operators,
	};

	int count = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < count; i++)
		tests[i]();

	printf("\n  Results: %d/%d passed", tests_passed, tests_run);
	if (tests_failed > 0) printf(" (%d FAILED)", tests_failed);
	printf("\n\n ---- END TEST ----\n\n");

	cleanup();
	return (tests_failed == 0) ? 0 : 1;
}
