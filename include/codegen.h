#pragma once

#include "ast.h"
#include "util.h"

#include <assert.h>
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

class ASTValueGen : public virtual ASTFunctor {
    ModuleGenerator* generator;
    llvm::Value* value;

public:
    void apply(const ASTExternNode &extern_node) override;
    void apply(const ASTDefnNode &defn_node) override;
    void apply(const VariableExpr &var_expr) override;
    void apply(const LiteralDoubleExpr &double_expr) override;
    void apply(const BinOpExpr &bin_op_expr) override;
    void apply(const CallExpr &call_expr) override;

    llvm::Value* get_value() const { return value; }

    ASTValueGen(ModuleGenerator* generator) : generator(generator), value(nullptr) {}
};
