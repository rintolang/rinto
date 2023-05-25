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
        Gcc_tree(tree t) : _t(t) {}

        tree get_tree() const
        { return this->_t; }

        tree set_tree(tree t)
        { this->_t = t;}

private:
        tree _t;
};

// Backend types are represented by trees.

// Backend representation of an expression.
class Bexpression : public Gcc_tree
{ public: Bexpression(tree t) : Gcc_tree(t) {} };

// Backend representation of a statement.
class Bstatement : public Gcc_tree
{
public:
        Bstatement(tree t) : Gcc_tree(t) {}

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
{ public: Bvariable(tree t) : Gcc_tree(t) {} };

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
        ~Gcc_backend();

        // Return the supercontext tree.
        tree* supercx_tree()
        { return &this->_supercx_tree; }

        // Return the variable map.
        std::unordered_map<Named_object*, Bvariable*>* var_map()
        { return &this->_var_map; }

        // Variable.
        Bvariable* variable(Named_object* obj);

        // Expressions.

        // Build an invalid expression tree.
        Bexpression* invalid_expression()
        { return new Bexpression(error_mark_node); }

        // Build a unary expression tree.
        Bexpression* unary_expression
        (RIN_OPERATOR op, Bexpression* expr, Location loc);

        // Build a binary expression tree.
        Bexpression* binary_expression
        (RIN_OPERATOR op, Bexpression* left, Bexpression* right, Location loc);

        // Build a variable reference tree.
        Bexpression* var_reference(Bvariable* var, Location loc);

        // Build a float expression tree.
        Bexpression* float_expression(const mpfr_t* val, Location loc);

        /*
         * A conditional expression usually wraps a binary
         * expression, so its underlying Bexpression can
         * just be returned.
         */
        Bexpression* conditional_expression(Bexpression* cond, Location loc)
        { return cond; }

        // Statements.

        // Creates a statement_list tree from statements in a scope.
        inline tree unfold_scope(Scope* scope);

        // Create an invalid statement tree.
        Bstatement* invalid_statement()
        { return new Bstatement(error_mark_node); }

        // Create a variable declaration statement tree.
        Bstatement* var_dec_statement(Bvariable* var);

        // Create an assignment statement tree.
        Bstatement* assignment_statement
        (Bexpression* lhs, Bexpression* rhs, Location loc);

        // Create increment/decrement statement trees.
        Bstatement* inc_statement(Bexpression* unary, Location loc);
        Bstatement* dec_statement(Bexpression* unary, Location loc);

        // Create an if statement tree.
        Bstatement* if_statement(Bexpression* cond, Scope* then, Location loc);

        // Create a for statement tree.
        Bstatement* for_statement
        (Bstatement* ind, Bstatement* cond, Bstatement* inc,
         Scope* then_block, Location loc);

        // Create an expression statement tree.
        Bstatement* expression_statement(Bexpression* expr, Location loc);

        // Create a compound statement tree.
        Bstatement* compound_statement(Bstatement* first, Bstatement* second, Location loc);

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
extern location_t gcc_location(const Location loc);

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
