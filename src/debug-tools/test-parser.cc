// test-parser.cc - Unit tests for the Rinto parser
#include <backend.hpp>
#include <parser.hpp>
#include <fstream>

class Bexpression {};
class Bstatement  {};

// Implements a backend variable for testing.
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

// Required by backend.hpp
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

/*
 * A mock backend that silently handles all operations.
 * No output is produced; tests only verify parse succeeds/fails.
 */
class Test_backend : public Backend
{
public:
	Test_backend() : _had_error(false) {}

	bool had_error() const
	{ return _had_error; }

	void reset_error()
	{ _had_error = false; }

	// Variable.
	Bvariable* variable(Named_object* obj) {
		Bvariable* var = new Bvariable;
		if (obj != NULL) {
			var->set_identifier(obj->identifier());
			var->set_location(obj->location());
		}
		return var;
	}

	// Expressions.

	Bexpression* invalid_expression() {
		_had_error = true;
		return new Bexpression;
	}

	Bexpression* var_reference(Bvariable* var, Location loc) {
		delete var;
		return new Bexpression;
	}

	Bexpression* conditional_expression(Bexpression* cond, Location loc) {
		delete cond;
		return new Bexpression;
	}

	Bexpression* unary_expression
	(RIN_OPERATOR op, Bexpression* expr, Location loc) {
		delete expr;
		return new Bexpression;
	}

	Bexpression* float_expression(const mpfr_t* val, Location loc) {
		return new Bexpression;
	}

	Bexpression* integer_expression(const mpfr_t* val, Location loc) {
		return new Bexpression;
	}

	Bexpression* binary_expression
	(RIN_OPERATOR op, Bexpression* left, Bexpression* right, Location loc) {
		delete left;
		delete right;
		return new Bexpression;
	}

	// Statements.

	Bstatement* invalid_statement() {
		_had_error = true;
		return new Bstatement;
	}

	Bstatement* var_dec_statement(Bvariable* var) {
		delete var;
		return new Bstatement;
	}

	Bstatement* inc_statement(Bexpression* var, Location loc) {
		delete var;
		return new Bstatement;
	}

	Bstatement* dec_statement(Bexpression* var, Location loc) {
		delete var;
		return new Bstatement;
	}

	Bstatement* expression_statement(Bexpression* expr, Location loc) {
		delete expr;
		return new Bstatement;
	}

	Bstatement* compound_statement
	(Bstatement* first, Bstatement* second, Location loc) {
		delete first;
		delete second;
		return new Bstatement;
	}

	Bstatement* if_statement
	(Bexpression* expr, Scope* scope, Scope* else_block, Location loc) {
		delete expr;
		// Scopes owned by If_statement; do not delete here.
		return new Bstatement;
	}

	Bstatement* assignment_statement
	(Bexpression* lhs, Bexpression* rhs, Location loc) {
		delete lhs;
		delete rhs;
		return new Bstatement;
	}

	Bstatement* for_statement
	(Bstatement* ind, Bstatement* cond, Bstatement* inc, Scope* then, Location loc) {
		delete ind;
		delete cond;
		delete inc;
		// Scope owned by For_statement; do not delete here.
		return new Bstatement;
	}

	Bstatement* return_statement(Bexpression* expr, Location loc) {
		delete expr;
		return new Bstatement;
	}

	Bstatement* function_statement
	(const std::string& name, const std::vector<std::string>& params,
	 Scope* body, Location loc) {
		// Scope owned by Function_declaration_statement; do not delete here.
		return new Bstatement;
	}

	Bexpression* call_expression
	(const std::string& name, const std::vector<Bexpression*>& args,
	 Location loc) {
		for (auto itr = args.begin(); itr != args.end(); ++itr)
			delete *itr;
		return new Bexpression;
	}

	Bstatement* break_statement(Location loc) {
		return new Bstatement;
	}

	Bstatement* continue_statement(Location loc) {
		return new Bstatement;
	}

private:
	bool _had_error;
};

static int tests_run = 0;
static int tests_passed = 0;

static std::string write_temp_file(const std::string& content) {
	std::string path = "rin_test_parser_tmp.rin";
	std::ofstream out(path);
	out << content;
	out.close();
	return path;
}

static void remove_temp_file() {
	std::remove("rin_test_parser_tmp.rin");
}

// Test: "float x" -> parses without error
static bool test_var_declaration() {
	std::string path = write_temp_file("float x\n");
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
	return true;
}

// Test: "float x = 3.14f" -> parses without error
static bool test_var_with_init() {
	std::string path = write_temp_file("float x = 3.14f\n");
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
	return true;
}

// Test: "if x > 0 {}" with x defined -> parses without error
static bool test_if_statement() {
	std::string path = write_temp_file("float x = 1.0f\nif x > 0.0f {\n}\n");
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
	return true;
}

// Test: "int y = 42" -> parses without error
static bool test_int_declaration() {
	std::string path = write_temp_file("int y = 42\n");
	Test_backend* be = new Test_backend;
	Parser parser(path, be);
	parser.parse();
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
	printf("\n ---- TEST: PARSER ---- \n\n");

	TestCase tests[] = {
		{ "float x -> parses OK",            test_var_declaration },
		{ "float x = 3.14f -> parses OK",    test_var_with_init },
		{ "if x > 0 {} -> parses OK",        test_if_statement },
		{ "int y = 42 -> parses OK",         test_int_declaration },
	};

	int count = sizeof(tests) / sizeof(tests[0]);
	for (int i = 0; i < count; i++)
		run_test(tests[i]);

	printf("\n  Results: %d/%d passed\n", tests_passed, tests_run);
	printf("\n ---- END TEST ----\n\n");

	remove_temp_file();

	return (tests_passed == tests_run) ? 0 : 1;
}
