#include <backend.hpp>
#include <parser.hpp>

class Bexpression {};
class Bstatement  {};

// Implements a backend variable.
class Bvariable
{
public:
        /*
         * Keep track of location and identifier string
         * for printing debug information to console.
         */

        std::string identifier()
        { return this->_identifier; }

        Location location()
        { return this->_location; }

        void set_identifier(const std::string& ident)
        { this->_identifier = ident; }

        void set_location(const Location& loc)
        { this->_location = loc; }

        bool has_identifier()
        { return (this->_identifier != ""); }

private:
        std::string _identifier = "";
        Location    _location   = File::unknown_location();
};

// Deletes a statement.
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

// Implements a mock backend, just the bare minimum to compile the scanner.
class Parser_backend : public Backend
{
public:

        /*
         * Some expressions/statements require non-NULL Bexpression
         * and Bstatement inputs, therefore all functions return
         * an instance. These instances will be deallocated by the
         * scope (frontend/backend.hpp) destructor.
         */

        // Variable.
        Bvariable* variable(Named_object* obj) override
        {
                std::string identifier = "NULL";
                Location loc = File::unknown_location();
                Bvariable* var = new Bvariable;

                if (obj != NULL) {
                        identifier = obj->identifier();
                        var->set_identifier(identifier);
                        loc = obj->location();
                        var->set_location(loc);
                }

                rin_inform(loc, "CREATED VAR OBJECT: '%s'",
                        identifier.c_str());

                return var;
        }

        // Expressions.

        Bexpression* invalid_expression() override
        {
                rin_warning_at(File::unknown_location(), 0,
                        "RECEIVED INVALID EXPRESSION SIGNAL");
                return new Bexpression;
        }

        Bexpression* var_reference(Bvariable* var, const Location& loc) override
        {
                std::string identifier = "NULL";
                if (var != NULL && var->has_identifier())
                        identifier = var->identifier();

                delete var;

                rin_inform(loc, "CREATED VAR REF WITH IDENT: %s",
                        identifier.c_str());

                return new Bexpression;
        }

        Bexpression* conditional_expression(Bexpression* cond, const Location& loc) override
        {
                rin_inform(loc, "CREATED COND EXPRESSION");
                delete cond;
                return new Bexpression;
        }

        Bexpression* unary_expression
        (RIN_OPERATOR op, Bexpression* expr, const Location& loc) override
        {
                rin_inform(loc, "CREATED UNARY EXPRESSION WITH OP: %s",
                        operator_name(op).c_str());
                delete expr;
                return new Bexpression;
        }

        Bexpression* float_expression(const mpfr_t* val, const Location& loc) override
        {
                if (val) {
                        long i;
                        char* abc = mpfr_get_str(NULL, &i, 10, 16, *val, MPFR_RNDN);
                        rin_inform(loc, "CREATED FLOAT EXPRESSION WITH VAL: %s", abc);
                        mpfr_free_str(abc);
                        return new Bexpression;
                }

                rin_inform(loc, "CREATED FLOAT EXPRESSION WITH VAL: NULL");
                return new Bexpression;
        }

        Bexpression* integer_expression(const mpfr_t* val, const Location& loc) override
        {
                if (val) {
                        long i;
                        char* abc = mpfr_get_str(NULL, &i, 10, 16, *val, MPFR_RNDN);
                        rin_inform(loc, "CREATED INTEGER EXPRESSION WITH VAL: %s", abc);
                        mpfr_free_str(abc);
                        return new Bexpression;
                }

                rin_inform(loc, "CREATED INTEGER EXPRESSION WITH VAL: NULL");
                return new Bexpression;
        }

        Bexpression* binary_expression
        (RIN_OPERATOR op, Bexpression* left, Bexpression* right, const Location& loc) override
        {
                rin_inform(loc, "CREATED BINARY EXPRESSION WITH: %s",
                        operator_name(op).c_str());
                delete left;
                delete right;

                return new Bexpression;
        }

        // Statements.

        Bstatement* invalid_statement() override
        {
                rin_inform(File::unknown_location(), "CREATED INVALID STATEMENT\n");
                return new Bstatement;
        }

        Bstatement* var_dec_statement(Bvariable* var) override
        {
                rin_inform(var->location(),
                        "CREATED VARIABLE DECLARATION STMT FOR VAR: %s\n",
                        var->identifier().c_str());
                delete var;
                return new Bstatement;
        }

        Bstatement* inc_statement(Bexpression* var, const Location& loc) override
        {
                rin_inform(loc, "CREATED INC STMT\n");
                delete var;
                return new Bstatement;
        }

        Bstatement* dec_statement(Bexpression* var, const Location& loc) override
        {
                rin_inform(loc, "CREATED DEC STMT\n");
                delete var;
                return new Bstatement;
        }

        Bstatement* expression_statement(Bexpression* expr, const Location& loc) override
        {
                rin_inform(loc, "CREATED EXPRESSION STATEMENT\n");
                delete expr;
                return new Bstatement;
        }

        Bstatement* compound_statement
        (Bstatement* first, Bstatement* second, const Location& loc) override
        {
                rin_inform(loc, "CREATED COMPOUND STATEMENT\n");
                delete first;
                delete second;

                return new Bstatement;
        }

        Bstatement* if_statement
        (Bexpression* expr, Scope* scope, Scope* else_block, const Location& loc) override
        {
                if (else_block)
                        rin_inform(loc, "CREATED IF-ELSE STATEMENT\n");
                else
                        rin_inform(loc, "CREATED IF STATEMENT\n");
                delete expr;
                // Scopes are owned by the If_statement; do not delete here.

                return new Bstatement;
        }

        Bstatement* assignment_statement
        (Bexpression* lhs, Bexpression* rhs, const Location& loc) override
        {
                rin_inform(loc, "CREATED ASSIGNMENT STATEMENT\n");
                delete lhs;
                delete rhs;

                return new Bstatement;
        }

        Bstatement* for_statement
        (Bstatement* ind, Bstatement* cond, Bstatement* inc, Scope* then, const Location& loc) override
        {
                rin_inform(loc, "CREATED FOR STATEMENT\n");
                delete ind;
                delete cond;
                delete inc;
                // Scope owned by For_statement; do not delete here.

                return new Bstatement;
        }

        Bstatement* return_statement(Bexpression* expr, const Location& loc) override
        {
                rin_inform(loc, "CREATED RETURN STATEMENT\n");
                delete expr;
                return new Bstatement;
        }

        Bstatement* function_statement
        (const std::string& name, const std::vector<std::string>& params,
         Scope* body, const Location& loc) override
        {
                rin_inform(loc, "CREATED FUNCTION DECLARATION: %s\n",
                        name.c_str());
                // Scope owned by Function_declaration_statement; do not delete here.
                return new Bstatement;
        }

        Bexpression* call_expression
        (const std::string& name, const std::vector<Bexpression*>& args,
         const Location& loc) override
        {
                rin_inform(loc, "CREATED CALL EXPRESSION: %s\n",
                        name.c_str());
                for (auto itr = args.begin(); itr != args.end(); ++itr)
                        delete *itr;
                return new Bexpression;
        }

        Bstatement* break_statement(const Location& loc) override
        {
                rin_inform(loc, "CREATED BREAK STATEMENT\n");
                return new Bstatement;
        }

        Bstatement* continue_statement(const Location& loc) override
        {
                rin_inform(loc, "CREATED CONTINUE STATEMENT\n");
                return new Bstatement;
        }

        // Scope Signals.

        Scope* enter_scope() override
        {
                rin_inform(File::unknown_location(), "ENTERED SCOPE");
                return Backend::enter_scope();
        }

        Scope* leave_scope() override
        {
                rin_inform(File::unknown_location(), "LEFT SCOPE");
                return Backend::leave_scope();
        }
};

int main(int argc, char** argv)
{
        printf("\n ---- DEBUG TOOL: PARSER ---- \n\n");
        if (argc < 2) {
                rin_fatal_error(File::unknown_location(),
                                "Expected .rin file directory as command line argument");
                rin_inform(File::unknown_location(),
                           "\tSample usage: ./a.out MY_FILE.rin");
                return 0;
        }

        std::string path(argv[1]);
        Parser_backend* be = new Parser_backend;
        Parser parser(path, be);
        parser.parse();

        printf("\n ---- END DEBUG TOOL ----\n\n");

        return 0;
}
