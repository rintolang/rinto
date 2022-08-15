#include "tokens.h"

std::map<std::string, token> tokenMap;
bool                         _inited = false;
void                         _initMap();

token getSymbolPair(token sym)
{
        if (!_inited) _initMap();

        /* Calculate offset */
        int diff = operator_right - operator_left;

        /* Left hand side operator, i.e: (, [, etc. */
        if (sym > operator_left && sym < operator_left_end)
                return (token)(sym + diff);

        /* Right hand side operator, i.e: ), ], etc. */
        if (sym > operator_right && sym < operator_right_end)
                return (token)(sym - diff);

        return ILLEGAL;
}

token lookup(std::string str)
{
        if (!_inited) _initMap();

        /* If key doesn't exist, then map returns 0. Fortunately,
         * that corresponds to the ILLEGAL token.
         */
        return tokenMap[str];
}

tokenType typeOf(token tok)
{
        if (!_inited) _initMap();

        if (tok == 1) return t_Symbol;

        if (tok > type_beg && tok < type_end)
                return t_Type;

        if (tok > mod_beg && tok < mod_end)
                return t_Modifier;

        if (tok > expr_beg && tok < expr_end)
                return t_Keyword;

        if (tok > state_beg && tok < state_end)
                return t_State;

        if (tok > operator_beg && tok < operator_end)
                return t_Symbol;

        return t_ILLEGAL;
}

tokenType typeOf(char tok) {
        std::string s;
        s += tok;
        return typeOf(s);
}

tokenType typeOf(std::string tok) {
        tokenType type = typeOf(lookup(tok));
        if (type != t_ILLEGAL) return type;

        /* Check for literals */
        if (
                /* Integer */
                std::regex_match(tok.begin(), tok.end(), std::regex(R"(\d+)")) ||

                /* Hex Integer */
                std::regex_match(tok.begin(), tok.end(), std::regex(R"(0x[ABCDEFabcdef]+)")) ||

                /* Float/double */
                std::regex_match(tok.begin(), tok.end(), std::regex(R"(\d+\.\d+)")) ||
                std::regex_match(tok.begin(), tok.end(), std::regex(R"((\d+f|\d+\.\d+f))")) ||

                /* String */
                std::regex_match(tok.begin(), tok.end(), std::regex(R"(\"\w+\")")) ||

                /* Character */
                std::regex_match(tok.begin(), tok.end(), std::regex(R"(\'\w\')"))

           ) return t_Literal;

        /* Identifier */
        if (std::regex_match(tok.begin(), tok.end(), std::regex(R"(([a-z]|[A-Z])\w+)")))
                return t_Ident;

        return t_ILLEGAL;
}

std::string toString(token tok)
{
        if (tok == EOL) return "EOL";
        for(auto &it : tokenMap) {
                if(it.second == tok)
                        return it.first;
        }

        return "";
}

std::string toString(tokenType tok)
{
        switch (tok) {
        case t_Type:
                return "Type";
        case t_Modifier:
                return "Modifier";
        case t_Keyword:
                return "Keyword";
        case t_State:
                return "True/False";
        case t_Symbol:
                return "Symbol";
        case t_Ident:
                return "Identity";
        case t_Literal:
                return "Literal";
        default:
                return "Illegal";
        }
}

void _initMap()
{
        typedef std::pair<const std::string, token> tokenPair;

        /* Types (and EOL) */
        tokenMap.insert(tokenPair("\n", EOL));
        tokenMap.insert(tokenPair("int", RID_INT));
        tokenMap.insert(tokenPair("float", RID_FLOAT));
        tokenMap.insert(tokenPair("char", RID_CHAR));
        tokenMap.insert(tokenPair("string", RID_STRING));
        tokenMap.insert(tokenPair("double", RID_DOUBLE));
        tokenMap.insert(tokenPair("void", RID_VOID));
        tokenMap.insert(tokenPair("bool", RID_BOOL));

        /* Modifiers */
        tokenMap.insert(tokenPair("unsigned", RID_UNSIGNED));
        tokenMap.insert(tokenPair("long", RID_LONG));
        tokenMap.insert(tokenPair("const", RID_CONST));
        tokenMap.insert(tokenPair("short", RID_SHORT));

        /* Expressions */
        tokenMap.insert(tokenPair("if", RID_IF));
        tokenMap.insert(tokenPair("else", RID_ELSE));
        tokenMap.insert(tokenPair("for", RID_FOR));
        tokenMap.insert(tokenPair("break", RID_BREAK));
        tokenMap.insert(tokenPair("continue", RID_CONTINUE));
        tokenMap.insert(tokenPair("return", RID_RETURN));
        tokenMap.insert(tokenPair("goto", RID_GOTO));
        tokenMap.insert(tokenPair("sizeof", RID_SIZEOF));

        /* States */
        tokenMap.insert(tokenPair("true", RID_TRUE));
        tokenMap.insert(tokenPair("false", RID_FALSE));

        /* Opreators */
        tokenMap.insert(tokenPair("+", ADD));
        tokenMap.insert(tokenPair("-", SUB));
        tokenMap.insert(tokenPair("*", MUL));
        tokenMap.insert(tokenPair("/", QUO));
        tokenMap.insert(tokenPair("%", REM));

        tokenMap.insert(tokenPair("&", AND));
        tokenMap.insert(tokenPair("|", OR));
        tokenMap.insert(tokenPair("^", XOR));
        tokenMap.insert(tokenPair("<<", SHL));
        tokenMap.insert(tokenPair(">>", SHR));
        tokenMap.insert(tokenPair("&^", AND_NOT));

        tokenMap.insert(tokenPair("+=", ADD_ASSIGN));
        tokenMap.insert(tokenPair("-=", SUB_ASSIGN));
        tokenMap.insert(tokenPair("*=", MUL_ASSIGN));
        tokenMap.insert(tokenPair("/=", QUO_ASSIGN));
        tokenMap.insert(tokenPair("%=", REM_ASSIGN));

        tokenMap.insert(tokenPair("&=", AND_ASSIGN));
        tokenMap.insert(tokenPair("|=", OR_ASSIGN));
        tokenMap.insert(tokenPair("^=", XOR_ASSIGN));
        tokenMap.insert(tokenPair("<<=", SHL_ASSIGN));
        tokenMap.insert(tokenPair(">>=", SHR_ASSIGN));
        tokenMap.insert(tokenPair("&^=", AND_NOT_ASSIGN));

        tokenMap.insert(tokenPair("&&", LAND));
        tokenMap.insert(tokenPair("||", LOR));
        tokenMap.insert(tokenPair("++", INC));
        tokenMap.insert(tokenPair("--", DEC));

        tokenMap.insert(tokenPair("==", EQL));
        tokenMap.insert(tokenPair("<", LSS));
        tokenMap.insert(tokenPair(">", GTR));
        tokenMap.insert(tokenPair("=", ASSIGN));
        tokenMap.insert(tokenPair("!", NOT));

        tokenMap.insert(tokenPair("!=", NEQ));
        tokenMap.insert(tokenPair("<=", LEQ));
        tokenMap.insert(tokenPair(">=", GEQ));

        tokenMap.insert(tokenPair(",", COMMA));
        tokenMap.insert(tokenPair(".", PERIOD));
        tokenMap.insert(tokenPair(";", SEMICOLON));
        tokenMap.insert(tokenPair(":", COLON));

        /* Left-hand-side operators */
        tokenMap.insert(tokenPair("(", LPAREN));
        tokenMap.insert(tokenPair("[", LBRACK));
        tokenMap.insert(tokenPair("{", LBRACE));

        /* Right-hand-side operators */
        tokenMap.insert(tokenPair(")", RPAREN));
        tokenMap.insert(tokenPair("]", RBRACK));
        tokenMap.insert(tokenPair("}", RBRACE));

        _inited = true;
}
