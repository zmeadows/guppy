#include "util.h"
#include "lexer.h"


bool operator ==(const Token &t1, const Token &t2) {
    return std::tie(t1.type, t1.contents) == std::tie(t2.type, t2.contents);
}

bool operator !=(const Token &t1, const Token &t2) {
    return !(t1 == t2);
}

std::vector<Token>
tokenize(const std::string &program)
{
    std::vector<Token> tokens;
    std::string::const_iterator it = program.begin();
    unsigned linum = 1;
    unsigned colnum = 1;

    /* advance the iterator without accidentally stepping past
     * the end of the program string */
    auto safe_advance = [&it, &program, &linum, &colnum]() -> void {
        if (it != program.end()) {
            if (*it == '\n') {
                linum++;
                colnum = 1;
            } else {
                colnum++;
            }
            it++;
        }
    };

    /* helper function to add line/column number to token constructor */
    auto make_token = [&linum, &colnum]
        (Token::Type tt, const std::string &s) -> Token {
            return Token(tt, s, linum, colnum - s.size());
    };

    while (it != program.end())
    {
        /******************************/
        /* WHITESPACE | TAB | NEWLINE */
        /******************************/
        while (isspace(*it)) safe_advance();

        /*****************************/
        /* IDENTIFIER | DEF | EXTERN */
        /*****************************/
        if (isalpha(*it)) { // first letter must be alphabetic
            std::string identifier_str;
            do {
                identifier_str.push_back(*it);
                safe_advance();
            } while (isalnum(*it)); // the rest can be alphanumeric

            auto rit = RESERVED_IDENTIFIERS.find(identifier_str);
            if (rit != RESERVED_IDENTIFIERS.end()) {
                tokens.push_back(make_token(rit->second, identifier_str));
            } else {
                tokens.push_back(make_token(Token::Type::IDENTIFIER, identifier_str));
            }

        /**********/
        /* NUMBER */
        /**********/
        } else if (isdigit(*it)) {
            std::string double_str;

            do {
                double_str.push_back(*it);
                safe_advance();
            } while (isdigit(*it) || *it == '.');

            tokens.push_back(make_token(Token::Type::NUMERIC_LITERAL, double_str));

        /***********/
        /* COMMENT */
        /***********/
        } else if (*it == '#') {

            do safe_advance(); while
                (it != program.end() && *it != '\n' && *it != '\r');

        /*******************************/
        /* OPERATOR OR RESERVED SYMBOL */
        /*******************************/
            // TODO: check that length of operator <= 3
        } else if (isopch(*it)) { // OPERATOR
            std::string operator_str;
            do {
                operator_str.push_back(*it);
                safe_advance();
            } while (isopch(*it));

            tokens.push_back(make_token(Token::Type::OPERATOR, operator_str));

        /***************/
        /* END OF FILE */
        /***************/
        } else if ( (int) *it == 0 ) { // null/eof
            tokens.push_back(make_token(Token::Type::END_OF_FILE, ""));

        /***************************/
        /* RESERVED | UNRECOGNIZED */
        /***************************/
        } else {
            std::string symbol;
            bool is_reserved_symbol = false;

            do {
                symbol.push_back(*it);
                is_reserved_symbol = contains(RESERVED_SYMBOLS, symbol);
                safe_advance();
            } while (!isspace(*it) && !is_reserved_symbol && symbol.size() <= 3);

            if (is_reserved_symbol) {
                tokens.push_back(make_token(Token::Type::RESERVED_SYMBOL, symbol));
            } else {
                std::ostringstream err_msg;
                err_msg << "unrecognized character '";
                err_msg << *it << "' encountered during lexing";
                throw std::runtime_error(err_msg.str());
            }
        }
    }

    if (tokens.back().type != Token::Type::END_OF_FILE)
        tokens.push_back(Token(Token::Type::END_OF_FILE,""));

    return tokens;
}

void print_token(const Token &t) {
    static const std::map<Token::Type, std::string> TOKEN_TYPE_STRING_MAP = {
        {Token::Type::DEFN             , "DEFN"},
        {Token::Type::EXTERN           , "EXTERN"},
        {Token::Type::IDENTIFIER       , "IDENTIFIER"},
        {Token::Type::NUMERIC_LITERAL  , "NUMERIC_LITERAL"},
        {Token::Type::RESERVED_SYMBOL  , "RESERVED_SYMBOL"},
        {Token::Type::OPERATOR         , "OPERATOR"},
        {Token::Type::END_OF_FILE      , "EOF"}
    };

    std::string type_str = TOKEN_TYPE_STRING_MAP.find(t.type)->second;

    std::cout << type_str << " (" << t.linum << ":" << t.colnum <<
        "):\t\t" << t.contents << std::endl;
}
