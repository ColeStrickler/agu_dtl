#ifndef RSRC_ANALYSIS_PASS_HPP
#define RSRC_ANALYSIS_PASS_HPP
#include "ast.hpp"
#include <vector>
#include <map>
#include <string>
#include <algorithm>


namespace DTL {



struct DTLUnitAllocation
{
	int nMultUnits;
	int nAddUnits;
	int nPassThrough;
};


struct DTLResources
{
	DTLResources() : ForLoopsNeeded(0), nConstsNeeded(0), nOutStatements(0), nPassThrough(0)
	{

	}

	int GetPassThroughNeeded()
	{
		int pass_needed = 0;
		for (auto& e: LayerFuncUnitAllocations)
		{
			for (auto& l : e.second)
			{
				const DTLUnitAllocation& alloc_layer = l.second;
				int pass = alloc_layer.nPassThrough;
				pass_needed = std::max(pass_needed, pass);
			}
		}
		nPassThrough = pass_needed;
		return nPassThrough;
	}

	int GetLayersNeeded()
	{
		int layers_needed = 0;
		for (auto& e: LayerFuncUnitAllocations)
			layers_needed = std::max(layers_needed, static_cast<int>(e.second.size())); // compile time cast
		nLayersNeeded = layers_needed;
		return layers_needed;
	}

	int GetMultUnitsNeeded()
	{
		int mult_needed = 0;
		for (auto& e: LayerFuncUnitAllocations)
		{
			for (auto& l : e.second)
			{
				const DTLUnitAllocation& alloc_layer = l.second;
				int mult = alloc_layer.nMultUnits;
				mult_needed = std::max(mult_needed, mult);
			}
		}
		nMultNeeded = mult_needed;
		return nMultNeeded;
	}


	/*
	
	
	*/
	int GetAddUnitsNeeded()
	{
		int add_needed = 0;
		for (auto& e: LayerFuncUnitAllocations)
		{
			for (auto& l : e.second)
			{
				const DTLUnitAllocation& alloc_layer = l.second;
				int add = alloc_layer.nAddUnits;
				add_needed = std::max(add_needed, add);
			}
		}
		nAddNeeded = add_needed;
		return nAddNeeded;
	}


	std::string toString()
	{
		std::string ret;
		ret += "ForLoopsNeeded: " + std::to_string(ForLoopsNeeded) + ", ";
		ret += "Layers Needed: " + std::to_string(GetLayersNeeded()) + ", ";
		ret += "nConstsNeeded: " + std::to_string(nConstsNeeded) + ", ";
		ret += "nOutStatements: " + std::to_string(nOutStatements) + ", ";

		for (auto& e: LayerFuncUnitAllocations)
		{
			// e.first = outStatement#
			// l.first = layer#
			ret += "\n-------------------------------------------------\n";
			for (auto& l: e.second)
			{
				const DTLUnitAllocation& alloc_layer = l.second;
				int mult = alloc_layer.nMultUnits;
				int add = alloc_layer.nAddUnits;
				int pass = alloc_layer.nPassThrough;
				auto mult_units = std::to_string(mult);
				auto add_units = std::to_string(add);
				auto pass_thru = std::to_string(pass);
				ret += "Layer " + std::to_string(l.first) + ", OutStatement" + std::to_string(e.first) + ": MultUnits(" + mult_units + ") AddUnits(" + add_units + ")" + " PassThrough(" + pass_thru + ")\n";

			}
		}
		return ret;
	}



	int CurrentOutStatement() {return nOutStatements;};

	int ForLoopsNeeded;
	int nConstsNeeded;
	int nOutStatements;
	int nLayersNeeded;
	int nMultNeeded;
	int nAddNeeded;
	int nPassThrough;
	std::map<int, std::map<int, DTLUnitAllocation>> LayerFuncUnitAllocations;
};

    
class ResourceAnalysis{
public:
	static ResourceAnalysis * build(ProgramNode * astIn){
		ResourceAnalysis * resourceAnalysis = new ResourceAnalysis;
		if (!resourceAnalysis) return nullptr;
		
		astIn->resourceAnalysis(resourceAnalysis, 0);
		resourceAnalysis->ast = astIn;

		resourceAnalysis->GetResources()->GetLayersNeeded(); // must be computed
		
		return resourceAnalysis;

	}

	static void RegMapConst(ASTNode* node, ResourceAnalysis* ra)
	{
		static int constReg = 0;
		ra->ConstRegMapping.insert(std::make_pair(node,constReg));
		constReg++;
	}

	/*
		Returns -1 on failure;
	*/
	int GetConstRegMapping(ASTNode* node)
	{
		auto it = ConstRegMapping.find(node);
		if (it != ConstRegMapping.end())
			return it->second;
		return -1;
	}

	std::unordered_map<ASTNode*, int> ConstRegMapping; // Register assignments for Constants
	ProgramNode * ast;
	void UseForLoopRegister() 	{ResourcesNeeded->ForLoopsNeeded++;}
	void UseNewConst() 			{ResourcesNeeded->nConstsNeeded++;}
	void UseNewOutStatement()	{ResourcesNeeded->nOutStatements++;}
	void UseNewAddUnitLayer(int layer) {
		ResourcesNeeded->LayerFuncUnitAllocations[ResourcesNeeded->CurrentOutStatement()][layer].nAddUnits++;
	}
	void UseNewMultUnitLayer(int layer) {
		ResourcesNeeded->LayerFuncUnitAllocations[ResourcesNeeded->CurrentOutStatement()][layer].nMultUnits++;
	}
	
	void UseNewPassThroughLayer(int layer) {
		ResourcesNeeded->LayerFuncUnitAllocations[ResourcesNeeded->CurrentOutStatement()][layer].nPassThrough++;
	}

	DTLResources* GetResources() const {return ResourcesNeeded;}
private:
	DTLResources* ResourcesNeeded;


	ResourceAnalysis() : ResourcesNeeded(new DTLResources())
	{

	}
};





}

#endif