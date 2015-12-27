#include "ast_printer.h"

void
ASTPrinter::append_line_to_output(std::ostringstream &line)
{
    if (tab_level == 0) {
        node_output << line.str() << '\n';
    } else {
        for (int i = 0; i < tab_level; i++)
            node_output << '|' << "  ";
        node_output << line.str() << '\n';
    }
    line.str("");
    line.clear();
}

void
ASTPrinter::process_prototype(const Prototype &proto)
{
    std::ostringstream tmp;

    tmp << "FUNCTION NAME: " << proto.name;
    append_line_to_output(tmp);

    tmp << "FUNCTION ARGS: ";
    for (auto arg : proto.args)
        tmp << arg <<  " ";

    append_line_to_output(tmp);
}

void
ASTPrinter::apply(const ASTExternNode &extern_node)
{
    std::ostringstream tmp;
    tmp << "EXTERN:";
    append_line_to_output(tmp);
    tab_level++;

    process_prototype(*extern_node.prototype);
    tab_level--;

    node_output << std::string(1,'\n');
}

void
ASTPrinter::apply(const ASTDefnNode &defn_node)
{
    std::ostringstream tmp;
    tmp << "DEFN:";
    append_line_to_output(tmp);
    tab_level++;

    process_prototype(*defn_node.prototype);

    tmp << "BODY:";
    append_line_to_output(tmp);
    tab_level++;

    defn_node.body->accept(*this);

    tab_level -= 2;
    node_output << std::string(1,'\n');
}

void
ASTPrinter::apply(const VariableExpr &var_expr)
{
    std::ostringstream tmp;
    tmp << "VARIABLE: " << var_expr.name;
    append_line_to_output(tmp);
}

void
ASTPrinter::apply(const LiteralDoubleExpr &double_expr)
{
    std::ostringstream tmp;
    tmp << "DOUBLE: " << double_expr.value;
    append_line_to_output(tmp);
}

void
ASTPrinter::apply(const BinOpExpr &bin_op_expr)
{
    std::ostringstream tmp;
    tmp << "BINOP: " << bin_op_expr.binop;
    append_line_to_output(tmp);
    tab_level++;

    tmp << "LHS:";
    bin_op_expr.LHS->accept(*this);

    tmp << "RHS:";
    bin_op_expr.RHS->accept(*this);
    tab_level--;

}

void
ASTPrinter::apply(const CallExpr &call_expr)
{
    std::ostringstream tmp;
    tmp << "CALL:";
    append_line_to_output(tmp);
    tab_level++;

    tmp << "FUNCTION: " << call_expr.callee;
    append_line_to_output(tmp);
    tmp << "ARGUMENTS: ";
    append_line_to_output(tmp);
    tab_level++;

    unsigned int i = 1;
    for (auto &a : call_expr.args) {
        tmp << "ARG " << i << ":";
        append_line_to_output(tmp);
        i++;
        tab_level++;
        a->accept(*this);
        tab_level--;
    }
}

void
print_ast(const AST &ast)
{
    ASTPrinter printer = ASTPrinter();
    std::string output;
    for (auto const &node : ast)
    {
        node->accept(printer);
        printer.get_node_output(output);
        std::cout << output;
    }
}
