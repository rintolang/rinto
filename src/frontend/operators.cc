#include "operators.hpp"

/*
 * Corresponds to the operator enumeration in operators.hpp.
 * Passing an operator as an index will return its precedence.
 * -1 means the operator can be parsed but takes no precedence.
 * -2 means stop parsing.
 */
int OPERATOR_PRECEDENCE[] =
{
        -2, 4, 4, 5, 5,  5,  1,  0,  0,  0,  2,  3, 3,
        -2, 6, 2, 3, 3, -1, -2, -2, -1, -2, -2, -2
};

/*
 * The operator map enables O(1) conversion from an operator string to the
 * respective RIN_OPERATOR
 */
bool __operator_map_is_inited__ = false;
std::unordered_map<std::string, RIN_OPERATOR> __operator_map__;
void __init_op_map__()
{
        if (__operator_map_is_inited__)
                return;

        typedef std::pair<const std::string, RIN_OPERATOR> op_pair;

        __operator_map__.insert(op_pair("+", OPER_ADD));
        __operator_map__.insert(op_pair("-", OPER_SUB));
        __operator_map__.insert(op_pair("*", OPER_MUL));
        __operator_map__.insert(op_pair("/", OPER_QUO));
        __operator_map__.insert(op_pair("%", OPER_REM));

        __operator_map__.insert(op_pair("&&", OPER_LAND));
        __operator_map__.insert(op_pair("||", OPER_LOR));
        __operator_map__.insert(op_pair("++", OPER_INC));
        __operator_map__.insert(op_pair("--", OPER_DEC));

        __operator_map__.insert(op_pair("==", OPER_EQL));
        __operator_map__.insert(op_pair("<", OPER_LSS));
        __operator_map__.insert(op_pair(">", OPER_GTR));
        __operator_map__.insert(op_pair("=", OPER_ASSIGN));
        __operator_map__.insert(op_pair("!", OPER_NOT));

        __operator_map__.insert(op_pair("!=", OPER_NEQ));
        __operator_map__.insert(op_pair("<=", OPER_LEQ));
        __operator_map__.insert(op_pair(">=", OPER_GEQ));

        // Left-hand-side operators
        __operator_map__.insert(op_pair("(", OPER_LPAREN));
        __operator_map__.insert(op_pair("[", OPER_LBRACK));
        __operator_map__.insert(op_pair("{", OPER_LBRACE));

        // Right-hand-side operators
        __operator_map__.insert(op_pair(")", OPER_RPAREN));
        __operator_map__.insert(op_pair("]", OPER_RBRACK));
        __operator_map__.insert(op_pair("}", OPER_RBRACE));

        // Semicolon
        __operator_map__.insert(op_pair(";", OPER_SEMICOLON));

        __operator_map_is_inited__ = true;
}

bool is_paired_symbol(RIN_OPERATOR symbol)
{
        if (symbol >= OPER_LPAREN && symbol <= OPER_RBRACE)
                return true;
        return false;
}

bool is_lefthand_op(RIN_OPERATOR symbol) {
        return (symbol >= OPER_LPAREN && symbol <= OPER_LBRACE);
}

bool is_righthand_op(RIN_OPERATOR symbol) {
        return (symbol >= OPER_RPAREN && symbol <= OPER_RBRACE);
}

RIN_OPERATOR get_symbol_pair(RIN_OPERATOR symbol)
{
        if (symbol >= OPER_LPAREN && symbol <= OPER_LBRACE)
                return (RIN_OPERATOR)(symbol + 3);
        if (symbol >= OPER_RPAREN && symbol <= OPER_RBRACE)
                return (RIN_OPERATOR)(symbol - 3);

        return OPER_ILLEGAL;
}

bool is_rin_operator(char c)
{
        __init_op_map__();
        std::string op(1, c);
        return is_rin_operator(op);
}

bool is_rin_operator(const std::string& str)
{
        __init_op_map__();
        return __operator_map__[str];
}

RIN_OPERATOR op_lookup(const std::string& str)
{
        __init_op_map__();
        return __operator_map__[str];
}

std::string operator_name(RIN_OPERATOR op)
{
        /*
         * Crude approach but searching for a key by its value in
         * __operator_map__ is still O(n).
         */
        switch (op) {
        case OPER_ILLEGAL:   return "illegal operator";
        case OPER_ADD:       return "'+' operator";
        case OPER_SUB:       return "'-' operator";
        case OPER_MUL:       return "'*' operator";
        case OPER_QUO:       return "'/' operator";
        case OPER_REM:       return "'%' operator";
        case OPER_LAND:      return "'&&' operator";
        case OPER_LOR:       return "'||' operator";
        case OPER_INC:       return "'++' operator";
        case OPER_DEC:       return "'--' operator";
        case OPER_EQL:       return "'==' operator";
        case OPER_LSS:       return "'<' operator";
        case OPER_GTR:       return "'>' operator";
        case OPER_ASSIGN:    return "'=' operator";
        case OPER_NOT:       return "'!' operator";
        case OPER_NEQ:       return "'!=' operator";
        case OPER_LEQ:       return "'<=' operator";
        case OPER_GEQ:       return "'>=' operator";
        case OPER_LPAREN:    return "'(' operator";
        case OPER_LBRACK:    return "'[' operator";
        case OPER_LBRACE:    return "'{' operator";
        case OPER_RPAREN:    return "')' operator";
        case OPER_RBRACK:    return "']' operator";
        case OPER_RBRACE:    return "'}' operator";
        case OPER_SEMICOLON: return "';' operator";
        default:             return "illegal operator";
        }
}
