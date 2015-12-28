#pragma once

#include "ast.h"
#include "util.h"

#include <map>
#include <memory>
#include <string>

#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"

struct ModuleGenerator {
    const std::string file_path;
    std::unique_ptr<llvm::Module> llvm_module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<std::map<std::string, llvm::Value*>> named_values;

    ModuleGenerator() : builder(std::make_unique<llvm::IRBuilder<>>(llvm::getGlobalContext())) {}
};

template < typename A, typename B >
class Accumulator : public virtual B {
    static_assert(is_traverser<B>::value,
            "ASTTraversable type in Accumulator must be NodeTraverse or ExprTraverser");
protected:
    ModuleGenerator* const generator;
    A value;

public:
    A get() { return value; }

    Accumulator(ModuleGenerator* generator, A def_val)
        : generator(generator), value(def_val) {}
};

struct ValueGen : public Accumulator<llvm::Value*, ExprTraverser> {
    void apply(const VariableASTExpr &var_expr) override;
    void apply(const LiteralDoubleASTExpr &double_expr) override;
    void apply(const BinOpASTExpr &bin_op_expr) override;
    void apply(const CallASTExpr &call_expr) override;

    ValueGen(ModuleGenerator* generator) :
        Accumulator(generator, nullptr) {}
};

struct FunctionGen : public Accumulator<llvm::Function*, NodeTraverser> {
    void apply(const ExternASTNode &extern_node) override;
    void apply(const DefnASTNode &defn_node) override;

    FunctionGen(ModuleGenerator* generator) :
        Accumulator(generator, nullptr) {}
};
