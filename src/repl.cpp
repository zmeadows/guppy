#include "repl.h"

void
repl()
{
    std::string user_input;
    Parser parser = Parser();
    AST ast;

    auto process_line = [&parser, &user_input, &ast](std::string new_user_input_line) -> void {
        try
        {
            user_input += new_user_input_line;
            ast = parser.parse_text(user_input);
            user_input.clear();
        }

        catch (ParseIncomplete inc_exc)
        {
            std::cout << "INCOMPLETE INPUT ENCOUNTERED..." << std::endl;
            user_input += " ";
            return;
        }

        catch (ParseError perr)
        {
            //std::cout << perr.what();
            return;
        }
    };

    std::string user_line, printer_output;
    ASTPrinter printer = ASTPrinter();

    while (true)
    {
        if (user_input.empty()) {
            std::cout << "guppy> " << std::flush;
        } else {
            std::cout << '\t' << std::flush;
        }
        std::getline(std::cin, user_line);

        if (user_line == ":q") {
            break;
        } else {
            process_line(user_line);
        }

        for (auto const &a : ast)
        {
            a->accept(printer);
        }
        ast.clear();

    }
}


