#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <regex>
#include <vector>
#include <stack>
#include <queue>
#include <stdexcept>
#include <regex>
#include <cstdarg>

#include "tokens.h"
#include "rin-file.h"
#include "lang.h"

/*
 * A scanError arises when the scanner detects a
 * formatting or syntactical error.
 */
typedef struct {
        std::string msg;
        position    pos;

} scanError;

/* scanCb is a wrapper for scanner callback functions. */
typedef struct {
        void (*cbIdent)     (std::string, position) = nullptr;
        bool (*cbNewfile)   (std::string)           = nullptr;
        void (*cbScanError) (scanError)             = nullptr;
        void (*cbEOF)       ()                      = nullptr;

} scanCb;

/*
 * scanToken represents the main output structure of
 * the scanner. It describes the token, its type and
 * (if applicable) its literal and string values.
 */
typedef struct {
        tokenType   type;
        token       tok;
        std::string str;
        std::string literal;

} scanToken;

/*
 * A scanExpect describes a token that we expect to
 * eventually receive.
 */
typedef struct {
        scanToken have;

        /*
         * Want types:
         *  0. Expect tokens.
         *  1. Expect tokenTypes.
         *  2. Expect any of tokens.
         *  3. Expect any of token types.
         */
        char                   wantType;

        std::vector<tokenType>  T;
        std::vector<token>      K;

        /*
         * Whether the expectation must be
         * fulfilled immediately.
         */
        bool                   next = false;
        position               pos;

} scanExpect;

class Scanner
{
private:
        /* Scanner state machine. */
        RinFile&               Source;
        scanCb                 Callbacks;
        std::vector<scanError> Errors;
        std::stack<scanExpect> Expecting;
        std::queue<scanToken>  Buffer;

        void skipWhitespace();
        bool isWhitespace(char ch);

        /*
         * Expect particular token/tokenTypes. expectNext,
         * if true, requires the expectation to be fulfilled
         * by the next token - or by EOF if false. wantType
         * describes how the expectation is evaluated, count
         * is the number of tokens tok.
         */
        void expect(scanToken have, bool expectNext,
                    char wantType, int count, token tok...);

        void expect(scanToken have, bool expectNext,
                    char wantType, int count, tokenType tok...);

        /*
         * acknowledge ingests a token and handles scanner
         * expectations. It may also issue an error for some
         * unexpected tokens.
         */
        void acknowledge(scanToken tok);

        /*
         * Reads the source and appends the next token to
         * the Buffer. Returns true if there are more
         * tokens to read.
         */
        bool scanNext();

        /* Adds an error to the Errors vector. */
        void errorf(const char* fmt...);

        /*
         * Reads the Expecting stack and generates an error
         * for each remaining expectation. It assumes that
         * the scanner has finished reading. Returns true
         * if an error has been issued.
         */
        bool errorsFromExpect();

        /*
         * Reads the latest Expecting stack element and
         * evaluates whether tok matchess expecations. It
         * returns true if an error is detected.
         */
        bool errorsFromExpect(scanToken tok);
public:
        explicit Scanner(RinFile& src) : Source(src) {};

        /*
         * Sets cbs as the Scanner's callback functions. See
         * defition of scanCb for available callbacks.
         */
        void setCallbacks(scanCb cbs);

        /*
         * Returns a buffer with the next three tokens.
         * Three tokens are all that's needed for iteratively
         * parsing rin files.
         */
        int bufferTokens(scanToken (&buffer)[3]);

        /* Appends the scanner's errors to the errors vector.*/
        void getErrors(std::vector<scanError>& errors);

        /*
         * Given a scanToken vector 'tokens', getTokens reads
         * the Source, starting from its current position and ending
         * on its last file, and pushes each token to the vector.
         */
        void getTokens(std::vector<scanToken>& tokens);
};

#endif /* SCANNER_H  */
