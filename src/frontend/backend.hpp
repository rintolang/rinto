#ifndef RIN_BACKEND_HPP
#define RIN_BACKEND_HPP

// MPFR must remain first
#include <mpfr.h>

#include <rin-system.hpp>

#include <string>
#include <stack>
#include <regex>
#include <deque>
#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include "operators.hpp"

#define RIN_ASSERT(EXPR)  BE_ASSERT(EXPR)
#define RIN_UNREACHABLE() BE_UNREACHABLE()

class Bexpression;
class Bstatement;
class Bvariable;

/*
 * Implements deleting a statement. Otherwise, deleting an incomplete type
 * may cause undefined behavior.
 */
extern void delete_stmt(Bstatement* stmt);

// A location represents a position in a file.
struct Location {
        int offset;
        int line;
        int column;
        std::string filename;
};

// Diagnostics.hpp
extern void rin_error_at(const Location, const char* fmt, ...);

// A named object is anything that is referenced by an identifier
class Named_object
{
public:
        Named_object(const std::string& ident, Location loc)
                : _identifier(ident), _location(loc)
        {}

        const std::string identifier()
        { return this->_identifier; }

        Location location()
        { return this->_location; }

private:
        std::string _identifier;
        Location    _location;
};

/*
 * Represents compilation scope. Since functions are not yet
 * implemented, scopes can only be linear and nested (meaning no
 * two un-nested scopes can exist in parallel). Keeps track of
 * a series of statements and of variable definitions.
 */
class Scope
{
public:
        /*
         * Create a new scope that branches off of parent. Statements in
         * scope can reference variables defined by their ancestors.
         * If parent is NULL, then the scope is the supercontext.
         */
        Scope(Scope* parent = NULL)
                : _parent(parent)
        {}

        ~Scope()
        {
                // Delete all defined named objects
                if (!this->ident_map.empty()) {
                        for (auto itr = this->ident_map.begin(); itr != this->ident_map.end(); ++itr)
                                delete itr->second;
                }

                if (!this->_statements.empty()) {
                        for (auto itr = this->_statements.begin(); itr != this->_statements.end(); ++itr)
                                delete_stmt(*itr);
                }
        }

        // List of statements contained within the scope
        typedef std::vector<Bstatement*> Statement_list;

        // Returns the scope's parent
        Scope* parent()
        { return this->_parent; }

        // Lookup a defined Named_object by its string. Returns NULL
        Named_object* lookup(const std::string& ident)
        { return this->ident_map[ident]; }

        // Test whether an identifier string has been defined.
        bool is_defined(const std::string& ident)
        {
                if (this->lookup(ident))
                        return true;

                if (this->parent())
                        return this->parent()->is_defined(ident);

                return false;
        }

        /*
         * Defines a new named_object in the scope.
         * As long as statements are evaluated sequentially, then there is
         * no risk of a statement referencing an undefined object.
         */
        Named_object* define_obj(const std::string& ident, Location loc)
        {
                if (this->is_defined(ident)) {
                        rin_error_at(loc, "Duplicate definition of %s", &(ident)[0]);
                        return NULL;
                }

                Named_object* obj = new Named_object(ident, loc);
                this->ident_map[ident] = obj;
                return obj;
        }

        // Undefine an object.
        void undefine_obj(const std::string* ident)
        {
                RIN_ASSERT(ident);
                this->ident_map[*ident] = NULL;
        }

        // Return the number of statements.
        unsigned int size()
        { return this->_statements.size(); }

        // Return all statements
        Statement_list* statements()
        { return &this->_statements; }

        // Inserts a statement into the statement list
        void push_statement(Bstatement* state)
        {
                if (!state) return;
                this->_statements.push_back(state);
        }

        // Pop the most recently inserted statement and return it
        Bstatement* pop_statement()
        {
                Bstatement* last = this->_statements.back();
                this->_statements.pop_back();
                return last;
        }

        // Returns the ith statement in the list
        Bstatement* statement(unsigned int i)
        {
                RIN_ASSERT(this->_statements.size() > i);
                return this->_statements[i];
        }

private:
        Scope* _parent;

        std::unordered_map
        <std::string, Named_object*> ident_map;

        Statement_list _statements;
};

class Backend
{
public:
        Backend()
        {
                this->_supercontext = new Scope(NULL);
                this->_current_scope = this->_supercontext;
        }

        virtual ~Backend()
        {
                if (this->_current_scope == this->_supercontext) {
                        delete this->_current_scope;
                        return;
                }

                delete this->_supercontext;
                delete this->_current_scope;
        }

        // Return the supercontext.
        Scope* supercontext()
        { return this->_supercontext; }

        // Return the current scope.
        Scope* current_scope()
        { return this->_current_scope; }

        /*
         * Enters a new scope nested below the previous current scope.
         * Always returns the most current scope.
         */
        Scope* enter_scope()
        {
                this->_current_scope = new Scope(this->_current_scope);
                return this->_current_scope;
        }

        Scope* enter_scope(Scope* scope)
        {
                this->_current_scope = scope;
                return this->_current_scope;
        }

        /*
         * Exits out of the current scope and into the parent scope.
         * Always returns the most current scope.
         */
        Scope* leave_scope()
        {
                this->_current_scope = this->_current_scope->parent();
                return this->_current_scope;
        }

        // Push a statement to the current scope
        void push_statement(Bstatement* statement)
        {
                RIN_ASSERT(this->_current_scope);
                this->_current_scope->push_statement(statement);
        }

        // Pop a statement from current scope
        Bstatement* pop_statement()
        {
                RIN_ASSERT(this->_current_scope);
                return this->_current_scope->pop_statement();
        }

        // Variables

        // Returns a variable derived from a named object.
        virtual Bvariable* variable(Named_object* obj) = 0;

        // Expressions

        // Return an invalid expression
        virtual Bexpression* invalid_expression() = 0;

        // Return an expression for a unary operation.
        virtual Bexpression* unary_expression
        (RIN_OPERATOR op, Bexpression* expr, Location) = 0;

        // Return an expression for a binary operation.
        virtual Bexpression* binary_expression
        (RIN_OPERATOR op, Bexpression* left, Bexpression* right, Location) = 0;

        // Return an expression which is a reference to a variable.
        virtual Bexpression* var_reference(Bvariable* var, Location) = 0;

        /*
         * Return an expression which is a reference to a float.
         * Val pointer will be deleted along with underlying expression,
         * therefore it must be reinitialized/copied using mpfr_init_set.
         */
        virtual Bexpression* float_expression(const mpfr_t* val, Location) = 0;

        // Return a reference to a conditional expression
        virtual Bexpression* conditional_expression(Bexpression* cond, Location) = 0;

        // Statements.

        // Returns an invalid statement which allows compilation to continue.
        virtual Bstatement* invalid_statement() = 0;

        // Returns a variable declaration statement.
        virtual Bstatement* var_dec_statement(Bvariable* var) = 0;

        /*
         * Returns an assignment statement. Currently, only variables
         * can be assigned to, so lhs must be a var reference expression.
         */
        virtual Bstatement* assignment_statement
        (Bexpression* lhs, Bexpression* rhs, Location) = 0;

        /*
         * Create an increment/decrement statement. Expression must be a
         * unary increment/decrement expression.
         */
        virtual Bstatement* inc_statement(Bexpression*) = 0;
        virtual Bstatement* dec_statement(Bexpression*) = 0;

        /*
         * Returns an if-statement. The expression should be a conditional
         * expression, and the scope must be used to parse through the
         * if-statement's constituent statements.
         */
        virtual Bstatement* if_statement(Bexpression*, Scope*, Location) = 0;

        /*
         * Returns a for-loop statement. Cond must be a conditional statement,
         * inc must be an increment/decrement statement. The statements may be NULL.
         * The scope keeps track of statements within the for-loop, they
         * must be parsed by this function.
         */
        virtual Bstatement* for_statement
        (Bstatement* cond, Bstatement* inc, Scope*, Location) = 0;

        /*
         * Returns an expression wrapped as a statement. The expression must
         * be conditional.
         */
        virtual Bstatement* expression_statement(Bexpression* expr, Location) = 0;

        /*
         * Return a compound statement which builds a stored pair of
         * statements consecutively.
         */
        virtual Bstatement* compound_statement
        (Bstatement* first, Bstatement* second, Location loc) = 0;

private:
        Scope* _supercontext;
        Scope* _current_scope;
};

#endif // RIN_BACKEND_HPP
