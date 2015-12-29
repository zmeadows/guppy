#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

/*
<UNIT>      ::= <NODE>*
<NODE>   ::= <DECLARATION> | <DEFINITION>
<DECLARATION> ::= EXTERN <PROTOTYPE>
<PROTOTYPE>   ::= IDENTIFIER OPEN_PAREN (IDENTIFIER COMMA ?)* CLOSE_PAREN
<DEFINITION>  ::= DEFN <PROTOTYPE> OPEN_CURL_BRACKET <E> CLOSE_CURL_BRACKET

<E> ::= <EXPR(0)>
<EXPR(p)>  ::= <P> (<BINOP> <EXPR(q)>)*
<P> ::= IDENTIFIER | DOUBLE | <CALL_EXPR> | <PAREN_EXPR>
<CALL_EXPR>   ::= IDENTIFIER OPEN_PAREN (<EXPR> COMMA ?)* CLOSE_PAREN
<PAREN_EXPR>  ::= OPEN_PAREN <EXPR> CLOSE_PAREN
<BINOP> ::= "+" | "-" | "*" | "/" | "^" | "<"
*/

class NodeTraverser;
class ExprTraverser;

template< typename T >
struct is_traverser {
    static const bool value = false;
};

template<>
struct is_traverser< NodeTraverser > {
    static const bool value = true;
};

template<>
struct is_traverser< ExprTraverser > {
    static const bool value = true;
};

template < typename T >
struct ASTTraversable {
    static_assert(is_traverser<T>::value,
            "ASTTraversable type must be NodeTraverse or ExprTraverser");

    virtual void inject(T &traverser) const = 0;
    virtual ~ASTTraversable() {}
};

struct ASTNode : public virtual ASTTraversable<NodeTraverser> {
    virtual ~ASTNode() {}
};

struct ASTExpr : public virtual ASTTraversable<ExprTraverser> {
    virtual ~ASTExpr() {}
};

struct PrototypeAST {
    const std::string name;
    const std::vector<std::string> args;

    PrototypeAST(const std::string &name, const std::vector<std::string> &args)
        : name(name), args(std::move(args)) {}
};

struct ExternASTNode : public virtual ASTNode {
    std::unique_ptr<PrototypeAST> prototype;

    void inject(NodeTraverser &traverser) const override;
    ExternASTNode(std::unique_ptr<PrototypeAST> prototype)
        : prototype(std::move(prototype)) {}
};

struct DefnASTNode : public virtual ASTNode {
    std::unique_ptr<PrototypeAST> prototype;
    std::unique_ptr<ASTExpr> body;

    void inject(NodeTraverser &traverser) const override;
    DefnASTNode(std::unique_ptr<PrototypeAST> prototype, std::unique_ptr<ASTExpr> body)
        : prototype(std::move(prototype)), body(std::move(body)) {}
};

struct VariableASTExpr : public virtual ASTExpr {
    const std::string name;

    void inject(ExprTraverser &traverser) const override;
    VariableASTExpr(const std::string &name) : name(name) {}
};

struct LiteralDoubleASTExpr : public virtual ASTExpr {
    const double value;

    void inject(ExprTraverser &traverser) const override;
    LiteralDoubleASTExpr(double value) : value(value) {}
};

struct BinOpASTExpr : public virtual ASTExpr {
    const std::string binop;
    std::unique_ptr<ASTExpr> LHS, RHS;

    void inject(ExprTraverser &traverser) const override;
    BinOpASTExpr(std::string binop, std::unique_ptr<ASTExpr> LHS,
            std::unique_ptr<ASTExpr> RHS)
        : binop(binop), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

struct CallASTExpr : public virtual ASTExpr {
    const std::string callee;
    std::vector<std::unique_ptr<ASTExpr>> args;

    void inject(ExprTraverser &traverser) const override;
    CallASTExpr(const std::string &callee, std::vector<std::unique_ptr<ASTExpr>> args)
        : callee(callee), args(std::move(args)) {}
};

class NodeTraverser {
public:
    virtual void apply_to(const ExternASTNode &extern_node) = 0;
    virtual void apply_to(const DefnASTNode &defn_node) = 0;

    virtual ~NodeTraverser() {}
};

class ExprTraverser {
public:
    virtual void apply_to(const VariableASTExpr &var_expr) = 0;
    virtual void apply_to(const LiteralDoubleASTExpr &double_expr) = 0;
    virtual void apply_to(const BinOpASTExpr &bin_op_expr) = 0;
    virtual void apply_to(const CallASTExpr &call_expr) = 0;

    virtual ~ExprTraverser() {}
};

typedef std::vector<std::unique_ptr<ASTNode>> AST;
