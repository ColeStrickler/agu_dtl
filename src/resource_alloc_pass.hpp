#ifndef RSRC_ALLOC_PASS_HPP
#define RSRC_ALLOC_PASS_HPP
#include "resource_analysis_pass.hpp"
#include <vector>
namespace DTL
{
struct AGUHardwareStat
{
};


struct LoopReg
{
    int init_value;
    int increment_condition;
};



class FuncUnit
{
public:
    FuncUnit(int regAssignment, int inputA, int inputB) : RegAssignment(regAssignment), InputA(inputA), InputB(inputB)
    {

    }

    ~FuncUnit()
    {

    }

private:
    int RegAssignment;
    int InputA;
    int InputB;


};



class AddUnit :FuncUnit
{
public:
    AddUnit(int regAssignment, int inputA, int inputB) : FuncUnit(regAssignment, inputA, inputB)
    {

    }

    ~AddUnit()
    {

    }



};


class MultUnit : FuncUnit
{
public:
    MultUnit(int regAssignment, int inputA, int inputB) : FuncUnit(regAssignment, inputA, inputB)
    {

    }

    ~MultUnit()
    {
        
    }
};




class AGULayer
{
public:
    AGULayer()
    {

    }

    ~AGULayer()
    {

    }


    void MapAddUnit(AddUnit* unit)
    {

        
    }


    void MapMultUnit()
    {

    }

    void MapFuncUnit(FuncUnit* unit)
    {
        inputRouting.push_back(unit);
    }

    std::vector<FuncUnit*> inputRouting;
private:
    int maxAddUnit;
    int maxMultUnit;
    int usedAddUnit;
    int usedMultUnit

}


/*
    We will add method onto this such that if we need less layers
    than the actual number of layers, then we just pass the finished
    value through to the end
*/
class OutStmtRouting
{
public:
    OutStmtRouting(int layerAddUnits, int layerMultUnits, int nLayers) : LayerAddUnitCount(layerAddUnits), LayerMultUnitCount(layerMultUnits)
    {
        for (int i = 0; i < nLayers; i++)
            LayerRouting.push_back(new AGULayer);
    }

    ~OutStmtRouting()
    {

    }

    int RequestAddUnit(int layer, int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerRouting.size());
        LayerRouting[layer]->MapFuncUnit(new AddUnit())

    }

    int RequestMultUnit(int layer,  int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerRouting.size());
    }

    
private:
    std::vector<AGULayer*> LayerRouting;
    int LayerAddUnitCount;
    int LayerMultUnitCount;
    
}



class ResourceAllocation{
public:
	static void* build(ResourceAnalysis* ra){
		ResourceAllocation * resourceAlloc = new ResourceAllocation;
		if (!resourceAlloc) return nullptr;

        /*
            We need to allocate layers inside of resourceAlloc.

            We currently have all the registers, both ForLoop and Constants
            being mapped to indexes. Inside of the OutStmtNode we need to map
            these starting from the bottom up. We then will generate intermediate values
            as we move back up the tree, and we need to do this all the way up.

        */

        resourceAlloc->rsrcAnalysis = ra;
		

        
        //ra->ast->resourceAllocation(resourceAlloc);
        return nullptr;
		//return resourceAnalysis->GetResources();
	}


	void AllocLoopRegister(int initVal, int maxVal)
    {
        loopRegisters.push_back({initVal, maxVal});
    }


    /*
        We now know which for loop register is mapped to this ID
    */
    void MapForLoopReg(std::string idName)
    {
        idForLoopRegMap[idName] = loopRegisters.size();
    }

private:

    // [OutStmtNode # -> [Layer # -> N]]
    std::unordered_map<int, >



    std::unordered_map<std::string, int> idForLoopRegMap;
    std::vector<LoopReg> loopRegisters;
    ResourceAnalysis* rsrcAnalysis;
	ResourceAllocation() 
	{

	}
};

}







#endif