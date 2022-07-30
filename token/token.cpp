#include "token.h"

std::map<std::string, token> tokens;

void initTokens()
{
  tokenPair pairs[] = {
    /* Modifiers */
    tokenPair("unsigned", RID_UNSIGNED),
    tokenPair("long", RID_LONG),
    tokenPair("const", RID_CONST),
    tokenPair("short", RID_SHORT),

    /* Types */
    tokenPair("int", RID_INT),
    tokenPair("float", RID_FLOAT),
    tokenPair("char", RID_CHAR),
    tokenPair("string", RID_STRING),
    tokenPair("double", RID_DOUBLE),
    tokenPair("void", RID_VOID),
    tokenPair("struct", RID_STRUCT),
    tokenPair("bool", RID_BOOL),

    /* Expressions */
    tokenPair("if", RID_IF),
    tokenPair("else", RID_ELSE),
    tokenPair("for", RID_FOR),
    tokenPair("switch", RID_SWITCH),
    tokenPair("case", RID_CASE),
    tokenPair("break", RID_CONTINUE),
    tokenPair("return", RID_RETURN),
    tokenPair("goto", RID_GOTO),
    tokenPair("while", RID_WHILE),
    tokenPair("default", RID_DEFAULT),
    tokenPair("sizeof", RID_SIZEOF),

    /* State */
    tokenPair("true", RID_TRUE),
    tokenPair("false", RID_FALSE),
    tokenPair("null", RID_NULL)
  };

  for (int i = 0; i<27; i++) {
    tokens.insert(pairs[i]);
  }

}

token lookup(std::string tok)
{
  if (tokens.find(tok) == tokens.end()) {
    return ILLEGAL;
  }

  return tokens[tok];
}

bool isType(std::string tok)
{
  token t = lookup(tok);
  if (t > type_beg && t < type_end) {
    return true;
  }

  return false;
}

bool isModifier(std::string tok)
{
  token t = lookup(tok);
  if (t > mod_beg && t < mod_end) {
    return true;
  }

  return false;
}

bool isExpression(std::string tok)
{
  token t = lookup(tok);
  if (t > expr_beg && t < expr_end) {
    return true;
  }

  return false;
}

bool isState(std::string tok)
{
  token t = lookup(tok);
  if (t > state_beg && t < state_end) {
    return true;
  }

  return false;
}

bool isIdentifier(std::string tok)
{
  if (lookup(tok) == ILLEGAL) {
      return true;
  }

  return false;
}

