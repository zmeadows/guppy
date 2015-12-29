#pragma once

#include "ast.h"
#include "util.h"

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

struct UnitGeneratorContext {
    std::unique_ptr<llvm::Module> llvm_module;
    llvm::IRBuilder<> builder;
    std::map<std::string, llvm::Value*> named_values;

    UnitGeneratorContext()
        : llvm_module(std::make_unique<llvm::Module>("__UNIT__", llvm::getGlobalContext())),
        builder(llvm::IRBuilder<>(llvm::getGlobalContext())) {}
};

template < typename A, typename B >
class Accumulator : public virtual B {
    static_assert(is_traverser<B>::value,
            "ASTTraversable type in Accumulator must be NodeTraverse or ExprTraverser");
protected:
    UnitGeneratorContext* const context;
    A result;

public:
    virtual A extract() { return result; }

    Accumulator(UnitGeneratorContext* context, A def_val)
        : context(context), result(def_val) {}

    virtual ~Accumulator() {}
};

struct FunctionGen : public Accumulator<llvm::Function*, NodeTraverser> {
    void apply_to(const ExternASTNode &extern_node) override;
    void apply_to(const DefnASTNode &defn_node) override;

    llvm::Function* process_prototype(const PrototypeAST &proto);

    llvm::Function* extract() override {
        auto result_copy = result;
        result = nullptr;
        return result_copy;
    }

    FunctionGen(UnitGeneratorContext* context) : Accumulator(context, nullptr) {}
};

struct ValueGen : public Accumulator<llvm::Value*, ExprTraverser> {
    void apply_to(const VariableASTExpr &var_expr) override;
    void apply_to(const LiteralDoubleASTExpr &double_expr) override;
    void apply_to(const BinOpASTExpr &bin_op_expr) override;
    void apply_to(const CallASTExpr &call_expr) override;

    llvm::Value* extract() override {
        auto result_copy = result;
        result = nullptr;
        return result_copy;
    }

    ValueGen(UnitGeneratorContext* context) : Accumulator(context, nullptr) {}
};

