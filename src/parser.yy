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

%left LESS
%left CROSS
%left STAR 

%type <DTL::ProgramNode*> program
%type <DTL::ExpNode*> factor
%type <DTL::ExpNode*> term
%type <DTL::ExpNode*> expr
%type <DTL::StmtNode*> unarystmt
%type <DTL::StmtNode*> constdecl
%type <DTL::ForStmtNode*> forstatement
%type <std::vector<DTL::StmtNode*>> constdecls
%type <std::vector<DTL::StmtNode*>> outstatements
%type <DTL::StmtNode*> outstatement
%type <DTL::IDNode*> loc
%type <DTL::TypeNode*> type
%type <DTL::IntLitNode*> intlit



%%
program: constdecls forstatement 
        {
            $1.push_back($2);
            *root = new ProgramNode($1);
            $$ = *root;
        }
constdecls: constdecls constdecl
        {
            $1.push_back($2);
            $$ = $1;
        }
        | /* empty */ 
        {
            auto ret = std::vector<DTL::StmtNode*>();
            $$ = ret;
        }
constdecl: type loc ASSIGN intlit SEMICOL
        {
            const Position * p = new Position($1->pos(), $5->pos());
            $$ = new ConstDeclNode(p, $1, $2, $4);
        }
forstatement: FOR LPAREN constdecl expr SEMICOL unarystmt RPAREN LCURLY forstatement RCURLY
        {
            const Position * p = new Position($1->pos(), $10->pos());
            std::vector<StmtNode*> stmt_vec;
            stmt_vec.push_back($9);
            $$ = new ForStmtNode(p, $3, $4, $6, stmt_vec);

        }
        | FOR LPAREN constdecl expr SEMICOL unarystmt RPAREN LCURLY outstatements RCURLY
        {
            const Position * p = new Position($1->pos(), $10->pos());
            $$ = new ForStmtNode(p, $3, $4, $6, $9);
        }
outstatements : outstatements outstatement
        {
            $1.push_back($2);
            $$ = $1;
        }
        | outstatement
        {
            std::vector<StmtNode*> stmt_vec;
            stmt_vec.push_back($1);
            $$ = stmt_vec;
        }
outstatement : OUT ASSIGN expr SEMICOL
            {
                const Position * p = new Position($1->pos(), $4->pos());
                $$ = new OutStmtNode(p, $3);
            }

type : INT // we may add other types?
    {
        $$ = new IntTypeNode($1->pos());
    }
expr: expr CROSS expr
    {
        const Position * p = new Position($1->pos(), $3->pos());
        $$ = new PlusNode(p, $1, $3);
    }
    | expr LESS expr
    {
        const Position * p = new Position($1->pos(), $3->pos());
        $$ = new LessNode(p, $1, $3);
    }
    | expr STAR expr
    {
        const Position * p = new Position($1->pos(), $3->pos());
        $$ = new TimesNode(p, $1, $3);
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



term: factor 
    {
        $$ = $1;
    }
    | LPAREN expr RPAREN
    {
        $$ = $2;
    }
factor: intlit
    {
        $$ = $1;
    }
    | loc 
    {
        $$ = $1;
    }
intlit : INTLITERAL
    {
        $$ = new IntLitNode($1->pos(), $1->num());
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