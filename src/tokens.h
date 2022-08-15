#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <map>
#include <regex>

#include "lang.h"

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 5d5fff6 (use linux-style comments)
/*
 * Symbols such as (, [, { naturally come in pairs. getSymbolPair returns the
 * right-hand symbol given a left-hand symbol, or vice versa. If not a paired
 * symbol (i.e 'int'), then ILLEGAL is returned.
 */
<<<<<<< HEAD
=======
/* Symbols such as (, [, { naturally come in pairs. getSymbolPair returns the
   right-hand symbol given a left-hand symbol, or vice versa. If not a paired
   symbol (i.e 'int'), then ILLEGAL is returned.*/
>>>>>>> ca7c841 (created tokens)
=======
>>>>>>> 5d5fff6 (use linux-style comments)
token getSymbolPair(token sym);

/* Given a string 'str', lookup returns the assosciated token. */
token lookup(std::string str);

<<<<<<< HEAD
<<<<<<< HEAD
=======
>>>>>>> 5d5fff6 (use linux-style comments)
/*
 * Given a token or string, typeOf returns the assosciated tokenType. If passed
 * as a string, the function can also evaluate whether the string is a literal
 * or identifier.
 */
<<<<<<< HEAD
=======
/* Given a token or string, typeOf returns the assosciated tokenType. If passed
   as a string, the function can also evaluate whether the string is a literal
   or identifier.*/
>>>>>>> ca7c841 (created tokens)
=======
>>>>>>> 5d5fff6 (use linux-style comments)
tokenType typeOf(token tok);
tokenType typeOf(char tok);
tokenType typeOf(std::string tok);

/* Get token as string */
std::string toString(token tok);
std::string toString(tokenType tok);

#endif
