#ifndef TYPES_HPP
#define TYPES_HPP
#include <string>
#include <vector>

namespace DTL {



class BasicType;
class ErrorType;

enum BaseType {
    INT, BOOL, VOID
};



class DataType{
public:
	virtual std::string getString() const = 0;
	virtual const BasicType * asBasic() const { return nullptr; }
	//virtual const FnType * asFn() const { return nullptr; }
	virtual const ErrorType * asError() const { return nullptr; }
	virtual bool isVoid() const { return false; }
	virtual bool isInt() const { return false; }
	virtual bool isBool() const { return false; }
	//virtual bool isString() const { return false; }
	//virtual bool isClass() const { return false; }
	//virtual bool isImmutable() const { return false; }
	//virtual bool validVarType() const = 0 ;
	//virtual size_t getSize() const = 0;
protected:
};




class ErrorType : public DataType{
public:
	static ErrorType * produce(){
		//Note: this static member will only ever be initialized
		// ONCE, no matter how many times the function is called.
		// That means there will only ever be 1 instance of errorType
		// in the entire codebase.
		static ErrorType * error = new ErrorType();
		return error;
	}
	virtual const ErrorType * asError() const override { return this; }
	virtual std::string getString() const override {
		return "ERROR";
	}
	//virtual bool validVarType() const override { return false; }
	//virtual size_t getSize() const override { return 0; }
private:
	ErrorType(){
		/* private constructor, can only
		be called from produce */
	}
	size_t line;
	size_t col;
};

//DataType subclass for all scalar types
class BasicType : public DataType{
public:
	static BasicType * VOID(){
		return produce(BaseType::VOID);
	}
	static BasicType * BOOL(){
		return produce(BaseType::BOOL);
	}
	static BasicType * INT(){
		return produce(BaseType::INT);
	}
	//static BasicType * STRING(){
	//	return produce(BaseType::STRING);
	//}

	//Create a scalar type. If that type already exists,
	// return the known instance of that type. Making sure
	// there is only 1 instance of a class for a given set
	// of fields is known as the "flyweight" design pattern
	// and ensures that the memory needs of a program are kept
	// down: rather than having a distinct type for every base
	// INT (for example), only one is constructed and kept in
	// the flyweights list. That type is then re-used anywhere
	// it's needed.

	//Note the use of the static function declaration, which
	// means that no instance of BasicType is needed to call
	// the function.
	static BasicType * produce(BaseType base){
		//Note the use of the static local variable, which
		//means that the flyweights variable persists between
		// multiple calls to this function (it is essentially
		// a global variable that can only be accessed
		// in this function).
		static std::vector<BasicType *> flyweights;
		for(BasicType * fly : flyweights){
			if (fly->getBaseType() == base){
				return fly;
			}
		}
		BasicType * newType = new BasicType(base);
		flyweights.push_back(newType);
		return newType;
	}
	const BasicType * asBasic() const override {
		return this;
	}
	BasicType * asBasic(){
		return this;
	}
	bool isInt() const override {
		return myBaseType == BaseType::INT;
	}
	bool isBool() const override {
		return myBaseType == BaseType::BOOL;
	}
	virtual bool isVoid() const override {
		return myBaseType == BaseType::VOID;
	}
	//virtual bool isString() const override {
	//	return myBaseType == BaseType::STRING;
	//}
	//virtual bool validVarType() const override {
	//	return !isVoid();
	//}
	virtual BaseType getBaseType() const { return myBaseType; }
	virtual std::string getString() const {return "BasicType";}


	//virtual size_t getSize() const override {
	//	switch (myBaseType) {
	//	case BaseType::BOOL: return 8;
	//	case BaseType::STRING: return 8;
	//	case BaseType::INT: return 8;
	//	case BaseType::VOID: return 8;
	//	}
	//	throw new InternalError("getSize of unknown type");
	//	/*
	//	if (isBool()){ return 8; }
	//	else if (isString()){ return 8; }
	//	else if (isVoid()){ return 8; }
	//	else if (isInt()){ return 8; }
	//	else if (isShort()){ return 1; }
	//	else { return 0; }
	//	*/
	//}
	//~BasicType() = default;  // declare virtual destructor
private:
	BasicType(BaseType base)
	: myBaseType(base){ }
	BaseType myBaseType;
};



}


#endif