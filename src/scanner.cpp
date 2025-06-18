#include <fstream>
#include "scanner.hpp"

using namespace DTL;

using TokenKind =   DTL::Parser::token;
using Lexeme =      DTL::Parser::semantic_type;

void Scanner::outputTokens(std::ostream& outstream){
	Lexeme lastMatch;
	//Token * t = lex.as<Token *>();
	int tokenKind;
	while(true){
		tokenKind = this->yylex(&lastMatch);
		if (tokenKind == TokenKind::END){
			outstream << "EOF" 
			  << " [" << this->lineNum 
			  << "," << this->colNum << "]"
			  << std::endl;
			return;
		} else {
			//outstream << lex.as<Token *>()->toString()
			outstream << lastMatch.as<Token *>()->toString()
			  << std::endl;
		}
	}
}