%require "3.2"
%parse-param {void*& result}
%start prog

%{
#include <iostream>
#include <string>
#include <vector>
#include "lex.h"
int yylex();
void yyerror(void* result, char* s);
%}
%union {
  int num;
  bool boolean;
  char letter;
  std::string* id;
  ExpNode* exp;
  StmtNode* statement;
  DefnNode* definition;
  std::vector<StmtNode*>* statementlist;
  //std::vector<ExpNode*>* exp_list;
  ProgNode* program;
  std::vector<std::string*>* idlist;
  ExpNode* condclause;
  //QuotedNode* quoted;
  //QuasiNode* quasiquoted;
}
%type <exp> expr quoted
%type <statementlist> stmt seq exprlist defnlist quotedlist
//%type <statementlist> letdefnlist
%type <program> prog
%type <idlist> identifierlist
%type <definition> defn
%type <condclause> condclause
//%type <quasiquoted> quasiquoted

%token <id> IDENTIFIER PRIMOP STRING
%token <num> NUMBER
%token <boolean> BOOLEAN
%token <letter> CHAR
%token LPAREN
%token RPAREN
%token LBRACK
%token RBRACK
%token LAMBDA
%token LOCAL
%token COMMA
%token LETREC
%token LET
%token LETSTAR
%token DEFINE
%token AND
%token OR
%token IF
%token COND
%token ELSE
%token RARROW
%token BEGINN // BEGIN is reserved for bison/flex
%token VOID
%token DEFSTRUCT
%token SQUOTE
%token QUOTE
%token QUASIQUOTE
%token AT

%%

identifierlist: identifierlist IDENTIFIER { $$->push_back($2); }
                | %empty { $$ = new std::vector<std::string*>{}; };

/*
cons: LPAREN CONS expr cons RPAREN { $$ = new ConsNode{$3,$4}; }
    | EMPTY { $$ = new EmptyNode{}; };

| cons { $$ = $1; }
*/

expr: LPAREN LAMBDA LPAREN identifierlist RPAREN expr RPAREN { $$ = new LambdaNode{*$4,*$6}; }
     | LPAREN LAMBDA IDENTIFIER expr RPAREN { $$ = new AltLambdaNode{*$3,*$4}; }
     | LPAREN expr exprlist RPAREN { $$ = new AppNode{*$2,*$3}; }
     | LPAREN IF expr expr expr RPAREN { $$ = new IfNode{*$3,*$4,*$5}; }
     | IDENTIFIER { $$ = new IdNode{*$1}; }
     | NUMBER { $$ = new NumNode{$1}; }
     | BOOLEAN { $$ = new BoolNode{$1}; }
     | LPAREN AND exprlist RPAREN { $$ = new AndOrNode{true,*$3}; }
     | LPAREN OR exprlist RPAREN { $$ = new AndOrNode{false,*$3}; }
     | CHAR { $$ = new CharNode{$1}; }
     | STRING { $$ = new StrNode{*$1}; }
     | LPAREN COND condclause RPAREN { $$ = $3; }
     | LPAREN BEGINN seq RPAREN { $$ = new BeginNode{*$3}; }
     | LPAREN BEGINN RPAREN { $$ = new BeginNode{*(new std::vector<StmtNode*>{})}; } // could just produce a VoidNode instead...
     | LPAREN LOCAL LBRACK defnlist RBRACK seq RPAREN { auto vec = $4; vec->insert(vec->end(), $6->begin(), $6->end()); $$ = new BeginNode{*vec}; }
     | PRIMOP { $$ = new PrimOpNode{*$1}; }
     /*
     | LPAREN LETSTAR LPAREN letdefnlist RPAREN seq RPAREN { auto vec = $4; vec->insert(vec->end(), $6->begin(), $6->end()); $$ = new BeginNode{*vec}; }
     | LPAREN LET LPAREN letdefnlist RPAREN seq RPAREN { $$ = new LetNode{*$4,*$6}; }
     | LPAREN LETREC LPAREN letdefnlist RPAREN seq RPAREN { $$ = new LetrecNode{*$4,*$6}; }
     | LPAREN VOID exprlist RPAREN { $$ = new VoidNode{}; }
     */
     | SQUOTE quoted { $$ = new QuotedNode{*$2}; }
     | LPAREN QUOTE quoted RPAREN { $$ = new QuotedNode{*$3}; }
     /*
     | SQUOTE quasiquoted { $$ = new QuasiNode{*$2}; }
     | LPAREN QUASIQUOTE quasiquoted RPAREN { $$ = new QuasiNode{*$3}; }
     */
     | SQUOTE LPAREN RPAREN { $$ = new QuotedNode{*(new ListNode{*(new std::vector<StmtNode*>{})})}; };
     | LPAREN QUOTE LPAREN RPAREN RPAREN { $$ = new QuotedNode{*(new ListNode{*(new std::vector<StmtNode*>{})})}; };

quoted: IDENTIFIER { $$ = new IdNode{*$1}; }
      | NUMBER { $$ = new NumNode{$1}; }
      | STRING { $$ = new StrNode{*$1}; }
      | CHAR { $$ = new CharNode{$1}; }
      | BOOLEAN { $$ = new BoolNode{$1}; }
      | LPAREN quotedlist RPAREN { $$ = new ListNode{*$2}; }
      | SQUOTE quoted { $$ = new QuotedNode{*$2}; }
      | COMMA quoted {}
      | COMMA AT quoted {};

quotedlist: quotedlist quoted { $$->push_back(new QuotedNode{*$2}); }
          | quoted { $$ = new std::vector<StmtNode*>{}; $$->push_back(new QuotedNode{*$1}); };
/*
quasiquoted: IDENTIFIER { return $1; }
           | NUMBER { return $1; }
           | STRING { return $1; }
           //| CHAR { return $1; }
           //| LPAREN quasilist RPAREN {}
           | SQUOTE quasiquoted {}
           | COMMA expr {}
           | COMMA AT expr {};
*/
exprlist: exprlist expr { $$->push_back($2); }
         | %empty { $$ = new std::vector<StmtNode*>{}; };

defnlist: defnlist defn { $$->push_back($2); }
         | %empty { $$ = new std::vector<StmtNode*>{}; }
/*
letdefnlist: letdefnlist LBRACK IDENTIFIER expr RBRACK { $$->push_back(new DefnNode{*$3,*$4}); }
          | %empty { $$ = new std::vector<StmtNode*>{}; };
*/
stmt: stmt expr { $$->push_back($2); }
     | stmt defn { $$->push_back($2); }
     | expr {
       $$ = new std::vector<StmtNode*>{};
       $$->push_back($1);
     }
     | defn {
       $$ = new std::vector<StmtNode*>{};
       $$->push_back($1);
     };

seq: stmt expr { $$ = $1; $$->push_back($2); }
   | expr { $$ = new std::vector<StmtNode*>{}; $$->push_back($1); };

// (if test-expr (begin ...) (if ...))

condclause: LBRACK expr seq RBRACK condclause { $$ = new IfNode{*$2, *(new BeginNode{*$3}), *$5}; }
          | LBRACK ELSE seq RBRACK condclause { $$ = new IfNode{*(new BoolNode{true}),*(new BeginNode{*$3}), *$5}; }
          | LBRACK expr RARROW expr RBRACK condclause {
            std::vector<StmtNode*>* args = new std::vector<StmtNode*>;
            args->push_back($2);
            $$ = new IfNode{*$2,*(new AppNode{*$4,*args}), *$6};
          }
          | LBRACK expr RBRACK condclause { $$ = new IfNode{*$2,*($2->clone()),*$4}; }
          | %empty { $$ = new VoidNode; }

defn: LPAREN DEFINE IDENTIFIER expr RPAREN { $$ = new DefnNode{*$3,*$4}; }
    | LPAREN DEFINE LPAREN IDENTIFIER identifierlist RPAREN expr RPAREN { $$ = new DefnNode{*$4, *(new LambdaNode{*$5,*$7})}; }
    | LPAREN DEFSTRUCT IDENTIFIER LPAREN identifierlist RPAREN RPAREN {};

prog: %empty { result = nullptr; };
     | stmt { $$ = new ProgNode{*$1}; result = $$; };

%%

void yyerror(void* result, char* s) {
    fprintf(stderr, "Parse error: %s\n", s);
};
