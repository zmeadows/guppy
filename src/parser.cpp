#include "parser.h"

bool
Parser::accept(const Token &acceptable_token)
{
    if (token_iter == tokens.end()) {
        throw ParseIncomplete();
    } else if (*token_iter == acceptable_token) {
        token_iter++;
        return true;
    } else {
        return false;
    }
}

bool
Parser::accept(const Token::Type acceptable_type)
{
    if (token_iter == tokens.end()) {
        throw ParseIncomplete();
    } else if (token_iter->type == acceptable_type) {
        token_iter++;
        return true;
    } else {
        return false;
    }
}

bool
Parser::accept_and_store(const Token::Type acceptable_type,
        std::string &container)
{
    if (token_iter == tokens.end()) {
        throw ParseIncomplete();
    } else if (token_iter->type == acceptable_type) {
        container = token_iter->contents;
        token_iter++;
        return true;
    } else {
        return false;
    }
}

void
Parser::expect(const Token &acceptable_token)
{
    if (token_iter == tokens.end()) {
        throw ParseIncomplete();
    } else if (*token_iter == acceptable_token) {
        token_iter++;
    } else {
        throw ParseError(*token_iter, "unexpected token encountered.");
    }
}

void
Parser::expect(const Token::Type expected_type)
{
    if (token_iter->type == expected_type) {
        token_iter++;
    } else {
        throw ParseError(*token_iter, "unexpected token encountered.");
    }
}

void
Parser::expect_and_store(const Token::Type expected_type,
        std::string &container)
{
    if (token_iter == tokens.end()) {
        throw ParseIncomplete();
    } else if (token_iter->type == expected_type) {
        container = token_iter->contents;
        token_iter++;
    } else {
        throw ParseError(*token_iter, "unexpected token encountered.");
    }
}

int
Parser::get_prec(const std::string &s) const
{
    for (auto const &b : settings.binops) {
        if (b.symbol == s) return b.prec;
    }

    throw ParseError(*token_iter, "unrecognized operator encountered");
}

BinOp::Associativity
Parser::get_assoc(const std::string &s) const
{
    for (auto const &b : settings.binops) {
        if (b.symbol == s) return b.assoc;
    }

    throw ParseError(*token_iter, "unrecognized operator encountered");
}

std::unique_ptr<Prototype>
Parser::parse_prototype()
{
    std::string func_name;
    expect_and_store(Token::Type::IDENTIFIER, func_name);

    expect(Token(Token::Type::RESERVED_SYMBOL, "("));

    std::vector<std::string> arg_names;
    std::string arg;
    while (accept_and_store(Token::Type::IDENTIFIER, arg)) {
        arg_names.push_back(arg);
        accept(Token(Token::Type::RESERVED_SYMBOL, ","));
    }

    expect(Token(Token::Type::RESERVED_SYMBOL, ")"));

    return std::make_unique<Prototype>(func_name, std::move(arg_names));
}

std::unique_ptr<ASTNode>
Parser::parse_statement()
{
    if (accept(Token::Type::DEFN)) {
        return parse_defn();
    } else if (accept(Token::Type::EXTERN)) {
        return parse_extern();
    } else {
        return parse_top_level_expression();
    }
}

std::unique_ptr<ASTNode>
Parser::parse_defn()
{
    auto prototype = parse_prototype();
    auto expr = parse_expr();

    return std::make_unique<ASTDefnNode>(std::move(prototype), std::move(expr));
}

std::unique_ptr<ASTNode>
Parser::parse_extern()
{
    auto prototype = parse_prototype();

    return std::make_unique<ASTExternNode>(std::move(prototype));
}

std::unique_ptr<ASTNode>
Parser::parse_top_level_expression()
{
    /* treat top level function as anonymous function with no arguments */
    auto expr = parse_expr();
    auto prototype = std::make_unique<Prototype>("__ANON__", std::vector<std::string>());

    return std::make_unique<ASTDefnNode>(std::move(prototype), std::move(expr));
}

std::unique_ptr<Expr>
Parser::parse_expr()
{
    return parse_expr(0);
}

std::unique_ptr<Expr>
Parser::parse_expr(int p)
{
    std::unique_ptr<Expr> LHS = parse_primary_expr();

    while (token_iter->type == Token::Type::OPERATOR
            && get_prec(token_iter->contents) >= p)
    {
        std::string binop_str = token_iter->contents;
        token_iter++;

        int q;
        switch (get_assoc(binop_str))
        {
            case BinOp::Associativity::LEFT:
                q = get_prec(binop_str) + 1;
                break;
            case BinOp::Associativity::RIGHT:
                q = get_prec(binop_str);
                break;
        }

        auto RHS = parse_expr(q);

        // TODO: use shared_ptr or something to have single instance of binops?
        // for a big file, all these strings will add up
        LHS = std::make_unique<BinOpExpr>(binop_str, std::move(LHS), std::move(RHS));
    }

    return LHS;
}

std::unique_ptr<Expr>
Parser::parse_primary_expr()
{
    if (token_iter->type == Token::Type::IDENTIFIER) {
        return parse_identifier_expr();
    } else if (token_iter->type == Token::Type::NUMERIC_LITERAL) {
        return parse_numeric_literal_expr();
    } else if (token_iter->type == Token::Type::RESERVED_SYMBOL) {
        if (token_iter->contents == "(") {
            return parse_paren_expr();
        } else {
            throw ParseError(*token_iter, "unexpected token encountered when attempting to parse primary expression.");
        }
    } else if (token_iter->type == Token::Type::END_OF_FILE) {
        throw ParseIncomplete();
    } else {
        throw ParseError(*token_iter, "unexpected token encountered when attempting to parse primary expression.");
    }
}

std::unique_ptr<Expr>
Parser::parse_identifier_expr()
{
    std::string identifier_name;
    expect_and_store(Token::Type::IDENTIFIER, identifier_name);

    if (accept(Token(Token::Type::RESERVED_SYMBOL, "("))) {
        /* found open parenthesis so identifier is function call */

        std::vector<std::unique_ptr<Expr>> args;
        if (*token_iter != Token(Token::Type::RESERVED_SYMBOL, ")")) {
            do
                args.push_back(parse_expr());
            while (accept(Token(Token::Type::RESERVED_SYMBOL, ",")));
        }

        expect(Token(Token::Type::RESERVED_SYMBOL, ")"));

        return std::make_unique<CallExpr>(identifier_name, std::move(args));

    } else { /* otherwise it is just a variable */
        return std::make_unique<VariableExpr>(identifier_name);
    }
}

std::unique_ptr<Expr>
Parser::parse_numeric_literal_expr()
{
    std::string double_str;
    expect_and_store(Token::Type::NUMERIC_LITERAL, double_str);

    return std::make_unique<LiteralDoubleExpr>(std::stod(double_str));
}

std::unique_ptr<Expr>
Parser::parse_paren_expr()
{
    expect(Token(Token::Type::RESERVED_SYMBOL, "("));
    auto enclosed_expr = parse_expr();
    expect(Token(Token::Type::RESERVED_SYMBOL, ")"));

    return enclosed_expr;
}

AST
Parser::get_ast(const std::string &statement)
{
    AST ast;
    tokens = tokenize(statement);
    token_iter = tokens.begin();

    while (token_iter->type != Token::Type::END_OF_FILE) {
        ast.push_back(parse_statement());
    }

    tokens.clear();
    return std::move(ast);
}
