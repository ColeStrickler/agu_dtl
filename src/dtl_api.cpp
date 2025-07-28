#include "dtl_api.hpp"

#define ERR(msg) (std::cerr << msg << std::endl)




DTL::API::API() 
{
    ReadHardwareInfo();
}

DTL::API::API(AGUHardwareStat *hwStat) : ControlBaseAddress(0x4000000), hwStat(hwStat)
{
}

void DTL::API::ReadHardwareInfo()
{
    std::cerr << "DTL::API::ReadHardwareInfo() not yet implemented" << std::endl;
    assert(false);
}

bool DTL::API::CompileAndProgramHardware(const std::string &dtlProgram) 
{
    if (!Compile(dtlProgram))
        return false;
    return ProgramHardware();
}

bool DTL::API::Compile(const std::string &dtlProgram)
{
    std::istringstream input(dtlProgram);
    DTL::ProgramNode * root = nullptr;
    DTL::Scanner scanner(&input);
	DTL::Parser parser(scanner, &root);
    int err = parser.parse();
	int errCode = parser.parse();
	
	if (err != 0){ printf("parse() errCode: %d\n", err); return false; }

    auto na = DTL::NameAnalysis::build(root);
	if (na == nullptr)
	{
		ERR("Failed Name Analysis");
		return false;
	}
	auto ta = DTL::TypeAnalysis::build(na);
	if (ta == nullptr)
	{
        ERR("Failed Type Analysis");
        return false;
    }
    root = static_cast<DTL::ProgramNode*>(DTL::ASTTransformPass::Transform(root));
    if (root == nullptr)
    {
        ERR("ASTTransformPass() failed");
        return false;
    }
		
	auto ra = DTL::ResourceAnalysis::build(root);
    if (ra == nullptr)
    {
        ERR("ResourceAnalysis Failed");
        return false;
    }


    ralloc = DTL::ResourceAllocation::build(ra, hwStat);
    if (ralloc == nullptr)
    {
        ERR("ResourceAllocation failed\n");
        return false;
    }
    return true;
}

bool DTL::API::ProgramHardware()
{
    ralloc->DoControlWrites(ControlBaseAddress);
    ralloc->DoInitStateRegisters(ControlBaseAddress);
    return true;
}
