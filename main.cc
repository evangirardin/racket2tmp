#include <string>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "lex.h"
#include "lexer.h"
#include "parser.h"

bool church = false;
bool full = false;
extern size_t linecount;

const char* harness = "__SOURCE_FROM_EVAL__";

int usage(const char* name) {
  std::cout << "Usage: " << name << " [options]\nOptions:" << std::endl;
  std::cout << "-church  Reinterpret numbers and booleans in Church form." << std::endl;
  std::cout << "-full    Write a C++20 TMP program to evaluate the converted expression." << std::endl;
  return 1;
}

inline bool file_exists(const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

// This parser supports all features from the "Intermediate Student with Lambda"
// teaching language, with the exception of test-case, library-require, package,
// and (time expr).
// It also supports a few advanced features such as void, full-Racket cond,
// 0-arg functions, and begin.

int main(int argc, char* argv[]) {
  rl_outstream = (!isatty(STDERR_FILENO) && isatty(STDOUT_FILENO)) ? stdout : stderr;
  bool interactive = (isatty(STDIN_FILENO) && (isatty(STDERR_FILENO) || isatty(STDOUT_FILENO)));
  if (interactive) {
    if (!file_exists(".r2t_history")) write_history(".r2t_history");
    using_history(); read_history(".r2t_history");
  }
  void* output;
  for (int i = 1; i < argc; ++i) {
    std::string arg{argv[i]};
    if (arg == "-church") church = true;
    else if (arg == "-full") full = true;
    else return usage(argv[0]);
  }

  if (yyparse(output)) {
    std::cerr << "Syntax error!" << std::endl;
    return 1;
  }

  if (full) std::cout << harness << "int main() {";

  ProgNode* output_conv = static_cast<ProgNode*>(output);
  if (output) std::cout << *output_conv;

  if (full) std::cout << "}" << std::endl;

  delete output_conv;
  if (interactive) {
    append_history(linecount,".r2t_history");
    history_truncate_file(".r2t_history", 50);
    rl_clear_history();
  }
  yylex_destroy();
  return 0;
}
