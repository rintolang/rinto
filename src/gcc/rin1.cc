/*
 * rin1 is the GCC compiler for the Rinto programming language.
 * The resulting 'grin' CMD tool exposes rin1 to the user.
 *
 * rin1.cc implements rin1 by interfacing with the GCC backend
 * and Rinto frontend.
 */

#include "gcc-backend.hpp"

// Language-dependent contents of a type.
struct GTY(()) lang_type
{ char dummy; };

// Language-dependent contents of a decl.
struct GTY(()) lang_decl
{ char dummy; };

/*
 * Language-dependent contents of an identifier.
 * This must include a tree_identifier.
 */
struct GTY(()) lang_identifier
{ struct tree_identifier common; };

// The resulting tree type.
union GTY((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE"),
            chain_next ("CODE_CONTAINS_STRUCT (TREE_CODE (&%h.generic), "
                        "TS_COMMON) ? ((union lang_tree_node *) TREE_CHAIN "
                        "(&%h.generic)) : NULL"))) lang_tree_node
{
        union tree_node GTY ((tag ("0"), desc ("tree_node_structure (&%h)"))) generic;
        struct lang_identifier GTY ((tag ("1"))) identifier;
};

// Unused.
struct GTY(()) language_function
{ int dummy; };

// Language hooks.

// Called by GCC to initialize the frontend.
static bool rin_langhook_init(void)
{
        void_list_node = build_tree_list(NULL_TREE, void_type_node);
        build_common_builtin_nodes();

        // The default precision for floating point numbers.
        mpfr_set_default_prec(256);

        return true;
}

// Called at the end of compilation.
static void rin_langhook_finish(void)
{
        rin_delete_backend();
        rin_delete_parser();
}

// GCC calls this to parse a file.
static void rin_langhook_parse_file(void)
{
        /*
         * From GCC:
         * int num_in_fnames      : number of files to parse.
         * const char** in_fnames : Paths of files to parse.
         */
        RIN_ASSERT(num_in_fnames);
        std::string path(in_fnames[0]);

        // Warn the user if parsing > 1 file.
        if (num_in_fnames > 1)
                rin_inform(File::unknown_location(),
                        "Can only parse one file at a time."
                        "Currently parsing: %s", in_fnames[0]);

        // Open file with parser.
        Parser* parse = new Parser(path, (Backend*)rin_get_backend());
        rin_set_parser(parse);
        parse->parse();

        TreeChain main_subblocks;
        TreeChain main_decl_chain;

        // Gather main fn body statements.

        tree main_list = alloc_stmt_list();
        tree& main_cx = *rin_get_backend()->supercx_tree();

        Scope::Statement_list* stmts = parse->backend()->supercontext()->statements();
        for (auto itr = stmts->begin(); itr != stmts->end(); ++itr) {
                if (!(*itr)->is_block()) {
                        append_to_statement_list((*itr)->get_tree(), &main_list);
                        continue;
                }

                main_subblocks.append((*itr)->get_tree());
        }

        // Gather variable declaration chain.
        Scope::Var_map* vars = parse->backend()->supercontext()->variables();
        std::unordered_map<Named_object*, Bvariable*> var_map = *rin_get_backend()->var_map();
        for (auto itr = vars->begin(); itr != vars->end(); ++itr) {
                Named_object* obj = itr->second;
                main_decl_chain.append(var_map[obj]->get_tree());
        }

        tree main_block = build_block(main_decl_chain.first,
                main_subblocks.first, NULL_TREE, NULL_TREE);

        tree main_block_bind = build3(BIND_EXPR, void_type_node,
                main_decl_chain.first, main_list, main_block);

        // Set any subblocks to have the main_block as their parent.
        for (tree it = main_subblocks.first; it != NULL_TREE; it = BLOCK_CHAIN(it))
                BLOCK_SUPERCONTEXT(it) = main_block;

        // Finish main function.
        tree resdecl = build_decl(UNKNOWN_LOCATION, RESULT_DECL,
                NULL_TREE, integer_type_node);

        DECL_CONTEXT(resdecl) = main_cx;
        DECL_RESULT(main_cx) = resdecl;

        tree set_result = build2(INIT_EXPR, void_type_node, DECL_RESULT(main_cx),
                build_int_cst_type(integer_type_node, 0));

        tree return_stmt = build1(RETURN_EXPR, void_type_node, set_result);
        append_to_statement_list(return_stmt, &main_list);

        BLOCK_SUPERCONTEXT(main_block) = main_cx;
        DECL_INITIAL(main_cx) = main_block;
        DECL_SAVED_TREE(main_cx) = main_block_bind;

        DECL_EXTERNAL(main_cx) = 0;
        DECL_PRESERVE_P(main_cx) = 1;

        // Convert from GENERIC to GIMPLE.
        gimplify_function_tree(main_cx);

        // Insert it into GCC graph.
        cgraph_node::finalize_function(main_cx, true);
        main_cx = NULL_TREE;

        rin_langhook_finish();
}

/*
 * Return the GENERIC type to use given a machine
 * mode 'mode' and order of precision 'unsignedp'.
 */
static tree rin_langhook_type_for_mode(enum machine_mode mode, int unsignedp)
{
        // Real types.
        if (mode == TYPE_MODE (float_type_node))
                return float_type_node;

        if (mode == TYPE_MODE (double_type_node))
                return double_type_node;

        // Integer types.
        if (mode == TYPE_MODE (intQI_type_node))
                return unsignedp ? unsigned_intQI_type_node : intQI_type_node;

        if (mode == TYPE_MODE (intHI_type_node))
                return unsignedp ? unsigned_intHI_type_node : intHI_type_node;

        if (mode == TYPE_MODE (intSI_type_node))
                return unsignedp ? unsigned_intSI_type_node : intSI_type_node;

        if (mode == TYPE_MODE (intDI_type_node))
                return unsignedp ? unsigned_intDI_type_node : intDI_type_node;

        if (mode == TYPE_MODE (intTI_type_node))
                return unsignedp ? unsigned_intTI_type_node : intTI_type_node;

        if (mode == TYPE_MODE (integer_type_node))
                return unsignedp ? unsigned_type_node : integer_type_node;

        if (mode == TYPE_MODE (long_integer_type_node))
                return unsignedp ? long_unsigned_type_node : long_integer_type_node;

        if (mode == TYPE_MODE (long_long_integer_type_node))
                return unsignedp ? long_long_unsigned_type_node
                        : long_long_integer_type_node;

        // Complex types.
        if (COMPLEX_MODE_P (mode)) {
                if (mode == TYPE_MODE (complex_float_type_node))
                        return complex_float_type_node;

                if (mode == TYPE_MODE (complex_double_type_node))
                        return complex_double_type_node;

                if (mode == TYPE_MODE (complex_long_double_type_node))
                        return complex_long_double_type_node;

                if (mode == TYPE_MODE (complex_integer_type_node) && !unsignedp)
                        return complex_integer_type_node;
        }

        // gcc_unreachable
        return NULL;
}

static tree rin_langhook_type_for_size
(unsigned int bits ATTRIBUTE_UNUSED, int unsignedp ATTRIBUTE_UNUSED)
{
        gcc_unreachable();
        return NULL;
}

// No built-in functions yet.
static tree rin_langhook_builtin_function(tree decl)
{ return decl; }

static bool rin_langhook_global_bindings_p(void)
{
        gcc_unreachable();
        return true;
}

static tree rin_langhook_pushdecl (tree decl ATTRIBUTE_UNUSED)
{ gcc_unreachable (); }

static tree rin_langhook_getdecls (void)
{ return NULL; }

// See gcc/gcc/langhooks.h and gcc/gcc/langhooks-def.h
#undef LANG_HOOKS_NAME
#undef LANG_HOOKS_INIT
#undef LANG_HOOKS_PARSE_FILE
#undef LANG_HOOKS_TYPE_FOR_MODE
#undef LANG_HOOKS_TYPE_FOR_SIZE
#undef LANG_HOOKS_BUILTIN_FUNCTION
#undef LANG_HOOKS_GLOBAL_BINDINGS_P
#undef LANG_HOOKS_PUSHDECL
#undef LANG_HOOKS_GETDECLS

#define LANG_HOOKS_NAME "Rinto"
#define LANG_HOOKS_INIT rin_langhook_init
#define LANG_HOOKS_PARSE_FILE rin_langhook_parse_file
#define LANG_HOOKS_TYPE_FOR_MODE rin_langhook_type_for_mode
#define LANG_HOOKS_TYPE_FOR_SIZE rin_langhook_type_for_size
#define LANG_HOOKS_BUILTIN_FUNCTION rin_langhook_builtin_function
#define LANG_HOOKS_GLOBAL_BINDINGS_P rin_langhook_global_bindings_p
#define LANG_HOOKS_PUSHDECL rin_langhook_pushdecl
#define LANG_HOOKS_GETDECLS rin_langhook_getdecls

struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

// Will be generated by GCC.
#include "gt-rinto-rin1.h"
#include "gtype-rinto.h"
