#include "resource_alloc_pass.hpp"
#include "ast.hpp"

void DTL::ProgramNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);
}



void DTL::ConstDeclNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{

    /*
        We have already mapped the ID of the constant declared here to a register
    */


    return; 

}



void DTL::PostIncStmtNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    return;
}



void DTL::ForStmtNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
    int x = Collapse(ralloc);
    if (x != 0)
    {
        std::cerr << "Error in ForStmtNode::Collapse() during resourceAllocation()\n";
        exit(-1);
    }

    ralloc->AllocLoopRegister(RegInitValue, RegMaxValue);

    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);

}

void DTL::OutStmtNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    myExp->resourceAllocation(ralloc, true);
}


void DTL::IntTypeNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    
}



void DTL::IDNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
}

void DTL::IntLitNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
}

void DTL::PlusNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    auto outstmt = ralloc->GetCurrentOutStatement();
    assert(outstmt != nullptr);
    myExp1->resourceAllocation(ralloc, depth+1);
    myExp2->resourceAllocation(ralloc, depth+1);

    NODETAG exp1Tag = myExp1->getTag();

    switch(exp1Tag)
    {
        
    }



}

void DTL::TimesNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
}

void DTL::LessNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
}

void DTL::LessEqNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    
}


/*
    We offer these methods to collapse for loop into a structure
    suitable for resource allocation. 
*/
int DTL::ForStmtNode::Collapse(ResourceAllocation* ralloc)
{
    int m = myCondExp->Collapse(ralloc);
    int init = myInit->Collapse(ralloc);

    RegMaxValue = m;
    RegInitValue = init;
    return 0;
}


int DTL::ConstDeclNode::Collapse(ResourceAllocation* ralloc)
{
    myID->Collapse(ralloc);
    int val = myVal->Collapse(ralloc);
    return val;
}

int DTL::IntLitNode::Collapse(ResourceAllocation* ralloc)
{
    return myNum;
}


int DTL::OutStmtNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // should never hit
    return 0;
}


int DTL::PlusNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // should never hit
    return 0;
}

int DTL::TimesNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // should never hit
    return 0;
}

int DTL::LessNode::Collapse(ResourceAllocation* ralloc)
{
    
    return myExp2->Collapse(ralloc);
}

int DTL::LessEqNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // possibly implement later
    return 0;
}

int DTL::PostIncStmtNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // should never hit
    return 0;
}

int DTL::IDNode::Collapse(ResourceAllocation* ralloc)
{
    ralloc->MapForLoopReg(getName());
    return 0;
}

int DTL::IntTypeNode::Collapse(ResourceAllocation* ralloc)
{
    assert(false); // should never hit
    return 0;
}

