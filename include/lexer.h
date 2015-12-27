#pragma once

#include "util.h"

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

struct Token {
    enum class Type {
        DEFN,
        EXTERN,
        IDENTIFIER,
        OPERATOR,
        RESERVED_SYMBOL,
        NUMERIC_LITERAL,
        END_OF_FILE
    } type;

    std::string contents;
    unsigned linum, colnum;

    Token(Token::Type type, const std::string &contents,
            unsigned linum, unsigned colnum);

    Token(Token::Type type, const std::string &contents);
};

void print_token(const Token &t);

bool operator ==(const Token &t1, const Token &t2);
bool operator !=(const Token &t1, const Token &t2);

static const std::map<std::string, Token::Type> RESERVED_IDENTIFIERS = {
    {"defn", Token::Type::DEFN},
    {"extern", Token::Type::EXTERN},
};

static const std::set<std::string> RESERVED_SYMBOLS = {
    ",", ";", ":", "(", ")"
};

static inline bool isopch(const char c) {
    static const std::set<char> ALLOWED_BIN_OP_CHARS = {
        '<', '>', '+', '-', '*', '!', '@', '$', '%', '^', '&', '|'
    };
    return contains(ALLOWED_BIN_OP_CHARS, c);
}

std::vector<Token> tokenize(const std::string &program);
