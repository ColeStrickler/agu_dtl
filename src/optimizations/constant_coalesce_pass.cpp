#include "constant_coalesce_pass.hpp"
#include "ast.hpp"

using namespace DTL;


bool DTL::ConstantCoalescePass::HasCoalesceableNodes() 
{
    for (auto& e: m_CoalesceCollectionMap)
    {
        if (e.second.size() > 1) // more than one constant shares a value -> can be coalesced
            return true;
    }
    return false;
}


bool DTL::ConstantCoalescePass::ToCoalesce(int val) {
    if (m_CoalesceCollectionMap.find(val) != m_CoalesceCollectionMap.end()) 
    {
        return m_CoalesceCollectionMap.at(val).size() > 1;
    }   
    return false;
}


void DTL::ConstantCoalescePass::AddIDMapping(int val, std::string id) 
{
    m_CoalesceNewIDMap.insert({val, id});
}

std::string DTL::ConstantCoalescePass::GetIDMapping(int val) {
    /*
        THIS SHOULD ALWAYS HIT, WE HAVE A PROBLEM IF IT DOESNT

        SHOULD BE MAPPED INSIDE OF ProgramNode->CoalesceConst()
    */
    assert(m_CoalesceNewIDMap.find(val) != m_CoalesceNewIDMap.end());
    return m_CoalesceNewIDMap.at(val);
}

void DTL::ConstantCoalescePass::AddToCollectionMap(int val, ASTNode *node)
{
    if (m_CoalesceCollectionMap.find(val) != m_CoalesceCollectionMap.end())
    {
        m_CoalesceCollectionMap[val].insert(node);
    }
    else
    {
        m_CoalesceCollectionMap[val] = std::unordered_set<ASTNode*>();
        m_CoalesceCollectionMap[val].insert(node);
    }
}

std::string DTL::ConstantCoalescePass::GenIDString() 
{
    return "constVarID" + std::to_string(m_IDTicker++);
}

ASTNode *DTL::ProgramNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass,int pass) {

    
    if (pass == 0)
    {
        /*
            On pass 1 we just collect data
        */
        for (auto& stmt: myStatements)
            stmt->ConstCoalesce(coalesce_pass, pass);

    }
    else if (pass == 1)
    {
        /*
            On pass 2 we coalesce nodes
        */
    
        if (coalesce_pass->HasCoalesceableNodes())
        {
            

            /*
                Now we create new coalesced constant mappings
            */
            std::vector<StmtNode*> newStatements;
            for (auto& e: coalesce_pass->m_CoalesceCollectionMap)
            {
                if (e.second.size() > 1)
                {
                    std::string autoGenIDString = coalesce_pass->GenIDString();
                    coalesce_pass->AddIDMapping(e.first, autoGenIDString);
                    auto intlit = new IntLitNode(new Position(0, 0, 0, 0), e.first);
                    auto type = new IntTypeNode(new Position(0, 0, 0, 0));
                    auto genID = new IDNode(new Position(0, 0, 0, 0), autoGenIDString);
                    auto constNode = new ConstDeclNode(new Position(0, 0, 0, 0), type, genID, intlit);
                    newStatements.push_back(constNode);
                }
                
            }


            for (int i = 0; i < myStatements.size(); i++)
            {
                auto stmt = (StmtNode*)myStatements[i]->ConstCoalesce(coalesce_pass, pass);
                if (stmt != nullptr)
                    newStatements.push_back(stmt);
            }

            /*
                We replace old statements with the new ones which include coalescings
            */
            myStatements = newStatements;
          
        }
    
    }


    return this;

}


ASTNode *DTL::ConstDeclNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) 
{


  int val = myVal->GetVal();

  if (pass == 0)
  {
        coalesce_pass->AddToCollectionMap(val, this);
        return this;
  }
  else if (pass == 1)
  {
        if (coalesce_pass->ToCoalesce(val))
        {
            delete this;
            return nullptr;
        }    
  }

    

 return this;
}


ASTNode *DTL::ConstArrayDeclNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
  return this;
}


ASTNode *DTL::PostIncStmtNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
  return this;
}


ASTNode *DTL::ForStmtNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) 
{
    for (int i = 0; i < myStatements.size(); i++)
    {
        myStatements[i] = (StmtNode*)myStatements[i]->ConstCoalesce(coalesce_pass, pass);
    }
    return this;
}


ASTNode *DTL::OutStmtNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
    auto exp = myExp->ConstCoalesce(coalesce_pass, pass);
    myExp = (ExpNode*)exp;
    return this;
}


ASTNode *DTL::ArrayIndexNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
  return this;
}


ASTNode *DTL::BinaryExpNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
        auto left = myExp1->ConstCoalesce(coalesce_pass, pass);
    myExp1 = (ExpNode*)left;
    
    auto right = myExp2->ConstCoalesce(coalesce_pass, pass);
    myExp2 = (ExpNode*)right;
    

    return this;
}

ASTNode *DTL::UnaryExpNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
  return this;
}



ASTNode *DTL::TypeNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
  return this;
}


ASTNode *DTL::IDNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
    if (pass == 1)
    {
        return this;
    }
    else if (pass == 0)
    {
        /*
            THIS IS EXPECTING WE ALREADY PROPAGATED THE CONSTANTS
        */
        return this;
    }
    assert(false); // only should be 1 or 0
}


ASTNode *DTL::IntLitNode::ConstCoalesce(DTL::ConstantCoalescePass *coalesce_pass, int pass) {
    if (pass == 1)
    {
        if (coalesce_pass->ToCoalesce(myNum))
        {
            auto idstring = coalesce_pass->GetIDMapping(myNum);
            auto idnode = new IDNode(new Position(0, 0, 0, 0), idstring);
            delete this;
            return idnode;
        }
        return this;
    }
    else if (pass == 0)
    {
        coalesce_pass->AddToCollectionMap(myNum, this);
        return this;
    }
    assert(false); // only should be 1 or 0
}

