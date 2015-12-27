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

public:
    void apply(const ASTExternNode &extern_node);
    void apply(const ASTDefnNode &defn_node);
    void apply(const VariableExpr &var_expr);
    void apply(const LiteralDoubleExpr &double_expr);
    void apply(const BinOpExpr &bin_op_expr);
    void apply(const CallExpr &call_expr);

    void get_node_output(std::string &container) {
        container = node_output.str();
        node_output.str("");
        node_output.clear();
        tab_level = 0;
    }

    ASTPrinter() : tab_level(0), node_output(std::ostringstream()) {}
};

void print_ast(const AST &ast);
