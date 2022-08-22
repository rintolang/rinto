#include "scanner.h"

void Scanner::errorf(const char* fmt...)
{
        va_list args;
        va_start(args, fmt);
        std::string str = "error: ";

        while (*fmt != '\0') {
                std::string append(1, *fmt);

                if (*fmt == '%' && *(fmt+1) == 's') {
                        const char* i = va_arg(args, const char*);
                        append = "";

                        while (*i != '\0') {
                                append += *i;
                                ++i;
                        }

                        fmt++;
                }
                if (*fmt == '%' && *(fmt+1) == 'i') {
                        int i  = va_arg(args, int);
                        append = "";
                        append += (char)i;
                        ++fmt;
                }

                str += append;
                ++fmt;
        }

        scanError err = {str, Source.pos()};
        Errors.push_back(err);

        /* Clear away of error by scanning until next whitespace */
        int ch = Source.getChar();
        while (ch != ' ' && ch != '\t' && ch != '\n' && ch != -1)
                ch = Source.getChar();

        if (Callbacks.cbScanError != nullptr)
                Callbacks.cbScanError(err);

        va_end(args);
}

void Scanner::expect
(
        scanToken have,
        bool expectNext,
        char wantType,
        int count,
        token tok...
)
{
        scanExpect    exct;
        exct.have     = have;
        exct.wantType = wantType;
        exct.pos      = Source.pos();
        exct.next     = expectNext;

        std::va_list args;
        va_start(args, count);
        token arg = tok;

        for (int i = 0; i<count; i++) {
                exct.K.push_back(arg);
                arg = va_arg(args, token);
        }

        va_end(args);
        Expecting.push(exct);
}

void Scanner::expect
(
        scanToken have,
        bool expectNext,
        char wantType,
        int count,
        tokenType tok...
)
{
        scanExpect    exct;
        exct.have     = have;
        exct.wantType = wantType;
        exct.pos      = Source.pos();
        exct.next     = expectNext;

        std::va_list args;
        va_start(args, count);
        tokenType arg = tok;

        for (int i = 0; i<count; i++) {
                exct.T.push_back(arg);
                arg = va_arg(args, tokenType);
        }

        va_end(args);
        Expecting.push(exct);
}

bool Scanner::errorsFromExpect(scanToken tok)
{
        bool out = false;
        if (Expecting.size() > 0) {
                scanExpect top = Expecting.top();
                switch (top.wantType) {
                case 0: /* Expect tokens */
                {
                        token expcToken = top.K.front();
                        if (top.next && tok.tok != expcToken) {
                                errorf("Expected %s but got %s instead",
                                       &toString(expcToken)[0], &toString(tok.tok)[0]);

                                out = true;
                                top.K.erase(top.K.begin());
                        }

                        if (tok.tok == expcToken)
                                top.K.erase(top.K.begin());

                        /* All expectations have been cleared */
                        if (top.K.size() == 0)
                                Expecting.pop();
                }
                        break;
                case 1: /* Expect tokenTypes */
                {
                        tokenType expct = top.T.front();
                        if (top.next && tok.type != expct) {
                                errorf("Expected %s but got %s instead",
                                       &toString(expct)[0], &toString(tok.tok)[0]);

                                out = true;
                                top.T.erase(top.T.begin());
                        }

                        if (tok.type == expct)
                                top.T.erase(top.T.begin());

                        if (top.T.size() == 0)
                                Expecting.pop();
                }
                        break;
                case 2: /* Expect any of tokens */
                {
                        if (top.K.size() == 0)
                                Expecting.pop();

                        for (token tk : top.K) {
                                if (tk == tok.tok) {
                                        top.K.erase(top.K.begin());
                                        break;
                                }
                        }

                        if (top.K.size() > 1) {
                                errorf("Unexpected token %s. Did you mean to use %s or %s instead?",
                                       &toString(tok.tok)[0], &toString(top.K.front())[0],
                                       &toString(top.K.back())[0]
                                );
                        } else {
                                errorf("Unexpected token %s. Did you mean to use %s instead?",
                                       &toString(tok.tok)[0], &toString(top.K.front())[0]);
                        }

                        out = true;
                        top.K.erase(top.K.begin());
                }
                        break;
                case 3: /* Expect any of token types */
                {
                        if (top.T.size() == 0)
                                Expecting.pop();

                        for (tokenType type : top.T) {
                                if (type == tok.tok) {
                                        top.T.erase(top.T.begin());
                                        break;
                                }
                        }

                        if (top.T.size() > 1) {
                                errorf("Unexpected token %s with type %s. Did you mean to use type %s or %s instead?",
                                       &toString(tok.tok)[0], &toString(tok.type)[0],
                                       &toString(top.T.front())[0], &toString(top.T.back())[0]
                                );
                        } else {
                                errorf("Unexpected token %s with type %s. Did you mean to use type %s instead?",
                                       &toString(tok.tok)[0], &toString(tok.type)[0],
                                       &toString(top.T.front())[0]);
                        }

                        out = true;
                        top.T.erase(top.T.begin());
                }
                        break;
                }
        }

        return out;
}

bool Scanner::errorsFromExpect()
{
        if (Expecting.size() == 0) return false;

        while (Expecting.size() != 0) {
                scanExpect top = Expecting.top();

                switch (top.wantType) {
                case 0: /* Expect tokens */
                case 2: /* Expect any of tokens */
                        errorf("Expected %s token, but reached EOF instead.",
                               &toString(top.K.front())[0]);

                        Errors.back().pos = top.pos;
                        break;
                case 1: /* Expect types */
                case 3: /* Expect any of types */
                        errorf("Expected %s token type, but reached EOF instead.",
                               &toString(top.T.front())[0]);

                        Errors.back().pos = top.pos;
                        break;
                }

                Expecting.pop();
        }

        return true;
}

/* TODO: More conditions will be added once parser is written. */
void Scanner::acknowledge(scanToken tok)
{
        /* Must check before errorsFromExpect, which might remove
         * the top stack element.
         */
        if (tok.tok > operator_right && tok.tok < operator_right_end) {
                if (Expecting.empty()) goto rhs_errorf;
                if (Expecting.top().have.tok != getSymbolPair(tok.tok)) {
                        Expecting.pop();
rhs_errorf:             errorf("Pair-symbol %s is unmatched.", &toString(tok.tok)[0]);
                        return;
                }
        }

        /* Passes tok to errorsFromExpect, which will push
         * errors if tok doesn't meet expectations.
         */
        if (errorsFromExpect(tok)) return;

        /* Left operator must eventually be closed by right-side
         * operator
         */
        if (tok.tok > operator_left && tok.tok < operator_left_end)
                expect(tok, false, 0, 1, getSymbolPair(tok.tok));

        /* Modifier tokens are always followed by types. */
        if (tok.type == t_Modifier)
                expect(tok, true, 1, 1, (tokenType)t_Type);
}

void Scanner::skipWhitespace()
{
        int ch = Source.peek();
        while (ch != -1 && isWhitespace(ch)) {
                Source.getChar();
                ch = Source.peek();
        }
}

bool Scanner::isWhitespace(char ch)
{
        /* Newline chars are a token. */
        if (ch == ' ' || ch == '\t')
                return true;

        return false;
}

bool Scanner::scanNext()
{
        /* It is assumed that the scanNext always begins with the
         * start of a file or after having scanned a token.
         */

        skipWhitespace();
        int ch = Source.getChar();
        if (ch == -1) return false;

        /* Newline character */
        if (ch == '\n') {
                Buffer.emplace((scanToken){t_Symbol, EOL, "EOL", ""});
                return true;
        }

        /* AT THIS POINT, WE HAVE NON-WHITESPACE CHAR */
        std::string tok(1, ch);

        /* String literals */
        if (ch == '\'' || ch == '\"') {
                int stop = ch;

                /* Keep iterating until the closing operator (' or ")
                 * is found.
                 */
                ch = Source.getChar();
                while (ch != stop) {
                        if (ch == -1) {
                                errorf("Expected end of literal but got EOF instead.");
                                return false;
                        }
                        tok += ch;
                        ch = Source.getChar();
                }
                tok += ch;

                /* String literals are expected to be followed by
                 * whitespace, newline or symbol char
                 */
                int peek = Source.peek();
                if (!isWhitespace((char)peek) && peek != '\n' &&
                    typeOf(peek) != t_Symbol && peek != -1)
                {
                        errorf("Unknown character '%i' following literal with value: %s",
                               peek, &tok[0]);

                        /* Dont care if peek() is EOF, it will get captured
                         * at next iteration.
                         */
                        return true;
                }

                Buffer.emplace((scanToken){t_Literal, ILLEGAL, tok, tok});
                return true;
        }


        /* Hex integer */
        if (ch == '0' && Source.peek() == 'x') {
                /* We already know next char is x */
                ch = Source.getChar();
                tok += ch;

                /* Loop as long as next character is hex,
                 * append it to tok.
                 */
                int peek = Source.peek();
                std::regex hex(R"([a-f, A-F, 0-9])");
                std::string regexStr(1, (char)peek);

                while (std::regex_match(regexStr, hex)) {
                        ch = Source.getChar();
                        tok += ch;
                        peek = Source.peek();
                        regexStr[0] = (char)peek;
                }

                /* Ensure the entire word is hex - no non-hex
                 *  should follow the token.
                 */
                if (!isWhitespace((char)peek) && peek != '\n' &&
                    typeOf(peek) != t_Symbol && peek != -1)
                {
                        errorf("Found invalid hex character '%i' in hex literal: %s",
                               Source.peek(), &tok[0]);

                        return true;
                }

                Buffer.emplace((scanToken){t_Literal, ILLEGAL, tok, tok});
                return true;
        }

        /* Ch is any digit */
        std::regex digitRegex(R"([0-9])");
        if (std::regex_match(tok, digitRegex)) {
                bool dot = false;
                int peek = Source.peek();
                std::string regexStr(1, (char)peek);

getDigit:       while (std::regex_match(regexStr, digitRegex)) {;
                        tok += Source.getChar();
                        peek = Source.peek();
                        regexStr[0] = (char)peek;
                }

                /* Continue reading if a dot is detected, but ensure
                 * that this only ever happens once.
                 */
                if (!dot && peek == '.') {
                        dot = true;
                        tok += Source.getChar();
                        peek = Source.peek();
                        goto getDigit;
                }

                if (peek == 'f') {
                        tok += Source.getChar();
                        peek = Source.peek();
                }

                if (!isWhitespace((char)peek) && peek != '\n' &&
                    typeOf(peek) != t_Symbol && peek != -1)
                {
                        errorf("Unexpected char '%i' at end of digit literal '%s'",
                               Source.peek(), &tok[0]);

                        return true;
                }

                Buffer.emplace((scanToken){t_Literal, ILLEGAL, tok, tok});
                return true;
        }

        /* Ch is a word character */
        if (std::regex_match(tok, std::regex(R"([a-z,A-Z])"))) {
                int peek = Source.peek();
                while (!isWhitespace((char)peek) && peek != -1
                       && typeOf(peek) != t_Symbol)
                {
                        tok += Source.getChar();
                        peek = Source.peek();
                }

                tokenType wordType = typeOf(tok);

                /* Enqueues an error */
                if (wordType == t_ILLEGAL) {
                        errorf("Unexpected or unknown identifier: %s", &tok[0]);
                        return true;
                }

                /* Literals should've already been captured, but
                 * there's no harm in checking again.
                 */
                if (wordType == t_Literal) {
                        Buffer.emplace((scanToken){t_Literal, ILLEGAL, tok, tok});
                        return true;
                }

                /* Return an identifier */
                if (wordType == t_Ident) {
                        Buffer.emplace((scanToken){t_Ident, ILLEGAL, tok, ""});
                        if (Callbacks.cbIdent != nullptr)
                                Callbacks.cbIdent(tok, Source.pos());

                        return true;
                }

                /* If not literal, identifier or error, then no special
                 *  treatment is needed. Just return.
                 */
                Buffer.emplace((scanToken){typeOf(tok), lookup(tok), tok, ""});
                return true;
        }

        /* Symbols and operators */
        if (typeOf(tok) == t_Symbol) {
                while (typeOf(tok + (char)Source.peek()) == t_Symbol)
                        tok += Source.getChar();

                Buffer.emplace((scanToken){t_Symbol, lookup(tok), tok, ""});
                return true;
        }

        /* Misplaced/unidentified character */
        errorf("Unknown/illegal identifier: %s", &tok[0]);
        return true;
}

void Scanner::setCallbacks(scanCb cbs) {
        Callbacks = cbs;
}

int Scanner::bufferTokens(scanToken (&buffer)[3])
{
        int i = 0;
        while ((scanNext() || Buffer.size()) && i < 3) {
                scanToken tok = Buffer.front();
                acknowledge(tok);

                buffer[i] = tok;
                Buffer.pop();

                i++;
        }

        if (Source.eof() && Buffer.size() == 0) {
                if (Callbacks.cbEOF != nullptr)
                        Callbacks.cbEOF();

                std::string newPath = Source.next();
                if (Callbacks.cbNewfile != nullptr && newPath != "")
                        Callbacks.cbNewfile(newPath);
        }

        return i;
}

void Scanner::getErrors(std::vector<scanError>& errors) {
        errorsFromExpect();
        errors = Errors;
}

void Scanner::getTokens(std::vector<scanToken>& tokens)
{
        scanToken buffer[3];
        int i = bufferTokens(buffer);
        while (i) {
                for (int x=0; i<i;i++)
                        tokens.push_back(buffer[i]);
                i = bufferTokens(buffer);
        }
}
