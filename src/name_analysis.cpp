#include "name_analysis.hpp"
#include "errors.hpp"
#include "ast.hpp"


using namespace DTL;



bool ProgramNode::nameAnalysis(SymbolTable *symTab)
{
   
    symTab->enterScope();
    for (auto& stmt: myStatements)
    {
        if (!stmt->nameAnalysis(symTab))
            return false;
    }
    symTab->leaveScope();
    return true;
}


bool ForStmtNode::nameAnalysis(SymbolTable *symTab)
{
    symTab->enterScope();
    auto init =  myInit->nameAnalysis(symTab);

    /*
        We need to impose the constraint of making sure the init variable is the same
        as the left hand side of the condition statement
    */


    auto cond = myCondExp->nameAnalysis(symTab);
    assert(cond);
    bool outer = init && cond && myUpdateStmt->nameAnalysis(symTab);
    if (!outer)
        return false;
    for (auto& stmt: myStatements)
    {
        if (!stmt->nameAnalysis(symTab))
            return false;
    }

    symTab->leaveScope();
    return true;
}


/*
    For now this is the only way we can declare variables
*/
bool ConstDeclNode::nameAnalysis(SymbolTable* symTab)
{
    auto name = myID->getName();
    auto type = myType->getType();
    if (symTab->clash(name))
        return NameErr::multiDecl(myID->pos());

    if (type->asError() || type->isVoid())
        return NameErr::badVarType(myID->pos());


    auto sym = new VarSymbol(name, type);
    myID->attachSymbol(sym);
    symTab->insert(sym);
    return myVal->nameAnalysis(symTab);
}



bool DTL::ConstArrayDeclNode::nameAnalysis(SymbolTable *symTab)
{
    
    auto name = myID->getName();
    auto type = BasicType::produce(BaseType::INTARRAY);
    if (symTab->clash(name))
        return NameErr::multiDecl(myID->pos());

    if (type->asError() || type->isVoid() || type->isInt())
        return NameErr::badVarType(myID->pos());


    auto sym = new VarSymbol(name, type); 
    myID->attachSymbol(sym);
    symTab->insert(sym);
    return true;
}


bool BinaryExpNode::nameAnalysis(SymbolTable* symTab)
{
    return myExp1->nameAnalysis(symTab) && myExp2->nameAnalysis(symTab);
}

bool UnaryExpNode::nameAnalysis(SymbolTable* symTab)
{
    return myExp->nameAnalysis(symTab);
}

bool PostIncStmtNode::nameAnalysis(SymbolTable* symTab)
{
    return myLoc->nameAnalysis(symTab);
}


bool OutStmtNode::nameAnalysis(SymbolTable* symTab)
{
    return myExp->nameAnalysis(symTab);
}

bool DTL::ArrayIndexNode::nameAnalysis(SymbolTable *symTab)
{
    
    return myID->nameAnalysis(symTab) && myIndexVar->nameAnalysis(symTab);
}



bool IDNode::nameAnalysis(SymbolTable* symTab)
{
    std::string myName = this->getName();
	SemSymbol * sym = symTab->find(myName);
	if (sym == nullptr){
        printf("false\n");
		return NameErr::undeclID(pos());
	}
	this->attachSymbol(sym);
	return true;
}





