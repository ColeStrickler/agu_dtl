#ifndef DTL_ERRORS_H
#define DTL_ERRORS_H

#define EXPAND2(x) #x
#define EXPAND1(x) EXPAND2(x)
#define CODELOC __FILE__ ":" EXPAND1(__LINE__) " - "
#define TODO(x) throw new ToDoError(CODELOC #x);

#include <iostream>
#include "position.hpp"

namespace DTL{



/* This class is used to encapsulate error messages that the 
   user of the compiler will see in cases where the spec wants 
   a specific output format. */
class Report{
public:
	static void fatal(
		const Position * pos,
		const char * msg
	){
		std::cerr << "FATAL " 
		<< pos->span()
		<< ": " 
		<< msg  << std::endl;
	}

	static void fatal(
		const Position * pos,
		const std::string msg
	){
		fatal(pos,msg.c_str());
	}
};



/* This class is used to denote a situation where the 
   compiler (or compiler-writer) made a mistake */
class InternalError{
public:
	InternalError(const char * msgIn) : myMsg(msgIn){}
	std::string msg(){ return myMsg; }
private:
	std::string myMsg;
};

/* This class is used to denote a situation where the 
   user of the compiler made a mistake not otherwise 
   specified in the error output specifications */
class UserError{
public:
	UserError(const char * msgIn) : myMsg(msgIn){}
	std::string msg(){ return myMsg; }
private:
	std::string myMsg;
};


/* Instances of this class are thrown to denote a situation where you
   (the student) probably need to fill in / change some functionality.
   Note that you may need to fill in / change functionality in 
   places where there is no ToDoError */
class ToDoError{
public:
	ToDoError(const char * msgIn) : myMsg(msgIn){}
	const char * msg(){ return myMsg; }
private:
	const char * myMsg;
};


class NameErr{
public:
static bool undeclID(const Position * pos){
	Report::fatal(pos, "Undeclared identifier");
	return false;
}
static bool badVarType(const Position * pos){
	Report::fatal(pos, "Invalid type in declaration");
	return false;
}
static bool multiDecl(const Position * pos){
	Report::fatal(pos, "Multiply declared identifier");
	return false;
}
};





}

#endif