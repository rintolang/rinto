#include "gcc-backend.hpp"

/*
 * Delete a Bstatement. Required because frontend
 * sees Bstatement as an incomplete type.
 */
void delete_stmt(Bstatement* stmt)
{ delete stmt; }

// Whether a file's location is currently being read.
bool gcc_loc_infile = false;

// Convert a frontend location to GCC location type.
location_t gcc_location(const Location loc)
{
        if (loc == File::unknown_location())
                return UNKNOWN_LOCATION;

        if (gcc_loc_infile)
                linemap_add(line_table, LC_LEAVE, 0, NULL, 0);

        // Enter a file.
        linemap_add(line_table, LC_ENTER, 0, &loc.filename[0], LOC_LINE_BEGIN);

        // Enter a line. Line counts on the frontend begin at 0.
        linemap_line_start(line_table, loc.line + LOC_LINE_BEGIN, 1);
        gcc_loc_infile = true;

        return linemap_position_for_column(line_table, loc.column + LOC_COLUMN_BEGIN);
}

// Build the supercontext.
Gcc_backend::Gcc_backend()
{
        // Main fn parameters: (int, char**)
        tree main_fndecl_type_param[] = {
                integer_type_node,
                build_pointer_type(build_pointer_type(char_type_node))
        };

        // Main fn declaration: int main(int, char**)
        tree main_fndecl_type = build_function_type_array
                (integer_type_node, 2, main_fndecl_type_param);

        this->_supercx_tree = build_fn_decl("main", main_fndecl_type);
}

Gcc_backend::~Gcc_backend()
{
        // Clear stored bvariables.
        for (auto itr = this->_var_map.begin(); itr != this->_var_map.end(); ++itr)
                delete itr->second;
}

Bvariable* Gcc_backend::variable(Named_object* obj)
{
        if (this->_var_map[obj])
                return this->_var_map[obj];

        tree decl = build_decl(gcc_location(obj->location()),
                VAR_DECL, get_identifier(obj->identifier().c_str()),
                float_type_node);

        DECL_CONTEXT(decl) = this->_supercx_tree;
        this->_var_map[obj] = new Bvariable(decl);

        return _var_map[obj];
}

Bexpression* Gcc_backend::unary_expression
(RIN_OPERATOR op, Bexpression* expr, Location loc)
{
        RIN_ASSERT(expr);
        tree expr_tree = expr->get_tree();

        if (expr_tree == error_mark_node ||
            TREE_TYPE(expr_tree) == error_mark_node) {
                delete expr;
                return this->invalid_expression();
        }

        tree type_tree = TREE_TYPE(expr_tree);
        enum tree_code code = TRUTH_NOT_EXPR;

        /*
         * Although ++, -- are unary operators, they are
         * handled by inc_statement and dec_statement.
         */
        if (op == OPER_INC || op == OPER_DEC)
                return expr;

        // Bad operator.
        if (op == OPER_NOT)
                RIN_UNREACHABLE();

        tree ret = fold_build1_loc(gcc_location(loc),
                code, type_tree, expr_tree);

        delete expr;
        return new Bexpression(ret);
}

tree convert (tree type, tree expr)
{
        if (type == error_mark_node || expr == error_mark_node
            || TREE_TYPE(expr) == error_mark_node)
                return error_mark_node;

        if (type == TREE_TYPE(expr))
                return expr;

        if (TYPE_MAIN_VARIANT(type) == TYPE_MAIN_VARIANT(TREE_TYPE(expr)))
                return fold_convert(type, expr);

        switch (TREE_CODE(type))
        {
        case VOID_TYPE:
        case BOOLEAN_TYPE:
                return fold_convert(type, expr);
        case INTEGER_TYPE:
                return fold(convert_to_integer(type, expr));
        case POINTER_TYPE:
                return fold(convert_to_pointer(type, expr));
        case REAL_TYPE:
                return fold(convert_to_real(type, expr));
        case COMPLEX_TYPE:
                return fold(convert_to_complex(type, expr));
        default:
                break;
        }

        RIN_UNREACHABLE();
}

Bexpression* Gcc_backend::binary_expression
(RIN_OPERATOR op, Bexpression* left, Bexpression* right, Location loc)
{
        RIN_ASSERT(left && right);
        tree left_tree = left->get_tree();
        tree right_tree = right->get_tree();

        // Should be valid.
        if (left_tree == error_mark_node ||
            right_tree == error_mark_node) {
                delete left;
                delete right;
                return this->invalid_expression();
        }

        enum tree_code code = operator_to_tree_code(op, TREE_TYPE(left_tree));
        bool use_left_type = op != OPER_LOR && op != OPER_LAND;
        tree type_tree = use_left_type ? TREE_TYPE(left_tree) : TREE_TYPE(right_tree);
        tree computed_type = excess_precision_type(type_tree);

        if (computed_type != NULL_TREE) {
                left_tree = convert(computed_type, left_tree);
                right_tree = convert(computed_type, right_tree);
                type_tree = computed_type;
        }

        tree ret = fold_build2_loc(gcc_location(loc), code,
                type_tree, left_tree, right_tree);

        delete left;
        delete right;
        return new Bexpression(ret);
}

Bexpression* Gcc_backend::var_reference(Bvariable* var, Location loc)
{
        RIN_ASSERT(var);
        tree ret = var->get_tree();
        if (ret == error_mark_node) {
                delete var;
                return this->invalid_expression();
        }

        return new Bexpression(ret);
}

Bexpression* Gcc_backend::float_expression(const mpfr_t* val, Location loc)
{
        RIN_ASSERT(val);

        REAL_VALUE_TYPE r1;
        real_from_mpfr(&r1, *val, float_type_node, GMP_RNDN);
        REAL_VALUE_TYPE r2;
        real_convert(&r2, TYPE_MODE(float_type_node), &r1);

        tree ret = build_real(float_type_node, r2);
        return new Bexpression(ret);
}

inline tree Gcc_backend::unfold_scope(Scope* scope)
{
        RIN_ASSERT(scope);
        tree list = alloc_stmt_list();
        Scope::Statement_list* stmts = scope->statements();
        for (auto itr = stmts->begin(); itr != stmts->end(); ++itr) {
                if (!(*itr)->is_block())
                        append_to_statement_list((*itr)->get_tree(), &list);
        }

        return list;
}

Bstatement* Gcc_backend::var_dec_statement(Bvariable* var)
{
        tree decl = var->get_tree();
        TREE_USED(decl) = 1;

        tree stmt = build1_loc(EXPR_LOCATION(decl),
                DECL_EXPR, void_type_node, decl);

        return new Bstatement(stmt);
}

Bstatement* Gcc_backend::assignment_statement(Bexpression* lhs, Bexpression* rhs, Location loc)
{
        tree ass_stmt = build2_loc(gcc_location(loc), MODIFY_EXPR,
                 void_type_node, lhs->get_tree(), rhs->get_tree());

        delete lhs;
        delete rhs;
        return new Bstatement(ass_stmt);
}

Bstatement* Gcc_backend::inc_statement(Bexpression* unary, Location loc)
{
        /*
         * Unary is actually just a variable reference.
         * It expands into var = var + 1;
         */
        tree t = build2_loc(gcc_location(loc), MODIFY_EXPR, void_type_node,
                unary->get_tree(), build2_loc(gcc_location(loc),
                PLUS_EXPR, integer_type_node, unary->get_tree(),
                build_int_cst_type(::integer_type_node, 1)));

        delete unary;
        return new Bstatement(t);
}

Bstatement* Gcc_backend::dec_statement(Bexpression* unary, Location loc)
{
        /*
         * Unary is actually just a variable reference.
         * It expands into var = var - 1;
         */
        tree t = build2_loc(gcc_location(loc), MODIFY_EXPR, void_type_node,
                unary->get_tree(), build2_loc(gcc_location(loc),
                MINUS_EXPR, integer_type_node, unary->get_tree(),
                build_int_cst_type(::integer_type_node, 1)));

        delete unary;
        return new Bstatement(t);
}

Bstatement* Gcc_backend::if_statement(Bexpression* cond, Scope* then, Location loc)
{
        RIN_ASSERT(cond);
        RIN_ASSERT(then);

        tree cond_tree = cond->get_tree();
        tree then_tree = unfold_scope(then);
        if (cond_tree == error_mark_node ||
            then_tree == error_mark_node) {
                delete cond;
                delete then;
                return this->invalid_statement();
        }

        tree ret = build3_loc(gcc_location(loc), COND_EXPR,
                void_type_node, cond_tree, then_tree, NULL_TREE);

        delete cond;
        delete then;
        return new Bstatement(ret);
}

Bstatement* Gcc_backend::for_statement
(Bstatement* ind, Bstatement* cond, Bstatement* inc, Scope* then_block, Location loc)
{
        // Get induction, condition, increment trees.
        tree ind_tree  = (ind)  ? ind->get_tree()  : NULL_TREE;
        tree cond_tree = (cond) ? cond->get_tree() : NULL_TREE;
        tree inc_tree  = (inc)  ? inc->get_tree()  : NULL_TREE;

        delete ind;
        delete cond;
        delete inc;

        TreeChain subblocks, var_decl_chain;

        // Gather then-block scope.
        tree then_block_list = alloc_stmt_list();
        Scope::Statement_list* stmts = then_block->statements();
        for (auto itr = stmts->begin(); itr != stmts->end(); ++itr) {
                if (!(*itr)->is_block()) {
                        append_to_statement_list((*itr)->get_tree(), &then_block_list);
                        continue;
                }

                subblocks.append((*itr)->get_tree());
        }

        // Append the induction statement to the for-loop body.
        append_to_statement_list(ind_tree, &then_block_list);

        // Gather variable declaration chain.
        Scope::Var_map* vars = then_block->variables();
        for (auto itr = vars->begin(); itr != vars->end(); ++itr) {
                Named_object* obj = itr->second;
                tree var = this->_var_map[obj]->get_tree();
                var_decl_chain.append(var);
        }

        tree then_block_tree = build_block(var_decl_chain.first,
                subblocks.first, NULL_TREE, NULL_TREE);

        /*
         * Add the block to the current scope so that it may be picked
         * up as a subblock by parent for-statements.
         */
        Bstatement* then_block_tree_stmt = new Bstatement(then_block_tree);
        then_block_tree_stmt->set_is_block();
        this->current_scope()->push_statement(then_block_tree_stmt);

        // Set the subblocks to have the new block as their parent.
        for (tree it = subblocks.first; it != NULL_TREE; it = BLOCK_CHAIN(it))
                BLOCK_SUPERCONTEXT(it) = then_block_tree;

        // Create block bind expression. // for_body_stmt aka for_body_stmt_list
        tree then_block_bind = build3(BIND_EXPR, void_type_node,
                var_decl_chain.first, then_block_list, then_block_tree);

        /*
         * Creates the following statement list:
         *
         * while_check_label_expr:
         *         if (FOR_LOOP_CONDITION) goto while_body_label_decl
         *         else goto end_of_while_label_decl
         * while_body_label_expr:
         *         1. { execute for-loop body statements }
         *         2. increment/decrement statement.
         *.        3. goto while_check_label_decl.
         * end_of_while_label_expr:
         *         exit for-loop.
         */

        // Wraps the entire for-loop statement.
        tree master_stmt_list = alloc_stmt_list();

        tree while_check_label_decl = build_decl(EXPR_LOCATION(cond_tree),
                LABEL_DECL, get_identifier("while_check"), void_type_node);
        DECL_CONTEXT(while_check_label_decl) = this->_supercx_tree;

        tree while_check_label_expr = build1_loc(EXPR_LOCATION(cond_tree), LABEL_EXPR,
                void_type_node, while_check_label_decl);

        append_to_statement_list(while_check_label_expr, &master_stmt_list);

        tree while_body_label_decl = build_decl(EXPR_LOCATION(then_block_list),
                LABEL_DECL, get_identifier("while_body"), void_type_node);
        DECL_CONTEXT(while_body_label_decl) = this->_supercx_tree;

        tree end_of_while_label_decl = build_decl(EXPR_LOCATION(then_block_list),
                LABEL_DECL, get_identifier("end_of_while"), void_type_node);
        DECL_CONTEXT(end_of_while_label_decl) = this->_supercx_tree;

        tree cond_expr = build3_loc(EXPR_LOCATION(cond_tree), COND_EXPR,
                void_type_node, cond_tree, build1_loc(EXPR_LOCATION(cond_tree),
                GOTO_EXPR, void_type_node, while_body_label_decl),
                build1_loc(EXPR_LOCATION(cond_tree), GOTO_EXPR, void_type_node,
                end_of_while_label_decl));

        append_to_statement_list(cond_expr, &master_stmt_list);
        tree while_body_label_expr = build1_loc(EXPR_LOCATION(then_block_bind),
                LABEL_EXPR, void_type_node, while_body_label_decl);

        append_to_statement_list(while_body_label_expr, &master_stmt_list);
        append_to_statement_list(then_block_bind, &master_stmt_list);

        tree goto_check = build1_loc(UNKNOWN_LOCATION, GOTO_EXPR, void_type_node,
                while_check_label_decl);
        append_to_statement_list(goto_check, &master_stmt_list);

        tree end_of_while_label_expr = build1_loc(UNKNOWN_LOCATION, LABEL_EXPR,
                void_type_node, end_of_while_label_decl);
        append_to_statement_list(end_of_while_label_expr, &master_stmt_list);

        delete then_block;
        return new Bstatement(master_stmt_list);
}

Bstatement* Gcc_backend::expression_statement(Bexpression* expr, Location loc)
{
        RIN_ASSERT(expr);
        tree t = expr->get_tree();
        delete expr;

        return new Bstatement(t);
}

Bstatement* Gcc_backend::compound_statement(Bstatement* first, Bstatement* second, Location loc)
{
        tree stmt_list = NULL_TREE;
        tree t1 = first->get_tree();
        tree t2 = second->get_tree();
        if (t1 == error_mark_node || t2 == error_mark_node) {
                delete first;
                delete second;
                return this->invalid_statement();
        }

        append_to_statement_list(t1, &stmt_list);
        append_to_statement_list(t2, &stmt_list);

        /*
         * If neither statement has any side effects, stmt_list
         * can be NULL at this point.
         */
        if (stmt_list == NULL_TREE)
                stmt_list = integer_zero_node;

        delete first;
        delete second;
        return new Bstatement(stmt_list);
}

Gcc_backend* grin_be = new Gcc_backend;
Parser* grin_parse = NULL;

// Get the GRIN instance of the backend.
Gcc_backend* rin_get_backend()
{ return grin_be; }

// Delete GRIN instance of backend.
void rin_delete_backend()
{ delete grin_be; }

void rin_set_parser(Parser* parser)
{ grin_parse = parser; }

Parser* rin_get_parser()
{ return grin_parse; }

void rin_delete_parser()
{ delete grin_parse; }

// Convert a frontend operator to an equivalent GCC tree_code.
enum tree_code operator_to_tree_code(RIN_OPERATOR op, tree type)
{
        enum tree_code code;
        switch(op) {
        case OPER_ADD:
                code = PLUS_EXPR;
                break;
        case OPER_SUB:
                code = MINUS_EXPR;
                break;
        case OPER_MUL:
                code = MULT_EXPR;
                break;
        case OPER_QUO:
                if (TREE_CODE(type) == REAL_TYPE ||
                    TREE_CODE(type) == COMPLEX_TYPE)
                        code = RDIV_EXPR;
                else code = TRUNC_DIV_EXPR;
                break;
        case OPER_REM:
                code = TRUNC_MOD_EXPR;
                break;
        case OPER_LAND:
                code = TRUTH_ANDIF_EXPR;
                break;
        case OPER_LOR:
                code = TRUTH_ORIF_EXPR;
                break;
        case OPER_EQL:
                code = EQ_EXPR;
                break;
        case OPER_LSS:
                code = LT_EXPR;
                break;
        case OPER_GTR:
                code = GT_EXPR;
                break;
        case OPER_NEQ:
                code = NE_EXPR;
                break;
        case OPER_LEQ:
                code = LE_EXPR;
                break;
        case OPER_GEQ:
                code = GE_EXPR;
                break;
        default:
                RIN_UNREACHABLE();
        }

        return code;
}
