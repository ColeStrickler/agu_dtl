#ifndef AGU_TOKENS_H
#define AGU_TOKENS_H
#include <string>
#include "position.hpp"


namespace DTL {
// base class

class Token{
public:
	Token(Position * pos, int kindIn);
	virtual std::string toString();
	size_t line() const;
	size_t col() const;
	int kind() const;
	const Position * pos() const;
protected:
	const Position * myPos;
private:
	const int myKind;
};

class IDToken : public Token{
public:
	IDToken(Position * posIn, std::string valIn);
	const std::string value() const;
	virtual std::string toString() override;
private:
	const std::string myValue;
	
};


class IntLitToken : public Token{
public:
	IntLitToken(Position * posIn, int numIn);
	virtual std::string toString() override;
	int num() const;
private:
	const int myNum;
};
}
#endif