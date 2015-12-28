#pragma once

#include "ast.h"

#include <iostream>
#include <memory>
#include <sstream>

class ASTPrinter : public virtual ASTFunctor {
    unsigned int tab_level;
    std::ostringstream node_output;
    void append_line_to_output(std::ostringstream &line);
    void process_prototype(const Prototype &proto);
    void print_node_output();

public:
    void apply(const ASTExternNode &extern_node) override;
    void apply(const ASTDefnNode &defn_node) override;
    void apply(const VariableExpr &var_expr) override;
    void apply(const LiteralDoubleExpr &double_expr) override;
    void apply(const BinOpExpr &bin_op_expr) override;
    void apply(const CallExpr &call_expr) override;

    ASTPrinter() : tab_level(0), node_output(std::ostringstream()) {}
};

