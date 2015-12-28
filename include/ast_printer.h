#pragma once

#include "ast.h"

#include <iostream>
#include <memory>
#include <sstream>

class ASTPrinter :
    public virtual NodeTraverser,
    public virtual ExprTraverser
{
    unsigned int tab_level;
    std::ostringstream node_output;
    void append_line_to_output(std::ostringstream &line);
    void process_prototype(const PrototypeAST &proto);
    void print_node_output();

public:
    void apply(const ExternASTNode &extern_node) override;
    void apply(const DefnASTNode &defn_node) override;
    void apply(const VariableASTExpr &var_expr) override;
    void apply(const LiteralDoubleASTExpr &double_expr) override;
    void apply(const BinOpASTExpr &bin_op_expr) override;
    void apply(const CallASTExpr &call_expr) override;

    ASTPrinter() : tab_level(0), node_output(std::ostringstream()) {}
};

