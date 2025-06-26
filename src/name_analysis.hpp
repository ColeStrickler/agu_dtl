#ifndef NAME_ANALYSIS_HPP
#define NAME_ANALYSIS_HPP

#include "ast.hpp"
#include "symbol_table.hpp"


namespace DTL {



    
class NameAnalysis{
public:
	static NameAnalysis * build(ProgramNode * astIn){
		NameAnalysis * nameAnalysis = new NameAnalysis;
		SymbolTable * symTab = new SymbolTable();
		printf("NameAnalysis::build()\n");
		bool res = astIn->nameAnalysis(symTab);
		printf("yolo\n");
		delete symTab;
		if (!res){ return nullptr; }

		nameAnalysis->ast = astIn;
		return nameAnalysis;
	}
	ProgramNode * ast;

private:
	NameAnalysis(){
	}
};





}












#endif