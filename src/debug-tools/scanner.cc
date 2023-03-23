#include <backend.hpp>
#include <scanner.hpp>

// Free input memory and return NULL.
#define FREE_MEM1(obj1)                   \
{                                         \
        delete obj1;                      \
        return NULL;                      \
}

#define FREE_MEM2(obj1, obj2)             \
{                                         \
        delete obj1;                      \
        delete obj2;                      \
        return NULL;                      \
}

#define FREE_MEM4(obj1, obj2, obj3, obj4) \
{                                         \
        delete obj1;                      \
        delete obj2;                      \
        delete obj3;                      \
        delete obj4;                      \
        return NULL;                      \
}

class Bexpression {};
class Bstatement  {};
class Bvariable   {};

// Deletes a statement.
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

// Implements a mock backend, just the bare minimum to compile the scanner.
class Scanner_backend : public Backend
{
public:

        /*
         * None of these functions will actually be called by the scanner
         * so we don't care about returning anything non-NULL or any of
         * the inputs. Just the bare minimum necessary for the scanner
         * to compile.
         */

        // Variable.

        Bvariable* variable(Named_object* obj)
        { FREE_MEM1(obj); }

        // Expressions.

        Bexpression* var_reference(Bvariable* var, Location loc)
        { FREE_MEM1(var); }

        Bexpression* conditional_expression(Bexpression* cond, Location loc)
        { FREE_MEM1(cond); }

        Bexpression* unary_expression
        (RIN_OPERATOR op, Bexpression* expr, Location loc)
        { FREE_MEM1(expr); }

        Bexpression* float_expression(const mpfr_t* val, Location loc);

        Bexpression* binary_expression
        (RIN_OPERATOR op, Bexpression* left, Bexpression* right, Location loc)
        { FREE_MEM2(left, right); }

        // Statements.

        Bstatement* var_dec_statement(Bvariable* var)
        { FREE_MEM1(var); }

        Bstatement* inc_statement(Bexpression* var)
        { FREE_MEM1(var); }

        Bstatement* dec_statement(Bexpression* var)
        { FREE_MEM1(var); }

        Bstatement* expression_statement(Bexpression* expr, Location loc)
        { FREE_MEM1(expr); }

        Bstatement* compound_statement
        (Bstatement* first, Bstatement* second, Location loc)
        { FREE_MEM2(first, second); }

        Bstatement* if_statement(Bexpression* expr, Scope* scope, Location loc)
        { FREE_MEM2(expr, scope); }

        Bstatement* assignment_statement
        (Bexpression* lhs, Bexpression* rhs, Location loc)
        { FREE_MEM2(lhs, rhs); }

        Bstatement* for_statement
        (Bstatement* ind, Bstatement* cond, Bstatement* inc, Scope* scope, Location loc)
        { FREE_MEM4(ind, cond, inc, scope); }
};

Bexpression* Scanner_backend::float_expression(const mpfr_t* val, Location loc)
{
        /*
         * 'val' is a pointer to the mpfr float that's an attribute of some
         * Expression::float_expression instance. It is not dynamically copied
         * here, so it does not need to be freed (it is freed by
         * Expression::float_expression).
         */
        return NULL;
}

// The scanner debug tool program
int main(int argc, char** argv)
{
        printf("\n ---- DEBUG TOOL: SCANNER ---- \n\n");
        if (argc < 2) {
                rin_fatal_error(File::unknown_location(),
                        "Expected .rin file directory as command line argument");
                rin_inform(File::unknown_location(),
                        "\tSample usage: ./a.out MY_FILE.rin");
                return 0;
        }

        std::string path(argv[1]);

        Scanner sc(path);
        while (sc.has_next()) {
                Token tk = sc.next_token();
                Location l = tk.location();
                printf("%s:%d:%d: %s \t %s\n", &l.filename[0], l.line, l.column,
                       &tk.classification_as_string()[0], tk.str());
        }

        printf("\n\nSCANNER ERRORS: \n");
        sc.consume_errors();

        printf("\n ---- END DEBUG TOOL ----\n\n");

        return 0;
}
