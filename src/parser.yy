%skeleton "lalr1.cc"
%require "3.0"
%debug
%defines
%define api.namespace{DTL}
%define api.parser.class {Parser}
%define api.value.type variant
%define parse.error verbose
%output "parser.cc"
%token-table



%code requires{
	#include <list>
	#include "tokens.hpp"
	#include "ast.hpp"
	namespace DTL {
		class Scanner;
	}


//The following definition is required when 
// we don't use the %locations directive (which we won't)
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

//End "requires" code
}

%parse-param { DTL::Scanner &scanner }
%parse-param { DTL::ProgramNode** root}
%code {
   // C std code for utility functions
   #include <iostream>
   #include <cstdlib>
   #include <fstream>

   // Our code for interoperation between scanner/parser
   #include "scanner.hpp"
   #include "ast.hpp"
   #include "tokens.hpp"

  //Request tokens from our scanner member, not 
  // from a global function
  #undef yylex
  #define yylex scanner.yylex



}

%token                       END   0 "end file"
%token	<DTL::Token *>       ASSIGN
%token	<DTL::IDToken *>     ID
%token	<DTL::Token *>       INT
%token	<DTL::IntLitToken *> INTLITERAL
%token	<DTL::Token *>       LCURLY
%token	<DTL::Token *>       LESS
%token	<DTL::Token *>       LPAREN
%token	<DTL::Token *>       CROSS
%token	<DTL::Token *>       POSTINC
%token	<DTL::Token *>       RCURLY
%token	<DTL::Token *>       RPAREN
%token	<DTL::Token *>       SEMICOL
%token	<DTL::Token *>       STAR
%token  <DTL::Token *>       FOR
%token  <DTL::Token *>       OUT

%left CROSS
%left STAR 


%type <DTL::ExpNode*> factor
%type <DTL::ExpNode*> term
%type <DTL::ExpNode*> expr
%type <DTL::StmtNode*> unarystmt
%type <DTL::LocNode*> loc


%%
program: constdecls forstatement {
    
}
constdecls: constdecls constdecl
        {
            // return vector of const decls
        }
        | /* empty */ ;
constdecl: type loc ASSIGN INTLITERAL SEMICOL
        {

        }
forstatement: FOR LPAREN type loc ASSIGN INTLITERAL SEMICOL loc LESS factor SEMICOL unarystmt RPAREN LCURLY forstatement RCURLY
        {

        }
        | FOR LPAREN type loc ASSIGN INTLITERAL SEMICOL loc LESS factor SEMICOL unarystmt RPAREN LCURLY outstatements RCURLY
        {

        }
outstatements : outstatements outstatement
        {

        }
        | outstatement
        {

        }
outstatement : OUT ASSIGN expr SEMICOL
            {

            }

type : INT // we may add other types?
    {
        // need to add type nodes
    }
expr: expr CROSS term
    {
        const Position * p = new Position($1->pos(), $3->pos());
        $$ = new PlusNode(p, $1, $3);
    }
    | term
    {
        $$ = $1;
    }

unarystmt: loc POSTINC
        {
            const Position* p = new Position($1->pos(), $2->pos());
            $$ = new PostIncStmtNode(p, $1);
        }



term: term STAR factor
    {
        const Position * p = new Position($1->pos(), $3->pos());
        $$ = new TimesNode(p, $1, $3);
    }
    | factor 
    {
        $$ = $1;
    }
factor: INTLITERAL 
    {
        $$ = new IntLitNode($1->pos(), $1->num());
        printf("Intlitnode\n");
    }
    | loc 
    {
        $$ = $1;
    }
loc : ID 
    {
        $$ = new IDNode($1->pos(), $1->value());
    }


%%


void DTL::Parser::error(const std::string& msg) {

	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}