#ifndef GCC_RIN_BACKEND_HPP
#define GCC_RIN_BACKEND_HPP

#include <frontend/backend.hpp>
#include <frontend/parser.hpp>

// GCC
#include "config.h"
#include "system.h"
#include "input.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "diagnostic.h"
#include "opts.h"
#include "cgraph.h"
#include "fold-const.h"
#include "realmpfr.h"
#include "real.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "stringpool.h"
#include "stor-layout.h"
#include "debug.h"
#include "print-tree.h"
#include "convert.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "common/common-target.h"

// A class wrapping a GCC tree type.
class Gcc_tree
{
public:
        explicit Gcc_tree(tree t) : _t(t) {}

        tree get_tree() const
        { return this->_t; }

        void set_tree(tree t)
        { this->_t = t;}

private:
        tree _t;
};

// Backend types are represented by trees.

// Backend representation of an expression.
class Bexpression : public Gcc_tree
{ public: explicit Bexpression(tree t) : Gcc_tree(t) {} };

// Backend representation of a statement.
class Bstatement : public Gcc_tree
{
public:
        explicit Bstatement(tree t) : Gcc_tree(t) {}

        // Whether this statement represents a block.

        void set_is_block()
        { this->_is_block = true; }

        bool is_block()
        { return this->_is_block; }

private:
        bool _is_block = false;
};

/*
 * Backend representation of a variable.
 * All types are floats so this is treated
 * the same as any other Gcc_tree structure.
 */
class Bvariable : public Gcc_tree
{ public: explicit Bvariable(tree t) : Gcc_tree(t) {} };

// Interface for creating TREE_CHAIN's.
struct TreeChain {
        tree first;
        tree last;

        TreeChain() : first(), last() {}

        // Add tree to treechain.
        void append(tree t)
        {
                RIN_ASSERT(t != NULL_TREE);
                if (first == NULL_TREE) {
                        first = last = t;
                        return;
                }

                TREE_CHAIN(last) = t;
                last = t;
        }
};

class Gcc_backend : public Backend
{
public:
        Gcc_backend();
        ~Gcc_backend() override;

        // Return the supercontext tree.
        tree* supercx_tree()
        { return &this->_supercx_tree; }

        // Return the variable map.
        std::unordered_map<Named_object*, Bvariable*>* var_map()
        { return &this->_var_map; }

        // Variable.
        Bvariable* variable(Named_object* obj) override;

        // Expressions.

        // Build an invalid expression tree.
        Bexpression* invalid_expression() override
        { return new Bexpression(error_mark_node); }

        // Build a unary expression tree.
        Bexpression* unary_expression
        (RIN_OPERATOR op, Bexpression* expr, const Location& loc) override;

        // Build a binary expression tree.
        Bexpression* binary_expression
        (RIN_OPERATOR op, Bexpression* left, Bexpression* right, const Location& loc) override;

        // Build a variable reference tree.
        Bexpression* var_reference(Bvariable* var, const Location& loc) override;

        // Build a float expression tree.
        Bexpression* float_expression(const mpfr_t* val, const Location& loc) override;

        // Build an integer expression tree.
        Bexpression* integer_expression(const mpfr_t* val, const Location& loc) override;

        /*
         * A conditional expression usually wraps a binary
         * expression, so its underlying Bexpression can
         * just be returned.
         */
        Bexpression* conditional_expression(Bexpression* cond, const Location& loc) override
        { return cond; }

        // Statements.

        // Creates a statement_list tree from statements in a scope.
        inline tree unfold_scope(Scope* scope);

        // Create an invalid statement tree.
        Bstatement* invalid_statement() override
        { return new Bstatement(error_mark_node); }

        // Create a variable declaration statement tree.
        Bstatement* var_dec_statement(Bvariable* var) override;

        // Create an assignment statement tree.
        Bstatement* assignment_statement
        (Bexpression* lhs, Bexpression* rhs, const Location& loc) override;

        // Create increment/decrement statement trees.
        Bstatement* inc_statement(Bexpression* unary, const Location& loc) override;
        Bstatement* dec_statement(Bexpression* unary, const Location& loc) override;

        // Create an if statement tree with optional else block.
        Bstatement* if_statement
        (Bexpression* cond, Scope* then, Scope* else_block, const Location& loc) override;

        // Create a for statement tree.
        Bstatement* for_statement
        (Bstatement* ind, Bstatement* cond, Bstatement* inc,
         Scope* then_block, const Location& loc) override;

        // Create an expression statement tree.
        Bstatement* expression_statement(Bexpression* expr, const Location& loc) override;

        // Create a compound statement tree.
        Bstatement* compound_statement(Bstatement* first, Bstatement* second, const Location& loc) override;

        // Create a return statement tree.
        Bstatement* return_statement(Bexpression* expr, const Location& loc) override;

        // Create a function declaration statement tree (stub).
        Bstatement* function_statement
        (const std::string& name, const std::vector<std::string>& params,
         Scope* body, const Location& loc) override;

        // Create a function call expression tree (stub).
        Bexpression* call_expression
        (const std::string& name, const std::vector<Bexpression*>& args,
         const Location& loc) override;

        // Create a break statement (placeholder).
        Bstatement* break_statement(const Location& loc) override;

        // Create a continue statement (placeholder).
        Bstatement* continue_statement(const Location& loc) override;

private:

        // The super context (as a GCC tree).
        tree _supercx_tree;

        /*
         * Maps named_object's to Bvariables - this way they're only ever
         * built once.
         */
        std::unordered_map<Named_object*, Bvariable*> _var_map;
};

// gcc-backend.cc

// Converts a frontend location to location_t.
extern location_t gcc_location(const Location& loc);

// Returns/deletes the backend instance.
extern Gcc_backend* rin_get_backend();
extern void rin_delete_backend();

// Returns/delets the parser instance.
extern Parser* rin_get_parser();
extern void rin_delete_parser();

extern void rin_set_parser(Parser* parser);

// Converts a RIN_OPERATOR to a GCC tree_code.
extern enum tree_code operator_to_tree_code(RIN_OPERATOR op, tree type);

#endif // GCC_RIN_BACKEND_HPP
