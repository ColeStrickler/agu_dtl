#include "resource_alloc_pass.hpp"
#include "ast.hpp"

void DTL::ProgramNode::resourceAllocation(ResourceAllocation *ralloc, bool alloc)
{
    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);
}



void DTL::ConstDeclNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{

    /*
        We have already mapped the ID of the constant declared here to a register
    */


    return; 

}



void DTL::PostIncStmtNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    return;
}



void DTL::ForStmtNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    int x = Collapse();
    if (x != 0)
    {
        std::cerr << "Error in ForStmtNode::Collapse() during resourceAllocation()\n";
        exit(-1);
    }
    ralloc->AllocLoopRegister(RegInitValue, RegMaxValue);

    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);

}

void DTL::OutStmtNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    myExp->resourceAllocation(ralloc, true);
}


void DTL::IntTypeNode::resourceAllocation(ResourceAllocation *ralloc, bool alloc)
{
    
}



void DTL::IDNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}

void DTL::IntLitNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}

void DTL::PlusNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}

void DTL::TimesNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}

void DTL::LessNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}

void DTL::LessEqNode::resourceAllocation(ResourceAllocation* ralloc, bool alloc)
{
    
}