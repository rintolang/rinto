// parser.cc - Recursive descent parser and AST construction
#include "parser.hpp"

Parser::Parser(Scanner* scanner, Backend* backend)
{
        RIN_ASSERT(backend);
        RIN_ASSERT(scanner);
        this->_scanner = scanner;
        this->_backend = backend;
}

Parser::Parser(const std::string& path, Backend* backend)
{
        RIN_ASSERT(backend);
        this->_scanner = new Scanner(path);
        this->_backend = backend;
}

Parser::~Parser()
{
        delete this->_scanner;
        delete this->_backend;
}

void Parser::parse(bool is_supercontext)
{
        while (this->_scanner->has_next()) {
                /*
                 * If not supercontext, then parse until a '}' is encountered,
                 * which marks the end of the current scope.
                 */
                if (!is_supercontext && EXPECT_RIGHT_BRACE(this->_scanner->peek_token())) {
                        this->_scanner->next_token();
                        this->_backend->leave_scope();
                        return;
                }

                /*
                 * Prevents errors being issued when a non-super scope has no
                 * statements, i.e for-loop with no statements.
                 */
                if (this->_scanner->peek_token().classification() == Token::TOKEN_EOL) {
                        this->_scanner->next_token();
                        continue;
                }

                Statement* next = this->parse_next();
                RIN_ASSERT(next != NULL);
                if (!next->is_invalid())
                        this->_backend->push_statement(next->get_backend(this->_backend));
                delete next;
        }

        // Issue Scanner errors, if any.
        this->_scanner->consume_errors();
}

Statement* Parser::parse_next()
{
        Token tk = this->_scanner->peek_token();
        if (tk.classification() == Token::TOKEN_EOF)
                goto is_invalid_statement;

        if (tk.classification() == Token::TOKEN_EOL) {
                this->_scanner->next_token();
                return this->parse_next();
        }

        if (tk.classification() == Token::TOKEN_RID) {
                // Parse if statement
                if (tk.rid() == RID_IF)
                        return this->parse_if_statement();

                // Parse for statement
                if (tk.rid() == RID_FOR)
                        return this->parse_for_statement();

                // Parse while statement
                if (tk.rid() == RID_WHILE)
                        return this->parse_while_statement();

                // Parse variable declaration
                if (tk.rid() == RID_FLOAT || tk.rid() == RID_INT
                    || tk.rid() == RID_VAR)
                        return this->parse_var_dec_statement();

                // Parse function declaration
                if (tk.rid() == RID_FN)
                        return this->parse_function_declaration();

                // Parse return statement
                if (tk.rid() == RID_RETURN)
                        return this->parse_return_statement();

                // Parse break statement
                if (tk.rid() == RID_BREAK) {
                        Token brk = this->_scanner->next_token();
                        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());
                        return Statement::make_break(brk.location());
                }

                // Parse continue statement
                if (tk.rid() == RID_CONTINUE) {
                        Token cont = this->_scanner->next_token();
                        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());
                        return Statement::make_continue(cont.location());
                }

                // Switch statement (not yet implemented)
                if (tk.rid() == RID_SWITCH) {
                        rin_error_at(tk.location(),
                                "switch statements are not yet implemented");
                        this->_scanner->next_token();
                        this->_scanner->skip_line();
                        return Statement::make_invalid(tk.location());
                }

                // Hanging reserved identifier
                rin_error_at(tk.location(), "Malformed identifier %s", tk.str());
                goto is_invalid_statement;
        }

        if (tk.classification() == Token::TOKEN_IDENT) {
                Token op = this->_scanner->peek_nth_token(1);
                if (op.classification() != Token::TOKEN_OPERATOR) {
                        rin_error_at(op.location(),
                                "Expected operator after identifier, but received %s instead",
                                op.str());
                        goto is_invalid_statement;
                }

                // Parse function call statement: name(args)
                if (op.op() == OPER_LPAREN) {
                        Token ident = this->_scanner->next_token();
                        return this->parse_call_statement(
                                *ident.identifier(), ident.location());
                }

                // Parse increment/decrement statement
                if (op.op() == OPER_INC || op.op() == OPER_DEC) {
                        return this->parse_inc_dec_statement();
                }

                /*
                 * If not inc/dec, we assume its a var assignment and
                 * let the parse_assignment_statement handle errors.
                 */
                return this->parse_assignment_statement();
        }

is_invalid_statement:
        /*
         * Skip to the next statement boundary. Consume tokens until we
         * find EOL, EOF, semicolon, or see a right brace (which ends
         * the current scope). Do not consume the right brace itself.
         */
        while (this->_scanner->has_next()) {
                Token peek = this->_scanner->peek_token();
                if (peek.classification() == Token::TOKEN_EOF ||
                    peek.classification() == Token::TOKEN_EOL)
                        break;
                if (peek.classification() == Token::TOKEN_OPERATOR &&
                    peek.op() == OPER_SEMICOLON)
                        break;
                if (EXPECT_RIGHT_BRACE(peek))
                        break;
                this->_scanner->next_token();
        }
        if (this->_scanner->has_next() && !EXPECT_RIGHT_BRACE(this->_scanner->peek_token()))
                this->_scanner->next_token();
        return Statement::make_invalid(tk.location());
}

// --- Statements ---

Statement* Parser::parse_if_statement()
{
        // Verify IF token.
        Token if_rid = this->_scanner->next_token();
        RIN_ASSERT(if_rid.classification() == Token::TOKEN_RID);
        RIN_ASSERT(if_rid.rid() == RID_IF);

        // Gather condition.
        Expression* condition = this->parse_conditional_expression();

        // Skip EOL tokens before opening brace (allows brace on next line)
        while (this->_scanner->peek_token().classification() == Token::TOKEN_EOL)
                this->_scanner->next_token();

        // Opening brace error
        Token expect_lbrace = this->_scanner->next_token();
        if (!condition && !EXPECT_LEFT_BRACE(expect_lbrace)) {
                rin_error_at(expect_lbrace.location(),
                             "If-statement expected left-brace '{' but received %s instead",
                             expect_lbrace.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(if_rid.location());
        }

        // Error already issued by parse_conditional_expression.
        if (!condition) {
                this->_scanner->skip_line();
                return Statement::make_invalid(if_rid.location());
        }

        // Parse if-statement scope
        Scope* if_stmt_scope = this->_backend->enter_scope();
        this->parse(false);

        If_statement* if_stmt = new If_statement(condition, if_stmt_scope,
                if_rid.location());

        // Check for else / else-if
        Token peek = this->_scanner->peek_token();
        if (peek.classification() == Token::TOKEN_RID && peek.rid() == RID_ELSE) {
                this->_scanner->next_token();

                Token next = this->_scanner->peek_token();

                // else if { ... }
                if (next.classification() == Token::TOKEN_RID && next.rid() == RID_IF) {
                        Scope* else_scope = this->_backend->enter_scope();
                        Statement* else_if = this->parse_if_statement();
                        if (!else_if->is_invalid())
                                this->_backend->push_statement(
                                        else_if->get_backend(this->_backend));
                        delete else_if;
                        this->_backend->leave_scope();
                        if_stmt->set_else_block(else_scope);
                }
                // else { ... }
                else if (EXPECT_LEFT_BRACE(next)) {
                        this->_scanner->next_token();
                        Scope* else_scope = this->_backend->enter_scope();
                        this->parse(false);
                        if_stmt->set_else_block(else_scope);
                }
                else {
                        rin_error_at(next.location(),
                                "Expected '{' or 'if' after 'else', but received %s instead",
                                next.str());
                }
        }

        return if_stmt;
}

Statement* Parser::parse_for_statement()
{
        // Verify FOR token.
        Token for_rid = this->_scanner->next_token();
        RIN_ASSERT(for_rid.classification() == Token::TOKEN_RID);
        RIN_ASSERT(for_rid.rid() == RID_FOR);

        this->backend()->enter_scope();

        // Induction statement
        Token semicolon = this->_scanner->peek_token();
        Statement* ind_stmt = NULL;
        if (!EXPECT_SEMICOLON(semicolon)) {
                ind_stmt = this->parse_next();
                if (ind_stmt->is_invalid())
                        return ind_stmt;
        } else this->_scanner->next_token();

        /*
         * If the induction statement is an if-statement or for-loop, then we
         * to manually check if the next token is a semicolon.
         */
        if(ind_stmt && (ind_stmt->classification() == Statement::STATEMENT_FOR ||
                        ind_stmt->classification() == Statement::STATEMENT_IF)) {
                semicolon = this->_scanner->next_token();
                if (!EXPECT_SEMICOLON(semicolon)) {
                        rin_error_at(semicolon.location(),
                                "For-loop expected semicolon (';') after statement but received %s instead",
                                semicolon.str());
                        delete ind_stmt;
                        this->_scanner->skip_line();
                        return Statement::make_invalid(semicolon.location());
                }
        }

        // Condition.
        semicolon = this->_scanner->peek_token();
        Statement* cond_stmt = NULL;
        if (!EXPECT_SEMICOLON(semicolon)) {
                Expression* cond = this->parse_conditional_expression(OPER_SEMICOLON);

                // Try and get a conditional expression
                if (cond) {
                        cond_stmt = Statement::make_expression(cond, cond->location());
                        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());
                }

                // Found neither a semicolon nor a statement; Error already issued.
                if (!cond_stmt) {
                        delete ind_stmt;
                        this->_scanner->skip_line();
                        return Statement::make_invalid(semicolon.location());
                }
        } else this->_scanner->next_token();

        // Increment
        Token lbrace = this->_scanner->peek_token();
        Statement* incdec_stmt = NULL;
        if (!EXPECT_LEFT_BRACE(lbrace) && lbrace.classification() == Token::TOKEN_IDENT) {
                Token peek_op = this->_scanner->peek_nth_token(1);

                if (peek_op.classification() != Token::TOKEN_OPERATOR) {
                        rin_error_at(peek_op.location(),
                                     "For-loop expected increment, decrement, or assignment statement");
                        this->_scanner->skip_line();
                        delete ind_stmt;
                        delete cond_stmt;
                        return Statement::make_invalid(peek_op.location());
                }

                /*
                 * If the operator after the identifier is an assignment or
                 * compound assignment, parse a full assignment statement.
                 */
                if (peek_op.op() == OPER_ASSIGN ||
                    peek_op.op() == OPER_ADD_ASSIGN ||
                    peek_op.op() == OPER_SUB_ASSIGN ||
                    peek_op.op() == OPER_MUL_ASSIGN ||
                    peek_op.op() == OPER_QUO_ASSIGN) {
                        incdec_stmt = this->parse_assignment_statement();
                        if (incdec_stmt->is_invalid()) {
                                delete ind_stmt;
                                delete cond_stmt;
                                return incdec_stmt;
                        }
                } else if (peek_op.op() == OPER_INC || peek_op.op() == OPER_DEC) {
                        Token ident = this->_scanner->next_token();
                        Token oper = this->_scanner->next_token();

                        Named_object* obj = this->backend()->current_scope()->lookup(*ident.identifier());
                        if (!obj) {
                                rin_error_at(ident.location(), "'%s' is undefined", ident.str());
                                delete ind_stmt;
                                delete cond_stmt;
                                return Statement::make_invalid(ident.location());
                        }

                        Expression* ref = Expression::make_var_reference(obj, ident.location());
                        Expression* unary = Expression::make_unary(oper.op(), ref, oper.location());

                        incdec_stmt = (oper.op() == OPER_INC) ? Statement::make_inc(unary) :
                                Statement::make_dec(unary);
                } else {
                        rin_error_at(peek_op.location(),
                                     "For-loop expected increment, decrement, or assignment statement");
                        this->_scanner->skip_line();
                        delete ind_stmt;
                        delete cond_stmt;
                        return Statement::make_invalid(peek_op.location());
                }
        }

        // Skip EOL tokens before opening brace (allows brace on next line)
        while (this->_scanner->peek_token().classification() == Token::TOKEN_EOL)
                this->_scanner->next_token();

        // Opening brace
        lbrace = this->_scanner->next_token();
        if (!EXPECT_LEFT_BRACE(lbrace)) {
                rin_error_at(lbrace.location(),
                             "For-loop expected left-brace '{' but received %s instead",
                             lbrace.str());
                this->_scanner->skip_line();
                delete ind_stmt;
                delete cond_stmt;
                delete incdec_stmt;
                return Statement::make_invalid(lbrace.location());
        }

        // Parse statements in for-loop scope.
        Scope* loop_scope = this->_backend->enter_scope();
        this->parse(false);
        this->_backend->leave_scope();

        // Create for-loop statement
        For_statement* loop_stmt = new For_statement(ind_stmt, cond_stmt,
                incdec_stmt, for_rid.location());
        loop_stmt->add_statements(loop_scope);

        return loop_stmt;
}

Statement* Parser::parse_while_statement()
{
        // Verify WHILE token.
        Token while_rid = this->_scanner->next_token();
        RIN_ASSERT(while_rid.classification() == Token::TOKEN_RID);
        RIN_ASSERT(while_rid.rid() == RID_WHILE);

        this->backend()->enter_scope();

        // Condition.
        Expression* cond = this->parse_conditional_expression();
        Statement* cond_stmt = NULL;
        if (cond) {
                cond_stmt = Statement::make_expression(cond, cond->location());
        } else {
                this->_scanner->skip_line();
                this->backend()->leave_scope();
                return Statement::make_invalid(while_rid.location());
        }

        // Skip EOL tokens before opening brace (allows brace on next line)
        while (this->_scanner->peek_token().classification() == Token::TOKEN_EOL)
                this->_scanner->next_token();

        // Opening brace
        Token lbrace = this->_scanner->next_token();
        if (!EXPECT_LEFT_BRACE(lbrace)) {
                rin_error_at(lbrace.location(),
                             "While-loop expected left-brace '{' but received %s instead",
                             lbrace.str());
                this->_scanner->skip_line();
                delete cond_stmt;
                this->backend()->leave_scope();
                return Statement::make_invalid(lbrace.location());
        }

        // Parse statements in while-loop scope.
        Scope* loop_scope = this->_backend->enter_scope();
        this->parse(false);
        this->_backend->leave_scope();

        // Reuse for-loop with NULL induction and NULL increment.
        For_statement* loop_stmt = new For_statement(NULL, cond_stmt,
                NULL, while_rid.location());
        loop_stmt->add_statements(loop_scope);

        return loop_stmt;
}

Statement* Parser::parse_var_dec_statement()
{
        // Verify type keyword token (float, int, or var).
        Token type_rid = this->_scanner->next_token();
        RIN_ASSERT(type_rid.rid() == RID_FLOAT || type_rid.rid() == RID_INT
                   || type_rid.rid() == RID_VAR);

        Token ident = this->_scanner->peek_token();
        if (ident.classification() != Token::TOKEN_IDENT) {
                rin_error_at(ident.location(),
                        "Variable declaration expected identifier but received %s instead. Expected a variable name",
                        ident.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(ident.location());
        }

        /*
         * Henceforth, the statement can either end in a semicolon or an
         * assignment statement
         */

        if (EXPECT_SEMICOLON(this->_scanner->peek_nth_token(1))) {
                // Consume identifier & semicolon tokens
                this->_scanner->next_token();
                this->_scanner->next_token();

                // Create var declaration
                Named_object* obj = this->backend()->current_scope()->
                        define_obj(*ident.identifier(), ident.location());

                if (!obj) return Statement::make_invalid(ident.location());
                return Statement::make_variable_declaration(obj);
        }

        // Create a declaration and then parse it's assignment
        Named_object* obj = this->backend()->current_scope()->
                define_obj(*ident.identifier(), ident.location());

        // Redefinition.
        if (!obj) return Statement::make_invalid(ident.location());

        Statement* declr = Statement::make_variable_declaration(obj);
        Statement* assign = this->parse_assignment_statement();

        // Create compound statement
        return Statement::make_compound(declr, assign, declr->location());
}

Statement* Parser::parse_assignment_statement()
{
        // Verify IDENT token
        Token ident = this->_scanner->next_token();
        RIN_ASSERT(ident.classification() == Token::TOKEN_IDENT);

        /*
         * Verify ASSIGN or compound-assign operator; don't use RIN_ASSERT
         * because this might've been called by the var declaration parser,
         * in which case we'd want to return an error.
         */
        Token assign = this->_scanner->next_token();
        RIN_OPERATOR compound_op = OPER_ILLEGAL;
        if (assign.classification() == Token::TOKEN_OPERATOR) {
                switch (assign.op()) {
                case OPER_ADD_ASSIGN: compound_op = OPER_ADD; break;
                case OPER_SUB_ASSIGN: compound_op = OPER_SUB; break;
                case OPER_MUL_ASSIGN: compound_op = OPER_MUL; break;
                case OPER_QUO_ASSIGN: compound_op = OPER_QUO; break;
                case OPER_ASSIGN:     break;
                default:
                        rin_error_at(assign.location(),
                                "Assignment statement expected '=' operator, but received %s instead. Did you mean '='?",
                                assign.str());
                        this->_scanner->skip_line();
                        return Statement::make_invalid(assign.location());
                }
        } else {
                rin_error_at(assign.location(),
                        "Assignment statement expected '=' operator, but received %s instead. Did you mean '='?",
                        assign.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(assign.location());
        }

        // Create left-hand variable reference expression.
        Named_object* obj = this->backend()->current_scope()->lookup(*ident.identifier());
        if (!obj) {
                rin_error_at(ident.location(), "'%s' is undeclared",
                        ident.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(ident.location());
        }

        Expression* lhs_ref = Expression::make_var_reference(obj, ident.location());

        // Parse right-hand side expression.
        Expression* binary = this->parse_binary_expression();
        if (!binary) {
                this->_scanner->skip_line();
                delete lhs_ref;
                return Statement::make_invalid(assign.location());
        }

        /*
         * For compound assignments (+=, -=, *=, /=), desugar into
         * x = x op rhs. Create a second var reference for the RHS.
         */
        if (compound_op != OPER_ILLEGAL) {
                Expression* rhs_ref = Expression::make_var_reference(obj, ident.location());
                binary = Expression::make_binary(compound_op, rhs_ref, binary, assign.location());
        }

        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());
        return Statement::make_assignment(lhs_ref, binary, ident.location());
}

Statement* Parser::parse_inc_dec_statement()
{
        Token ident = this->_scanner->next_token();
        Token op = this->_scanner->next_token();
        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());

        // Create var reference
        Named_object* obj = this->backend()->current_scope()->
                lookup(*ident.identifier());
        if (!obj) {
                rin_error_at(ident.location(), "'%s' is undefined",
                        ident.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(ident.location());
        }

        Expression* var_reference = Expression::make_var_reference
                (obj, ident.location());

        // Create unary expression
        Expression* unary = Expression::make_unary
                (op.op(), var_reference, ident.location());

        return (op.op() == OPER_INC) ? Statement::make_inc(unary) :
                Statement::make_dec(unary);
}

Statement* Parser::parse_function_declaration()
{
        // Consume 'fn' token.
        Token fn_tok = this->_scanner->next_token();
        RIN_ASSERT(fn_tok.classification() == Token::TOKEN_RID);
        RIN_ASSERT(fn_tok.rid() == RID_FN);

        // Read function name identifier.
        Token name_tok = this->_scanner->next_token();
        if (name_tok.classification() != Token::TOKEN_IDENT) {
                rin_error_at(name_tok.location(),
                        "Function declaration expected identifier but received %s instead",
                        name_tok.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(fn_tok.location());
        }
        std::string name = *name_tok.identifier();

        // Consume '(' token.
        Token lparen = this->_scanner->next_token();
        if (lparen.classification() != Token::TOKEN_OPERATOR ||
            lparen.op() != OPER_LPAREN) {
                rin_error_at(lparen.location(),
                        "Function declaration expected '(' but received %s instead",
                        lparen.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(fn_tok.location());
        }

        // Read comma-separated parameter identifiers until ')'.
        std::vector<std::string> params;
        Token next = this->_scanner->peek_token();
        if (!(next.classification() == Token::TOKEN_OPERATOR &&
              next.op() == OPER_RPAREN)) {
                while (true) {
                        Token param = this->_scanner->next_token();
                        if (param.classification() != Token::TOKEN_IDENT) {
                                rin_error_at(param.location(),
                                        "Function parameter expected identifier but received %s instead",
                                        param.str());
                                this->_scanner->skip_line();
                                return Statement::make_invalid(fn_tok.location());
                        }
                        params.push_back(*param.identifier());

                        // Check for comma or closing paren.
                        Token sep = this->_scanner->peek_token();
                        if (sep.classification() == Token::TOKEN_OPERATOR &&
                            sep.op() == OPER_RPAREN)
                                break;

                        // Expect comma between parameters.
                        Token comma = this->_scanner->next_token();
                        if (comma.classification() != Token::TOKEN_OPERATOR) {
                                rin_error_at(comma.location(),
                                        "Expected ',' or ')' in parameter list but received %s instead",
                                        comma.str());
                                this->_scanner->skip_line();
                                return Statement::make_invalid(fn_tok.location());
                        }
                }
        }

        // Consume ')' token.
        Token rparen = this->_scanner->next_token();
        if (rparen.classification() != Token::TOKEN_OPERATOR ||
            rparen.op() != OPER_RPAREN) {
                rin_error_at(rparen.location(),
                        "Function declaration expected ')' but received %s instead",
                        rparen.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(fn_tok.location());
        }

        // Consume '{' token.
        Token lbrace = this->_scanner->next_token();
        if (!EXPECT_LEFT_BRACE(lbrace)) {
                rin_error_at(lbrace.location(),
                        "Function declaration expected '{' but received %s instead",
                        lbrace.str());
                this->_scanner->skip_line();
                return Statement::make_invalid(fn_tok.location());
        }

        // Enter function body scope and define parameters as named objects.
        Scope* body = this->_backend->enter_scope();
        for (auto itr = params.begin(); itr != params.end(); ++itr) {
                body->define_obj(*itr, fn_tok.location());
        }

        // Parse function body.
        this->parse(false);

        return Statement::make_function(name, params, body, fn_tok.location());
}

Statement* Parser::parse_return_statement()
{
        // Consume 'return' token.
        Token ret_tok = this->_scanner->next_token();
        RIN_ASSERT(ret_tok.classification() == Token::TOKEN_RID);
        RIN_ASSERT(ret_tok.rid() == RID_RETURN);

        // Check for empty return (semicolon or EOL immediately after).
        Token next = this->_scanner->peek_token();
        if (EXPECT_SEMICOLON(next)) {
                this->_scanner->next_token();
                return Statement::make_return(NULL, ret_tok.location());
        }

        // Parse the return expression.
        Expression* expr = this->parse_binary_expression();
        if (!expr) {
                this->_scanner->skip_line();
                return Statement::make_invalid(ret_tok.location());
        }

        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());
        return Statement::make_return(expr, ret_tok.location());
}

Statement* Parser::parse_call_statement(const std::string& name, Location loc)
{
        // Consume '(' token.
        Token lparen = this->_scanner->next_token();
        RIN_ASSERT(lparen.classification() == Token::TOKEN_OPERATOR);
        RIN_ASSERT(lparen.op() == OPER_LPAREN);

        // Parse comma-separated argument expressions until ')'.
        std::vector<Expression*> args;
        Token next = this->_scanner->peek_token();
        if (!(next.classification() == Token::TOKEN_OPERATOR &&
              next.op() == OPER_RPAREN)) {
                while (true) {
                        Expression* arg = this->parse_binary_expression();
                        if (!arg) {
                                // Clean up already-parsed args.
                                for (auto itr = args.begin(); itr != args.end(); ++itr)
                                        delete *itr;
                                this->_scanner->skip_line();
                                return Statement::make_invalid(loc);
                        }
                        args.push_back(arg);

                        // Check for closing paren.
                        Token sep = this->_scanner->peek_token();
                        if (sep.classification() == Token::TOKEN_OPERATOR &&
                            sep.op() == OPER_RPAREN)
                                break;

                        // TODO: Comma handling -- currently expressions parse
                        // until semicolon, so comma must be handled differently
                        // once a comma operator exists.
                        break;
                }
        }

        // Consume ')' token.
        Token rparen = this->_scanner->next_token();
        if (rparen.classification() != Token::TOKEN_OPERATOR ||
            rparen.op() != OPER_RPAREN) {
                rin_error_at(rparen.location(),
                        "Function call expected ')' but received %s instead",
                        rparen.str());
                for (auto itr = args.begin(); itr != args.end(); ++itr)
                        delete *itr;
                this->_scanner->skip_line();
                return Statement::make_invalid(loc);
        }

        EXPECT_SEMICOLON_ERR(this->_scanner->next_token());

        Expression* call = Expression::make_call(name, args, loc);
        return Statement::make_expression(call, loc);
}

// --- Expressions ---

/*
 * Represents an expression or operator in an abstract syntax tree.
 * Used in expression parsing.
 */
class Expression_node
{
public:
        // The node type
        enum node_type { INVALID_NODE, OPERATOR_NODE, VAR_NODE, FLOAT_NODE };

        Expression_node(Token token, Backend* backend)
        {
                this->_location = token.location();
                this->_str = token.string();
                this->_backend = backend;

                Token::Classification cls = token.classification();
                if (cls == Token::TOKEN_IDENT) {
                        _type = VAR_NODE;

                        Named_object* obj = this->_backend->current_scope()->
                                lookup(*token.identifier());

                        if (!obj) {
                                rin_error_at(token.location(), "'%s' is undefined", token.str());
                                _type = INVALID_NODE;
                                return;
                        }

                        Expression* var_ref = Expression::make_var_reference
                                (obj, token.location());

                        this->_value.expr = var_ref;

                } else if (cls == Token::TOKEN_FLOAT) {
                        _type = FLOAT_NODE;

                        Expression* flt = Expression::make_float
                                (token.float_value(), token.location());

                        this->_value.expr = flt;

                } else if (cls == Token::TOKEN_INTEGER) {
                        _type = FLOAT_NODE;

                        Expression* intExpr = Expression::make_integer
                                (token.int_value(), token.location());

                        this->_value.expr = intExpr;

                } else if (cls == Token::TOKEN_OPERATOR) {
                        _type = OPERATOR_NODE;
                        this->_value.oper = token.op();

                } else {
                        rin_error_at(token.location(),
                                "Token '%s' is not an expression, operator, or variable reference",
                                token.str());
                        _type = INVALID_NODE;
                }
        }

        Backend* backend()
        { return this->_backend; }

        // Return the rightmost child or itself.
        Expression_node* rightmost_child()
        {
                if (this->is_invalid())
                        return NULL;

                if (this->type() == VAR_NODE || this->type() == FLOAT_NODE)
                        return this;

                if (this->is_unary() && this->left_child != NULL)
                        return this->left_child->rightmost_child();

                if (this->left_child != NULL && this->right_child == NULL)
                        return this->left_child->rightmost_child();

                if (this->right_child != NULL)
                        return this->right_child->rightmost_child();

                return NULL;
        }

        /*
         * Expression_node does NOT own the underlying Expression*.
         *
         * Expressions are created during node construction (for VAR_NODE
         * and FLOAT_NODE types) and later extracted by get_expression(),
         * which incorporates them into the final AST tree. After
         * get_expression() returns, the Expression* is owned by the
         * caller (or by a parent Expression such as Binary_expression
         * or Unary_expression).
         *
         * On error paths, expr_abort() is called to delete the
         * Expression* before the node is destroyed. This two-phase
         * cleanup avoids double-free: the destructor only deletes
         * child nodes (left_child, right_child), never the Expression*.
         */
        ~Expression_node()
        {
                delete left_child;
                delete right_child;
        }

        /*
         * Abort is for when expression parsing fails and we need to
         * clean up the underlying Expression*. Only non-operator,
         * non-invalid nodes hold an Expression* that must be freed.
         * This must be called before deleting the node on error paths,
         * since ~Expression_node() intentionally does NOT delete the
         * expression (see ownership comment above).
         */
        void expr_abort()
        {
                if (_type != OPERATOR_NODE && _type != INVALID_NODE)
                        delete this->_value.expr;
        }

        Location location()
        { return this->_location; }

        bool is_invalid()
        { return (_type == INVALID_NODE); }

        node_type type()
        { return this->_type; }

        // Returns true if the node is an operator and is unary.
        bool is_unary()
        {
                if (this->_type != OPERATOR_NODE)
                        return false;
                if (this->_value.oper == OPER_INC || this->_value.oper == OPER_DEC ||
                    this->_value.oper == OPER_NOT || this->_value.oper == OPER_NEG ||
                    this->_value.oper == OPER_BNOT)
                        return true;
                return false;
        }

        // Return the underlying expression (if applicable)
        Expression* expression()
        {
                RIN_ASSERT(_type != INVALID_NODE && _type != OPERATOR_NODE);
                return this->_value.expr;
        }

        // Return the underlying operator (if applicable)
        RIN_OPERATOR oper()
        {
                RIN_ASSERT(_type == OPERATOR_NODE);
                return this->_value.oper;
        }

        // Return the precedence of the underlying operator.
        int precedence()
        {
                if (_type != OPERATOR_NODE)
                        return -1;
                return OPERATOR_PRECEDENCE[(int)this->_value.oper];
        }

        // Returns true if this is an operator and a close parenthesis.
        bool is_close_paren()
        {
                if (_type != OPERATOR_NODE)
                        return false;
                if (this->_value.oper != OPER_RPAREN)
                        return false;
                return true;
        }

        // Returns true if this is an operator and an open parenthesis.
        bool is_open_paren()
        {
                if (_type != OPERATOR_NODE)
                        return false;
                if (this->_value.oper != OPER_LPAREN)
                        return false;
                return true;
        }

        // Returns the C-string pointer of the expression for debug.
        const char* str()
        { return this->_str.c_str(); }

        // Adds a child to the node
        void add_child(Expression_node* a, Expression_node* b = NULL) {
                RIN_ASSERT(this->_type == OPERATOR_NODE);
                if (b == NULL) RIN_ASSERT(this->is_unary());
                left_child = a;
                right_child = b;
        }

        // Recursively traverses the node to produce an expression.
        Expression* get_expression()
        {
                RIN_ASSERT(this->_type != INVALID_NODE);
                if (this->_type != OPERATOR_NODE)
                        return this->_value.expr;

                RIN_ASSERT(this->left_child);
                Expression* left_expr = left_child->get_expression();

                if (this->is_unary())
                        return Expression::make_unary
                                (this->_value.oper, left_expr, this->_location);

                RIN_ASSERT(this->right_child);
                Expression* right_expr = right_child->get_expression();

                return Expression::make_binary
                        (this->_value.oper, left_expr, right_expr, this->_location);
        }

private:
        // Unary nodes only take a left child
        node_type _type;

        Location _location;

        Backend* _backend;

        // Own str
        std::string _str;

        // Children
        Expression_node* left_child  = NULL;
        Expression_node* right_child = NULL;

        /*
         * Value of node is either an operator, or a var_reference or float
         * expression
         */
        union {
                RIN_OPERATOR oper;
                Expression* expr;
        } _value;
};

/*
 * Must return NULL if no valid expression is found. Must not consume
 * semicolon (or semicolon-equivalent) operators.
 */
Expression* Parser::parse_binary_expression()
{ return this->parse_expression(OPER_SEMICOLON); }

/*
 * Must return NULL if no valid expression is found. Must not consume
 * '{' operators.
 */
Expression* Parser::parse_conditional_expression(RIN_OPERATOR terminal)
{
        Expression* cond = this->parse_expression(terminal);
        if (!cond) return NULL;
        return Expression::make_conditional(cond, cond->location());
}

/*
 * Abort expression parsing. Cleans up all Expression_node objects in
 * both the operator stack and output queue. For each node, expr_abort()
 * is called first to delete any owned Expression* (non-operator,
 * non-invalid nodes), then the node itself is deleted (which recursively
 * deletes child nodes but NOT their expressions).
 */
void __abort_expr_parse
(std::stack<Expression_node*>& op, std::deque<Expression_node*> out)
{
        // Abort operator stack
        while(!op.empty()) {
                op.top()->expr_abort();
                delete op.top();
                op.pop();
        }

        // Abort output queue
        while (!out.empty()) {
                out.front()->expr_abort();
                delete out.front();
                out.pop_front();
        }
}

// Recursively parses a child of an expression node
Expression_node* __parse_ast_node(std::deque<Expression_node*>& output, bool& printed)
{
        if (output.empty()) return NULL;
        Expression_node* child = output.back();
        output.pop_back();

        if (child->type() == Expression_node::INVALID_NODE) {
                child->expr_abort();
                delete child;
                return NULL;
        }

        if (child->type() == Expression_node::VAR_NODE ||
            child->type() == Expression_node::FLOAT_NODE)
                return child;

        // --- Child is an operator ---
        Expression_node* right = __parse_ast_node(output, printed);
        if (!right) {
                child->expr_abort();
                delete child;
                return NULL;
        }

        if (child->is_unary()) {
                child->add_child(right);
                return child;
        }

        /*
         * If left most operator is missing, then that means
         * a binary operator was used incorrectly.
         */
        Expression_node* left = __parse_ast_node(output, printed);
        if (!left) {
                child->expr_abort();
                if (!printed) {
                        rin_error_at(child->location(),
                                     "Invalid type argument of %s",
                                     child->str());
                        printed = true;
                }
                delete child;
                return NULL;
        }

        child->add_child(left, right);
        return child;
}

// Parses what it assumes to be an expression until terminal operator
Expression* Parser::parse_expression(RIN_OPERATOR terminal)
{
        Location start_loc = this->_scanner->peek_token().location();

        // Shunting yard algorithm
        std::stack<Expression_node*> operators;
        std::deque<Expression_node*> output;

        bool resolves = false;
        Token token = this->_scanner->peek_token();
        Token prev_token = token;
        while (true) {
                Expression_node* node = NULL;
                switch (token.classification()) {

                case Token::TOKEN_OPERATOR:
                        if (token.op() == terminal) {
                                resolves = true;
                                goto exit_loop;
                        }

                        if (OPERATOR_PRECEDENCE[(int)token.op()] == -2) {
                                this->_scanner->next_token();
                                rin_error_at(token.location(),
                                        "Cannot use %s in expression",
                                        token.str());
                                __abort_expr_parse(operators, output);
                                return NULL;
                        }

                        /*
                         * Ternary expressions (? :) are not yet fully implemented.
                         * Issue an error if encountered.
                         */
                        if (token.op() == OPER_TERNARY || token.op() == OPER_COLON) {
                                this->_scanner->next_token();
                                rin_error_at(token.location(),
                                        "ternary expressions are not yet fully implemented");
                                __abort_expr_parse(operators, output);
                                return NULL;
                        }

                        /*
                         * Bitwise NOT (~) is always unary. Treat it
                         * the same as unary minus/negation.
                         */
                        if (token.op() == OPER_BNOT) {
                                node = new Expression_node(token, this->backend());
                                break;
                        }

                        /*
                         * Detect unary minus: '-' is unary when it appears at the
                         * start of an expression or after another operator (except
                         * close paren, INC, DEC which produce values).
                         */
                        if (token.op() == OPER_SUB && output.empty() && operators.empty()) {
                                Token neg_tok = Token::make_operator_token(OPER_NEG, token.location());
                                node = new Expression_node(neg_tok, this->backend());
                                break;
                        }
                        if (token.op() == OPER_SUB &&
                            prev_token.classification() == Token::TOKEN_OPERATOR &&
                            prev_token.op() != OPER_RPAREN &&
                            prev_token.op() != OPER_INC &&
                            prev_token.op() != OPER_DEC) {
                                Token neg_tok = Token::make_operator_token(OPER_NEG, token.location());
                                node = new Expression_node(neg_tok, this->backend());
                                break;
                        }

                        // --- DOES NOT BREAK ---

                case Token::TOKEN_FLOAT:
                case Token::TOKEN_INTEGER:
                case Token::TOKEN_IDENT:
                        node = new Expression_node(token, this->backend());
                        break;

                case Token::TOKEN_EOL:

                        /*
                         * Since EOL counts as a semicolon, stop parsing the
                         * expression. Unless the previous token is an operator,
                         * in which case the expression continues onto the next
                         * line.
                         */
                        if (terminal == OPER_SEMICOLON &&
                            prev_token.classification() != Token::TOKEN_OPERATOR) {
                                resolves = true;
                                goto exit_loop;
                        }

                        goto next_token;

                case Token::TOKEN_EOF:

                        /*
                         * EOF can count as a semicolon as long as it is not
                         * preceded by another operator (with parenthesis
                         * being the only exception).
                         */
                        if (terminal == OPER_SEMICOLON &&
                            (prev_token.classification() != Token::TOKEN_OPERATOR
                            || OPERATOR_PRECEDENCE[prev_token.op()] == -1
                            || prev_token.op() == OPER_INC
                            || prev_token.op() == OPER_DEC)) {
                                resolves = true;
                                goto exit_loop;
                        }

                        token = this->_scanner->next_token();
                        prev_token = token;
                        rin_error_at(token.location(),
                                     "Unresolved expression: reached EOF before expected %s",
                                     operator_name(terminal).c_str());
                        __abort_expr_parse(operators, output);
                        return NULL;

                case Token::TOKEN_STRING:
                case Token::TOKEN_CHARACTER:
                        token = this->_scanner->next_token();
                        prev_token = token;
                        rin_error_at(token.location(),
                                     "Cannot use %s token as expression value (NOT IMPLEMENTED)",
                                     token.classification_as_string().c_str());
                        __abort_expr_parse(operators, output);
                        return NULL;

                case Token::TOKEN_RID:
                        token = this->_scanner->next_token();
                        prev_token = token;
                        rin_error_at(token.location(),
                                     "Cannot use reserved identifier '%s' in expression",
                                     token.str());
                        __abort_expr_parse(operators, output);
                        return NULL;

                default:
                        token = this->_scanner->next_token();
                        prev_token = token;

                        // If invalid, then error already issued by scanner
                        __abort_expr_parse(operators, output);
                        return NULL;
                }

                if (node->is_invalid()) {
                        // Error already issued by Expression_node constructor
                        __abort_expr_parse(operators, output);
                        this->_scanner->next_token();
                        return NULL;
                }

                // If the token is a float or ident, add it to the output queue
                if (node->type() == Expression_node::FLOAT_NODE ||
                    node->type() == Expression_node::VAR_NODE) {
                        output.push_back(node);
                        goto next_token;
                }

                //  --- Node is an operator ---

                // If open paren, add to stack
                if (node->is_open_paren()) {
                        operators.push(node);
                        goto next_token;
                }

                /*
                 * If close paren, move operators into output queue
                 * until an open parenthesis is found. If not found,
                 * then the close parenthesis is unmatched.
                 */
                if (node->is_close_paren()) {
                        while (!operators.empty() && !operators.top()->is_open_paren()) {
                                output.push_back(operators.top());
                                operators.pop();
                        }

                        // Unmatched parenthesis
                        if (operators.empty() || !operators.top()->is_open_paren()) {
                                rin_error_at(token.location(), "Unmatched close parenthesis");
                                __abort_expr_parse(operators, output);

                                // Consume the parenthesis to prevent re-emitting errors.
                                prev_token = this->_scanner->next_token();
                                return NULL;
                        }

                        // Pop open parenthesis
                        operators.pop();
                        goto next_token;
                }

                // Pop any operators whose precedence is >= current operator
                while(!operators.empty() && (operators.top()->precedence() >= node->precedence())) {
                        output.push_back(operators.top());
                        operators.pop();
                }

                // Add operator to stack
                operators.push(node);

        next_token:
                prev_token = this->_scanner->next_token();
                token = this->_scanner->peek_token();
        }
exit_loop:
        token = this->_scanner->peek_token();
        prev_token = token;

        /*
         * If it doesn't resolve, then abort parsing and return a new
         * expression. The new expression will be deleted once the
         * function caller (i.e parse_for_statement) realizes that the
         * terminating operator is not there.
         */
        if (!resolves) {
                __abort_expr_parse(operators, output);
                return NULL;
        }

        // Add any remaining operators to the output queue
        while (!operators.empty()) {
                output.push_back(operators.top());
                operators.pop();
        }

        // If the expression was empty, then it'll break here.
        if (output.empty()) {
                rin_error_at(start_loc, "Expected expression");
                return NULL;
        }

        // --- Create Abstract Syntax Tree ---
        bool __found_err = false;
        Expression_node* super_root = __parse_ast_node(output, __found_err);
        if (!super_root) {
                __abort_expr_parse(operators, output);
                return NULL;
        }

        // Missing operator
        if (!output.empty()) {
                Expression_node* rightmost = super_root->rightmost_child();
                rin_error_at(super_root->location(), "Expected ';' before '%s'",
                        rightmost->str());
                __abort_expr_parse(operators, output);
                delete super_root;
                return NULL;
        }

        // --- Done Parsing ---
        Expression* binary = super_root->get_expression();

        /*
         * Delete the Expression_node tree. This recursively deletes all
         * child nodes via ~Expression_node(), but does NOT delete the
         * Expression* values they held — those have been incorporated
         * into the AST by get_expression() and are now owned by the
         * returned Expression* (e.g., as children of Binary_expression
         * or Unary_expression nodes).
         */
        delete super_root;
        return binary;
}
