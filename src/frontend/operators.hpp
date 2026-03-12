// operators.hpp - Operator enumeration, precedence, and utility functions
#ifndef RIN_OPERATORS_HPP
#define RIN_OPERATORS_HPP

#include <rin-system.hpp>

enum RIN_OPERATOR {
        OPER_ILLEGAL = 0,
        OPER_ADD,    OPER_SUB,    OPER_MUL,    OPER_QUO,
        OPER_REM,    OPER_LAND,   OPER_LOR,    OPER_INC,
        OPER_DEC,    OPER_EQL,    OPER_LSS,    OPER_GTR,
        OPER_ASSIGN, OPER_NOT,    OPER_NEQ,    OPER_LEQ,
        OPER_GEQ,    OPER_LPAREN, OPER_LBRACK, OPER_LBRACE,
        OPER_RPAREN, OPER_RBRACK, OPER_RBRACE, OPER_SEMICOLON,
        OPER_NEG, // Internal: unary negation (not scanned, created during parsing)
        OPER_ADD_ASSIGN, OPER_SUB_ASSIGN, OPER_MUL_ASSIGN, OPER_QUO_ASSIGN,
        OPER_BAND,   OPER_BOR,    OPER_BXOR,   OPER_BNOT,
        OPER_LSHIFT, OPER_RSHIFT,
        OPER_TERNARY, OPER_COLON,
        OPER_COMMA
};

/*
 * Returns whether this is a paired symbol.
 * ex. pairs: ( -> ), [ -> ], { -> }
 */
bool is_paired_symbol(RIN_OPERATOR symbol);

// Returns whether the symbol is a lefthand or righthand operator
bool is_lefthand_op(RIN_OPERATOR symbol);
bool is_righthand_op(RIN_OPERATOR symbol);

// Returns the symbol's pair, or OPER_ILLEGAL
RIN_OPERATOR get_symbol_pair(RIN_OPERATOR symbol);

// Converts a string into an operator, or returns OPER_ILLEGAL
RIN_OPERATOR op_lookup(const std::string& str);

// Returns true if RIN operator
bool is_rin_operator(char c);
bool is_rin_operator(const std::string& str);

// Gets an operator's name as a string
std::string operator_name(RIN_OPERATOR op);

#endif // RIN_OPERATORS_H
