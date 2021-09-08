%option noyywrap

%{
#include <unistd.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <iostream>
#include <string>
#include "lex.h"
#include "parser.h"
int yylex();
size_t linecount = 0;
#define YY_INPUT(buf,result,max_size) result = getinput(buf, max_size);

int getinput(char *buf, size_t size) {
  char *line;
  if (feof(yyin)) return YY_NULL;
  if (isatty(STDIN_FILENO) && (isatty(STDERR_FILENO) || isatty(STDOUT_FILENO))) line = readline("> ");
  else {
    line = (char*)malloc(size*sizeof(char));
    if (getline(&line, &size, stdin) == -1) { free(line); return YY_NULL; };
  }

  if (!line) return YY_NULL;
  if (strlen(line) > size-2) {
    fprintf(stderr,"input line too long\n");
    return YY_NULL;
  }
  sprintf(buf,"%s\n",line);
  if (isatty(STDIN_FILENO) && (isatty(STDERR_FILENO) || isatty(STDOUT_FILENO))) {
    add_history(line); ++linecount;
  }
  free(line);
  return strlen(buf);
}
%}

DIGIT [0-9]
LETTER [a-zA-Z]
TEST (>=?)|(<=?)|=
PRIMOP [+*/&%\|\-]|not|cons

%%
"(" { return LPAREN; }
")" { return RPAREN; }
"[" { return LBRACK; }
"]" { return RBRACK; }
lambda|λ { return LAMBDA; }
"local" { return LOCAL; }
"letrec" { return LETREC; }
"let" { return LET; }
"let*" { return LETSTAR; }
"define" { return DEFINE; }
"if" { return IF; }
"cond" { return COND; }
"else" { return ELSE; }
"or" { return OR; }
"and" { return AND; }
"begin" { return BEGINN; }
"quote" { return QUOTE; }
"quasiquote" { return QUASIQUOTE; }
"define-struct" { return DEFSTRUCT; }
"#\\"{LETTER} { yylval.letter = yytext[2]; return CHAR; }
"#\space" { yylval.letter = ' '; return CHAR; }
"#\tab" { yylval.letter = '\t'; return CHAR; }
"#\newline" { yylval.letter = '\n'; return CHAR; }
#t(rue)?|#T {
  yylval.boolean = true;
  return BOOLEAN;
}
#f(alse)?|#F {
  yylval.boolean = false;
  return BOOLEAN;
}
"'" { return SQUOTE; }
{PRIMOP} {
  yylval.id = new std::string{yytext};
  return PRIMOP;
}
"@" { return AT; }
"modulo" {
  yylval.id = new std::string{"%"};
  return PRIMOP;
}
{TEST} {
  yylval.id = new std::string{yytext};
  return PRIMOP;
}
"=>" { return RARROW; }
\"[^\"]*\" { yylval.id = new std::string{yytext}; return STRING; }
-?{DIGIT}+ {
  yylval.num = std::stoi(yytext);
  return NUMBER;
}
[ \t\r\n] ;
[^\",\'`\(\)\[\]\{\};#\| \t\r\n]+ {
  yylval.id = new std::string{yytext};
  return IDENTIFIER;
}

%%
