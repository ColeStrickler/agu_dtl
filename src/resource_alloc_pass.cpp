#include "resource_alloc_pass.hpp"
#include "ast.hpp"

void DTL::ProgramNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    printf("ProgramNode::resourceAllocation()\n");
    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);
}



void DTL::ConstDeclNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{

    /*
        We have already mapped the ID of the constant declared here to a register
        in ResourceAnalysis
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
    ralloc->MapForLoopReg(GetInitVar());

    for (auto& stmt: myStatements)
        stmt->resourceAllocation(ralloc, false);

}

void DTL::OutStmtNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    ralloc->NewOutStatement();
    myExp->resourceAllocation(ralloc, 0);
    ralloc->SetAnswer(myExp);
}


void DTL::IntTypeNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    return;
}



void DTL::IDNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    auto out = ralloc->GetCurrentOutStatement();
    auto& ra = ralloc->rsrcAnalysis;

    int id = ra->GetConstRegMapping(getName());

    if (id == -1) // if a for loop register
    {       
        id = ralloc->ForLoopIDToMapping(getName());
    }

    out->MapNodeFuncUnit(this, id, depth); // we just shift the mapping over
}

void DTL::IntLitNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    auto out = ralloc->GetCurrentOutStatement();
    auto& ra = ralloc->rsrcAnalysis;
    out->MapNodeFuncUnit(this, ra->GetConstRegMapping(std::to_string(myNum)), depth); // we just shift the mapping over
}

void DTL::PlusNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    auto outstmt = ralloc->GetCurrentOutStatement();
    assert(outstmt != nullptr);
    myExp1->resourceAllocation(ralloc, depth+1);
    myExp2->resourceAllocation(ralloc, depth+1);

    if (isPassThrough())
    {
        ralloc->PassThru(depth, this, myExp1);
    }
    else
    {
        ralloc->AddUnit(depth, this, myExp1, myExp2);
    }

}

void DTL::TimesNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    auto outstmt = ralloc->GetCurrentOutStatement();
    assert(outstmt != nullptr);
    myExp1->resourceAllocation(ralloc, depth+1);
    myExp2->resourceAllocation(ralloc, depth+1);

    ralloc->MultUnit(depth, this, myExp1, myExp2);
    
}

void DTL::LessNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    return;
}

void DTL::LessEqNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{
    return;
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

bool DTL::OutStmtRouting::PrintDigraph(const std::string &file)
{
	std::ofstream outfile(file);  // open for writing
	if (!outfile.is_open()) {
		std::cerr << "Failed to open file.\n";
		return false;
	}

	outfile << "digraph G {\n";
    /*
        We will map the constants in layer0
    */
    //for (int i = 0; i < LayerRouting.size(); i++)
    //{
    //    outfile << LayerRouting[i]->PrintDigraph(i);
    //}

    for (auto& e: LayerRouting)
    {
        int i = e.first;
        printf("i %d\n", i);
        outfile << e.second->PrintDigraph(i+1);
    }


	outfile << "}\n";

    outfile.close();

    return true;
}

std::string DTL::OutStmtRouting::PrintControlWrites(uint64_t baseaddr, int numoutstatement) const
{
    std::string ret;
    printf("outstmt num %d\n", numoutstatement);
    for (auto& e: LayerRouting)
    {
         int i = e.first;
         ret += e.second->PrintControlWrites(baseaddr, numoutstatement, i, hwStat);
         
    }
    return ret;
}

std::string DTL::AGULayer::PrintControlWrites(uint64_t baseaddr, int numOutStmt, int layer, AGUHardwareStat *hwstat)
{
    std::string ret;
    for (auto& unit : inputRouting)
    {
        /*
            I think this is correct, we map inputs to the units assignment, but I am not completely sure this will map correctly

            We will want to check and confirm here first when we debugs
        */
        ret += hwstat->PrintControlWrite(baseaddr, numOutStmt, layer, unit->InputA, unit->RegAssignment) + "\n";
        ret += hwstat->PrintControlWrite(baseaddr, numOutStmt, layer, unit->InputB, unit->RegAssignment) + "\n";
    }
    return ret;
}

std::string DTL::AGULayer::PrintDigraph(int layer) const
{
    std::string ret;
    printf("inputRouting.size() %d\n", inputRouting.size());
    for (auto& unit: inputRouting)
        ret += unit->toString(layer);


    return ret;
}

std::string DTL::MultUnit::toString(int layer)
{
    std::string node_name = "\"" + std::to_string(layer) + "_" + std::to_string(RegAssignment) + "\"";
    std::string label = node_name + "[label=\"Mult_" +  std::to_string(layer) + "_" + std::to_string(RegAssignment)+ "\"];";


    std::string inA = "\"" + std::to_string(layer-1) + "_" + std::to_string(InputA) + "\"" + " -> " + node_name + ";";
    std::string inB = "\"" + std::to_string(layer-1) + "_" + std::to_string(InputB) + "\"" + " -> " + node_name + ";";

    return label + "\n" + inA + "\n" + inB + "\n";
}

std::string DTL::PassThrough::toString(int layer)
{
    printf("pass thru\n");
    std::string node_name = "\"" + std::to_string(layer) + "_" + std::to_string(RegAssignment) + "\"";
    std::string label = node_name + "[label=\"PassThru" +  std::to_string(layer) + "_" + std::to_string(RegAssignment)+ "\"];";


    std::string inA = "\"" + std::to_string(layer-1) + "_" + std::to_string(InputA) + "\"" + " -> " + node_name + ";";

    return label + "\n" + inA + "\n";
}

std::string DTL::AddUnit::toString(int layer)
{
    std::string node_name = "\"" + std::to_string(layer) + "_" + std::to_string(RegAssignment) + "\"";
    std::string label = node_name  + "[label=\"Plus" +  std::to_string(layer) + "_" + std::to_string(RegAssignment)+ "\"];";


    std::string inA = "\"" + std::to_string(layer-1) + "_" + std::to_string(InputA) + "\"" + " -> " + node_name + ";";
    std::string inB = "\"" + std::to_string(layer-1) + "_" + std::to_string(InputB) + "\"" + " -> " + node_name + ";";

    return label + "\n" + inA + "\n" + inB + "\n";
}

void DTL::ResourceAllocation::PrintInitStateRegisters(const std::string &file, uint64_t baseaddr)
{
        std::ofstream outfile(file, std::ios::app);  // open for writing
        if (!outfile.is_open()) {
            std::cerr << "Failed to open file.\n";
            return;
        }

        std::string write;
        for (auto& f : loopRegisters)
        {
            write += hwStat->PrintForLoopWrite(baseaddr, f, 4);
            // We will place the for loop registers immediately after the routing registers
        }


        for (auto& c : rsrcAnalysis->ConstValueMap)
        {
            write += hwStat->PrintConstRegWrite(baseaddr, c.first, c.second, 4);
        }

        
        outfile << write;
        outfile.close();
    }