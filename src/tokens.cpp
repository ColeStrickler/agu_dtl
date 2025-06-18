#include "tokens.hpp" // Get the class declarations
#include "frontend.hh" // Get the TokenKind definitions

namespace DTL{

using TokenKind = DTL::Parser::token;
using Lexeme = DTL::Parser::semantic_type;

static std::string tokenKindString(int tokKind){
	switch(tokKind){
		case TokenKind::ASSIGN: return "ASSIGN";
		case TokenKind::CROSS: return "CROSS";
		case TokenKind::END: return "EOF";
		case TokenKind::ID: return "ID";
		case TokenKind::INT: return "INT";
		case TokenKind::INTLITERAL: return "INTLITERAL";
		case TokenKind::LCURLY: return "LCURLY";
		case TokenKind::LPAREN: return "LPAREN";
		case TokenKind::POSTINC: return "POSTINC";
		case TokenKind::RCURLY: return "RCURLY";
		case TokenKind::RPAREN: return "RPAREN";
		case TokenKind::SEMICOL: return "SEMICOL";
		case TokenKind::STAR: return "STAR";
        case TokenKind::FOR: return "FOR";
        case TokenKind::LESS: return "LESS";
        case TokenKind::OUT: return "OUT";
		default:
			return "OTHER"; //This should never happen
	}
}

Token::Token(Position * posIn, int kindIn)
  : myPos(posIn), myKind(kindIn){
}

std::string Token::toString(){
	return tokenKindString(kind())
	+ " " + myPos->begin();
}

int Token::kind() const { 
	return this->myKind; 
}

const Position * Token::pos() const {
	return myPos;
}

IDToken::IDToken(Position * posIn, std::string vIn)
  : Token(posIn, TokenKind::ID), myValue(vIn){ 
}

std::string IDToken::toString(){
	return tokenKindString(kind()) + ":"
	+ myValue + " " + myPos->begin();
}

const std::string IDToken::value() const { 
	return this->myValue; 
}

IntLitToken::IntLitToken(Position * pos, int numIn)
  : Token(pos, TokenKind::INTLITERAL), myNum(numIn){}

std::string IntLitToken::toString(){
	return tokenKindString(kind()) + ":"
	+ std::to_string(this->myNum) + " "
	+ myPos->begin();
}

int IntLitToken::num() const {
	return this->myNum;
}

} 