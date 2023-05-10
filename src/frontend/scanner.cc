#include "scanner.hpp"

const std::string __eof_string__ = "EOF";
const std::string __eol_string__ = "EOL";

bool Token::is_semicolon()
{
        if (_classification == TOKEN_EOL
            || _classification == TOKEN_EOF)
                return true;

        if (_classification == TOKEN_OPERATOR) {
                if (this->op() == OPER_SEMICOLON) {
                        return true;
                }
        }

        return false;
}

void Token::clear()
{
        if (this->_classification == TOKEN_FLOAT)
                mpfr_clear(this->value.float_value);
}

Token::Token(const Token& tok)
{
        if (tok._classification == TOKEN_INTEGER
            || tok._classification == TOKEN_FLOAT) {
                mpfr_init_set_str(this->value.float_value, &tok.token_string[0], 10, MPFR_RNDN);
        } else if (tok._classification == TOKEN_IDENT)
                this->value.id_name = new std::string(*tok.value.id_name);
        else if (tok._classification == TOKEN_OPERATOR)
                this->value.op_value = tok.value.op_value;
        else if (tok._classification == TOKEN_RID)
                this->value.rid = tok.value.rid;

        this->_location.filename = tok._location.filename;
        this->_location.offset   = tok._location.offset;
        this->_location.line     = tok._location.line;
        this->_location.column   = tok._location.column;
        this->_classification    = tok._classification;
        this->token_string       = tok.token_string;
}

Token Token::make_invalid_token(const std::string& str, Location loc)
{
        Token tok(TOKEN_INVALID, str, loc);
        return tok;
}

Token Token::make_eof_token(Location loc)
{
        Token tok(TOKEN_EOF, __eof_string__, loc);
        return tok;
}

Token Token::make_eol_token(Location loc)
{
        Token tok(TOKEN_EOL, __eol_string__, loc);
        return tok;
}

Token Token::make_rid_token(RID rid, Location loc)
{
        Token tok(TOKEN_RID, rid_as_string(rid), loc);
        tok.value.rid = rid;
        return tok;
}

Token Token::make_ident_token
(std::string val, Location loc)
{
        Token tok(TOKEN_IDENT, val, loc);
        tok.value.id_name = &tok.token_string;
        return tok;
}

Token Token::make_operator_token(RIN_OPERATOR op, Location loc)
{
        Token tok(TOKEN_OPERATOR, operator_name(op), loc);
        tok.value.op_value = op;
        return tok;
}

Token Token::make_float_token(const std::string& str, Location loc)
{
        Token tok(TOKEN_FLOAT, str, loc);
        mpfr_init_set_str(tok.value.float_value, &str[0], 10, MPFR_RNDN);
        return tok;
}

RID Token::rid() const
{
        RIN_ASSERT(this->_classification == TOKEN_RID);
        return this->value.rid;
}

const std::string* Token::identifier() const
{
        RIN_ASSERT(this->_classification == TOKEN_IDENT);
        return this->value.id_name;
}

RIN_OPERATOR Token::op() const
{
        RIN_ASSERT(this->_classification == TOKEN_OPERATOR);
        return this->value.op_value;
}

const mpfr_t* Token::float_value() const
{
        RIN_ASSERT(this->_classification == TOKEN_FLOAT);
        return &this->value.float_value;
}

std::string rid_as_string(RID rid)
{
        switch (rid) {
        case RID_INVALID:
                return "invalid keyword";
        case RID_FLOAT:
                return "float keyword";
        case RID_IF:
                return "if keyword";
        case RID_FOR:
                return "for keyword";
        default:
                return "unknown keyword";
        }
}

std::string Token::classification_as_string()
{
        switch (this->_classification) {
        case TOKEN_INVALID:
                return "invalid token";
        case TOKEN_EOF:
                return "EOF";
        case TOKEN_EOL:
                return "EOL";
        case TOKEN_RID:
                return rid_as_string(value.rid);
        case TOKEN_IDENT:
                return "identifier";
        case TOKEN_STRING:
                return "string literal";
        case TOKEN_OPERATOR:
                return operator_name(value.op_value);
        case TOKEN_CHARACTER:
                return "character literal";
        case TOKEN_INTEGER:
                return "integer literal";
        case TOKEN_FLOAT:
                return "float literal";
        }
}

void Scanner::skip_line()
{
        while (this->has_next()) {
                Token token = this->scan_token();
                this->acknowledge(token);
                Token::Classification cls = token.classification();
                if (cls == Token::TOKEN_EOF || cls == Token::TOKEN_EOL)
                        break;
        }
}

void Scanner::acknowledge(Token tok)
{
        if (tok.classification() != Token::TOKEN_OPERATOR)
                return;
        if (is_lefthand_op(tok.op()))
                return;
        if (!expect_matches.empty() && expect_matches.top().want == tok.op()) {
                expect_matches.pop();
                return;
        }
        if (is_righthand_op(tok.op())) {
                rin_error_at(tok.location(), "Unexpected %s",
                             &operator_name(tok.op())[0]);
        }
}

void Scanner::expect_match(Token tok, RIN_OPERATOR match)
{
        if (tok.classification() != Token::TOKEN_OPERATOR)
                return;

        RIN_OPERATOR symbol = tok.op();
        if (!is_lefthand_op(symbol))
                return;

        ExpectMatch expect;
        expect.loc = tok.location();
        expect.have = symbol;
        expect.want = match;

        expect_matches.push(expect);
}

int Scanner::skip_whitespace()
{
        if (src == NULL)
                return 0;

        int chars = 0;

        // Do not skip newlines, they are a token
        while (src->peek() == ' ' || src->peek() == '\t') {
                src->get_char();
                chars++;
        }

        return chars;
}

int Scanner::skip_comment()
{
        RIN_ASSERT(src);

        bool is_multiline = (src->peek() == '*');
        bool is_oneline = (src->peek() == '/');
        if (!is_multiline && !is_oneline)
                return 0;

        int chars = 0;
        char last_char = src->get_char();
        while (is_oneline && last_char != '\n') {
                last_char = src->get_char();
                if (last_char == -1)
                        return chars;
                chars++;
        }

        while (is_multiline) {
                last_char = src->get_char();
                if (last_char == -1)
                        return chars;
                if (last_char == '*' && src->peek() == '/') {
                        last_char = src->get_char();
                        chars += 2;
                        break;
                }
                chars++;
        }

        return chars;
}

Location Scanner::location()
{
        if (src == NULL) {
                Location empty;
                return empty;
        }
        return src->get_loc();
}

Token Scanner::scan_token()
{
        RIN_ASSERT(src);

        this->skip_whitespace();
        int ch = this->src->get_char();

        if (ch == -1)
                return this->make_eof_token();

        if (ch == '\n') {
                line_count++;
                return this->make_eol_token();
        }

        if (ch == '/' && this->skip_comment())
                return scan_token();

        // At this point, we have non-whitespace char
        std::string tokenStr(1, ch);

        /*
         * Gather multi-char operator
         * TODO: No need to check for is_operator of next token when &, | binary
         * operators are implemented
         */
        while (is_operator(tokenStr) || is_operator(tokenStr + (char)src->peek())) {
                if (is_operator(tokenStr + (char)src->peek())) {
                        int ch = src->get_char();
                        if (ch == '/' && (src->peek() == '/' || src->peek() == '*')) {
                                skip_comment();
                                goto assemble_oper;
                        }
                        tokenStr += (int)ch;
                        continue;
                }

        assemble_oper:
                RIN_OPERATOR oper = op_lookup(tokenStr);
                Token have = Scanner::make_operator(oper);
                if (is_paired_symbol(oper))
                        expect_match(have, get_symbol_pair(oper));

                return have;
        }

        // Gather non-operator word
        while (!is_whitespace(src->peek()) && !is_operator((char)src->peek()) && src->peek() != '\n')
                tokenStr += (char)src->get_char();

        // Gather float.
        if (is_int_literal(tokenStr) && src->peek() == '.') {
                tokenStr += (char)src->get_char();
                std::string literal(1, src->peek());
                while (is_int_literal(literal) || literal[0] == 'f') {
                        char next = (char)src->get_char();
                        tokenStr += next;
                        if (next == 'f')
                                break;
                        literal[0] = src->peek();
                }
        }

        // Will fire for floats as well as integers (integers currently treated as floats)
        if (is_float_literal(tokenStr)) {
                Token tok = Token::make_float_token(tokenStr, Scanner::location());
                return tok;
        }

        if (is_rid(tokenStr)) {
                RID rid = rid_lookup(tokenStr);
                return Token::make_rid_token(rid, Scanner::location());
        }

        if (is_valid_identifier(tokenStr))
                return Token::make_ident_token(tokenStr, Scanner::location());

        rin_error_at(location(), "Unknown keyword %s", &tokenStr[0]);
        return make_invalid_token(tokenStr);
}

bool Scanner::is_whitespace(int c)
{
        if (c == ' ' || c == -1 || c == '\t')
                return true;
        return false;
}

Token Scanner::next_token()
{
        if (!buffer.empty()) {
                Token tok = buffer.front();
                buffer.pop_front();
                this->acknowledge(tok);
                return tok;
        }

        RIN_ASSERT(this->src);

        if (this->src->has_next()) {
                Token tok = scan_token();
                acknowledge(tok);
                return tok;
        }

        return make_eof_token();
}

Token Scanner::peek_token()
{
        if (!buffer.empty())
                return buffer.front();

        RIN_ASSERT(this->src);

        if (src->has_next()) {
                Token tok = scan_token();
                buffer.emplace_back(tok);
                return tok;
        }

        return make_eof_token();
}

Token Scanner::peek_nth_token(unsigned int n)
{
        RIN_ASSERT(this->src);

        while (src->has_next() && buffer.size() <= n)
                buffer.emplace_back(scan_token());

        if (buffer.size() > n)
                return buffer[n];

        return make_eof_token();
}

void Scanner::reset()
{
        delete src;

        while(this->expect_matches.size() > 0)
                this->expect_matches.pop();

        this->line_count = 0;
        this->buffer.clear();
}

bool Scanner::has_next()
{
        if (!this->buffer.empty())
                return true;
        if (!this->src)
                return false;
        if (this->src->has_next())
                return true;
        return false;
}

bool Scanner::is_valid_identifier(const std::string& ident)
{
        if (std::regex_match(ident.begin(), ident.end(), std::regex(R"(([a-z]|[A-Z])(\w+|))")))
                return true;
        return false;
}

bool Scanner::is_hex_literal(const std::string& literal)
{
        if(std::regex_match(literal.begin(), literal.end(), std::regex(R"(0x[ABCDEFabcdef0123456789]+)")))
                return true;
        return false;
}

bool Scanner::is_string_literal(const std::string& literal)
{
        if (std::regex_match(literal.begin(), literal.end(), std::regex(R"(\"(\w|\s|)+\")")))
                return true;
        return false;
}

bool Scanner::is_char_literal(const std::string& literal)
{
        if (std::regex_match(literal.begin(), literal.end(), std::regex(R"(\'\w\')")))
                return true;
        return false;
}

bool Scanner::is_int_literal(const std::string& literal)
{
        if (std::regex_match(literal.begin(), literal.end(), std::regex(R"(\d+)")))
                return true;
        return false;
}

bool Scanner::is_float_literal(const std::string& literal)
{
        if (std::regex_match(literal.begin(), literal.end(), std::regex(R"(\d+\.\d+)")) ||
            std::regex_match(literal.begin(), literal.end(), std::regex(R"((\d+f|\d+\.\d+f))")))
                return true;
        return false;
}

bool Scanner::is_rid(const std::string& value)
{
        if (rid_lookup(value) != RID_INVALID)
                return true;
        return false;
}

bool Scanner::consume_errors()
{
        if (this->expect_matches.size() == 0)
                return false;
        while (this->expect_matches.size() > 0) {
                ExpectMatch exp = this->expect_matches.top();
                switch (exp.want) {
                case OPER_LPAREN:
                case OPER_RPAREN:
                        rin_error_at(exp.loc, "Unmatched parenthesis operator.");
                        break;
                case OPER_LBRACK:
                case OPER_RBRACK:
                        rin_error_at(exp.loc, "Unmatched bracket operator.");
                        break;
                case OPER_LBRACE:
                case OPER_RBRACE:
                        rin_error_at(exp.loc, "Unmatched brace operator.");
                        break;
                default:
                        rin_error_at(exp.loc, "Unmatched unexpected operator.");
                }
                this->expect_matches.pop();
        }
        return true;
}

RID rid_lookup(const std::string& val)
{
        if (val == "float")
                return RID_FLOAT;
        if (val == "if")
                return RID_IF;
        if (val == "for")
                return RID_FOR;

        return RID_INVALID;
}
