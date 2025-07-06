
#include <fstream>
#include <string.h>
#include "errors.hpp"
#include "scanner.hpp"
#include "frontend.hh"
#include "name_analysis.hpp"
#include "ast_transform_pass.hpp"
#include "resource_analysis_pass.hpp"
#include "resource_alloc_pass.hpp"

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
int main()
{
    try {
        writeTokenStream("./test.dtl", "./dtltokens.out");
        auto prog = parse("./test.dtl");
		assert(prog != nullptr);
		std::cout << prog->Check();

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

		auto ra = DTL::ResourceAnalysis::build(prog);
		


		/*
			Would actually read hardware stats
		*/
		auto hwStat = new DTL::AGUHardwareStat(64, 64, 64, 64, 64, 64, 64);




		auto ralloc = DTL::ResourceAllocation::build(ra, hwStat);

		std::cout << ra->GetResources()->toString() << "\n";
		prog->PrintAST("./astDigraph.dot");
    } catch (DTL::InternalError * e){
		std::cerr << "InternalError: " << e->msg() << std::endl;
		return 1;
	}

	
    printf("Successfully parsed!\n");
    return 0;
}