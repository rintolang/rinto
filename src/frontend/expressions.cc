#include "expressions.hpp"

Expression* Expression::make_invalid(Location loc)
{ return new Invalid_expression(loc); }

Expression* Expression::make_unary
(RIN_OPERATOR op, Expression* expr, Location loc)
{ return new Unary_expression(op, expr, loc); }

Expression* Expression::make_binary
(RIN_OPERATOR op, Expression* left, Expression* right, Location loc)
{ return new Binary_expression(op, left, right, loc); }

Expression* Expression::make_var_reference(Named_object* var, Location loc)
{ return new Var_expression(var, loc); }

Expression* Expression::make_conditional(Expression* cond, Location loc)
{ return new Conditional_expression(cond, loc); }

Expression* Expression::make_float(const mpfr_t* val, Location loc)
{ return new Float_expression(val, loc); }

// Invalid_expression implementation:

Bexpression* Invalid_expression::do_get_backend(Backend* backend)
{ RIN_UNREACHABLE(); }

// Unary_expression implementation:

Bexpression* Unary_expression::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);

        /*
         * Can only perform a unary operation with an increment or decrement
         * operator.
         */
        RIN_ASSERT(this->op() == OPER_INC || this->op() == OPER_DEC);

        // Can only perform a unary operation with a var reference
        if (this->operand()->classification() != Expression::EXPRESSION_VAR_REFERENCE) {
                rin_error_at(this->location(), "lvalue required as increment/decrement operand");
                return backend->invalid_expression();
        }

        Bexpression* expr = this->operand()->get_backend(backend);
        RIN_ASSERT(expr);

        return backend->unary_expression
                (this->op(), expr, this->location());
}

// Binary_expression implementation:

Binary_expression::~Binary_expression()
{
        delete _left;
        delete _right;
}

Bexpression* Binary_expression::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->left() != NULL && this->right() != NULL);

        /*
         * See operators.cc. If the precedence of the operator
         * is less than or equal to -1 then it cant be used
         * in a binary expression.
         */
        RIN_ASSERT(OPERATOR_PRECEDENCE[this->op()] > -1);

        /*
         * Binary expressions can only be formed from float, unary,
         * binary, or var reference children.
         */
        Expression_classification left_c = this->left()->classification();
        RIN_ASSERT(left_c == EXPRESSION_FLOAT || left_c == EXPRESSION_BINARY ||
                   left_c == EXPRESSION_UNARY || left_c == EXPRESSION_VAR_REFERENCE);

        Expression_classification right_c = this->right()->classification();
        RIN_ASSERT(right_c == EXPRESSION_FLOAT || right_c == EXPRESSION_BINARY ||
                   right_c == EXPRESSION_UNARY || right_c == EXPRESSION_VAR_REFERENCE);

        // Create backend expressions
        Bexpression* left = this->left()->get_backend(backend);
        Bexpression* right = this->right()->get_backend(backend);
        RIN_ASSERT(left != NULL && right != NULL);

        return backend->binary_expression(this->op(), left,
                right, this->location());
}

// Var_expression implementation:

Bexpression* Var_expression::do_get_backend(Backend* backend) {
        RIN_ASSERT(backend);

        Named_object* obj = this->named_object();
        RIN_ASSERT(obj);

        // Create the backend variable reference.
        Bvariable* var = backend->variable(obj);
        return backend->var_reference(var, this->location());
}

// Conditional_expression implementation:

Bexpression* Conditional_expression::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(this->condition());

        // Must be of type float, binary, unary, or var reference.
        Expression_classification cls = this->condition()->classification();
        RIN_ASSERT(cls == EXPRESSION_FLOAT || cls == EXPRESSION_BINARY ||
                   cls == EXPRESSION_UNARY || cls == EXPRESSION_VAR_REFERENCE);

        // Build backend expression.
        Bexpression* cond = this->condition()->get_backend(backend);
        RIN_ASSERT(cond);

        // Create backend conditional expression
        return backend->conditional_expression(cond, this->location());
}

// Float_expression implementation

Bexpression* Float_expression::do_get_backend(Backend* backend)
{
        RIN_ASSERT(backend);

        /*
         * Float expression implementation must copy this->value()
         * otherwise it will be deleted once this expression is
         * deallocated.
         */
        return backend->float_expression(this->value(), this->location());
}

