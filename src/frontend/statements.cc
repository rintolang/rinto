// statements.cc - Statement factory methods and backend code generation
#include "statements.hpp"

Statement* Statement::make_invalid(const Location& loc)
{ return new Invalid_statement(loc); }

Statement* Statement::make_variable_declaration(Named_object* var)
{ return new Variable_declaration_statement(var); }

Statement* Statement::make_assignment
(Expression* lhs, Expression* rhs, const Location& loc)
{ return new Assignment_statement(lhs, rhs, loc); }

Statement* Statement::make_inc(Expression* expr)
{ return new Inc_dec_statement(expr, true); }

Statement* Statement::make_dec(Expression* expr)
{ return new Inc_dec_statement(expr, false); }

Statement* Statement::make_if
(Expression* cond, Scope* then_block, const Location& loc)
{ return new If_statement(cond, then_block, loc); }

Statement* Statement::make_for
(Statement* ind, Statement* cond, Statement* inc, const Location& loc)
{ return new For_statement(ind, cond, inc, loc); }

Statement* Statement::make_expression(Expression* expr, const Location& loc)
{ return new Expression_statement(expr, loc); }

Statement* Statement::make_compound
(Statement* first, Statement* second, const Location& loc)
{ return new Compound_statement(first, second, loc); }

Statement* Statement::make_return(Expression* expr, const Location& loc)
{ return new Return_statement(expr, loc); }

Statement* Statement::make_break(const Location& loc)
{ return new Break_statement(loc); }

Statement* Statement::make_continue(const Location& loc)
{ return new Continue_statement(loc); }

Statement* Statement::make_function
(const std::string& name, const std::vector<std::string>& params,
 Scope* body, const Location& loc)
{ return new Function_declaration_statement(name, params, body, loc); }

// Assignment_statement implementation

Assignment_statement::~Assignment_statement()
{
        delete this->_lhs;
        delete this->_rhs;
}

Bstatement* Assignment_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->lhs() != NULL && this->rhs() != NULL);

        // Left-hand side of assignment statement must be a variable reference.
        Expression* lhs_expr = this->lhs();
        RIN_ASSERT(lhs_expr->classification() == Expression::EXPRESSION_VAR_REFERENCE);

        // Right-hand side can be float, integer, binary, unary, or var reference.
        Expression* rhs_expr = this->rhs();
        Expression::Expression_classification rhs_c = rhs_expr->classification();
        RIN_ASSERT(rhs_c == Expression::EXPRESSION_FLOAT   ||
                   rhs_c == Expression::EXPRESSION_INTEGER  ||
                   rhs_c == Expression::EXPRESSION_BINARY   ||
                   rhs_c == Expression::EXPRESSION_UNARY    ||
                   rhs_c == Expression::EXPRESSION_VAR_REFERENCE);

        /*
         * Build underlying expressions. Running get_backend()
         * on lhs_expr will check whether the variable reference
         * is defined on the current scope.
         */
        Bexpression* blhs = lhs_expr->get_backend(backend);
        Bexpression* brhs = rhs_expr->get_backend(backend);

        // Build assignment statement.
        return backend->assignment_statement(blhs, brhs, this->location());
}

// Variable_declaration_statement implementation

const std::string& Variable_declaration_statement::identifier() const
{
        RIN_ASSERT(this->_var);
        return this->_var->identifier();
}

Bstatement* Variable_declaration_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->var());

        Bvariable* bvar = backend->variable(this->var());
        return backend->var_dec_statement(bvar);
}

// If_statement implementation

If_statement::~If_statement()
{
        delete this->_cond;
        delete this->_else_block;
}

Bstatement* If_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->condition());
        RIN_ASSERT(this->then_block());

        // Condition expression must be conditional.
        Expression::Expression_classification cl =
                this->condition()->classification();
        RIN_ASSERT(cl == Expression::EXPRESSION_CONDITIONAL);

        // Build backend conditional expression.
        Bexpression* cond = this->condition()->get_backend(backend);

        // Build backend if-statement.
        return backend->if_statement(cond, this->then_block(),
                this->else_block(), this->location());
}

// For_statement implementation

For_statement::~For_statement()
{
        delete this->_ind;
        delete this->_cond;
        delete this->_inc;

        // _statements is owned by For_statement, set via add_statements().
        // The Scope* is not managed by Backend's scope stack after
        // leave_scope() is called in parse_for_statement/parse_while_statement.
        delete this->_statements;
}

Bstatement* For_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);

        Bstatement* bind  = (this->_ind)  ? this->_ind->get_backend(backend)  : NULL;
        Bstatement* bcond = (this->_cond) ? this->_cond->get_backend(backend) : NULL;
        Bstatement* binc  = (this->_inc)  ? this->_inc->get_backend(backend)  : NULL;

        return backend->for_statement(bind, bcond, binc,
                this->statements(), this->location());
}

// Inc_dec_statement implementation

Bstatement* Inc_dec_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->expr());

        // Expression must be unary.
        Expression::Expression_classification cl =
                this->expr()->classification();
        RIN_ASSERT(cl == Expression::EXPRESSION_UNARY);

        // Build expression.
        Bexpression* bexpr = this->expr()->get_backend(backend);

        return (this->is_inc()) ? backend->inc_statement(bexpr, this->location()) :
                backend->dec_statement(bexpr, this->location());
}

// Expression_statement implementation

Bstatement* Expression_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->expr());

        // Assert that the underlying statements is not invalid.
        RIN_ASSERT(this->expr()->classification() !=
                Expression::EXPRESSION_INVALID);

        // Build the expression.
        Bexpression* bexpr = this->expr()->get_backend(backend);
        RIN_ASSERT(bexpr);

        // Build the expression statement.
        return backend->expression_statement(bexpr, this->location());
}

// Compound_statement implementation

Compound_statement::~Compound_statement()
{
        delete _first;
        delete _second;
}

Bstatement* Compound_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->first());
        RIN_ASSERT(this->second());

        // Statements are not invalid.
        if (this->first()->classification() == STATEMENT_INVALID ||
            this->second()->classification() == STATEMENT_INVALID) {
                return backend->invalid_statement();
        }

        // Build statements.
        Bstatement* bfirst = this->first()->get_backend(backend);
        Bstatement* bsecond = this->second()->get_backend(backend);
        RIN_ASSERT(bfirst != NULL && bsecond != NULL);

        // Build backend compound statement.
        return backend->compound_statement(bfirst, bsecond, this->location());
}

Statement* Compound_statement::operator[](unsigned int i)
{
        switch(i) {
        case 0: return _first;
        case 1: return _second;
        default:
                // Cannot index more than 2 expressions.
                RIN_UNREACHABLE();
                return NULL;
        }
}

// Return_statement implementation

Bstatement* Return_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);

        Bexpression* bexpr = NULL;
        if (this->expr())
                bexpr = this->expr()->get_backend(backend);

        return backend->return_statement(bexpr, this->location());
}

// Function_declaration_statement implementation

Bstatement* Function_declaration_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);

        return backend->function_statement(this->name(), this->params(),
                this->body(), this->location());
}

// Break_statement implementation

Bstatement* Break_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        return backend->break_statement(this->location());
}

// Continue_statement implementation

Bstatement* Continue_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        return backend->continue_statement(this->location());
}
