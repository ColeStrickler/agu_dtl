#include "resource_alloc_pass.hpp"
#include "ast.hpp"



void DTL::ProgramNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    //printf("ProgramNode::resourceAllocation()\n");
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




void DTL::ConstArrayDeclNode::resourceAllocation(ResourceAllocation* ralloc, int depth)
{

    /*
        We have already mapped the ID of the ConstArray declared here to a register
        in ResourceAnalysis
    */

    std::vector<uint32_t> values;

    for (auto& val: myVals)
    {
        values->push_back(static_cast<uint32_t>(val->GetVal()));
    }

    ralloc->AllocConstArray(ralloc->rsrcAnalysis->GetConstArrayRegMapping(myID->getName()), values);

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

    /*
        Must do this here to successfully map onto loop register indexing scheme
    */
    ralloc->MapForLoopReg(GetInitVar());
    ralloc->AllocLoopRegister(RegInitValue, RegMaxValue);
    

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


void DTL::ArrayIndexNode::resourceAllocation(ResourceAllocation *ralloc, int depth)
{
    auto out = ralloc->GetCurrentOutStatement();
    auto& ra = ralloc->rsrcAnalysis;

    //int index_id = ra->GetConstRegMapping(myIndexVar->getName());
    //assert(index_id == -1); // We should never be indexing with a non loop
    index_id = ralloc->ForLoopIDToMapping(myIndexVar->getName());
    assert(index_id != -1);

    int array_id = ra->GetConstArrayRegMapping(myID->getName());
    assert(array_id != -1);


    // Ensure we do not already have a binding
    assert(ralloc->BindArrayIndex(array_id, index_id));


    out->MapNodeFuncUnit(this, array_id, depth);


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

int DTL::ConstArrayDeclNode::Collapse(ResourceAllocation *ralloc)
{
    assert(false); // should never hit
    return 0;
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


int DTL::ArrayIndexNode::Collapse(ResourceAllocation *ralloc)
{
    assert(false);
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
    //ralloc->MapForLoopReg(getName());
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

void DTL::OutStmtRouting::DoControlWrites(uint64_t baseaddr, int numoutstatement) const
{
    for (auto& e: LayerRouting)
    {
         int i = e.first;
         e.second->DoControlWrites(baseaddr, numoutstatement, i, hwStat);     
    }
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
        ret += hwstat->PrintControlWrite(baseaddr, numOutStmt, layer, unit->InputA, unit->RegAssignment);
        ret += hwstat->PrintControlWrite(baseaddr, numOutStmt, layer, unit->InputB, unit->RegAssignment);
    }
    return ret;
}

void DTL::AGULayer::DoControlWrites(uint64_t baseaddr, int numOutStmt, int layer, AGUHardwareStat *hwstat)
{
    for (auto& unit : inputRouting)
    {
        /*
            I think this is correct, we map inputs to the units assignment, but I am not completely sure this will map correctly

            We will want to check and confirm here first when we debugs
        */
        hwstat->DoControlWrite(baseaddr, numOutStmt, layer, unit->InputA, unit->RegAssignment);
        hwstat->DoControlWrite(baseaddr, numOutStmt, layer, unit->InputB, unit->RegAssignment);
    }
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


        write += "\nWRITE_UINT8(" + to_hex(baseaddr+USED_OUTSTMT_REG) + "," + to_hex(static_cast<uint8_t>(OutStatementRouting.size())) +   ");\n";
        write += "\nWRITE_UINT8(" + to_hex(baseaddr+USED_FORLOOP_REG) + "," + to_hex(static_cast<uint8_t>(loopRegisters.size())) +   ");\n";

        
        outfile << write;
        outfile.close();
}

void DTL::ResourceAllocation::DoInitStateRegisters(uint64_t baseAddr)
{
    for (auto& f : loopRegisters)
    {
        hwStat->DoForLoopWrite(baseAddr, f, 4);
        // We will place the for loop registers immediately after the routing registers
    }
    for (auto& c : rsrcAnalysis->ConstValueMap)
    {
       hwStat->DoConstRegWrite(baseAddr, c.first, c.second, 4);
    }

    WRITE_UINT8(baseAddr+USED_OUTSTMT_REG, static_cast<uint8_t>(OutStatementRouting.size()));
    WRITE_UINT8(baseAddr+USED_FORLOOP_REG, static_cast<uint8_t>(loopRegisters.size()));
}

void DTL::ResourceAllocation::DoControlWrites(uint64_t baseaddr)
{
    for (int i = 0; i < OutStatementRouting.size(); i++)
    {
        auto& outstmt = OutStatementRouting[i];
        outstmt->DoControlWrites(baseaddr, i);
    }
}

void DTL::ResourceAllocation::PrintControlWrites(const std::string &file,uint64_t baseaddr)
{
    std::ofstream outfile(file);  // open for writing
    if (!outfile.is_open()) {
        std::cerr << "Failed to open file.\n";
        return;
    }
    std::string out;
    for (int i = 0; i < OutStatementRouting.size(); i++)
    {
        auto& outstmt = OutStatementRouting[i];
        printf("here\n");
        out += outstmt->PrintControlWrites(baseaddr, i);
    }
    outfile << out;
    outfile.close();
    /*
        We still need to set up a way to write for loop and constant registers
    */
}

void DTL::AGUHardwareStat::DoForLoopWrite(uint64_t baseAddress, LoopReg &reg, uint32_t byte_width) 
{
    uint64_t addr = baseAddress + GetLoopRegsOffset() + reg.reg_num*byte_width;
    printf("DoForLoopWrite()1 0x%x, 0x%x\n", baseAddress, GetLoopRegsOffset() + reg.reg_num*byte_width);
    uint32_t write_value_ = static_cast<uint32_t>(reg.init_value);
    WRITE_UINT32(addr, write_value_);

    addr = baseAddress + GetLoopIncRegsOffset(byte_width) + reg.reg_num*byte_width; // these should align
    write_value_ = static_cast<uint32_t>(reg.increment_condition);
     printf("DoForLoopWrite()1 0x%x, 0x%x\n", baseAddress, GetLoopIncRegsOffset(byte_width) + reg.reg_num*byte_width);
    WRITE_UINT32(addr, write_value_);

    /*
        We now can write the magic values
        These are implemented backwards in the hardware to facilitate the unroll unit
    */
    addr = baseAddress + GetMagicRegsOffset(byte_width) + ((nForLoopRegisters-1-reg.reg_num)*bytesMagic);
    auto write_value64 = static_cast<uint64_t>(reg.hwDivMagic.M);
    WRITE_UINT64(addr, write_value64);
    addr += 0x8;
    write_value_ = static_cast<uint32_t>(reg.hwDivMagic.s);
    WRITE_UINT32(addr, write_value_);
    addr += 0x4;
    uint8_t write_val8 = static_cast<uint8_t>(reg.hwDivMagic.add_indicator); // this will be interpreted as boolean value in hardware
    WRITE_UINT8(addr, write_val8);
}


std::string DTL::AGUHardwareStat::PrintForLoopWrite(uint64_t baseAddress,
                                                    LoopReg &reg,
                                                    uint32_t byte_width) {
  uint64_t addr_ = baseAddress + GetLoopRegsOffset() + reg.reg_num * byte_width;
  std::string addr = to_hex(addr_);
  uint64_t write_value_ = reg.init_value;
  std::string write_value = to_hex(write_value_);

  // ret to loop register
  std::string ret = "\nWRITE_UINT32(" + addr + "," + write_value + ");\n";
  addr_ = baseAddress + GetLoopIncRegsOffset(byte_width) +
          reg.reg_num * byte_width; // these should align
  addr = to_hex(addr_);
  write_value = to_hex(static_cast<uint64_t>(reg.increment_condition));
  ret += "\nWRITE_UINT32(" + addr + "," + write_value + ");\n";

  /*
      We now can write the magic values
      These are implemented backwards in the hardware to facilitate the unroll
     unit
  */
  addr_ = baseAddress + GetMagicRegsOffset(byte_width) +
          ((nForLoopRegisters - 1 - reg.reg_num) * bytesMagic);
  addr = to_hex(addr_);
  write_value = to_hex(static_cast<uint64_t>(reg.hwDivMagic.M));
  ret += "\nWRITE_UINT64(" + addr + "," + write_value + ");\n";

  addr_ += 0x8;
  addr = to_hex(addr_);
  write_value = to_hex(static_cast<uint64_t>(reg.hwDivMagic.s));
  ret += "\nWRITE_UINT32(" + addr + "," + write_value + ");\n";

  addr_ += 0x4;
  addr = to_hex(addr_);
  uint8_t write_val8 = static_cast<uint8_t>(
      reg.hwDivMagic.add_indicator); // this will be interpreted as boolean
                                     // value in hardware
  write_value = to_hex(write_val8);
  ret += "\nWRITE_UINT8(" + addr + "," + write_value + ");\n";
  return ret;
}
