#ifndef RIN_STATEMENTS_HPP
#define RIN_STATEMENTS_HPP

#include "backend.hpp"
#include "expressions.hpp"

class Invalid_statement;
class Assignment_statement;
class Variable_declaration_statement;
class If_statement;
class For_statement;
class Inc_dec_statement;
class Expression_statement;
class Compound_statement;

// A statement is the highest programming abstraction in rinto.
class Statement
{
public:
        // The statement type
        enum Statement_classification {
                STATEMENT_INVALID,    STATEMENT_VARIABLE_DECLARATION,
                STATEMENT_ASSIGNMENT, STATEMENT_INCDEC,
                STATEMENT_IF,         STATEMENT_FOR,
                STATEMENT_EXPRESSION, STATEMENT_COMPOUND
        };

        Statement(Statement_classification cl, Location loc)
                : _classification(cl), _location(loc)
        {}

        virtual ~Statement() {}

        // Get the statement's classification
        Statement_classification classification()
        { return this->_classification; }

        // Get the statement's position
        Location location() const
        { return this->_location; }

        // Set whether this statement is invalid
        void set_is_invalid()
        { this->_classification = STATEMENT_INVALID; }

        // Return whether the statement is valid
        bool is_invalid() const
        { return (this->_classification == STATEMENT_INVALID); }

        // Make an invalid statement which resolves to a compiler error
        static Statement* make_invalid(Location loc);

        // Make a variable declaration statement
        static Statement* make_variable_declaration(Named_object* var);

        // Make an assignment statement
        static Statement* make_assignment
        (Expression* lhs, Expression* rhs, Location loc);

        // Increment/decrement statements
        static Statement* make_inc(Expression* expr);
        static Statement* make_dec(Expression* expr);

        // If statement that calls then_block if cond resolves to true
        static Statement* make_if
        (Expression* cond, Scope* then_block, Location loc);

        // Make a for statement
        static Statement* make_for(Scope* ind_scope, Location loc);

        // Make an expression statement
        static Statement* make_expression(Expression* expr, Location loc);

        static Statement* make_compound
        (Statement* first, Statement* second, Location loc);

        // Cast statements to their higher-order types
        Invalid_statement* invalid_statement()
        { return this->convert<Invalid_statement, STATEMENT_INVALID>(); }

        Assignment_statement* assignment_statement()
        { return this->convert<Assignment_statement, STATEMENT_ASSIGNMENT>(); }

        Variable_declaration_statement* variable_declaration_statement()
        {
                return this->convert
                <Variable_declaration_statement, STATEMENT_VARIABLE_DECLARATION>
                        ();
        }

        If_statement* if_statement()
        { return this->convert<If_statement, STATEMENT_IF>(); }

        For_statement* for_statement()
        { return this->convert<For_statement, STATEMENT_FOR>(); }

        Inc_dec_statement* inc_dec_statement()
        { return this->convert<Inc_dec_statement, STATEMENT_INCDEC>(); }

        Expression_statement* expression_statement()
        { return this->convert<Expression_statement, STATEMENT_EXPRESSION>(); }

        Compound_statement* compound_statement()
        { return this->convert<Compound_statement, STATEMENT_COMPOUND>(); }

        // Return the backend representation of the statement
        Bstatement* get_backend(Backend* backend)
        { return this->do_get_backend(backend); }

protected:
        virtual Bstatement* do_get_backend(Backend*) = 0;

private:
        Statement_classification _classification;
        Location                 _location;

        // Converts a statement to its higher-order class
        template<typename Statement_class, Statement_classification sc>
        Statement_class* convert()
        {
                return (this->_classification == sc
                        ? static_cast<Statement_class*>(this)
                        : NULL);
        }

        template<typename Statement_class, Statement_classification sc>
        const Statement_class* convert() const
        {
                return (this->_classification == sc
                        ? static_cast<const Statement_class*>(this)
                        : NULL);
        }
};

// An invalid statement is a statement which should resolve into an error
class Invalid_statement : public Statement
{
public:
        Invalid_statement(Location loc)
                : Statement(STATEMENT_INVALID, loc)
        {}

protected:
        Bstatement* do_get_backend(Backend* backend)
        { RIN_UNREACHABLE(); }
};

// An assignment statement is for assigning a value to a variable
class Assignment_statement : public Statement
{
public:
        Assignment_statement(Expression* lhs, Expression* rhs, Location loc)
                : Statement(STATEMENT_ASSIGNMENT, loc),
                  _lhs(lhs), _rhs(rhs)
        {}

        ~Assignment_statement();

        // Return left hand side of assignment
        Expression* lhs() const
        { return this->_lhs; }

        // Return right hand side of assignment
        Expression* rhs() const
        { return this->_rhs; }

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        // _lhs is a variable reference
        Expression* _lhs;

        // rhs is a binary or compound expression
        Expression* _rhs;
};

// A statement which declares a new variable
class Variable_declaration_statement : public Statement
{
public:
        Variable_declaration_statement(Named_object* var)
                : Statement(STATEMENT_VARIABLE_DECLARATION, var->location()),
                  _var(var)
        {}

        ~Variable_declaration_statement() {}

        Named_object* var()
        { return this->_var; }

        const std::string identifier();

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        Named_object* _var;
};

// If statement
class If_statement : public Statement
{
public:
        If_statement(Expression* cond, Scope* then_block, Location loc)
                : Statement(STATEMENT_IF, loc), _cond(cond),
                  _then_block(then_block)
        {}

        ~If_statement();

        Expression* condition() const
        { return this->_cond; }

        Scope* then_block() const
        { return this->_then_block; }

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        Expression* _cond;
        Scope*      _then_block;
};

// For-loop statement
class For_statement : public Statement
{
public:
        For_statement(Scope* ind_scope, Location loc)
                : Statement(STATEMENT_FOR, loc),
                  _ind_scope(ind_scope)
        {}

        ~For_statement() {}

        void add_statements(Scope* statements)
        { this->_statements = statements; }

        Scope* ind_scope()
        { return this->_ind_scope; }

        Scope* statements()
        { return this->_statements; }

        bool has_statements()
        { return (this->_statements != NULL); }

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        Scope* _ind_scope;

        // The statements to execute
        Scope* _statements = NULL;
};

// An increment (++) or decrement (--) statement.
class Inc_dec_statement : public Statement
{
public:
        Inc_dec_statement(Expression* expr, bool is_inc)
                : Statement(STATEMENT_INCDEC, expr->location()),
                  _is_inc(is_inc), _expr(expr)
        {}

        ~Inc_dec_statement()
        { delete this->_expr; }

        bool is_inc()
        { return this->_is_inc; }

        Expression* expr()
        { return this->_expr; }

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        bool        _is_inc;
        Expression* _expr;
};

/*
 * An expression statement is a standalone expression that's
 * evaluated as a statement. It may be used as the conditional
 * statement in a for-loop.
 */
class Expression_statement : public Statement
{
public:
        Expression_statement(Expression* expr, Location loc)
                : Statement(STATEMENT_EXPRESSION, loc),
                  _expr(expr)
        {}

        ~Expression_statement()
        { delete this->_expr; }

        Expression* expr()
        { return this->_expr; }

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        Expression* _expr;

};

/*
 * A compound statement represents a pair of statements
 * 'first' and 'second' that will be evaluated consecutively.
 */
class Compound_statement : public Statement
{
public:
        Compound_statement(Statement* first, Statement* second, Location loc)
                : Statement(STATEMENT_COMPOUND, loc)
        {
                RIN_ASSERT(first);
                RIN_ASSERT(second);
                _first  = first;
                _second = second;
        }

        ~Compound_statement();

        // Return the first statement.
        Statement* first()
        { return this->_first; }

        // Return the second statement.
        Statement* second()
        { return this->_second; }

        // Retrieve statements via the index operator.
        Statement* operator[](unsigned int i);

protected:
        Bstatement* do_get_backend(Backend* backend);

private:
        Statement* _first;
        Statement* _second;
};

#endif // RIN_STATEMENTS_HPP
