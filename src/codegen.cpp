#include "codegen.h"


void
FunctionGen::apply(const DefnASTNode &double_expr)
{
}

void
ValueGen::apply(const VariableASTExpr &var_expr)
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
ValueGen::apply(const LiteralDoubleASTExpr &double_expr)
{
    assert(value == nullptr);

    value = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(double_expr.value));
}

void
ValueGen::apply(const BinOpASTExpr &binop_expr)
{
    assert(value == nullptr);

    ValueGen lhs_valgen(generator);
    binop_expr.LHS->inject(lhs_valgen);
    llvm::Value* lhs_val = lhs_valgen.get();

    ValueGen rhs_valgen(generator);
    binop_expr.RHS->inject(lhs_valgen);
    llvm::Value* rhs_val = rhs_valgen.get();

    if (binop_expr.binop == "+") {
        value = generator->builder->CreateFAdd(lhs_val, rhs_val, "addtmp");
    } else if (binop_expr.binop == "-") {
        value = generator->builder->CreateFSub(lhs_val, rhs_val, "subtmp");
    } else if (binop_expr.binop == "*") {
        value = generator->builder->CreateFMul(lhs_val, rhs_val, "multmp");
    } else if (binop_expr.binop == "<") {
        lhs_val = generator->builder->CreateFCmpULT(lhs_val, rhs_val, "cmptmp");
        value = generator->builder->CreateUIToFP(lhs_val,
                llvm::Type::getDoubleTy(llvm::getGlobalContext()), "booltmp");
    } else {
        throw 0;
    }
}

void
FunctionGen::apply(const ExternASTNode &double_expr)
{
}

void
ValueGen::apply(const CallASTExpr &call_expr)
{
    assert(value == nullptr);

    llvm::Function* callee_func = generator->llvm_module->getFunction(call_expr.callee);

    if (!callee_func) throw -1;

    if (callee_func->arg_size() != call_expr.args.size()) throw -1;

    std::vector<llvm::Value*> arg_vals;

    for (auto const &a : call_expr.args)
    {
        ValueGen vg(generator);
        a->inject(vg);
        arg_vals.push_back(vg.get());
    }

    value = generator->builder->CreateCall(callee_func, arg_vals, "calltmp");
}
