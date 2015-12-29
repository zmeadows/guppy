#include "codegen.h"

llvm::Function*
FunctionGen::process_prototype(const PrototypeAST &proto)
{
    assert(result == nullptr);

    std::vector<llvm::Type*> type_vector(proto.args.size(),
            llvm::Type::getDoubleTy(llvm::getGlobalContext()));

    llvm::FunctionType *func_type = llvm::FunctionType::get(
            llvm::Type::getDoubleTy(llvm::getGlobalContext()), type_vector, false);

    llvm::Function* func = llvm::Function::Create(func_type,
            llvm::Function::ExternalLinkage, proto.name, context->llvm_module.get());

    unsigned int i = 0;
    for (auto &f_arg : func->args())
    {
        f_arg.setName(proto.args[i++]);
    }

    return func;
}

void
FunctionGen::apply_to(const ExternASTNode &extern_expr)
{
    assert(result == nullptr);

    result = process_prototype(*extern_expr.prototype);
}

void
FunctionGen::apply_to(const DefnASTNode &defn_expr)
{
    assert(result == nullptr);

    llvm::Function* function = context->llvm_module->getFunction(defn_expr.prototype->name);

    if (function == nullptr) {
        function = process_prototype(*defn_expr.prototype);
    }

    if (function == nullptr) {
        throw 0;
    }


    llvm::BasicBlock *bb = llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", function);
    context->builder.SetInsertPoint(bb);

    context->named_values.clear();
    for (auto &farg : function->args())
    {
        context->named_values[farg.getName()] = &farg;
    }

    ValueGen body_gen(context);
    defn_expr.body->inject(body_gen);

    llvm::Value* func_return_value = body_gen.extract();
    if (func_return_value != nullptr)
    {
        context->builder.CreateRet(func_return_value);
        llvm::verifyFunction(*function);
        result = function;
    } else {
        function->eraseFromParent();
        throw -1;
    }
}


void
ValueGen::apply_to(const VariableASTExpr &var_expr)
{
    assert(result == nullptr);

    auto it = context->named_values.find(var_expr.name);

    if (it != context->named_values.end()) {
        result = it->second;
    } else {
        throw 0;
    }
}

void
ValueGen::apply_to(const LiteralDoubleASTExpr &double_expr)
{
    assert(result == nullptr);

    result = llvm::ConstantFP::get(llvm::getGlobalContext(), llvm::APFloat(double_expr.value));
}

void
ValueGen::apply_to(const BinOpASTExpr &binop_expr)
{
    assert(result == nullptr);

    ValueGen lhs_valgen(context);
    binop_expr.LHS->inject(lhs_valgen);
    llvm::Value* lhs_val = lhs_valgen.extract();

    ValueGen rhs_valgen(context);
    binop_expr.RHS->inject(rhs_valgen);
    llvm::Value* rhs_val = rhs_valgen.extract();

    if (binop_expr.binop == "+") {
        result = context->builder.CreateFAdd(lhs_val, rhs_val, "addtmp");
    } else if (binop_expr.binop == "-") {
        result = context->builder.CreateFSub(lhs_val, rhs_val, "subtmp");
    } else if (binop_expr.binop == "*") {
        result = context->builder.CreateFMul(lhs_val, rhs_val, "multmp");
    } else if (binop_expr.binop == "<") {
        lhs_val = context->builder.CreateFCmpULT(lhs_val, rhs_val, "cmptmp");
        result = context->builder.CreateUIToFP(lhs_val,
                llvm::Type::getDoubleTy(llvm::getGlobalContext()), "booltmp");
    } else {
        throw 0;
    }
}

void
ValueGen::apply_to(const CallASTExpr &call_expr)
{
    assert(result == nullptr);

    llvm::Function* callee_func = context->llvm_module->getFunction(call_expr.callee);

    if (!callee_func) throw -1;

    if (callee_func->arg_size() != call_expr.args.size()) throw -1;

    std::vector<llvm::Value*> arg_vals;

    std::unique_ptr<ValueGen> vg;
    for (auto const &a : call_expr.args)
    {
        vg.reset(new ValueGen(context));
        a->inject(*vg);
        arg_vals.push_back(vg->extract());
    }

    result = context->builder.CreateCall(callee_func, arg_vals, "calltmp");
}

