// statements.hpp - Statement AST node types for the Rinto frontend
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
class Return_statement;
class Function_declaration_statement;
class Break_statement;
class Continue_statement;

// A statement is the highest programming abstraction in rinto.
class Statement
{
public:
        // The statement type
        enum Statement_classification {
                STATEMENT_INVALID,    STATEMENT_VARIABLE_DECLARATION,
                STATEMENT_ASSIGNMENT, STATEMENT_INCDEC,
                STATEMENT_IF,         STATEMENT_FOR,
                STATEMENT_EXPRESSION, STATEMENT_COMPOUND,
                STATEMENT_RETURN,     STATEMENT_FUNCTION,
                STATEMENT_BREAK,      STATEMENT_CONTINUE
        };

        Statement(Statement_classification cl, const Location& loc)
                : _classification(cl), _location(loc)
        {}

        virtual ~Statement() {}

        // Get the statement's classification
        Statement_classification classification() const
        { return this->_classification; }

        // Get the statement's position
        Location location() const
        { return this->_location; }

        // Change location.
        void set_location(const Location& loc)
        { this->_location = loc; }

        // Set whether this statement is invalid
        void set_is_invalid()
        { this->_classification = STATEMENT_INVALID; }

        // Return whether the statement is valid
        bool is_invalid() const
        { return (this->_classification == STATEMENT_INVALID); }

        // Make an invalid statement which resolves to a compiler error
        static Statement* make_invalid(const Location& loc);

        // Make a variable declaration statement
        static Statement* make_variable_declaration(Named_object* var);

        // Make an assignment statement
        static Statement* make_assignment
        (Expression* lhs, Expression* rhs, const Location& loc);

        // Increment/decrement statements
        static Statement* make_inc(Expression* expr);
        static Statement* make_dec(Expression* expr);

        // If statement that calls then_block if cond resolves to true
        static Statement* make_if
        (Expression* cond, Scope* then_block, const Location& loc);

        // Make a for statement
        static Statement* make_for
        (Statement* ind, Statement* cond, Statement* inc, const Location& loc);

        // Make an expression statement
        static Statement* make_expression(Expression* expr, const Location& loc);

        static Statement* make_compound
        (Statement* first, Statement* second, const Location& loc);

        // Make a return statement (expr may be NULL for void return)
        static Statement* make_return(Expression* expr, const Location& loc);

        // Make a function declaration statement
        static Statement* make_function
        (const std::string& name, const std::vector<std::string>& params,
         Scope* body, const Location& loc);

        // Make a break statement
        static Statement* make_break(const Location& loc);

        // Make a continue statement
        static Statement* make_continue(const Location& loc);

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

        Return_statement* return_statement()
        { return this->convert<Return_statement, STATEMENT_RETURN>(); }

        Function_declaration_statement* function_declaration_statement()
        { return this->convert<Function_declaration_statement, STATEMENT_FUNCTION>(); }

        Break_statement* break_statement()
        { return this->convert<Break_statement, STATEMENT_BREAK>(); }

        Continue_statement* continue_statement()
        { return this->convert<Continue_statement, STATEMENT_CONTINUE>(); }

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
        explicit Invalid_statement(const Location& loc)
                : Statement(STATEMENT_INVALID, loc)
        {}

protected:
        Bstatement* do_get_backend(Backend* backend) override
        { RIN_UNREACHABLE(); }
};

// An assignment statement is for assigning a value to a variable
class Assignment_statement : public Statement
{
public:
        Assignment_statement(Expression* lhs, Expression* rhs, const Location& loc)
                : Statement(STATEMENT_ASSIGNMENT, loc),
                  _lhs(lhs), _rhs(rhs)
        {}

        ~Assignment_statement() override;

        // Return left hand side of assignment
        Expression* lhs() const
        { return this->_lhs; }

        // Return right hand side of assignment
        Expression* rhs() const
        { return this->_rhs; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

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
        explicit Variable_declaration_statement(Named_object* var)
                : Statement(STATEMENT_VARIABLE_DECLARATION, var->location()),
                  _var(var)
        {}

        ~Variable_declaration_statement() override {}

        Named_object* var()
        { return this->_var; }

        const std::string& identifier() const;

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        // Not owned: Named_object is owned by the Scope's ident_map.
        Named_object* _var;
};

// If statement with optional else block
class If_statement : public Statement
{
public:
        If_statement(Expression* cond, Scope* then_block, const Location& loc)
                : Statement(STATEMENT_IF, loc), _cond(cond),
                  _then_block(then_block)
        {}

        ~If_statement() override;

        Expression* condition() const
        { return this->_cond; }

        Scope* then_block() const
        { return this->_then_block; }

        Scope* else_block() const
        { return this->_else_block; }

        void set_else_block(Scope* else_block)
        { this->_else_block = else_block; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        Expression* _cond;         // Owned: condition expression
        Scope*      _then_block;   // Not owned: managed by Backend scope stack
        Scope*      _else_block = NULL;  // Owned if set: deleted in destructor
};

// For-loop statement
class For_statement : public Statement
{
public:
        For_statement
        (Statement* ind, Statement* cond, Statement* inc, const Location& loc)
                : Statement(STATEMENT_FOR, loc),
                  _ind(ind), _cond(cond), _inc(inc)
        {}

        ~For_statement() override;

        void add_statements(Scope* statements)
        { this->_statements = statements; }

        Statement* ind()
        { return this->_ind; }

        Statement* cond()
        { return this->_cond; }

        Statement* inc()
        { return this->_inc; }

        Scope* statements()
        { return this->_statements; }

        bool has_statements()
        { return (this->_statements != NULL); }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        // Owned: induction, condition, increment statements (may be NULL).
        Statement* _ind;
        Statement* _cond;
        Statement* _inc;

        // Owned: the loop body scope, set via add_statements() (may be NULL).
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

        ~Inc_dec_statement() override
        { delete this->_expr; }

        bool is_inc()
        { return this->_is_inc; }

        Expression* expr()
        { return this->_expr; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

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
        Expression_statement(Expression* expr, const Location& loc)
                : Statement(STATEMENT_EXPRESSION, loc),
                  _expr(expr)
        {}

        ~Expression_statement() override
        { delete this->_expr; }

        Expression* expr()
        { return this->_expr; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

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
        Compound_statement(Statement* first, Statement* second, const Location& loc)
                : Statement(STATEMENT_COMPOUND, loc)
        {
                RIN_ASSERT(first);
                RIN_ASSERT(second);
                _first  = first;
                _second = second;
        }

        ~Compound_statement() override;

        // Return the first statement.
        Statement* first()
        { return this->_first; }

        // Return the second statement.
        Statement* second()
        { return this->_second; }

        // Retrieve statements via the index operator.
        Statement* operator[](unsigned int i);

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        Statement* _first;
        Statement* _second;
};

// A return statement optionally returns an expression value
class Return_statement : public Statement
{
public:
        Return_statement(Expression* expr, const Location& loc)
                : Statement(STATEMENT_RETURN, loc),
                  _expr(expr)
        {}

        ~Return_statement() override
        { delete this->_expr; }

        // Return the expression (may be NULL for void return)
        Expression* expr() const
        { return this->_expr; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        Expression* _expr;
};

// A function declaration statement
class Function_declaration_statement : public Statement
{
public:
        Function_declaration_statement
        (const std::string& name, const std::vector<std::string>& params,
         Scope* body, const Location& loc)
                : Statement(STATEMENT_FUNCTION, loc),
                  _name(name), _params(params), _body(body)
        {}

        ~Function_declaration_statement() override
        { delete this->_body; }

        // Return the function name
        const std::string& name() const
        { return this->_name; }

        // Return the parameter names
        const std::vector<std::string>& params() const
        { return this->_params; }

        // Return the function body scope
        Scope* body() const
        { return this->_body; }

protected:
        Bstatement* do_get_backend(Backend* backend) override;

private:
        std::string _name;
        std::vector<std::string> _params;
        Scope* _body;  // Owned: function body scope, deleted in destructor
};

// A break statement exits the innermost loop
class Break_statement : public Statement
{
public:
        explicit Break_statement(const Location& loc)
                : Statement(STATEMENT_BREAK, loc)
        {}

protected:
        Bstatement* do_get_backend(Backend* backend) override;
};

// A continue statement skips to the next iteration of the innermost loop
class Continue_statement : public Statement
{
public:
        explicit Continue_statement(const Location& loc)
                : Statement(STATEMENT_CONTINUE, loc)
        {}

protected:
        Bstatement* do_get_backend(Backend* backend) override;
};

#endif // RIN_STATEMENTS_HPP
