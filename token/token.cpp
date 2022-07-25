#include "token.h"

std::map<std::string, token> tokens;

void init_keywords() {
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
    token_pair("func", RID_FUNC),

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
    token_pair("null", RID_NULL)
  };

  for (int i = 0; i<27; i++) {
    tokens.insert(pairs[i]);
  }

}

token lookup(std::string tok) {
  if (tokens.find(tok) == tokens.end()) {
    return ILLEGAL;
  }

  return tokens[tok];
}

bool is_type(std::string tok) {
  token t = lookup(tok);
  if (t > type_beg && t < type_end) {
    return true;
  }

  return false;
}

bool is_modifier(std::string tok) {
  token t = lookup(tok);
  if (t > mod_beg && t < mod_end) {
    return true;
  }

  return false;
}

bool is_expression(std::string tok) {
  token t = lookup(tok);
  if (t > expr_beg && t < expr_end) {
    return true;
  }

  return false;
}

bool is_state(std::string tok) {
  token t = lookup(tok);
  if (t > state_beg && t < state_end) {
    return true;
  }

  return false;
}

bool is_identifier(std::string tok) {
  if (lookup(tok) == ILLEGAL) {
      return true;
  }

  return false;
}

