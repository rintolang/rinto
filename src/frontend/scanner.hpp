#ifndef RIN_SCANNER_HPP
#define RIN_SCANNER_HPP

#include "backend.hpp"
#include "diagnostic.hpp"
#include "file.hpp"

// Reserved identifiers
enum RID { RID_INVALID, RID_FLOAT, RID_FOR, RID_IF };

// Lookup an RID by string name
RID rid_lookup(const std::string& val);

// Return an RID as a string
std::string rid_as_string(RID rid);

class Token
{
public:
        // Token Types
        enum Classification {
                TOKEN_INVALID,  TOKEN_EOF,       TOKEN_EOL,
                TOKEN_RID,      TOKEN_IDENT,     TOKEN_STRING,
                TOKEN_OPERATOR, TOKEN_CHARACTER, TOKEN_INTEGER,
                TOKEN_FLOAT
        };

        Token(const Token& tok);

        ~Token() { this->clear(); }

        Classification classification() const
        { return _classification; }

        Location location()
        { return this->_location; }

        // Return the token's classification as a string. For debugging.
        std::string classification_as_string();

        // Return the token as a string
        char* str()
        { return &this->token_string[0]; }

        std::string string()
        { return this->token_string; }

        std::string test()
        { return this->token_string; }

        // Custom Token Type constructors
        static Token make_invalid_token
        (const std::string& str, Location loc);

        static Token make_eof_token(Location loc);
        static Token make_eol_token(Location loc);
        static Token make_rid_token(RID rid, Location loc);
        static Token make_operator_token(RIN_OPERATOR op, Location loc);
        static Token make_float_token(const std::string& str, Location loc);

        // Make an identifier token
        static Token make_ident_token(std::string name, Location loc);

        // True if Token is EOL or semicolon operator
        bool is_semicolon();

        /*
         * Return token values. Throws invalid argument error if not of
         * specified type
         */
        RID rid() const;
        const std::string* identifier() const;
        RIN_OPERATOR op() const;
        const mpfr_t* float_value() const;

private:
        // Private constructor: called by make_x_token functions
        Token(Classification c, std::string str, Location l)
                : _classification(c), _location(l), token_string(str)
                {}

        // Clears dynamically allocated mpfr/mpz, resets attributes to defaults
        void clear();

        // Token value - varies depending on classification.
        union {
                RID           rid;
                mpfr_t        float_value;
                RIN_OPERATOR  op_value;
                std::string*  id_name;
        } value;

        Classification _classification;
        Location       _location;
        std::string    token_string;
};

/*
 * An expect match structure helps validate whether paired operators are left unmatched.
 * It also stores a location to pinpoint the source of the error. If an open parenthesis
 * is scanned (e.g), the scanner will consequently expect a close parenthesis operator to
 * match it.
 */
struct ExpectMatch {
        Location loc;
        RIN_OPERATOR have = OPER_ILLEGAL;
        RIN_OPERATOR want = OPER_ILLEGAL;
};

class Scanner
{
public:
        Scanner() {};
        Scanner(File* file) : src(file) {};

        ~Scanner()
        { delete this->src; }

        // Instantiate a scanner directly from a file.
        Scanner(const std::string& path)
        { this->src = new File(path); }

        // Read tokens from the scanner source.
        Token next_token();
        Token peek_token();
        Token peek_nth_token(unsigned int n);
        bool has_next();

        // Read until semicolon or EOL - in case of malformed statement
        void skip_line();

        // Reset the scanner's attributes, restarts scanner.
        void reset();

        /*
         * Receive errors from unmatched expectations. Once received, expect_matches is emptied,
         * so this is best called when the scanner reaches EOF. Returns true if errors were pushed.
         */
        bool consume_errors();

        static bool is_operator(const std::string& op)
        { return is_rin_operator(op); }

        static bool is_operator(char c)
        { return is_rin_operator(c); }

        static bool is_rid(const std::string& value);
        static bool is_valid_identifier(const std::string& name);

        static bool is_hex_literal(const std::string& literal);
        static bool is_string_literal(const std::string& literal);
        static bool is_char_literal(const std::string& literal);
        static bool is_int_literal(const std::string& literal);
        static bool is_float_literal(const std::string& literal);

        /*
         * Given a string, these functions return their numerical values.
         * The value will subsequently be stored in the 'out' parameter.
         */
        static void string_to_int(const std::string& value, mpz_t out)
        { mpz_init_set_str(out, &value[0], 0); }
        static void float_to_int(const std::string& value, mpfr_t out)
        { mpfr_init_set_str(out, &value[0], 0, MPFR_RNDN); }

protected:

        /*
         * acknowledge acknowledges the receipt of a token. This is useful in evaluating
         * whether ExpectMatch expectations are being fulfilled. If an expected token is
         * acknowledged, then it is removed from expect_matches.
         */
        void acknowledge(Token tok);

        /*
         * Expect to eventually receive a particular token. This currently only works with
         * some paired operators (parenthesis, quotes, braces, etc). If only a token is passed,
         * then the expectation is taken to be its paired symbol. If a RIN_OPERATOR  match is
         * passed, then match is what's expected to be received.
         */
        void expect_match(Token tok)
        { expect_match(tok, get_symbol_pair(tok.op())); }
        void expect_match(Token tok, RIN_OPERATOR match);

        int skip_whitespace();
        bool is_whitespace(int c);
        int skip_comment();

        Location location();

        // Utility functions.
        Token make_operator(RIN_OPERATOR op)
        { return Token::make_operator_token(op, Scanner::location()); }

        Token make_invalid_token(const std::string& val)
        { return Token::make_invalid_token(val, Scanner::location()); }

        Token make_eof_token()
        { return Token::make_eof_token(Scanner::location()); }

        Token make_eol_token()
        { return Token::make_eol_token(Scanner::location()); }

        // Scan the source, lex it, and return the next token.
        Token scan_token();

private:
        File* src = nullptr;
        std::deque<Token> buffer;

        /*
         * The expect_matches stack keeps track of all ExpectMatch expectations. If it
         * is not empty by the time push_expect_errors() is called, then errors will be
         * issues to alert the user of missing parentheses, quotation marks, etc.
         */
        std::stack<ExpectMatch> expect_matches;
        int line_count = 0;
};

#endif // RIN_SCANNER_HPP
