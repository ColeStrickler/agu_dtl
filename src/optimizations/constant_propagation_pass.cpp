#include "constant_propagation_pass.hpp"
#include "ast.hpp"

using namespace DTL;


void DTL::ConstantPropagationPass::AddConstant(IDNode *id, IntLitNode *node)
{
    auto constVal = node->GetVal();
    auto idVal = id->getName();
    m_ConstantValues.insert({idVal, constVal});
}

int DTL::ConstantPropagationPass::FindConstant(IDNode *id)
{
    auto idKey = id->getName();
    if (m_ConstantValues.find(idKey) != m_ConstantValues.end())
    {
        return m_ConstantValues.at(idKey);
    }
    return -1;
}
void DTL::ConstantPropagationPass::IncPropCount() 
{
    m_PropCount++;
}

int DTL::ConstantPropagationPass::GetPropCount() { return m_PropCount; }

ASTNode* DTL::ProgramNode::ConstPropagation(ConstantPropagationPass *prop_pass) {
    for (int i = 0; i < myStatements.size(); i++)
    {
        myStatements[i] = (StmtNode*)myStatements[i]->ConstPropagation(prop_pass);
    }
        
    return this;
}


ASTNode *
DTL::ConstDeclNode::ConstPropagation(ConstantPropagationPass *prop_pass) {
  if (GetOpt()) {
    prop_pass->AddConstant(myID, myVal);
  }
  return this;
}


ASTNode *DTL::ConstArrayDeclNode::ConstPropagation(ConstantPropagationPass *prop_pass) {
    return this;
}


ASTNode *DTL::PostIncStmtNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
  return this;
}


ASTNode *DTL::ForStmtNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
    
    auto cond = myCondExp->ConstPropagation(prop_pass);
    myCondExp = (ExpNode*)cond;


    for (int i = 0; i < myStatements.size(); i++)
        myStatements[i] = (StmtNode*)myStatements[i]->ConstPropagation(prop_pass);
    return this;

}


ASTNode* DTL::OutStmtNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
    auto exp = myExp->ConstPropagation(prop_pass);
    myExp = (ExpNode*)exp;
    return this;
}


ASTNode * DTL::TypeNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
  return this;
}


ASTNode *DTL::ArrayIndexNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
  return this;
}



ASTNode *DTL::IDNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {

    int id_val = prop_pass->FindConstant(this);
    if (id_val == -1) // this will be the case for IDs from the ForLoop variables
        return this;


    // i dont think we use position after parsing so this should be okay ? 
    prop_pass->IncPropCount();
    auto intNode = new IntLitNode(myPos, id_val); 
    delete this;
    return intNode;
}

ASTNode *DTL::UnaryExpNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
  return this;
}



ASTNode *DTL::BinaryExpNode::ConstPropagation(DTL::ConstantPropagationPass *prop_pass) {
    auto left = myExp1->ConstPropagation(prop_pass);
    myExp1 = (ExpNode*)left;
    
    auto right = myExp2->ConstPropagation(prop_pass);
    myExp2 = (ExpNode*)right;
    

    return this;
}


