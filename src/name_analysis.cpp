#include "name_analysis.hpp"
#include "errors.hpp"


using namespace DTL;



bool ProgramNode::nameAnalysis(SymbolTable *symTab)
{
    symTab->enterScope();
    printf("ProgramNode::nameAnalysis()\n");
    for (auto& stmt: myStatements)
    {
        if (!stmt->nameAnalysis(symTab))
            return false;
    }
    symTab->leaveScope();
    return true;
}

bool ForStmtNode::nameAnalysis(SymbolTable* symTab)
{
    printf("ForStmtNode::nameAnalysis()\n");
    symTab->enterScope();
    auto init =  myInit->nameAnalysis(symTab);
    printf("init ok\n");

    auto cond = myCondExp->nameAnalysis(symTab);
    assert(cond);
    printf("cond ok\n");
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

bool ConstDeclNode::nameAnalysis(SymbolTable* symTab)
{
    printf("ConstDeclNode::nameAnalysis()\n");
    auto name = myID->getName();
    auto type = myType->getType();
    if (symTab->clash(name))
        return NameErr::multiDecl(myID->pos());

    if (type->asError() || type->isVoid())
        return NameErr::badVarType(myID->pos());

    symTab->insert(new VarSymbol(name, type));
    return myVal->nameAnalysis(symTab);
}

bool BinaryExpNode::nameAnalysis(SymbolTable* symTab)
{
    printf("BinaryExpNode::nameAnalysis()\n");
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




bool IDNode::nameAnalysis(SymbolTable* symTab)
{
    printf("IDNode::nameAnalysis()\n");
    std::string myName = this->getName();
	SemSymbol * sym = symTab->find(myName);
	if (sym == nullptr){
        printf("false\n");
		return NameErr::undeclID(pos());
	}
	this->attachSymbol(sym);
    printf("Done IDNode::nameAnalysis()\n");
	return true;
}





