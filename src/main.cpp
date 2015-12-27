#include "ast.h"
#include "ast_printer.h"
#include "parser.h"
#include "repl.h"

#include <cerrno>
#include <fstream>
#include <string>

std::string get_file_contents(const char *filename)
{
  std::ifstream in(filename, std::ios::in);
  if (in)
  {
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
  }
  throw(errno);
}

int main(void) {
    // std::string fstr = get_file_contents("foo.gup");
    // Parser p = Parser();
    // AST ast = p.get_ast(fstr);

    //print_ast(ast);
    repl();

    return 0;
}


