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
};


struct DTLResources
{
	DTLResources() : ForLoopsNeeded(0), nConstsNeeded(0), nOutStatements(0)
	{

	}

	int GetLayersNeeded() const
	{
		int layers_needed = 0;
		for (auto& e: LayerFuncUnitAllocations)
			layers_needed = std::max(layers_needed, static_cast<int>(e.second.size())); // compile time cast
		return layers_needed;
	}

	std::string toString() const
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
				auto mult_units = std::to_string(mult);
				auto add_units = std::to_string(add);
				ret += "Layer " + std::to_string(l.first) + ", OutStatement" + std::to_string(e.first) + ": MultUnits(" + mult_units + ") AddUnits(" + add_units + ")\n";

			}
		}
		return ret;
	}



	int CurrentOutStatement() {return nOutStatements;};

	int ForLoopsNeeded;
	int nConstsNeeded;
	int nOutStatements;
	std::map<int, std::map<int, DTLUnitAllocation>> LayerFuncUnitAllocations;
};

    
class ResourceAnalysis{
public:
	static DTLResources * build(ProgramNode * astIn){
		ResourceAnalysis * resourceAnalysis = new ResourceAnalysis;
		if (!resourceAnalysis) return nullptr;
		astIn->resourceAnalysis(resourceAnalysis, 0);

		return resourceAnalysis->GetResources();

	}
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
	

	
	DTLResources* GetResources() const {return ResourcesNeeded;}
private:
	DTLResources* ResourcesNeeded;


	ResourceAnalysis() : ResourcesNeeded(new DTLResources())
	{

	}
};





}

#endif