#ifndef LANG_H
#define LANG_H

typedef enum {
  t_ILLEGAL = 0,
  t_Type,  t_Modifier, t_Keyword, t_State, t_Symbol,
  t_Ident, t_Literal
} tokenType;

typedef enum {
        /* Basic */
        ILLEGAL = 0, EOL, R_EOF,

        /* Types */
        type_beg,
        RID_INT,  RID_FLOAT, RID_CHAR, RID_STRING, RID_DOUBLE,
        RID_VOID, RID_BOOL,
        type_end,

        /* Modifiers */
        mod_beg,
        RID_UNSIGNED, RID_LONG, RID_CONST, RID_SHORT,
        mod_end,

        /* Expressions */
        expr_beg,
        RID_IF,       RID_ELSE,   RID_FOR,  RID_BREAK,
        RID_CONTINUE, RID_RETURN, RID_GOTO, RID_SIZEOF,
        expr_end,

        /* State */
        state_beg,
        RID_TRUE, RID_FALSE, RID_NULL,
        state_end,

        /* Operators and delimiters */
        operator_beg,
        ADD,            // +
        SUB,            // -
        MUL,            // *
        QUO,            // /
        REM,            // %

        AND,            // &
        OR,             // |
        XOR,            // ^
        SHL,            // <<
        SHR,            // >>
        AND_NOT,        // &^

        ADD_ASSIGN,     // +=
        SUB_ASSIGN,     // -=
        MUL_ASSIGN,     // *=
        QUO_ASSIGN,     // /=
        REM_ASSIGN,     // %=

        AND_ASSIGN,     // &=
        OR_ASSIGN,      // |=
        XOR_ASSIGN,     // ^=
        SHL_ASSIGN,     // <<=
        SHR_ASSIGN,     // >>=
        AND_NOT_ASSIGN, // &^=

        LAND,           // &&
        LOR,            // ||
        INC,            // ++
        DEC,            // --

        EQL,            // ==
        LSS,            // <
        GTR,            // >
        ASSIGN,         // =
        NOT,            // !

        NEQ,            // !=
        LEQ,            // <=
        GEQ,            // >=

        COMMA,          // ,
        PERIOD,         // .
        SEMICOLON,      // ;
        COLON,          // :

        operator_left,
        LPAREN,         // (
        LBRACK,         // [
        LBRACE,         // {
        operator_left_end,

        operator_right,
        RPAREN,         // )
        RBRACK,         // ]
        RBRACE,         // }
        operator_right_end,

        operator_end
} token;

#endif
