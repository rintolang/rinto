#include "statements.hpp"

Statement* Statement::make_invalid(Location loc)
{ return new Invalid_statement(loc); }

Statement* Statement::make_variable_declaration(Named_object* var)
{ return new Variable_declaration_statement(var); }

Statement* Statement::make_assignment
(Expression* lhs, Expression* rhs, Location loc)
{ return new Assignment_statement(lhs, rhs, loc); }

Statement* Statement::make_inc(Expression* expr)
{ return new Inc_dec_statement(expr, true); }

Statement* Statement::make_dec(Expression* expr)
{ return new Inc_dec_statement(expr, false); }

Statement* Statement::make_if
(Expression* cond, Scope* then_block, Location loc)
{ return new If_statement(cond, then_block, loc); }

Statement* Statement::make_for
(Statement* ind_var, Statement* cond, Statement* inc, Location loc)
{ return new For_statement(ind_var, cond, inc, loc); }

Statement* Statement::make_expression(Expression* expr, Location loc)
{ return new Expression_statement(expr, loc); }

Statement* Statement::make_compound
(Statement* first, Statement* second, Location loc)
{ return new Compound_statement(first, second, loc); }

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
        Expression* lhs = this->lhs();
        RIN_ASSERT(lhs->classification() == Expression::EXPRESSION_VAR_REFERENCE);

        // Right-hand side can be float, binary, unary, or var reference.
        Expression* rhs = this->rhs();
        Expression::Expression_classification rhs_c = rhs->classification();
        RIN_ASSERT(rhs_c == Expression::EXPRESSION_FLOAT  ||
                   rhs_c == Expression::EXPRESSION_BINARY ||
                   rhs_c == Expression::EXPRESSION_UNARY  ||
                   rhs_c == Expression::EXPRESSION_VAR_REFERENCE);

        /*
         * Build underlying expressions. Running get_backend()
         * on lhs will check whether the variable reference
         * is defined on the current scope.
         */
        Bexpression* blhs = lhs->get_backend(backend);
        Bexpression* brhs = rhs->get_backend(backend);

        // Build assignment statement.
        return backend->assignment_statement(blhs, brhs, this->location());
}

// Variable_declaration_statement implementation

const std::string Variable_declaration_statement::identifier()
{
        RIN_ASSERT(this->_var);
        return this->_var->identifier();
}

Bstatement* Variable_declaration_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->var());

        // Assert that the variable is not already defined.
        RIN_ASSERT(backend->current_scope());
        if (backend->current_scope()->is_defined(this->identifier())) {
                rin_error_at(this->var()->location(), "Redefinition of '%s'",
                        &this->identifier()[0]);
                return backend->invalid_statement();
        }

        // Define the variable.
        backend->current_scope()->define_obj(this->identifier(),
                this->location());

        // Build the variable.
        Bvariable* var = backend->variable(this->var());

        // Build the variable declaration statement.
        return backend->var_dec_statement(var);
}

// If_statement implementation

If_statement::~If_statement()
{ delete this->_cond; }

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
                this->location());
}

// For_statement implementation

For_statement::~For_statement()
{
        delete this->_ind_var;
        delete this->_cond;
        delete this->_inc;
}

Bstatement* For_statement::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->has_statements());

        backend->enter_scope(this->statements());

        /*
         * Condition statement may be NULL.
         *     if not NULL, then it must be conditional.
         */
        Statement* cond_stmt = this->cond();
        Bstatement* cond_bstmt = NULL;
        if (cond_stmt != NULL) {
                /*
                 * The conditional expression is passed as an expression
                 * statement. An assertion will fail if it's not an
                 * expression statement.
                 */
                Expression_statement* cond_expr = cond_stmt->expression_statement();
                RIN_ASSERT(cond_expr->expr());

                // Assert that it's conditional.
                RIN_ASSERT(cond_expr->expr()->classification() ==
                           Expression::EXPRESSION_CONDITIONAL);

                cond_bstmt = cond_stmt->get_backend(backend);
        }

        /*
         * Increment statement may be NULL.
         *     if not NULL, then it must strictly be increment/decrement.
         */
        Statement* inc_stmt = this->inc();
        Bstatement* inc_bstmt = NULL;
        if (inc_stmt != NULL) {
                RIN_ASSERT(inc_stmt->classification() == STATEMENT_INCDEC);
                inc_bstmt = inc_stmt->get_backend(backend);
        }

        backend->leave_scope();

        // Build backend for-loop statement.
        return backend->for_statement(cond_bstmt, inc_bstmt,
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

        if (this->is_inc())
                return backend->inc_statement(bexpr);

        return backend->dec_statement(bexpr);
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
        Bstatement* first = this->first()->get_backend(backend);
        Bstatement* second = this->second()->get_backend(backend);
        RIN_ASSERT(first != NULL && second != NULL);

        // Build backend compound statement.
        return backend->compound_statement(first, second, this->location());
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
