
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

int main()
{
    try {
		auto allocator = new DTL::BuddyAllocator(0x1000ULL*0x1000ULL*128ULL);
		int bad = 0;
		for (int i = 0; i < 1000; i++)
		{
			std::vector<uint64_t> tmp;
			for (int j = 0; j < 100; j++)
			{
				auto addr = allocator->AllocNode(0x10000ULL);
				if (addr == BUDDY_ALLOC_FAILURE)
					bad++;
				else
					tmp.push_back(addr);
				//printf("got addr from allocator 0x%llx\n", addr);
			}

			for (auto& a : tmp)
			{
				allocator->FreeNode(a);
			}
			//break;
			//printf("here %d\n", bad);
			bad = 0;
		}
		
		//printf("got addr from allocator 0x%llx\n", addr);
		//allocator->FreeNode(addr);

		//compile();
		
		



		//ralloc->PrintDigraph(1, "./outStatement0Digraph");
    } catch (DTL::InternalError * e){
		std::cerr << "InternalError: " << e->msg() << std::endl;
		return 1;
	}

	
    printf("Successfully parsed!\n");
    return 0;
}