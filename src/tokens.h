#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <map>
#include <regex>

#include "lang.h"

/*
 * Symbols such as (, [, { naturally come in pairs. getSymbolPair returns the
 * right-hand symbol given a left-hand symbol, or vice versa. If not a paired
 * symbol (i.e 'int'), then ILLEGAL is returned.
 */
token getSymbolPair(token sym);

/* Given a string 'str', lookup returns the assosciated token. */
token lookup(std::string str);

/*
 * Given a token or string, typeOf returns the assosciated tokenType. If passed
 * as a string, the function can also evaluate whether the string is a literal
 * or identifier.
 */
tokenType typeOf(token tok);
tokenType typeOf(char tok);
tokenType typeOf(std::string tok);

/* Get token as string */
std::string toString(token tok);
std::string toString(tokenType tok);

#endif
