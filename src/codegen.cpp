#include "codegen.h"

void
ASTValueGen::apply(const ASTExternNode &double_expr)
{
}

void
ASTValueGen::apply(const ASTDefnNode &double_expr)
{
}

void
ASTValueGen::apply(const VariableExpr &var_expr)
{
    assert(value == nullptr);
    auto it = generator->named_values->find(var_expr.name);
    if (it != generator->named_values->end()) {
        value = it->second;
    } else {
        throw 0;
    }
}

void
ASTValueGen::apply(const LiteralDoubleExpr &double_expr)
{
    value = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(double_expr.value));
}

void
ASTValueGen::apply(const BinOpExpr &binop_expr)
{
    ASTValueGen lhs_valgen(generator);
    ASTValueGen rhs_valgen(generator);

    binop_expr.LHS->accept(lhs_valgen);
    binop_expr.RHS->accept(lhs_valgen);

    llvm::Value* lhs_val = lhs_valgen.get_value();
    llvm::Value* rhs_val = rhs_valgen.get_value();

    if (binop_expr.binop == "+") {
        value = generator->builder->CreateFAdd(lhs_val, rhs_val, "addtmp");
    } else if (binop_expr.binop == "-") {
        value = generator->builder->CreateFSub(lhs_val, rhs_val, "addtmp");
    } else if (binop_expr.binop == "*") {
        value = generator->builder->CreateFMul(lhs_val, rhs_val, "addtmp");
    } else if (binop_expr.binop == "<") {
    }
}

void
ASTValueGen::apply(const CallExpr &binop_expr)
{
}
