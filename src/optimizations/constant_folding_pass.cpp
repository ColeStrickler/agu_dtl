#include "constant_folding_pass.hpp"
#include "ast.hpp"

using namespace DTL;

ASTNode *DTL::ProgramNode::ConstFold(ConstantFoldPass* foldpass) 
{
    for (int i = 0; i < myStatements.size(); i++)
    {
        printf("%d\n", i);
        myStatements[i] = (StmtNode*)myStatements[i]->ConstFold(foldpass);
    }
        
    return this;
}

ASTNode *DTL::ConstDeclNode::ConstFold(ConstantFoldPass* foldpass) // at the moment we can just leave constants as is 
{
    return this;
}

ASTNode *DTL::ConstArrayDeclNode::ConstFold(ConstantFoldPass* foldpass) 
{
    return this;
}

ASTNode *DTL::PostIncStmtNode::ConstFold(ConstantFoldPass* foldpass) 
{
    return this;
}


ASTNode *DTL::ForStmtNode::ConstFold(ConstantFoldPass* foldpass) 
{
    auto cond = myCondExp->ConstFold(foldpass); // this will allow for statements like i < 400 * 300
    myCondExp = (ExpNode*)cond;

    for (int i = 0; i < myStatements.size(); i++)
        myStatements[i] = (StmtNode*)myStatements[i]->ConstFold(foldpass);
    return this;
}

ASTNode *DTL::OutStmtNode::ConstFold(ConstantFoldPass* foldpass) 
{
    auto exp = myExp->ConstFold(foldpass);
    myExp = (ExpNode*)exp;
    return this;
}

ASTNode *DTL::TypeNode::ConstFold(ConstantFoldPass* foldpass) 
{
    return this;
}

ASTNode *DTL::ArrayIndexNode::ConstFold(ConstantFoldPass* foldpass)
{
    return this;
}


ASTNode *DTL::IDNode::ConstFold(ConstantFoldPass* foldpass)
{
    return this;
}


ASTNode *DTL::UnaryExpNode::ConstFold(ConstantFoldPass* foldpass)
{
    /*
        The only unaryexpnode currently allowed is the PostIncStmtNode

        Which will not be optimized here
    */
   return this;
}

ASTNode *DTL::IntLitNode::ConstFold(ConstantFoldPass* foldpass) 
{
    return this;
}

ASTNode *DTL::PlusNode::ConstFold(ConstantFoldPass* foldpass) 
{
    auto left = myExp1->ConstFold(foldpass);
    auto right = myExp2->ConstFold(foldpass);

    if (left->getTag() == DTL::NODETAG::INTLITNODE && right->getTag() == DTL::NODETAG::INTLITNODE)
    {
        foldpass->IncFoldCount();
        auto lnode = reinterpret_cast<IntLitNode*>(left);
        auto rnode = reinterpret_cast<IntLitNode*>(right);

        auto lnode_val = lnode->GetVal();
        auto rnode_val = rnode->GetVal();

        /*
            These nodes have no children, and one of them can be reused
        */
       lnode->myNum = lnode_val + rnode_val;
       delete rnode; // delete rnode
        /*
            Delete current node

            This should be fine and completely safe since we are only working on references internal to the AST
       */
       delete this; 
       return lnode;     
    }



    myExp1 = (ExpNode*)left;
    myExp2 = (ExpNode*)right;
    return this;
}

ASTNode *DTL::TimesNode::ConstFold(ConstantFoldPass* foldpass)
{
    auto left = myExp1->ConstFold(foldpass);
    auto right = myExp2->ConstFold(foldpass);

    printf("tag1 %d, tag2 %d\n", left->getTag(), right->getTag());

    if (left->getTag() == DTL::NODETAG::INTLITNODE && right->getTag() == DTL::NODETAG::INTLITNODE)
    {
        foldpass->IncFoldCount();
        auto lnode = reinterpret_cast<IntLitNode*>(left);
        auto rnode = reinterpret_cast<IntLitNode*>(right);

        auto lnode_val = lnode->GetVal();
        auto rnode_val = rnode->GetVal();

        /*
            These nodes have no children, and one of them can be reused
        */
       lnode->myNum = lnode_val * rnode_val;
       delete rnode; // delete rnode
       /*
            Delete current node

            This should be fine and completely safe since we are only working on references internal to the AST
       */
       delete this; 
       return lnode;     
    }



    myExp1 = (ExpNode*)left;
    myExp2 = (ExpNode*)right;
    return this;
}


ASTNode *DTL::LessNode::ConstFold(ConstantFoldPass* foldpass) 
{
    auto left = myExp1->ConstFold(foldpass);
    auto right = myExp2->ConstFold(foldpass);
    myExp1 = (ExpNode*)left;
    myExp2 = (ExpNode*)right;
    return this;
}

ASTNode *DTL::LessEqNode::ConstFold(ConstantFoldPass* foldpass)
{
    auto left = myExp1->ConstFold(foldpass);
    auto right = myExp2->ConstFold(foldpass);
    myExp1 = (ExpNode*)left;
    myExp2 = (ExpNode*)right;
    return this;
}

void DTL::ConstantFoldPass::IncFoldCount()
{
    printf("IncFold!\n");
    m_FoldCount++;
}
