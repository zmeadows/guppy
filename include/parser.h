#pragma once

#include "ast.h"
#include "ast_printer.h"
#include "lexer.h"

#include <functional>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>


class ParseError : public std::runtime_error
{
    const Token err_token;
public:
    ParseError(const Token &err_token, std::string const &msg)
        : std::runtime_error(msg), err_token(err_token) {}
};

class ParseIncomplete //: public std::runtime_error
{
public:
    ParseIncomplete() {}
        //std::runtime_error("reached end of file before finished parsing.") {}
};

struct BinOp {
    std::string symbol;
    int prec;

    enum class Associativity {
        LEFT,
        RIGHT
    } assoc;

    BinOp(const std::string &symbol, int prec, Associativity assoc)
        : symbol(symbol), prec(prec), assoc(assoc) {}

    /* overloaded operator for use in std::set, should never be used otherwise */
    inline bool operator<(BinOp b) const { return symbol < b.symbol; }
};

const static std::set<BinOp> DEFAULT_BIN_OPS {
    BinOp("<", 10, BinOp::Associativity::LEFT),
        BinOp("+", 20, BinOp::Associativity::LEFT),
        BinOp("-", 20, BinOp::Associativity::LEFT),
        BinOp("*", 40, BinOp::Associativity::LEFT),
        BinOp("^", 50, BinOp::Associativity::RIGHT)
};

struct ParserSettings {
    std::set<BinOp> binops;

    // add_binop(const BinOp &new_binop);
    ParserSettings() : binops(DEFAULT_BIN_OPS) {}
};

class Parser {
    std::vector<Token> tokens;
    std::vector<Token>::const_iterator token_iter;

    void consumption_handler(bool condition,
            std::function<void(void)> success_action,
            std::function<void(void)> failure_action);

    bool accept(const Token &acceptable_type);
    bool accept(const Token::Type acceptable_type);
    bool accept_and_store(const Token::Type acceptable_type, std::string &container);

    void expect(const Token &expected_token);
    void expect(const Token::Type expected_type);
    void expect_and_store(const Token::Type expected_type, std::string &container);

    ParserSettings settings;
    inline int get_prec(const std::string &s) const;
    inline BinOp::Associativity get_assoc(const std::string &s) const;

    std::unique_ptr<PrototypeAST> parse_prototype();

    std::unique_ptr<ASTNode> parse_statement();
    std::unique_ptr<ASTNode> parse_defn();
    std::unique_ptr<ASTNode> parse_extern();
    std::unique_ptr<ASTNode> parse_top_level_expression();

    std::unique_ptr<ASTExpr> parse_expr();
    std::unique_ptr<ASTExpr> parse_expr(int p);
    std::unique_ptr<ASTExpr> parse_primary_expr();
    std::unique_ptr<ASTExpr> parse_identifier_expr();
    std::unique_ptr<ASTExpr> parse_numeric_literal_expr();
    std::unique_ptr<ASTExpr> parse_paren_expr();
    std::unique_ptr<ASTExpr> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ASTExpr> LHS);

public:
    Parser() : settings(ParserSettings()) {}

    AST parse_text(const std::string &text);

    void reset() { settings = ParserSettings(); }
};

