#include "ast_transform_pass.hpp"
#include "resource_alloc_pass.hpp"
#include "ast.hpp"
using namespace DTL;

/*
    Since we level out the tree by mapping operands that aren't ready to be operated on yet
    to PlusNode(operand, 0), we can check if this is a pass through value
*/
bool DTL::PlusNode::isPassThrough()
{
    return IsPassThrough;
}


/*
    We need this information in order to balance out the tree
*/
int DTL::OutStmtNode::GetMaxDepth()
{
    maxDepth = myExp->GetMaxDepth();
    return maxDepth;
}


int DTL::IDNode::GetMaxDepth()
{
    return 0;
}

int DTL::ArrayIndexNode::GetMaxDepth()
{
    return 0;
}


int DTL::IntLitNode::GetMaxDepth()
{
    return 0;
}

int DTL::PostIncStmtNode::GetMaxDepth()
{
    assert(false); // should never hit since we only call beginning from OutStmtNode
    return 0;
}


int DTL::PlusNode::GetMaxDepth()
{
    return 1 + std::max(myExp1->GetMaxDepth(), myExp2->GetMaxDepth());
}


int DTL::TimesNode::GetMaxDepth()
{
    return 1 + std::max(myExp1->GetMaxDepth(), myExp2->GetMaxDepth());
}



int DTL::LessNode::GetMaxDepth()
{
    assert(false); // should never hit since we only call beginning from OutStmtNode
    return 0;//1 + std::max(myExp1->GetMaxDepth(), myExp2->GetMaxDepth());
}

int DTL::LessEqNode::GetMaxDepth()
{
    assert(false); // should never hit since we only call beginning from OutStmtNode
    return 0;//1 + std::max(myExp1->GetMaxDepth(), myExp2->GetMaxDepth());
}



ASTNode *DTL::ProgramNode::TransformPass(uint8_t opt_flags)
{
    for (int i = 0; i < myStatements.size(); i++)
    {
        auto stmt = myStatements[i]->TransformPass(opt_flags);
        myStatements[i] = static_cast<StmtNode*>(stmt);
    }

    return this;
}

ASTNode *DTL::ConstDeclNode::TransformPass(uint8_t opt_flags)
{
    return this;
}


ASTNode *DTL::ConstDeclNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    return this;
}


ASTNode *DTL::ConstArrayDeclNode::TransformPass(uint8_t opt_flags)
{
    return this;
}

ASTNode *DTL::ConstArrayDeclNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    return this;
}


std::string DTL::ConstDeclNode::GetIDString() const
{
    return myID->getName();
}

std::string DTL::ConstArrayDeclNode::GetIDString() const {
  return myID->getName();
}

ASTNode *DTL::PostIncStmtNode::TransformPass(uint8_t opt_flags)
{
    return this;
}

ASTNode *DTL::PostIncStmtNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    return this;
}



ASTNode *DTL::ForStmtNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
   int req_depth = 0;
    for (auto& stmt: myStatements)
        req_depth = std::max(req_depth, stmt->GetMaxDepth());



    for (int i = 0; i < myStatements.size(); i++)
    {
        auto stmt = myStatements[i]->TransformPass(0, req_depth, opt_flags);
        myStatements[i] = static_cast<StmtNode*>(stmt);
    }

    return this;
}


ASTNode *DTL::ForStmtNode::TransformPass(uint8_t opt_flags)
{
    int req_depth = 0;
    for (auto& stmt: myStatements)
        req_depth = std::max(req_depth, stmt->GetMaxDepth());



    for (int i = 0; i < myStatements.size(); i++)
    {
        auto stmt = myStatements[i]->TransformPass(0, req_depth, opt_flags);
        myStatements[i] = static_cast<StmtNode*>(stmt);
    }

    return this;
}


ASTNode *DTL::OutStmtNode::TransformPass(uint8_t opt_flags)
{
    // we shall call the other method
    return this;
}

ASTNode *DTL::OutStmtNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    /*
        We are requiring the AST leaves to be of uniform depth at the leaves
    */
    int depthRequired = GetMaxDepth();
    myExp->TransformPass(0, depthRequired, opt_flags);

    return this;
}

ASTNode *DTL::IDNode::TransformPass(uint8_t opt_flags)
{
    return this;
}


ASTNode *DTL::ArrayIndexNode::TransformPass(uint8_t opt_flags)
{
    return this;
}

ASTNode *DTL::ArrayIndexNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    if (currDepth == RequiredDepth)
    {
        return this;
    }
    else
    {
        printf("ArrayIndexNode::TransformPass\n");
        auto ilnode = new IntLitNode(pos(), 0);
        auto dummyAdd = new PlusNode(pos(), this, ilnode);
        dummyAdd->setPassThrough();
        dummyAdd = static_cast<PlusNode*>(dummyAdd->TransformPass(currDepth, RequiredDepth, opt_flags));
        return dummyAdd;
    }
}




ASTNode *DTL::IntLitNode::TransformPass(uint8_t opt_flags)
{
    return this;
}


ASTNode* DTL::IntLitNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{
    if (currDepth == RequiredDepth)
    {
        return this;
    }
    else
    {
        auto ilnode = new IntLitNode(pos(), 0);
        auto dummyAdd = new PlusNode(pos(), this, ilnode);
        printf("intlit dummy 0x%x\n", ilnode);
        dummyAdd->setPassThrough();
        dummyAdd = static_cast<PlusNode*>(dummyAdd->TransformPass(currDepth, RequiredDepth, opt_flags));
        return dummyAdd;
    }
}


ASTNode *DTL::PlusNode::TransformPass(uint8_t opt_flags)
{
    return this; // only worried about restructing branches from OutStmtNode
}

ASTNode *DTL::IDNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{

    if (currDepth == RequiredDepth)
    {
        return this;
    }
    else
    {
        printf("ID pushdown\n");
        auto ilnode = new IntLitNode(pos(), 0);
        auto dummyAdd = new PlusNode(pos(),  static_cast<ExpNode*>(this->TransformPass(currDepth+1, RequiredDepth, opt_flags)), ilnode);
        dummyAdd->setPassThrough();
       
        //dummyAdd = static_cast<PlusNode*>(dummyAdd->TransformPass(currDepth, RequiredDepth, opt_flags));
        return dummyAdd;
    }

}



ASTNode *DTL::PlusNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{   
    bool bDepth = currDepth + 1 < RequiredDepth;
    bool n1 = myExp1->getTag() == NODETAG::INTLITNODE || myExp1->getTag() == NODETAG::IDNODE;
    bool n2 = myExp2->getTag() == NODETAG::INTLITNODE || myExp2->getTag() == NODETAG::IDNODE;
    bool greedy_add = (opt_flags & DTL_OPT_ADDGREEDY != 0);
    // no need to push too many pass throughs too base level --> possibly do this optimization with times as well
    if (n1 && n2 && bDepth && greedy_add) {
        printf("PlusNode pushdown\n");
        auto ilnode = new IntLitNode(pos(), 0);
        auto dummyAdd = new PlusNode(pos(), myExp1, myExp2);
        myExp1 = dummyAdd;
        myExp2 = ilnode;
        setPassThrough();
        dummyAdd->TransformPass(currDepth+1, RequiredDepth, opt_flags);
        return this;
    }
    printf("plusnode walk\n");
    myExp1 = static_cast<ExpNode*>(myExp1->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    if (!isPassThrough()) // we do not need to push zeros to the bottom
        myExp2 = static_cast<ExpNode*>(myExp2->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    return this;
}

ASTNode *DTL::TimesNode::TransformPass(uint8_t opt_flags)
{
    return this; // only worried about restructing branches from OutStmtNode
}

ASTNode *DTL::TimesNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{   
    bool bDepth = currDepth + 1 < RequiredDepth;
    bool n1 = myExp1->getTag() == NODETAG::INTLITNODE || myExp1->getTag() == NODETAG::IDNODE;
    bool n2 = myExp2->getTag() == NODETAG::INTLITNODE || myExp2->getTag() == NODETAG::IDNODE;
    bool greedy_times = (opt_flags & DTL_OPT_MULTGREEDY != 0);

    // no need to push too many pass throughs too base level --> possibly do this optimization with times as well
    // this effectively schedules as many ready operations as possible
    if(n1 && n2 && bDepth && greedy_times)
    {
        auto ilnode = new IntLitNode(pos(), 0);
        auto dummyPlus = new PlusNode(pos(), (ExpNode*)this->TransformPass(currDepth+1, RequiredDepth, opt_flags), ilnode);
        dummyPlus->setPassThrough();
        printf("times dummy 0x%x\n", ilnode);
        //myExp1 = nullptr;
        //myExp2 = nullptr;
        ;
        return dummyPlus;
    }



    myExp1 = static_cast<ExpNode*>(myExp1->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    myExp2 = static_cast<ExpNode*>(myExp2->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    return this;
}


ASTNode *DTL::LessNode::TransformPass(uint8_t opt_flags)
{
    return this; // only worried about restructing branches from OutStmtNode
}

ASTNode *DTL::LessNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{   
    myExp1 = static_cast<ExpNode*>(myExp1->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    myExp2 = static_cast<ExpNode*>(myExp2->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    return this;
}


ASTNode *DTL::LessEqNode::TransformPass(uint8_t opt_flags)
{
    return this; // only worried about restructing branches from OutStmtNode
}

ASTNode *DTL::LessEqNode::TransformPass(int currDepth, int RequiredDepth, uint8_t opt_flags)
{   
    myExp1 = static_cast<ExpNode*>(myExp1->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    myExp2 = static_cast<ExpNode*>(myExp2->TransformPass(currDepth+1, RequiredDepth, opt_flags));
    return this;
}



ASTNode *DTL::TypeNode::TransformPass(uint8_t opt_flags) { return this; }