#include "dtl_api.hpp"

#define ERR(msg) (std::cerr << msg << std::endl)




DTL::API::API() 
{
    ReadHardwareInfo();
}

DTL::API::API(AGUHardwareStat *hwStat) : ControlBaseAddress(0x4000000), hwStat(hwStat)
{
}

DTL::API::~API() {
    
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

static void writeTokenStream(const char * inPath, const char * outPath){
	std::ifstream inStream(inPath);
	if (!inStream.good()){
		std::string msg = "Bad input stream";
		msg += inPath;
		throw new DTL::InternalError(msg.c_str());
	}
	if (outPath == nullptr){
		std::string msg = "No tokens output file given";
		throw new DTL::InternalError(msg.c_str());
	}

	DTL::Scanner scanner(&inStream);
	if (strcmp(outPath, "--") == 0){
		scanner.outputTokens(std::cout);
	} else {
		std::ofstream outStream(outPath);
		if (!outStream.good()){
			std::string msg = "Bad output file ";
			msg += outPath;
			throw new DTL::InternalError(msg.c_str());
		}
		scanner.outputTokens(outStream);
		outStream.close();
	}
}


bool DTL::API::Compile(const std::string &dtlProgram)
{
    writeTokenStream("./aguconfig", "tokens.out");


    std::istringstream input(dtlProgram);
    DTL::ProgramNode * root = nullptr;
    DTL::Scanner scanner(&input);
	DTL::Parser parser(scanner, &root);
    parser.set_debug_level(1);  // Turn on debugging
    int err = parser.parse();
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
    ra->GetResources()->GetNeededResourceStats();

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
    
    ralloc->DoInitStateRegisters(ControlBaseAddress);
    printf("FinishInitStateRegs\n");
    ralloc->DoControlWrites(ControlBaseAddress);
    printf("Finished Control Writes\n");
    return true;
}

void DTL::API::SetBaseAddr(uint64_t newBaseAddr)
{
    ControlBaseAddress = newBaseAddr;
}