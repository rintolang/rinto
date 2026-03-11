// test-scanner.cc - Unit tests for the Rinto scanner/lexer
#include <backend.hpp>
#include <scanner.hpp>
#include <diagnostic.hpp>
#include <fstream>

class Bexpression {};
class Bstatement  {};
class Bvariable   {};

// Required by backend.hpp
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

static int tests_run = 0;
static int tests_passed = 0;

static std::string write_temp_file(const std::string& content) {
	std::string path = "rin_test_scanner_tmp.rin";
	std::ofstream out(path);
	out << content;
	out.close();
	return path;
}

static void remove_temp_file() {
	std::remove("rin_test_scanner_tmp.rin");
}

// Test: "float x" -> TOKEN_RID(FLOAT), TOKEN_IDENT("x")
static bool test_float_x() {
	std::string path = write_temp_file("float x");
	Scanner sc(path);

	Token t1 = sc.next_token();
	if (t1.classification() != Token::TOKEN_RID || t1.rid() != RID_FLOAT) {
		printf("  Expected TOKEN_RID(FLOAT), got %s\n",
			t1.classification_as_string().c_str());
		return false;
	}

	Token t2 = sc.next_token();
	if (t2.classification() != Token::TOKEN_IDENT) {
		printf("  Expected TOKEN_IDENT, got %s\n",
			t2.classification_as_string().c_str());
		return false;
	}
	if (t2.string() != "x") {
		printf("  Expected identifier 'x', got '%s'\n", t2.str());
		return false;
	}

	return true;
}

// Test: "42" -> TOKEN_INTEGER
static bool test_integer_literal() {
	std::string path = write_temp_file("42");
	Scanner sc(path);

	Token t1 = sc.next_token();
	if (t1.classification() != Token::TOKEN_INTEGER) {
		printf("  Expected TOKEN_INTEGER, got %s\n",
			t1.classification_as_string().c_str());
		return false;
	}
	if (t1.string() != "42") {
		printf("  Expected '42', got '%s'\n", t1.str());
		return false;
	}

	return true;
}

// Test: "3.14f" -> TOKEN_FLOAT
static bool test_float_literal() {
	std::string path = write_temp_file("3.14f");
	Scanner sc(path);

	Token t1 = sc.next_token();
	if (t1.classification() != Token::TOKEN_FLOAT) {
		printf("  Expected TOKEN_FLOAT, got %s\n",
			t1.classification_as_string().c_str());
		return false;
	}

	return true;
}

// Test: "x + y" -> TOKEN_IDENT, TOKEN_OPERATOR(ADD), TOKEN_IDENT
static bool test_binary_expression_tokens() {
	std::string path = write_temp_file("x + y");
	Scanner sc(path);

	Token t1 = sc.next_token();
	if (t1.classification() != Token::TOKEN_IDENT) {
		printf("  Expected TOKEN_IDENT for 'x', got %s\n",
			t1.classification_as_string().c_str());
		return false;
	}

	Token t2 = sc.next_token();
	if (t2.classification() != Token::TOKEN_OPERATOR || t2.op() != OPER_ADD) {
		printf("  Expected TOKEN_OPERATOR(ADD), got %s\n",
			t2.classification_as_string().c_str());
		return false;
	}

	Token t3 = sc.next_token();
	if (t3.classification() != Token::TOKEN_IDENT) {
		printf("  Expected TOKEN_IDENT for 'y', got %s\n",
			t3.classification_as_string().c_str());
		return false;
	}

	return true;
}

// Test: "+= -= *= /=" -> compound operators
static bool test_compound_operators() {
	std::string path = write_temp_file("+= -= *= /=");
	Scanner sc(path);

	struct {
		RIN_OPERATOR expected;
		const char* name;
	} ops[] = {
		{ OPER_ADD_ASSIGN, "+=" },
		{ OPER_SUB_ASSIGN, "-=" },
		{ OPER_MUL_ASSIGN, "*=" },
		{ OPER_QUO_ASSIGN, "/=" },
	};

	for (int i = 0; i < 4; i++) {
		Token tok = sc.next_token();
		if (tok.classification() != Token::TOKEN_OPERATOR || tok.op() != ops[i].expected) {
			printf("  Expected TOKEN_OPERATOR(%s), got %s\n",
				ops[i].name, tok.classification_as_string().c_str());
			return false;
		}
	}

	return true;
}

// Test: all RID tokens
static bool test_rid_tokens() {
	std::string path = write_temp_file("if else for while fn return break continue");
	Scanner sc(path);

	RID expected[] = {
		RID_IF, RID_ELSE, RID_FOR, RID_WHILE,
		RID_FN, RID_RETURN, RID_BREAK, RID_CONTINUE
	};

	for (int i = 0; i < 8; i++) {
		Token tok = sc.next_token();
		if (tok.classification() != Token::TOKEN_RID) {
			printf("  Token %d: expected TOKEN_RID, got %s\n",
				i, tok.classification_as_string().c_str());
			return false;
		}
		if (tok.rid() != expected[i]) {
			printf("  Token %d: expected RID %d, got RID %d\n",
				i, expected[i], tok.rid());
			return false;
		}
	}

	return true;
}

// Test: "// comment\nx" -> TOKEN_IDENT("x") (comment skipped)
static bool test_comment_skip() {
	std::string path = write_temp_file("// comment\nx");
	Scanner sc(path);

	// First token might be EOL from the newline; skip EOLs
	Token tok = sc.next_token();
	while (tok.classification() == Token::TOKEN_EOL)
		tok = sc.next_token();

	if (tok.classification() != Token::TOKEN_IDENT) {
		printf("  Expected TOKEN_IDENT after comment, got %s\n",
			tok.classification_as_string().c_str());
		return false;
	}
	if (tok.string() != "x") {
		printf("  Expected 'x', got '%s'\n", tok.str());
		return false;
	}

	return true;
}

// Test: "{ }" -> paired operators
static bool test_paired_operators() {
	std::string path = write_temp_file("{ }");
	Scanner sc(path);

	Token t1 = sc.next_token();
	if (t1.classification() != Token::TOKEN_OPERATOR || t1.op() != OPER_LBRACE) {
		printf("  Expected TOKEN_OPERATOR(LBRACE), got %s\n",
			t1.classification_as_string().c_str());
		return false;
	}

	Token t2 = sc.next_token();
	if (t2.classification() != Token::TOKEN_OPERATOR || t2.op() != OPER_RBRACE) {
		printf("  Expected TOKEN_OPERATOR(RBRACE), got %s\n",
			t2.classification_as_string().c_str());
		return false;
	}

	return true;
}

typedef bool (*TestFn)();

struct TestCase {
	const char* name;
	TestFn fn;
};

static void run_test(const TestCase& tc) {
	tests_run++;
	printf("  [%d] %s ... ", tests_run, tc.name);
	bool result = tc.fn();
	if (result) {
		tests_passed++;
		printf("PASS\n");
	} else {
		printf("FAIL\n");
	}
}

int main() {
	printf("\n ---- TEST: SCANNER ---- \n\n");

	TestCase tests[] = {
		{ "float x -> RID(FLOAT) IDENT(x)",    test_float_x },
		{ "42 -> INTEGER",                      test_integer_literal },
		{ "3.14f -> FLOAT",                     test_float_literal },
		{ "x + y -> IDENT OPER(ADD) IDENT",     test_binary_expression_tokens },
		{ "+= -= *= /= -> compound operators",  test_compound_operators },
		{ "RID tokens (if/else/for/etc.)",       test_rid_tokens },
		{ "// comment skipped",                  test_comment_skip },
		{ "{ } -> paired operators",             test_paired_operators },
	};

	int count = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < count; i++)
		run_test(tests[i]);

	printf("\n  Results: %d/%d passed\n", tests_passed, tests_run);
	printf("\n ---- END TEST ----\n\n");

	remove_temp_file();

	return (tests_passed == tests_run) ? 0 : 1;
}
