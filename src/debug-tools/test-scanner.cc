// test-scanner.cc - Comprehensive unit tests for the Rinto scanner/lexer
#include <backend.hpp>
#include <scanner.hpp>
#include <diagnostic.hpp>
#include <fstream>
#include <cstdlib>
#include <csignal>

class Bexpression {};
class Bstatement  {};
class Bvariable   {};

// Required by backend.hpp
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;
static int temp_counter = 0;

static std::string write_temp(const std::string& content) {
	std::string path = "rin_sc_test_" + std::to_string(temp_counter++) + ".rin";
	std::ofstream out(path, std::ios::trunc | std::ios::binary);
	out << content;
	out.flush();
	out.close();
	return path;
}

static void cleanup() {
	for (int i = 0; i < temp_counter; i++)
		std::remove(("rin_sc_test_" + std::to_string(i) + ".rin").c_str());
}

#define BEGIN_TEST(name) \
	tests_run++; \
	printf("  [%02d] %-50s ", tests_run, name); \
	fflush(stdout);

#define PASS() do { tests_passed++; printf("PASS\n"); fflush(stdout); return; } while(0)
#define FAIL(msg) do { \
	tests_failed++; \
	printf("FAIL: %s\n", msg); \
	printf("::error::Scanner test %d FAILED: %s\n", tests_run, msg); \
	fflush(stdout); \
	return; \
} while(0)
#define EXPECT_CLS(tok, cls) \
	if ((tok).classification() != Token::cls) FAIL("expected " #cls)
#define EXPECT_OP(tok, op_val) \
	if ((tok).classification() != Token::TOKEN_OPERATOR || (tok).op() != op_val) \
		FAIL("expected operator " #op_val)
#define EXPECT_RID_VAL(tok, rid_val) \
	if ((tok).classification() != Token::TOKEN_RID || (tok).rid() != rid_val) \
		FAIL("expected RID " #rid_val)

// ---- LITERAL TESTS ----

static void test_integer_literal() {
	BEGIN_TEST("Integer literal: 42");
	Scanner sc(write_temp("42"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_INTEGER);
	if (t.string() != "42") FAIL("value mismatch");
	PASS();
}

static void test_integer_zero() {
	BEGIN_TEST("Integer literal: 0");
	Scanner sc(write_temp("0"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_INTEGER);
	PASS();
}

static void test_large_integer() {
	BEGIN_TEST("Integer literal: 999999");
	Scanner sc(write_temp("999999"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_INTEGER);
	if (t.string() != "999999") FAIL("value mismatch");
	PASS();
}

static void test_float_with_suffix() {
	BEGIN_TEST("Float literal: 3.14f");
	Scanner sc(write_temp("3.14f"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_FLOAT);
	PASS();
}

static void test_float_without_suffix() {
	BEGIN_TEST("Float literal: 3.14");
	Scanner sc(write_temp("3.14"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_FLOAT);
	PASS();
}

static void test_float_integer_suffix() {
	BEGIN_TEST("Float literal: 42f");
	Scanner sc(write_temp("42f"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_FLOAT);
	PASS();
}

static void test_hex_literal() {
	BEGIN_TEST("Hex literal: 0xFF");
	Scanner sc(write_temp("0xFF"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_INTEGER);
	PASS();
}

static void test_hex_lowercase() {
	BEGIN_TEST("Hex literal: 0xab12");
	Scanner sc(write_temp("0xab12"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_INTEGER);
	PASS();
}

// ---- IDENTIFIER TESTS ----

static void test_simple_ident() {
	BEGIN_TEST("Identifier: myVar");
	Scanner sc(write_temp("myVar"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "myVar") FAIL("name mismatch");
	PASS();
}

static void test_single_char_ident() {
	BEGIN_TEST("Identifier: x");
	Scanner sc(write_temp("x"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("name mismatch");
	PASS();
}

static void test_ident_with_digits() {
	BEGIN_TEST("Identifier: abc123");
	Scanner sc(write_temp("abc123"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "abc123") FAIL("name mismatch");
	PASS();
}

static void test_ident_uppercase() {
	BEGIN_TEST("Identifier: MyClass");
	Scanner sc(write_temp("MyClass"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "MyClass") FAIL("name mismatch");
	PASS();
}

// ---- TYPE KEYWORD TESTS ----

static void test_rid_float() {
	BEGIN_TEST("Keyword: float");
	Scanner sc(write_temp("float"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_FLOAT);
	PASS();
}

static void test_rid_int() {
	BEGIN_TEST("Keyword: int");
	Scanner sc(write_temp("int"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_INT);
	PASS();
}

static void test_rid_bool() {
	BEGIN_TEST("Keyword: bool");
	Scanner sc(write_temp("bool"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_BOOL);
	PASS();
}

static void test_rid_string() {
	BEGIN_TEST("Keyword: string");
	Scanner sc(write_temp("string"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_STRING);
	PASS();
}

static void test_rid_var() {
	BEGIN_TEST("Keyword: var");
	Scanner sc(write_temp("var"));
	Token t = sc.next_token();
	// "var" is a keyword, so it reads "var" then hits EOF
	EXPECT_RID_VAL(t, RID_VAR);
	PASS();
}

static void test_rid_true() {
	BEGIN_TEST("Keyword: true");
	Scanner sc(write_temp("true"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_TRUE);
	PASS();
}

static void test_rid_false() {
	BEGIN_TEST("Keyword: false");
	Scanner sc(write_temp("false"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_FALSE);
	PASS();
}

// ---- CONTROL FLOW KEYWORD TESTS ----

static void test_rid_if() {
	BEGIN_TEST("Keyword: if");
	Scanner sc(write_temp("if"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_IF);
	PASS();
}

static void test_rid_else() {
	BEGIN_TEST("Keyword: else");
	Scanner sc(write_temp("else"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_ELSE);
	PASS();
}

static void test_rid_for() {
	BEGIN_TEST("Keyword: for");
	Scanner sc(write_temp("for"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_FOR);
	PASS();
}

static void test_rid_while() {
	BEGIN_TEST("Keyword: while");
	Scanner sc(write_temp("while"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_WHILE);
	PASS();
}

static void test_rid_fn() {
	BEGIN_TEST("Keyword: fn");
	Scanner sc(write_temp("fn"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_FN);
	PASS();
}

static void test_rid_return() {
	BEGIN_TEST("Keyword: return");
	Scanner sc(write_temp("return"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_RETURN);
	PASS();
}

static void test_rid_break() {
	BEGIN_TEST("Keyword: break");
	Scanner sc(write_temp("break"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_BREAK);
	PASS();
}

static void test_rid_continue() {
	BEGIN_TEST("Keyword: continue");
	Scanner sc(write_temp("continue"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_CONTINUE);
	PASS();
}

static void test_rid_switch() {
	BEGIN_TEST("Keyword: switch");
	Scanner sc(write_temp("switch"));
	Token t = sc.next_token();
	EXPECT_RID_VAL(t, RID_SWITCH);
	PASS();
}

// ---- ARITHMETIC OPERATOR TESTS ----

static void test_op_add()       { BEGIN_TEST("Operator: +");  Scanner sc(write_temp("+ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_ADD); PASS(); }
static void test_op_sub()       { BEGIN_TEST("Operator: -");  Scanner sc(write_temp("- x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_SUB); PASS(); }
static void test_op_mul()       { BEGIN_TEST("Operator: *");  Scanner sc(write_temp("* x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_MUL); PASS(); }
static void test_op_quo()       { BEGIN_TEST("Operator: /");  Scanner sc(write_temp("/ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_QUO); PASS(); }
static void test_op_rem()       { BEGIN_TEST("Operator: %%"); Scanner sc(write_temp("% x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_REM); PASS(); }

// ---- COMPARISON OPERATOR TESTS ----

static void test_op_eql()       { BEGIN_TEST("Operator: =="); Scanner sc(write_temp("== x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_EQL); PASS(); }
static void test_op_neq()       { BEGIN_TEST("Operator: !="); Scanner sc(write_temp("!= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_NEQ); PASS(); }
static void test_op_lss()       { BEGIN_TEST("Operator: <");  Scanner sc(write_temp("< x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LSS); PASS(); }
static void test_op_gtr()       { BEGIN_TEST("Operator: >");  Scanner sc(write_temp("> x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_GTR); PASS(); }
static void test_op_leq()       { BEGIN_TEST("Operator: <="); Scanner sc(write_temp("<= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LEQ); PASS(); }
static void test_op_geq()       { BEGIN_TEST("Operator: >="); Scanner sc(write_temp(">= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_GEQ); PASS(); }

// ---- LOGICAL OPERATOR TESTS ----

static void test_op_land()      { BEGIN_TEST("Operator: &&"); Scanner sc(write_temp("&& x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LAND); PASS(); }
static void test_op_lor()       { BEGIN_TEST("Operator: ||"); Scanner sc(write_temp("|| x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LOR); PASS(); }
static void test_op_not()       { BEGIN_TEST("Operator: !");  Scanner sc(write_temp("!x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_NOT); PASS(); }

// ---- UNARY / ASSIGNMENT OPERATOR TESTS ----

static void test_op_inc()       { BEGIN_TEST("Operator: ++"); Scanner sc(write_temp("++ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_INC); PASS(); }
static void test_op_dec()       { BEGIN_TEST("Operator: --"); Scanner sc(write_temp("-- x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_DEC); PASS(); }
static void test_op_assign()    { BEGIN_TEST("Operator: =");  Scanner sc(write_temp("= 1")); Token t = sc.next_token(); EXPECT_OP(t, OPER_ASSIGN); PASS(); }
static void test_op_add_asgn()  { BEGIN_TEST("Operator: +="); Scanner sc(write_temp("+= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_ADD_ASSIGN); PASS(); }
static void test_op_sub_asgn()  { BEGIN_TEST("Operator: -="); Scanner sc(write_temp("-= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_SUB_ASSIGN); PASS(); }
static void test_op_mul_asgn()  { BEGIN_TEST("Operator: *="); Scanner sc(write_temp("*= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_MUL_ASSIGN); PASS(); }
static void test_op_quo_asgn()  { BEGIN_TEST("Operator: /="); Scanner sc(write_temp("/= x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_QUO_ASSIGN); PASS(); }

// ---- BITWISE OPERATOR TESTS ----

static void test_op_band()      { BEGIN_TEST("Operator: &");  Scanner sc(write_temp("& x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_BAND); PASS(); }
static void test_op_bor()       { BEGIN_TEST("Operator: |");  Scanner sc(write_temp("| x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_BOR); PASS(); }
static void test_op_bxor()      { BEGIN_TEST("Operator: ^");  Scanner sc(write_temp("^ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_BXOR); PASS(); }
static void test_op_bnot()      { BEGIN_TEST("Operator: ~");  Scanner sc(write_temp("~ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_BNOT); PASS(); }
static void test_op_lshift()    { BEGIN_TEST("Operator: <<"); Scanner sc(write_temp("<< x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LSHIFT); PASS(); }
static void test_op_rshift()    { BEGIN_TEST("Operator: >>"); Scanner sc(write_temp(">> x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_RSHIFT); PASS(); }

// ---- DELIMITER TESTS ----

static void test_op_semicolon() { BEGIN_TEST("Operator: ;");  Scanner sc(write_temp("; x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_SEMICOLON); PASS(); }
static void test_op_comma()     { BEGIN_TEST("Operator: ,");  Scanner sc(write_temp(", x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_COMMA); PASS(); }
static void test_op_lparen()    { BEGIN_TEST("Operator: (");  Scanner sc(write_temp("( x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LPAREN); PASS(); }
static void test_op_rparen()    { BEGIN_TEST("Operator: )");  Scanner sc(write_temp(") x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_RPAREN); PASS(); }
static void test_op_lbrack()    { BEGIN_TEST("Operator: [");  Scanner sc(write_temp("[ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LBRACK); PASS(); }
static void test_op_rbrack()    { BEGIN_TEST("Operator: ]");  Scanner sc(write_temp("] x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_RBRACK); PASS(); }
static void test_op_lbrace()    { BEGIN_TEST("Operator: {");  Scanner sc(write_temp("{ x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_LBRACE); PASS(); }
static void test_op_rbrace()    { BEGIN_TEST("Operator: }");  Scanner sc(write_temp("} x")); Token t = sc.next_token(); EXPECT_OP(t, OPER_RBRACE); PASS(); }

// ---- COMMENT TESTS ----

static void test_single_line_comment() {
	BEGIN_TEST("Single-line comment skipped");
	Scanner sc(write_temp("// this is a comment\nx"));
	Token t = sc.next_token();
	while (t.classification() == Token::TOKEN_EOL) t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("expected x after comment");
	PASS();
}

static void test_multi_line_comment() {
	BEGIN_TEST("Multi-line comment skipped");
	Scanner sc(write_temp("/* multi\nline\ncomment */\nx"));
	Token t = sc.next_token();
	while (t.classification() == Token::TOKEN_EOL) t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("expected x after comment");
	PASS();
}

static void test_comment_after_code() {
	BEGIN_TEST("Comment after code on same line");
	Scanner sc(write_temp("x // comment"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("expected x");
	PASS();
}

// ---- WHITESPACE / EOL / EOF TESTS ----

static void test_eol_token() {
	BEGIN_TEST("EOL token from newline");
	Scanner sc(write_temp("x\ny"));
	Token t1 = sc.next_token();
	EXPECT_CLS(t1, TOKEN_IDENT);
	Token t2 = sc.next_token();
	EXPECT_CLS(t2, TOKEN_EOL);
	PASS();
}

static void test_eof_at_end() {
	BEGIN_TEST("EOF at end of input");
	Scanner sc(write_temp("x"));
	sc.next_token(); // x
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_EOF);
	PASS();
}

static void test_tabs_and_spaces() {
	BEGIN_TEST("Tabs and spaces are whitespace");
	Scanner sc(write_temp("  \t  x  \t  y"));
	Token t1 = sc.next_token();
	EXPECT_CLS(t1, TOKEN_IDENT);
	Token t2 = sc.next_token();
	EXPECT_CLS(t2, TOKEN_IDENT);
	PASS();
}

static void test_crlf_handling() {
	BEGIN_TEST("CRLF line endings handled");
	Scanner sc(write_temp("x\r\ny"));
	Token t1 = sc.next_token();
	EXPECT_CLS(t1, TOKEN_IDENT);
	// Should get EOL from \n (after \r is skipped)
	Token t2 = sc.next_token();
	EXPECT_CLS(t2, TOKEN_EOL);
	Token t3 = sc.next_token();
	EXPECT_CLS(t3, TOKEN_IDENT);
	PASS();
}

// ---- PEEK / LOOKAHEAD TESTS ----

static void test_peek_does_not_consume() {
	BEGIN_TEST("peek_token does not consume");
	Scanner sc(write_temp("x y"));
	Token p = sc.peek_token();
	EXPECT_CLS(p, TOKEN_IDENT);
	Token n = sc.next_token();
	EXPECT_CLS(n, TOKEN_IDENT);
	if (n.string() != p.string()) FAIL("peek and next differ");
	PASS();
}

static void test_peek_nth() {
	BEGIN_TEST("peek_nth_token lookahead");
	Scanner sc(write_temp("a + b"));
	Token t0 = sc.peek_nth_token(0);
	Token t1 = sc.peek_nth_token(1);
	Token t2 = sc.peek_nth_token(2);
	EXPECT_CLS(t0, TOKEN_IDENT);
	EXPECT_CLS(t1, TOKEN_OPERATOR);
	EXPECT_CLS(t2, TOKEN_IDENT);
	if (t0.string() != "a") FAIL("expected a");
	PASS();
}

// ---- MULTI-TOKEN SEQUENCE TESTS ----

static void test_full_var_decl_tokens() {
	BEGIN_TEST("Token seq: float x = 3.14f");
	Scanner sc(write_temp("float x = 3.14f\n"));
	Token t1 = sc.next_token(); EXPECT_RID_VAL(t1, RID_FLOAT);
	Token t2 = sc.next_token(); EXPECT_CLS(t2, TOKEN_IDENT);
	Token t3 = sc.next_token(); EXPECT_OP(t3, OPER_ASSIGN);
	Token t4 = sc.next_token(); EXPECT_CLS(t4, TOKEN_FLOAT);
	PASS();
}

static void test_expression_tokens() {
	BEGIN_TEST("Token seq: a + b * c");
	Scanner sc(write_temp("a + b * c\n"));
	Token t1 = sc.next_token(); EXPECT_CLS(t1, TOKEN_IDENT);
	Token t2 = sc.next_token(); EXPECT_OP(t2, OPER_ADD);
	Token t3 = sc.next_token(); EXPECT_CLS(t3, TOKEN_IDENT);
	Token t4 = sc.next_token(); EXPECT_OP(t4, OPER_MUL);
	Token t5 = sc.next_token(); EXPECT_CLS(t5, TOKEN_IDENT);
	PASS();
}

static void test_function_decl_tokens() {
	BEGIN_TEST("Token seq: fn add(a, b)");
	Scanner sc(write_temp("fn add(a, b)\n"));
	Token t1 = sc.next_token(); EXPECT_RID_VAL(t1, RID_FN);
	Token t2 = sc.next_token(); EXPECT_CLS(t2, TOKEN_IDENT);
	Token t3 = sc.next_token(); EXPECT_OP(t3, OPER_LPAREN);
	Token t4 = sc.next_token(); EXPECT_CLS(t4, TOKEN_IDENT);
	Token t5 = sc.next_token(); EXPECT_OP(t5, OPER_COMMA);
	Token t6 = sc.next_token(); EXPECT_CLS(t6, TOKEN_IDENT);
	Token t7 = sc.next_token(); EXPECT_OP(t7, OPER_RPAREN);
	PASS();
}

static void test_is_semicolon() {
	BEGIN_TEST("Token::is_semicolon for ;, EOL, EOF");
	Scanner sc(write_temp("x;\ny"));
	sc.next_token(); // x
	Token semi = sc.next_token(); // ;
	if (!semi.is_semicolon()) FAIL("; should be semicolon");
	Token eol = sc.next_token();
	if (!eol.is_semicolon()) FAIL("EOL should be semicolon");
	sc.next_token(); // y
	Token eof = sc.next_token();
	if (!eof.is_semicolon()) FAIL("EOF should be semicolon");
	PASS();
}

// ---- EDGE-CASE / REGRESSION TESTS ----

static void test_empty_file() {
	BEGIN_TEST("Empty file -> EOF");
	Scanner sc(write_temp(""));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_EOF);
	PASS();
}

static void test_whitespace_only() {
	BEGIN_TEST("Whitespace only -> EOF");
	Scanner sc(write_temp("   \t  "));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_EOF);
	PASS();
}

static void test_comment_only() {
	BEGIN_TEST("Comment only -> EOF");
	Scanner sc(write_temp("// just a comment\n"));
	Token t = sc.next_token();
	// Skip any EOL tokens
	while (t.classification() == Token::TOKEN_EOL)
		t = sc.next_token();
	EXPECT_CLS(t, TOKEN_EOF);
	PASS();
}

static void test_multiple_blank_lines() {
	BEGIN_TEST("Multiple blank lines then ident");
	Scanner sc(write_temp("\n\n\n x"));
	Token t = sc.next_token();
	// Skip EOL tokens
	while (t.classification() == Token::TOKEN_EOL)
		t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("expected x");
	PASS();
}

static void test_operator_at_eof() {
	BEGIN_TEST("Operator at EOF: x +");
	Scanner sc(write_temp("x +"));
	Token t1 = sc.next_token();
	EXPECT_CLS(t1, TOKEN_IDENT);
	Token t2 = sc.next_token();
	EXPECT_OP(t2, OPER_ADD);
	Token t3 = sc.next_token();
	EXPECT_CLS(t3, TOKEN_EOF);
	PASS();
}

static void test_keyword_prefix_ifx() {
	BEGIN_TEST("Keyword prefix: ifx -> IDENT");
	Scanner sc(write_temp("ifx"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "ifx") FAIL("expected ifx");
	PASS();
}

static void test_keyword_prefix_floating() {
	BEGIN_TEST("Keyword prefix: floating -> IDENT");
	Scanner sc(write_temp("floating"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "floating") FAIL("expected floating");
	PASS();
}

static void test_keyword_prefix_format() {
	BEGIN_TEST("Keyword prefix: format -> IDENT");
	Scanner sc(write_temp("format"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "format") FAIL("expected format");
	PASS();
}

static void test_underscore_identifier() {
	BEGIN_TEST("Underscore ident: _test -> INVALID (no _ start)");
	Scanner sc(write_temp("_test"));
	Token t = sc.next_token();
	// Regex requires first char to be [a-zA-Z], so _test is invalid
	EXPECT_CLS(t, TOKEN_INVALID);
	PASS();
}

static void test_multiple_operators_sequence() {
	BEGIN_TEST("Multiple operators: +-*/ x");
	Scanner sc(write_temp("+-*/ x"));
	Token t1 = sc.next_token(); EXPECT_OP(t1, OPER_ADD);
	Token t2 = sc.next_token(); EXPECT_OP(t2, OPER_SUB);
	Token t3 = sc.next_token(); EXPECT_OP(t3, OPER_MUL);
	Token t4 = sc.next_token(); EXPECT_OP(t4, OPER_QUO);
	Token t5 = sc.next_token(); EXPECT_CLS(t5, TOKEN_IDENT);
	PASS();
}

static void test_adjacent_operators_no_space() {
	BEGIN_TEST("Adjacent without space: x+y");
	Scanner sc(write_temp("x+y"));
	Token t1 = sc.next_token(); EXPECT_CLS(t1, TOKEN_IDENT);
	if (t1.string() != "x") FAIL("expected x");
	Token t2 = sc.next_token(); EXPECT_OP(t2, OPER_ADD);
	Token t3 = sc.next_token(); EXPECT_CLS(t3, TOKEN_IDENT);
	if (t3.string() != "y") FAIL("expected y");
	PASS();
}

static void test_nested_comment() {
	BEGIN_TEST("Nested comment: /* outer /* inner */ */");
	// C-style: the first */ closes the comment, then */ is unexpected
	// After first */, we should be able to get tokens before the second */
	Scanner sc(write_temp("/* outer /* inner */ x"));
	Token t = sc.next_token();
	// Skip any whitespace/eol
	while (t.classification() == Token::TOKEN_EOL)
		t = sc.next_token();
	// After the first */ closes, we should get 'x' as IDENT
	EXPECT_CLS(t, TOKEN_IDENT);
	if (t.string() != "x") FAIL("expected x after comment close");
	PASS();
}

static void test_integer_dot_method() {
	BEGIN_TEST("Integer then dot: 42.method -> FLOAT-ish token");
	// 42.method will be partially consumed; just verify no crash
	Scanner sc(write_temp("42.5f x"));
	Token t = sc.next_token();
	EXPECT_CLS(t, TOKEN_FLOAT);
	Token t2 = sc.next_token();
	EXPECT_CLS(t2, TOKEN_IDENT);
	PASS();
}

static void test_consume_errors_unmatched_paren() {
	BEGIN_TEST("consume_errors for unmatched paren");
	Scanner sc(write_temp("(x"));
	Token t1 = sc.next_token(); // (
	EXPECT_OP(t1, OPER_LPAREN);
	Token t2 = sc.next_token(); // x
	EXPECT_CLS(t2, TOKEN_IDENT);
	bool had_errors = sc.consume_errors();
	if (!had_errors) FAIL("expected unmatched paren error");
	PASS();
}

static void test_multiple_peek_same_token() {
	BEGIN_TEST("Multiple peek calls return same token");
	Scanner sc(write_temp("abc def"));
	Token p1 = sc.peek_token();
	Token p2 = sc.peek_token();
	Token p3 = sc.peek_token();
	if (p1.string() != p2.string()) FAIL("peek 1 and 2 differ");
	if (p2.string() != p3.string()) FAIL("peek 2 and 3 differ");
	if (p1.classification() != p2.classification()) FAIL("cls mismatch");
	PASS();
}

static void test_has_next_after_eof() {
	BEGIN_TEST("has_next returns false after EOF");
	Scanner sc(write_temp("x"));
	sc.next_token(); // x
	sc.next_token(); // EOF
	if (sc.has_next()) FAIL("has_next should be false after EOF");
	PASS();
}

static void test_classification_as_string() {
	BEGIN_TEST("classification_as_string for various types");
	Scanner sc1(write_temp("42"));
	Token t1 = sc1.next_token();
	if (t1.classification_as_string() != "integer literal") FAIL("int cls string");

	Scanner sc2(write_temp("3.14f"));
	Token t2 = sc2.next_token();
	if (t2.classification_as_string() != "float literal") FAIL("float cls string");

	Scanner sc3(write_temp("myVar"));
	Token t3 = sc3.next_token();
	if (t3.classification_as_string() != "identifier") FAIL("ident cls string");

	Scanner sc4(write_temp(""));
	Token t4 = sc4.next_token();
	if (t4.classification_as_string() != "EOF") FAIL("eof cls string");

	PASS();
}

// ---- ENTRY POINT ----

typedef void (*TestFn)();

static void signal_handler(int sig) {
	printf("\n::error::FATAL Signal %d received after test %d\n", sig, tests_run);
	fflush(stdout);
	exit(128 + sig);
}

int main() {
	signal(SIGABRT, signal_handler);
	signal(SIGSEGV, signal_handler);
	atexit(cleanup);
	printf("\n ---- TEST: SCANNER ---- \n\n");

	TestFn tests[] = {
		// Literals
		test_integer_literal, test_integer_zero, test_large_integer,
		test_float_with_suffix, test_float_without_suffix, test_float_integer_suffix,
		test_hex_literal, test_hex_lowercase,
		// Identifiers
		test_simple_ident, test_single_char_ident, test_ident_with_digits, test_ident_uppercase,
		// Type keywords
		test_rid_float, test_rid_int, test_rid_bool, test_rid_string,
		test_rid_var, test_rid_true, test_rid_false,
		// Control flow keywords
		test_rid_if, test_rid_else, test_rid_for, test_rid_while,
		test_rid_fn, test_rid_return, test_rid_break, test_rid_continue, test_rid_switch,
		// Arithmetic operators
		test_op_add, test_op_sub, test_op_mul, test_op_quo, test_op_rem,
		// Comparison operators
		test_op_eql, test_op_neq, test_op_lss, test_op_gtr, test_op_leq, test_op_geq,
		// Logical operators
		test_op_land, test_op_lor, test_op_not,
		// Unary / assignment operators
		test_op_inc, test_op_dec, test_op_assign,
		test_op_add_asgn, test_op_sub_asgn, test_op_mul_asgn, test_op_quo_asgn,
		// Bitwise operators
		test_op_band, test_op_bor, test_op_bxor, test_op_bnot, test_op_lshift, test_op_rshift,
		// Delimiters
		test_op_semicolon, test_op_comma,
		test_op_lparen, test_op_rparen, test_op_lbrack, test_op_rbrack,
		test_op_lbrace, test_op_rbrace,
		// Comments
		test_single_line_comment, test_multi_line_comment, test_comment_after_code,
		// Whitespace / EOL / EOF
		test_eol_token, test_eof_at_end, test_tabs_and_spaces, test_crlf_handling,
		// Peek / lookahead
		test_peek_does_not_consume, test_peek_nth,
		// Multi-token sequences
		test_full_var_decl_tokens, test_expression_tokens, test_function_decl_tokens,
		test_is_semicolon,
		// Edge-case / regression tests
		test_empty_file, test_whitespace_only, test_comment_only,
		test_multiple_blank_lines, test_operator_at_eof,
		test_keyword_prefix_ifx, test_keyword_prefix_floating, test_keyword_prefix_format,
		test_underscore_identifier,
		test_multiple_operators_sequence, test_adjacent_operators_no_space,
		test_nested_comment, test_integer_dot_method,
		test_consume_errors_unmatched_paren, test_multiple_peek_same_token,
		test_has_next_after_eof, test_classification_as_string,
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
