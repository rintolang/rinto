#include "token.h"

bool TOKENS_INIT = false;
std::map<std::string, token> tokens;

/* Assume unknown tokens are identities. Otherwise
returns the value assosciated when tok is taken
as a key to the 'tokens' map.*/
token lookup(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  if (tokens.find(tok) == tokens.end()) {
    return IDENT;
  }

  return tokens[tok];
}

bool is_type(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  token t = lookup(tok);
  if (t > type_beg && t < type_end) {
    return true;
  }

  return false;
}

bool is_modifier(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  token t = lookup(tok);
  if (t > mod_beg && t < mod_end) {
    return true;
  }

  return false;
}

bool is_expression(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  token t = lookup(tok);
  if (t > expr_beg && t < expr_end) {
    return true;
  }

  return false;
}

bool is_state(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  token t = lookup(tok);
  if (t > state_beg && t < state_end) {
    return true;
  }

  return false;
}

bool is_operator(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  token t = lookup(tok);
  if (t > operator_beg && t < operator_end){
    return true;
  }

  return false;
}

bool is_identifier(std::string tok) {
  if (!TOKENS_INIT) init_keywords();

  if (lookup(tok) == IDENT) {
      return true;
  }

  return false;
}

/* Assign keyword strings to their assosciated Token types.
Map must be initialized before calling token.h functions.*/
void init_keywords() {
  TOKENS_INIT = false;

  token_pair pairs[] = {
    /* Modifiers */
    token_pair("unsigned", RID_UNSIGNED),
    token_pair("long", RID_LONG),
    token_pair("const", RID_CONST),
    token_pair("short", RID_SHORT),

    /* Types */
    token_pair("int", RID_INT),
    token_pair("float", RID_FLOAT),
    token_pair("char", RID_CHAR),
    token_pair("string", RID_STRING),
    token_pair("double", RID_DOUBLE),
    token_pair("void", RID_VOID),
    token_pair("struct", RID_STRUCT),
    token_pair("bool", RID_BOOL),

    /* Expressions */
    token_pair("if", RID_IF),
    token_pair("else", RID_ELSE),
    token_pair("for", RID_FOR),
    token_pair("switch", RID_SWITCH),
    token_pair("case", RID_CASE),
    token_pair("break", RID_CONTINUE),
    token_pair("return", RID_RETURN),
    token_pair("goto", RID_GOTO),
    token_pair("while", RID_WHILE),
    token_pair("default", RID_DEFAULT),
    token_pair("sizeof", RID_SIZEOF),

    /* State */
    token_pair("true", RID_TRUE),
    token_pair("false", RID_FALSE),
    token_pair("null", RID_NULL),

    /* Operators */
    token_pair("+", ADD),
    token_pair("-", SUB),
    token_pair("*", MUL),
    token_pair("/", QUO),
    token_pair("%", REM),

    token_pair("&", AND),
    token_pair("|", OR),
    token_pair("^", XOR),
    token_pair("<<", SHL),
    token_pair(">>", SHR),
    token_pair("&^", AND_NOT),

    token_pair("+=", AND_ASSIGN),
    token_pair("-=", SUB_ASSIGN),
    token_pair("*=", MUL_ASSIGN),
    token_pair("/=", QUO_ASSIGN),
    token_pair("%=", REM_ASSIGN),

    token_pair("&=", AND_ASSIGN),
    token_pair("|=", OR_ASSIGN),
    token_pair("^=", XOR_ASSIGN),
    token_pair("<<=", SHL_ASSIGN),
    token_pair(">>=", SHR_ASSIGN),
    token_pair("&^=", AND_NOT_ASSIGN),

    token_pair("&&", LAND),
    token_pair("||", LOR),
    token_pair("<-", ARROW),
    token_pair("++", INC),
    token_pair("--", DEC),

    token_pair("==", EQL),
    token_pair("<", LSS),
    token_pair(">", GTR),
    token_pair("=", ASSIGN),
    token_pair("!", NOT),

    token_pair("!=", NEQ),
    token_pair("<=", LEQ),
    token_pair(">=", GEQ),

    token_pair("(", LPAREN),
    token_pair("[", LBRACK),
    token_pair("{", LBRACE),
    token_pair(",", COMMA),
    token_pair(".", PERIOD),

    token_pair(")", RPAREN),
    token_pair("]", RBRACK),
    token_pair("}", RBRACE),
    token_pair(";", SEMICOLON),
    token_pair(":", COLON)
  };

  // Insert into map
  for (int i = 0; i<72; i++) {
    tokens.insert(pairs[i]);
  }

}