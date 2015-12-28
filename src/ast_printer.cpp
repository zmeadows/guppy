#include "ast_printer.h"

void
ASTPrinter::append_line_to_output(std::ostringstream &line)
{
    if (tab_level == 0) {
        node_output << line.str() << '\n';
    } else {
        for (unsigned i = 0; i < tab_level; i++)
            node_output << '|' << "  ";
        node_output << line.str() << '\n';
    }
    line.str("");
    line.clear();
}

void
ASTPrinter::process_prototype(const PrototypeAST &proto)
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
ASTPrinter::print_node_output()
{
    std::cout << node_output.str() << std::endl;
    node_output.str("");
    node_output.clear();
    tab_level = 0;
}

void
ASTPrinter::apply(const ExternASTNode &extern_node)
{
    std::ostringstream tmp;
    tmp << "EXTERN:";
    append_line_to_output(tmp);
    tab_level++;

    process_prototype(*extern_node.prototype);

    print_node_output();
}

void
ASTPrinter::apply(const DefnASTNode &defn_node)
{
    std::ostringstream tmp;
    tmp << "DEFN:";
    append_line_to_output(tmp);
    tab_level++;

    process_prototype(*defn_node.prototype);

    tmp << "BODY:";
    append_line_to_output(tmp);
    tab_level++;

    defn_node.body->inject(*this);

    print_node_output();
}

void
ASTPrinter::apply(const VariableASTExpr &var_expr)
{
    std::ostringstream tmp;
    tmp << "VARIABLE: " << var_expr.name;
    append_line_to_output(tmp);
}

void
ASTPrinter::apply(const LiteralDoubleASTExpr &double_expr)
{
    std::ostringstream tmp;
    tmp << "DOUBLE: " << double_expr.value;
    append_line_to_output(tmp);
}

void
ASTPrinter::apply(const BinOpASTExpr &bin_op_expr)
{
    std::ostringstream tmp;
    tmp << "BINOP: " << bin_op_expr.binop;
    append_line_to_output(tmp);
    tab_level++;

    tmp << "LHS:";
    bin_op_expr.LHS->inject(*this);

    tmp << "RHS:";
    bin_op_expr.RHS->inject(*this);
    tab_level--;
}

void
ASTPrinter::apply(const CallASTExpr &call_expr)
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
        a->inject(*this);
        tab_level--;
    }

    tab_level -= 2;
}
