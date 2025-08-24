#include "ast.hpp"
#include "types.hpp"
#include "type_analysis.hpp"
#include "name_analysis.hpp"

using namespace DTL;
static const DataType * checkAssign(ExpNode * myDst, ExpNode * mySrc, TypeAnalysis * typing);

TypeAnalysis * TypeAnalysis::build(NameAnalysis * nameAnalysis){
	TypeAnalysis * typeAnalysis = new TypeAnalysis();
	auto ast = nameAnalysis->ast;
	typeAnalysis->ast = ast;

	
	ast->typeAnalysis(typeAnalysis);
	if (typeAnalysis->hasError){
		return nullptr;
	}
	
	return typeAnalysis;

}


void DTL::ProgramNode::typeAnalysis(TypeAnalysis *ta)
{
    for (auto& stmt: myStatements)
        stmt->typeAnalysis(ta);
}


void DTL::ForStmtNode::typeAnalysis(TypeAnalysis* ta)
{
	
	myInit->typeAnalysis(ta);
	myCondExp->typeAnalysis(ta);
	myUpdateStmt->typeAnalysis(ta);
	bool ok = true;
	

	for (auto& stmt: myStatements)
	{
		stmt->typeAnalysis(ta);
		auto res = ta->nodeType(stmt);
		if (res->asError())
		{
			ta->nodeType(this, ErrorType::produce());
			ok = false;
		}
	}


	/*
		We enforce very rigid format for the ForStmtNode to ensure that the user's
		specification is able to mapped successfully onto the hardware
	*/
	auto initTag = myInit->getTag();
	auto condTag = myCondExp->getTag();
	auto updateTag = myUpdateStmt->getTag();

	if (initTag != NODETAG::CONSTDECLNODE)
	{
		ta->errForLoopInit(myInit->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (condTag != NODETAG::LESSEQNODE && condTag != NODETAG::LESSNODE)
	{
		ta->errCond(myInit->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (updateTag != NODETAG::POSTINCNODE)
	{
		ta->errForLoopUpdate(myUpdateStmt->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	auto initType = ta->nodeType(myInit);
	auto condType = ta->nodeType(myCondExp);
	auto updateType = ta->nodeType(myUpdateStmt);


	if (!initType->isVoid())
	{
		ta->errForLoopInit(myInit->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!condType->isBool())
	{
		ta->errCond(myCondExp->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!updateType->isVoid())
	{
		ta->errForLoopUpdate(myUpdateStmt->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}


	if (ok)
		ta->nodeType(this, BasicType::VOID());
}

void DTL::PostIncStmtNode::typeAnalysis(TypeAnalysis *ta)
{
	ta->nodeType(this, BasicType::VOID());
}



void DTL::ConstArrayDeclNode::typeAnalysis(TypeAnalysis * ta)
{
		
	/*
		Maybe add some more to this later
	*/
	ta->nodeType(this, BasicType::INTARRAY());

	
	return;	
}



void DTL::ConstDeclNode::typeAnalysis(TypeAnalysis* ta)
{
	
	auto res = checkAssign(myID, myVal, ta);
	if (!res)
	{
		// propagate error
		ta->nodeType(this, ErrorType::produce());
	}
	else if (res->asError())
	{
		ta->errAssignOpr(this->pos());
		ta->nodeType(this, ErrorType::produce());
	}
	else
	{
		ta->nodeType(this, res);
	}
	return;	
}

void DTL::OutStmtNode::typeAnalysis(TypeAnalysis *ta)
{
	myExp->typeAnalysis(ta);

	auto type = ta->nodeType(myExp);
	if (!type->isInt())
	{
		ta->errAssignOpr(this->pos());
		ta->nodeType(this, ErrorType::produce());
	}

	ta->nodeType(this, BasicType::VOID());
}

void DTL::LessNode::typeAnalysis(TypeAnalysis* ta)
{
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	auto t1 = ta->nodeType(myExp1);
	auto t2 = ta->nodeType(myExp2);

	bool ok = true;
	
	if (!t1->isInt())
	{
		ta->errRelOpd(myExp1->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!t2->isInt())
	{
		ta->errRelOpd(myExp2->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (ok)
		ta->nodeType(this, BasicType::BOOL());
}



void DTL::LessEqNode::typeAnalysis(TypeAnalysis *ta)
{
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	auto t1 = ta->nodeType(myExp1);
	auto t2 = ta->nodeType(myExp2);

	bool ok = true;
	
	if (!t1->isInt())
	{
		ta->errRelOpd(myExp1->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!t2->isInt())
	{
		ta->errRelOpd(myExp2->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (ok)
		ta->nodeType(this, BasicType::BOOL());
}


void DTL::PlusNode::typeAnalysis(TypeAnalysis* ta)
{
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	auto t1 = ta->nodeType(myExp1);
	auto t2 = ta->nodeType(myExp2);

	bool ok = true;
	
	if (!t1->isInt())
	{
		ta->errMathOpd(myExp1->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!t2->isInt())
	{
		ta->errMathOpd(myExp2->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (ok)
		ta->nodeType(this, BasicType::INT());
}


void DTL::TimesNode::typeAnalysis(TypeAnalysis* ta)
{
	myExp1->typeAnalysis(ta);
	myExp2->typeAnalysis(ta);

	auto t1 = ta->nodeType(myExp1);
	auto t2 = ta->nodeType(myExp2);

	bool ok = true;
	
	if (!t1->isInt())
	{
		ta->errMathOpd(myExp1->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (!t2->isInt())
	{
		ta->errMathOpd(myExp2->pos());
		ta->nodeType(this, ErrorType::produce());
		ok = false;
	}

	if (ok)
		ta->nodeType(this, BasicType::INT());


}


void DTL::IntLitNode::typeAnalysis(TypeAnalysis* ta)
{
	ta->nodeType(this, BasicType::INT());
}



void DTL::IDNode::typeAnalysis(TypeAnalysis* ta)
{
	assert(getSymbol() != nullptr);
	const DataType * type = getSymbol()->getDataType();
	ta->nodeType(this, type);
}

void DTL::ArrayIndexNode::typeAnalysis(TypeAnalysis *ta)
{
	assert(myID->getSymbol() != nullptr);
	assert(myIndexVar->getSymbol() != nullptr);
	auto idType = myID->getSymbol()->getDataType();
	auto indexType = myIndexVar->getSymbol()->getDataType();
	if (!idType->isIntArray())
	{
		ta->errArrIndex(myID->pos());
		ta->nodeType(this, ErrorType::produce());
	}
	else if (!indexType->isInt())
	{
		ta->errArrIndex(myIndexVar->pos());
		ta->nodeType(this, ErrorType::produce());
	}
	else
	{
		// this will be interpreted as an
		ta->nodeType(this, BasicType::produce(BaseType::INT));
	}
}



static bool validAssignOpd(const DataType * type){
	if (type->isBool() || type->isInt() ){
		return true;
	}
	if (type->asError()){
		return true;
	}
	return false;
}

static bool type_isError(const DataType * type){
	return type != nullptr && type->asError();
}


static const DataType * checkAssign(ExpNode * myDst, ExpNode * mySrc, TypeAnalysis * typing){
	myDst->typeAnalysis(typing);
	mySrc->typeAnalysis(typing);
	const DataType * dstType = typing->nodeType(myDst);
	const DataType * srcType = typing->nodeType(mySrc);

	bool validOperands = true;
	bool knownError = type_isError(dstType) || type_isError(srcType);
	if (!validAssignOpd(dstType)){
		typing->errAssignOpd(myDst->pos());
		validOperands = false;
	}
	if (!validAssignOpd(srcType)){
		typing->errAssignOpd(mySrc->pos());
		validOperands = false;
	}
	if (!validOperands || knownError){
		//Error type, but due to propagation
		return nullptr; 
	}

	if (dstType == srcType){
		bool isError = false;
		//if (dstType->asFn()){
		//	typing->errAssignOpd(myDst->pos());
		//	typing->errAssignOpd(mySrc->pos());
		//}
		//else {
		//	return BasicType::VOID();
		//}
		return BasicType::VOID();
	}

	return ErrorType::produce();
}



