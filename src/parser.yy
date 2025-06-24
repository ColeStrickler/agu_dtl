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



%%
program: constdecls forstatement {
    
}
constdecls: constdecls constdecl
        | /* empty */ ;
constdecl: INT ID ASSIGN INTLITERAL SEMICOL ;
forstatement: FOR LPAREN INT ID ASSIGN INTLITERAL SEMICOL ID LESS factor SEMICOL unaryexpr RPAREN LCURLY forstatement RCURLY
         | FOR LPAREN INT ID ASSIGN INTLITERAL SEMICOL ID LESS factor SEMICOL unaryexpr RPAREN LCURLY outstatements RCURLY;
outstatements : outstatements outstatement
        | outstatement;
outstatement : OUT ASSIGN expr SEMICOL;
expr: expr CROSS term
    | unaryexpr
    | term ;

unaryexpr: ID POSTINC;
term: term STAR factor
    | factor ;
factor: INTLITERAL 
    {
        $$ = new IntLitNode($1->pos(), $1->num());
        printf("Intlitnode\n");
    }
    | ID {
        $$ = new IDNode($1->pos(), $1->value());
        printf("IDNode %s\n", $1->value().c_str());
    };
%%


void DTL::Parser::error(const std::string& msg) {

	std::cout << msg << std::endl;
	std::cerr << "syntax error" << std::endl;
}