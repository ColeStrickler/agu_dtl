%{
#include <string>
#include <limits.h>

/* Get our custom yyFlexScanner subclass */
#include "scanner.hpp"
#undef YY_DECL
#define YY_DECL int DTL::Scanner::yylex(DTL::Parser::semantic_type * const lval)

using TokenKind = DTL::Parser::token;

/* define yyterminate as returning an EOF token (instead of NULL) */
#define yyterminate() return ( TokenKind::END )

/* exclude unistd.h for Visual Studio compatibility. */
#define YY_NO_UNISTD_H

using TokenKind = DTL::Parser::token;
#define EXIT_ON_ERR 0

%}

/*%option debug*/
%option nodefault
%option yyclass="DTL::Scanner"
%option noyywrap
%option c++

DIGIT [0-9]
WHITESPACE [\040\t]
LETTER [a-zA-Z]
ESCAPEE [nt'"\\]

 /* */ 
BADESC [\\][^nt"\\\n]
STRELT (\\[nt"\\])|([^\\\n"])

%%
%{
	this->yylval = lval;
%}
out         {return makeBareToken(TokenKind::OUT); }
for         {return makeBareToken(TokenKind::FOR); }
int 	    { return makeBareToken(TokenKind::INT); }
"="         { return makeBareToken(TokenKind::ASSIGN); }
"+"         { return makeBareToken(TokenKind::CROSS); }
"{"	    { return makeBareToken(TokenKind::LCURLY); }
"<"	    { return makeBareToken(TokenKind::LESS); }
"("	    { return makeBareToken(TokenKind::LPAREN); }
"++"	    { return makeBareToken(TokenKind::POSTINC); }
"}"	    { return makeBareToken(TokenKind::RCURLY); }
")"	    { return makeBareToken(TokenKind::RPAREN); }
";"	    { return makeBareToken(TokenKind::SEMICOL); }
"*"	    { return makeBareToken(TokenKind::STAR); }
({LETTER}|_)({LETTER}|{DIGIT}|_)* { 
			  Position * pos = new Position(lineNum, colNum,
				lineNum, colNum + yyleng);
		            yylval->emplace<DTL::Token *>( 
		            new IDToken(pos, yytext));
		            colNum += yyleng;
		            return TokenKind::ID;
}

{DIGIT}+	    {
					  double asDouble = std::stod(yytext);
			          int intVal = atoi(yytext);
			          bool overflow = false;
			          if (asDouble > INT_MAX){ overflow = true; }

								std::string str = yytext;
				  			std::string suffix = "";
				  			for(int i = 0 ; i < str.length(); i++){
									if (str[i] != '0'){
										suffix = str.substr(i, std::string::npos);
										break;
									}
				  			}
			          if (suffix.length() > 10){ overflow = true; }

			          if (overflow){
										Position pos(lineNum,colNum,lineNum,colNum+yyleng);
				            errIntOverflow(&pos);
					    intVal = 0;
			          }
				  			Position * pos = new Position(lineNum, colNum,
									lineNum, colNum + yyleng);
			          yylval->emplace<DTL::Token *>( 
			              new IntLitToken(pos, intVal));
			          colNum += yyleng;
			          return TokenKind::INTLITERAL; 
}



\n|(\r\n)     { lineNum++; colNum = 1; }


[ \t]+	      { colNum += yyleng; }

.	          { 
		
		    Position pos(lineNum,colNum,lineNum,colNum+yyleng);
		    errIllegal(&pos, yytext);
		    #if EXIT_ON_ERR
		    exit(1);
		    #endif
	            this->colNum += yyleng;
}
%%