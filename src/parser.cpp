#include "parser.h"

void Parser::consumption_handler(bool condition,
        std::function<void(void)> success_action,
        std::function<void(void)> failure_action)
{
    bool ran_out_of_tokens = token_iter == tokens.end()
        || token_iter->type == Token::Type::END_OF_FILE;

    if (ran_out_of_tokens) {
        throw ParseIncomplete();
    } else if (condition) {
        success_action();
        token_iter++;
    } else {
        failure_action();
    }
}

bool
Parser::accept(const Token &acceptable_token)
{
    bool result;

    std::function<void()> s = [&result]() { result = true; };
    std::function<void()> f = [&result]() { result = false; };

    consumption_handler(*token_iter == acceptable_token, s, f);

    return result;
}

bool
Parser::accept(const Token::Type acceptable_type)
{
    bool result;

    std::function<void()> s = [&result]() { result = true; };
    std::function<void()> f = [&result]() { result = false; };

    consumption_handler(token_iter->type == acceptable_type, s, f);

    return result;
}

bool
Parser::accept_and_store(const Token::Type acceptable_type,
        std::string &container)
{
    bool result;

    std::function<void()> s = [&container, &result, this]() {
        container = this->token_iter->contents;
        result = true;
    };

    std::function<void()> f = [&result]() { result = false; };

    consumption_handler(token_iter->type == acceptable_type, s, f);

    return result;
}

void
Parser::expect(const Token &expected_token)
{

    std::function<void()> s = []() { return; };
    std::function<void()> f = [this]() {
        throw ParseError(*this->token_iter, "unexpected token encountered.");
    };

    consumption_handler(*token_iter == expected_token, s, f);
}

void
Parser::expect(const Token::Type expected_type)
{

    std::function<void()> s = []() { return; };
    std::function<void()> f = [this]() {
        throw ParseError(*this->token_iter, "unexpected token encountered.");
    };

    consumption_handler(token_iter->type == expected_type, s, f);
}

void
Parser::expect_and_store(const Token::Type expected_type, std::string &container)
{

    std::function<void()> s = [&container, this]() {
        container = this->token_iter->contents;
    };
    std::function<void()> f = [this]() {
        throw ParseError(*this->token_iter, "unexpected token encountered.");
    };

    consumption_handler(token_iter->type == expected_type, s, f);
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

std::unique_ptr<PrototypeAST>
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

    return std::make_unique<PrototypeAST>(func_name, std::move(arg_names));
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
    expect(Token(Token::Type::RESERVED_SYMBOL, "{"));
    auto expr = parse_expr();
    expect(Token(Token::Type::RESERVED_SYMBOL, "}"));

    return std::make_unique<DefnASTNode>(std::move(prototype), std::move(expr));
}

std::unique_ptr<ASTNode>
Parser::parse_extern()
{
    auto prototype = parse_prototype();

    return std::make_unique<ExternASTNode>(std::move(prototype));
}

std::unique_ptr<ASTNode>
Parser::parse_top_level_expression()
{
    /* treat top level function as anonymous function with no arguments */
    auto expr = parse_expr();
    auto prototype = std::make_unique<PrototypeAST>("__ANON__", std::vector<std::string>());

    return std::make_unique<DefnASTNode>(std::move(prototype), std::move(expr));
}

std::unique_ptr<ASTExpr>
Parser::parse_expr()
{
    return parse_expr(0);
}

std::unique_ptr<ASTExpr>
Parser::parse_expr(int p)
{
    std::unique_ptr<ASTExpr> LHS = parse_primary_expr();

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
        LHS = std::make_unique<BinOpASTExpr>(binop_str, std::move(LHS), std::move(RHS));
    }

    return LHS;
}

std::unique_ptr<ASTExpr>
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

std::unique_ptr<ASTExpr>
Parser::parse_identifier_expr()
{
    std::string identifier_name;
    expect_and_store(Token::Type::IDENTIFIER, identifier_name);

    if (accept(Token(Token::Type::RESERVED_SYMBOL, "("))) {
        /* found open parenthesis so identifier is function call */

        std::vector<std::unique_ptr<ASTExpr>> args;
        if (*token_iter != Token(Token::Type::RESERVED_SYMBOL, ")")) {
            do
                args.push_back(parse_expr());
            while (accept(Token(Token::Type::RESERVED_SYMBOL, ",")));
        }

        expect(Token(Token::Type::RESERVED_SYMBOL, ")"));

        return std::make_unique<CallASTExpr>(identifier_name, std::move(args));

    } else { /* otherwise it is just a variable */
        return std::make_unique<VariableASTExpr>(identifier_name);
    }
}

std::unique_ptr<ASTExpr>
Parser::parse_numeric_literal_expr()
{
    std::string double_str;
    expect_and_store(Token::Type::NUMERIC_LITERAL, double_str);

    return std::make_unique<LiteralDoubleASTExpr>(std::stod(double_str));
}

std::unique_ptr<ASTExpr>
Parser::parse_paren_expr()
{
    expect(Token(Token::Type::RESERVED_SYMBOL, "("));
    auto enclosed_expr = parse_expr();
    expect(Token(Token::Type::RESERVED_SYMBOL, ")"));

    return enclosed_expr;
}

AST
Parser::parse_text(const std::string &text)
{
    AST ast;
    tokens = tokenize(text);
    token_iter = tokens.begin();

    while (token_iter->type != Token::Type::END_OF_FILE) {
        ast.push_back(parse_statement());
    }

    tokens.clear();
    return std::move(ast);
}
