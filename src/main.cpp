
#include <fstream>
#include <string.h>
#include <chrono>
#include "errors.hpp"
#include "scanner.hpp"
#include "frontend.hh"
#include "name_analysis.hpp"
#include "ast_transform_pass.hpp"
#include "resource_analysis_pass.hpp"
#include "resource_alloc_pass.hpp"
#include "dtl_api.hpp"
#include "optimizations/optimization_passes.hpp"

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

static DTL::ProgramNode * parse(const char * inFile){
	std::ifstream inStream(inFile);
	if (!inStream.good()){
		std::string msg = "Bad input stream ";
		msg += inFile;
		throw new DTL::InternalError(msg.c_str());
	}

	//This pointer will be set to the root of the
	// AST after parsing
	DTL::ProgramNode * root = nullptr;
    DTL::Scanner scanner(&inStream);
	DTL::Parser parser(scanner, &root);
	

	int errCode = parser.parse();
	
	if (errCode != 0){ printf("parse() errCode: %d\n", errCode); return nullptr; }

	return root;
}



int compile()
{
	 writeTokenStream("./test.dtl", "./dtltokens.out");
		auto hwStat = new DTL::AGUHardwareStat(4, 4, 5, 5, 6, 4, 3, 1);
		auto start = std::chrono::high_resolution_clock::now();
        auto prog = parse("./test.dtl");
		assert(prog != nullptr);
		
		auto na = DTL::NameAnalysis::build(prog);
		if (na == nullptr)
		{
			printf("failed name analysis\n");
			return -1;
		}
		
		auto ta = DTL::TypeAnalysis::build(na);
		if (ta == nullptr)
			return -1;

		prog = static_cast<DTL::ProgramNode*>(DTL::ASTTransformPass::Transform(prog));
		
		auto ra = DTL::ResourceAnalysis::build(prog, hwStat);
		//std::cout << ra->GetResources()->toString() << "\n";
		
		
		

		auto ralloc = DTL::ResourceAllocation::build(ra, hwStat);
		auto end = std::chrono::high_resolution_clock::now();


		/*
			Would actually read hardware stats
		*/
		
		
		std::chrono::duration<double, std::milli> elapsed = end - start;

		std::cout << "Time taken: " << elapsed.count() << " ms\n";


		
		prog->PrintAST("./astDigraph.dot");
		


		ralloc->PrintControlWrites("./outcontrolseq", 0x4000000);
		ralloc->PrintInitStateRegisters("./outcontrolseq", 0x4000000);

}



#include <stdio.h>
std::string FileToString(const std::string& file_)
{
  std::ifstream file(file_);
    if (!file) {
        std::cerr << "Failed to open file\n";
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();  // Read entire file into the buffer
    std::string contents = buffer.str();
    return contents;
}



int main()
{
	auto hwStat = new DTL::AGUHardwareStat(4, 4, 5, 5, 6, 4, 3, 1);

    std::istringstream input(FileToString("./test.dtl"));
    DTL::ProgramNode * root = nullptr;
    DTL::Scanner scanner(&input);
	DTL::Parser parser(scanner, &root);
    //parser.set_debug_level(1);  // Turn on debugging
    int err = parser.parse();
	if (err != 0){ printf("parse() errCode: %d\n", err); return false; }

    auto na = DTL::NameAnalysis::build(root);
	if (na == nullptr) {

		return false;
	}
	auto ta = DTL::TypeAnalysis::build(na);
	if (ta == nullptr)
	{

        return false;
    }
	root->PrintAST("out_untransform.ast");
	printf("here\n");
	
	root = static_cast<DTL::ProgramNode*>(DTL::ConstantFoldPass::ConstantFold(root));
	root->PrintAST("out_constfold.ast");

    root = static_cast<DTL::ProgramNode*>(DTL::ASTTransformPass::Transform(root));
    if (root == nullptr)
    {

        return false;
    }
	root->PrintAST("out.ast");
		
	auto ra = DTL::ResourceAnalysis::build(root, hwStat);
    if (ra == nullptr)
    {
        //ERR("ResourceAnalysis Failed");
        return false;
    }
    ra->GetResources()->GetNeededResourceStats();
	auto rsrc_string = ra->GetResources()->toString();

    auto ralloc = DTL::ResourceAllocation::build(ra, hwStat);
    if (ralloc == nullptr)
    {
        //ERR("ResourceAllocation failed\n");
        return false;
    }

	ralloc->PrintControlWrites("regwrites.out", AGU_CONFIG_BASE);
	ralloc->PrintInitStateRegisters("regwrites.out", AGU_CONFIG_BASE);
	
    printf("Successfully parsed!\n");
    return 0;
}