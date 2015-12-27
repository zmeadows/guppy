#pragma once

#include <memory>
#include <string>
#include <vector>

class ASTFunctor;

class ASTFunctee {
public:
    virtual void accept(ASTFunctor &functor) const = 0;
    virtual ~ASTFunctee() {}
};

struct ASTNode : public virtual ASTFunctee {
    virtual ~ASTNode() {}
};

typedef std::vector<std::unique_ptr<ASTNode>> AST;

struct Expr : public virtual ASTFunctee {
    virtual ~Expr() {}
};

struct Prototype {
    const std::string name;
    const std::vector<std::string> args;

    Prototype(const std::string &name, const std::vector<std::string> &args)
        : name(name), args(std::move(args)) {}
};

struct ASTExternNode : public virtual ASTNode {
    std::unique_ptr<Prototype> prototype;

    void accept(ASTFunctor &functor) const;
    ASTExternNode(std::unique_ptr<Prototype> prototype)
        : prototype(std::move(prototype)) {}
};

struct ASTDefnNode : public virtual ASTNode {
    std::unique_ptr<Prototype> prototype;
    std::unique_ptr<Expr> body;

    void accept(ASTFunctor &functor) const;
    ASTDefnNode(std::unique_ptr<Prototype> prototype, std::unique_ptr<Expr> body)
        : prototype(std::move(prototype)), body(std::move(body)) {}
};

struct VariableExpr : public virtual Expr {
    const std::string name;

    void accept(ASTFunctor &functor) const;
    VariableExpr(const std::string &name) : name(name) {}
};

struct LiteralDoubleExpr : public virtual Expr {
    const double value;

    void accept(ASTFunctor &functor) const;
    LiteralDoubleExpr(double value) : value(value) {}
};

struct BinOpExpr : public virtual Expr {
    const std::string binop;
    std::unique_ptr<Expr> LHS, RHS;

    void accept(ASTFunctor &functor) const;
    BinOpExpr(std::string binop, std::unique_ptr<Expr> LHS,
            std::unique_ptr<Expr> RHS)
        : binop(binop), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

struct CallExpr : public virtual Expr {
    const std::string callee;
    std::vector<std::unique_ptr<Expr>> args;

    void accept(ASTFunctor &functor) const;
    CallExpr(const std::string &callee, std::vector<std::unique_ptr<Expr>> args)
        : callee(callee), args(std::move(args)) {}
};

class ASTFunctor {
public:
    virtual void apply(const ASTExternNode &extern_node) = 0;
    virtual void apply(const ASTDefnNode &defn_node) = 0;
    virtual void apply(const VariableExpr &var_expr) = 0;
    virtual void apply(const LiteralDoubleExpr &double_expr) = 0;
    virtual void apply(const BinOpExpr &bin_op_expr) = 0;
    virtual void apply(const CallExpr &call_expr) = 0;

    virtual ~ASTFunctor() {}
};

