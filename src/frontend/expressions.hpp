// expressions.hpp - Expression AST node types for the Rinto frontend
#ifndef RIN_EXPRESSIONS_HPP
#define RIN_EXPRESSIONS_HPP

#include "backend.hpp"

class Unary_expression;
class Binary_expression;
class Var_expression;
class Conditional_expression;
class Float_expression;
class Integer_expression;
class Call_expression;

// operators.cc
extern int OPERATOR_PRECEDENCE[];

// An expression is a statement's constituent
class Expression
{
public:
        // The type of expression
        enum Expression_classification {
                EXPRESSION_INVALID,  EXPRESSION_UNARY,
                EXPRESSION_BINARY,   EXPRESSION_VAR_REFERENCE,
                EXPRESSION_FLOAT,    EXPRESSION_CONDITIONAL,
                EXPRESSION_INTEGER,  EXPRESSION_CALL
        };

        Expression(Expression_classification cl, const Location& loc)
                : _classification(cl), _location(loc)
        {}

        virtual ~Expression() {}

        // Return the expression's classification
        Expression_classification classification() const
        { return this->_classification; }

        // Return whether the expression is invalid
        bool is_invalid() const
        { return (this->_classification == EXPRESSION_INVALID); }

        // Set classification to invalid
        void set_is_invalid()
        { this->_classification = EXPRESSION_INVALID; }

        Location location() const
        { return this->_location; }

        // Make an invalid expression
        static Expression* make_invalid(const Location& loc);

        // Make unary expression
        static Expression* make_unary
        (RIN_OPERATOR op, Expression* expr, const Location& loc);

        // Make binary expression
        static Expression* make_binary
        (RIN_OPERATOR op, Expression* left, Expression* right, const Location& loc);

        // Make variable reference
        static Expression* make_var_reference(Named_object* var, const Location& loc);

        // Make float expression
        static Expression* make_float(const mpfr_t* val, const Location& loc);

        // Make integer expression
        static Expression* make_integer(const mpfr_t* val, const Location& loc);

        // make conditional expression
        static Expression* make_conditional(Expression* cond, const Location& loc);

        // Make a function call expression
        static Expression* make_call
        (const std::string& name, std::vector<Expression*>& args, const Location& loc);

        // Converts the expression to a unary expression type
        Unary_expression* unary_expression()
        { return this->convert<Unary_expression, EXPRESSION_UNARY>(); }

        // Converts the expression to a binary expression type
        Binary_expression* binary_expression()
        { return this->convert<Binary_expression, EXPRESSION_BINARY>(); }

        // Converts the expression to a var expression type
        Var_expression* var_expression()
        { return this->convert<Var_expression, EXPRESSION_VAR_REFERENCE>(); }

        // Converts the expression to conditional expression type
        Conditional_expression* conditional_expression()
        { return this->convert<Conditional_expression, EXPRESSION_CONDITIONAL>(); }

        // Converts the expression to a float expression type
        Float_expression* float_expression()
        { return this->convert<Float_expression, EXPRESSION_FLOAT>(); }

        // Converts the expression to an integer expression type
        Integer_expression* integer_expression()
        { return this->convert<Integer_expression, EXPRESSION_INTEGER>(); }

        // Converts the expression to a call expression type
        Call_expression* call_expression()
        { return this->convert<Call_expression, EXPRESSION_CALL>(); }

        // Returns the backend representation of the expression
        Bexpression* get_backend(Backend* backend)
        { return this->do_get_backend(backend); }

protected:
        virtual Bexpression* do_get_backend(Backend*) = 0;

private:
        Expression_classification _classification;
        Location                  _location;

        // Convert an expression to the specified type
        template
        <typename Expression_class, Expression_classification expr_classification>
        Expression_class* convert()
        {
                return (this->_classification == expr_classification
                        ? static_cast<Expression_class*>(this)
                        : NULL);
        }

        template
        <typename Expression_class, Expression_classification expr_classification>
        const Expression_class* convert() const
        {
                return (this->_classification == expr_classification
                        ? static_cast<const Expression_class*>(this)
                        : NULL);
        }
};

// An invalid expression is an expression which should resolve into an error
class Invalid_expression : public Expression
{
public:
        explicit Invalid_expression(const Location& loc)
                : Expression(EXPRESSION_INVALID, loc)
        {}

protected:
        Bexpression* do_get_backend(Backend* backend) override;
};

/*
 * A unary expression represents an operation with one operand
 * i.e: myVar++, myVar--, etc.
 */
class Unary_expression : public Expression
{
public:
        Unary_expression(RIN_OPERATOR op, Expression* expr, const Location& loc)
                : Expression(EXPRESSION_UNARY, loc),
                  _op(op), _expr(expr)
        { RIN_ASSERT(expr); }

        ~Unary_expression() override
        { delete this->_expr; }

        // Return the operand
        Expression* operand() const
        { return this->_expr; }

        // Return the operator
        RIN_OPERATOR op() const
        { return this->_op; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        RIN_OPERATOR _op;
        Expression*  _expr;
};

/*
 * A binary expression represents an operation with two operands
 * i.e: myVarA + myVarB , myVarA % myVarB
 */
class Binary_expression : public Expression
{
public:
        Binary_expression
        (RIN_OPERATOR op, Expression* left, Expression* right, const Location& loc)
                : Expression(EXPRESSION_BINARY, loc),
                  _op(op), _left(left), _right(right)
        {}

        ~Binary_expression() override;

        // Return the operator
        RIN_OPERATOR op()
        { return this->_op; }

        // Return the left hand expression
        Expression* left()
        { return this->_left; }

        // Return the right hand expression
        Expression* right()
        { return this->_right; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        RIN_OPERATOR _op;
        Expression* _left;
        Expression* _right;
};

// A var_expression is a reference to a variable
class Var_expression : public Expression
{
public:
        Var_expression(Named_object* variable, const Location& loc)
                : Expression(EXPRESSION_VAR_REFERENCE, loc),
                  _variable(variable)
        {}

        ~Var_expression() override {}

        // Return the variable
        Named_object* named_object() const
        { return this->_variable; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        Named_object* _variable;
};

// A conditional expression evaluates to 0 (false) or non-zero (true)
class Conditional_expression : public Expression
{
public:
        Conditional_expression(Expression* cond, const Location& loc)
                : Expression(EXPRESSION_CONDITIONAL, loc),
                  _cond(cond)
        {}

        ~Conditional_expression() override
        { delete this->_cond;}

        // Return the condition
        Expression* condition()
        { return this->_cond; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        Expression* _cond;
};

// A float expression, i.e: 3.01 or 3f
class Float_expression : public Expression
{
public:
        Float_expression(const mpfr_t* val, const Location& loc)
                : Expression(EXPRESSION_FLOAT, loc)
        {
                RIN_ASSERT(val);
                mpfr_init_set(this->_val, *val, MPFR_RNDN);
        }

        ~Float_expression() override
        { mpfr_clear(_val); }

        // Whether the current value is equivalent to zero.
        bool is_zero_value() const
        {
                return mpfr_zero_p(this->_val) != 0
                        && mpfr_signbit(this->_val) == 0;
        }

        // Return a pointer to the mpfr_t float value.
        mpfr_t* value()
        { return &_val; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        mpfr_t _val;
};

// An integer expression, i.e: 42
class Integer_expression : public Expression
{
public:
        Integer_expression(const mpfr_t* val, const Location& loc)
                : Expression(EXPRESSION_INTEGER, loc)
        {
                RIN_ASSERT(val);
                mpfr_init_set(this->_val, *val, MPFR_RNDN);
        }

        ~Integer_expression() override
        { mpfr_clear(_val); }

        // Return a pointer to the mpfr_t integer value.
        mpfr_t* value()
        { return &_val; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        mpfr_t _val;
};

// A function call expression, i.e: myFunc(a, b)
class Call_expression : public Expression
{
public:
        Call_expression
        (const std::string& name, std::vector<Expression*>& args, const Location& loc)
                : Expression(EXPRESSION_CALL, loc),
                  _name(name), _args(args)
        {}

        ~Call_expression() override
        {
                for (auto itr = _args.begin(); itr != _args.end(); ++itr)
                        delete *itr;
        }

        // Return the function name
        const std::string& name() const
        { return this->_name; }

        // Return the arguments
        const std::vector<Expression*>& args() const
        { return this->_args; }

protected:
        Bexpression* do_get_backend(Backend* backend) override;

private:
        std::string _name;
        std::vector<Expression*> _args;
};

#endif // RIN_EXPRESSIONS_HPP
