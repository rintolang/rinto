#ifndef RIN_PARSER_HPP
#define RIN_PARSER_HPP

#include "scanner.hpp"
#include "statements.hpp"

/*
 * Expect token to be a semicolon or EOL. Token must be of type
 * defined in scanner.hpp
 */
#define EXPECT_SEMICOLON_ERR(token)                                                        \
{                                                                                          \
        if (!token.is_semicolon())                                                         \
                rin_error_at(token.location(),                                             \
                        "Expected semicolon at end of statement, but received %s instead", \
                         token.str());                                                     \
}

/*
 * Expect token to be a semicolon or EOL. Token must be of type
 * defined in scanner.hpp
 */
#define EXPECT_SEMICOLON(token) (token.is_semicolon())

/*
 * Expect token to be a right brace. Token must be of type
 * defined in scanner.hpp
 */
#define EXPECT_RIGHT_BRACE(token)                                                         \
(token.classification() == Token::TOKEN_OPERATOR && token.op() == OPER_RBRACE)

/*
 * Expect token to be a left brace. Token must be of type
 * defined in scanner.hpp
 */
#define EXPECT_LEFT_BRACE(token)                                                          \
(token.classification() == Token::TOKEN_OPERATOR && token.op() == OPER_LBRACE)

#define UNCLOSED_EXPR_PAREN_ERR(node) \
rin_error_at(node->location(), "Found unclosed parenthesis in expression");

// operators.cc
extern int OPERATOR_PRECEDENCE[];

// Parses a .RIN file into statement units
class Parser
{
public:
        // Create a parser from an instantiated scanner
        Parser(Scanner* scanner, Backend* backend);
        ~Parser();

        // Create a parser that reads from the specified path
        Parser(const std::string& path, Backend* backend);

        // Return the parser's scanner instance.
        Scanner* scanner()
        { return this->_scanner; }

        // Return the parser's backend instance.
        Backend* backend()
        { return this->_backend; }

        /*
         * If in a supercontext, then the parser will parse
         * every statement in the scanner/file. If in some
         * sub-scope, then the parser will parse until a
         * '}' token.
         */
        void parse(bool is_supercontext = true);

private:
        Scanner* _scanner;
        Backend* _backend;

        // Parses the next statement. Returns NULL if EOF.
        Statement* parse_next();

        // Parses an if statement
        Statement* parse_if_statement();

        // Parses a for statement
        Statement* parse_for_statement();

        // Parses a variable declaration
        Statement* parse_var_dec_statement();

        // Parses an assignment statement
        Statement* parse_assignment_statement();

        // Parses an increment/decrement statement
        Statement* parse_inc_dec_statement();

        // Parse expressions
        Expression* parse_binary_expression();

        Expression* parse_conditional_expression
        (RIN_OPERATOR terminal = OPER_LBRACE);

        /*
         * Parses what it assumes to be an expression until it reaches a
         * terminal operator
         */
        Expression* parse_expression(RIN_OPERATOR terminal);
};

#endif // RIN_PARSER_HPP
