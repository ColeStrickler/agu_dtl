#include "dead_code_elimination_pass.hpp"
#include "ast.hpp"

using namespace DTL;



void DTL::DeadCodeEliminationPass::AddConstDecl(ConstDeclNode *constDecl)
{
    m_UsedConstDecls.insert(constDecl->GetIDString());
}

void DTL::DeadCodeEliminationPass::AddConstArrayDecl(ConstArrayDeclNode *constArrayDecl) 
{
    m_UsedConstDecls.insert(constArrayDecl->GetIDString());
}

void DTL::DeadCodeEliminationPass::SetIsUsed(std::string id)
{
    // may have already been erased - so we check first
    if (m_UsedConstDecls.find(id) != m_UsedConstDecls.end())
        m_UsedConstDecls.erase(id);
}

bool DTL::DeadCodeEliminationPass::isInDeclMap(std::string id) {
    return m_UsedConstDecls.find(id) != m_UsedConstDecls.end();
}

ASTNode *DTL::ProgramNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    if (pass == 1)
    {
        std::vector<StmtNode*> newStmts;
        printf("\n\n\n\nstatements size!!!!!!!!1 %d\n\n\n\n", elim_pass->m_UsedConstDecls.size());
        for (int i = 0; i < myStatements.size(); i++) // 
        {
            auto stmt = myStatements[i];
            auto tag = stmt->getTag();
            if (tag == NODETAG::CONSTDECLNODE)
            {
                auto constdeclstmt = static_cast<ConstDeclNode*>(stmt);
                /*
                    If the constant is unused, it still exists in the m_UsedConstDecls set
                
                    We remove constant variables out of this set when their ID is used
                */

                if (elim_pass->isInDeclMap(constdeclstmt->GetIDString()))
                {
                    // not used
                    delete constdeclstmt;
                }
                else
                {
                    newStmts.push_back(constdeclstmt);
                }
            }
            else if (tag == NODETAG::CONSTARRAYDECLNODE)
            {

                auto constarraydeclstmt = static_cast<ConstArrayDeclNode*>(stmt);
                /*
                    If the constant is unused, it still exists in the m_UsedConstDecls set
                
                    We remove constant variables out of this set when their ID is used
                */

                if (elim_pass->isInDeclMap(constarraydeclstmt->GetIDString()))
                {
                    // not used
                    delete constarraydeclstmt;
                }
                else
                {
                    newStmts.push_back(constarraydeclstmt);
                }
            }
            else
            {
                newStmts.push_back(stmt); // handle for loop stmts
            }
        }

        myStatements = newStmts;
        return this;
    }
    else if (pass == 0)
    {
        for (auto& stmt: myStatements)
            stmt->DeadCodeElimination(elim_pass, pass);
        return this;
    }
    assert(false);
}

ASTNode *DTL::ConstDeclNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here

    elim_pass->AddConstDecl(this);
    return this;
}

ASTNode *DTL::ConstArrayDeclNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here

    elim_pass->AddConstArrayDecl(this);
    return this;
}


ASTNode *DTL::PostIncStmtNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
  assert(pass == 0); // pass 1 shouldn't reach here
  return this;
}

ASTNode *DTL::ForStmtNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here
    for (auto& stmt: myStatements)
        stmt->DeadCodeElimination(elim_pass, pass);
    return this;
}

ASTNode *DTL::OutStmtNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here
    myExp->DeadCodeElimination(elim_pass, pass);
    return this;
}

ASTNode *DTL::TypeNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(false); // never should get here, just need for inheritance
    return this;
}

ASTNode *DTL::ArrayIndexNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here
    auto idString = myID->getName();
    elim_pass->SetIsUsed(idString);
    return this;
}

ASTNode *DTL::IDNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here
    auto idString = getName();
    elim_pass->SetIsUsed(idString);
    return this;
}

ASTNode *DTL::UnaryExpNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(false); // should not ever reach here
    return this;
}

ASTNode *DTL::IntLitNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    return this;
}

ASTNode *DTL::BinaryExpNode::DeadCodeElimination(DTL::DeadCodeEliminationPass *elim_pass, int pass) {
    assert(pass == 0); // pass 1 shouldn't reach here
    myExp1->DeadCodeElimination(elim_pass, pass);
    myExp2->DeadCodeElimination(elim_pass, pass);
    return this;
}