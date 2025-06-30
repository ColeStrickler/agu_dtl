#ifndef RSRC_ALLOC_PASS_HPP
#define RSRC_ALLOC_PASS_HPP
#include "resource_analysis_pass.hpp"
#include <vector>
namespace DTL
{


class AGUHardwareStat
{
public:
    AGUHardwareStat(int nAdd, int nMult, int nLayers, int nConst, int nForLoop, int nPassThrough) :\
        nLayerAddUnits(nAdd), nLayerMultUnits(nMult), nConstRegisters(nConst), nForLoopRegisters(nForLoop), nLayers(nLayers), nLayerPassThrough(nPassThrough)
    {

    }

    ~AGUHardwareStat()
    {

    }

    bool CheckMeetHardwareConstaints(DTLResources* rsrc) const
    {
        return (rsrc->nAddNeeded <= nLayerAddUnits) && (rsrc->nMultNeeded <= nLayerMultUnits) &&\
            (rsrc->ForLoopsNeeded <= nForLoopRegisters) && (rsrc->nConstsNeeded <= nConstRegisters) && \
            (rsrc->nLayersNeeded <= nLayers) && (rsrc->nPassThrough <= nLayerPassThrough); 
    }

    int nLayerAddUnits;
    int nLayerMultUnits;
    int nLayers;
    int nConstRegisters;
    int nForLoopRegisters;
    int nLayerPassThrough;

};


struct LoopReg
{
    int init_value;
    int increment_condition;
};



class FuncUnit
{
public:

    /*
        Used for passthrough
    */
    FuncUnit(int regAssignment, int inputA) : RegAssignment(regAssignment), InputA(inputA)
    {

    }

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



class AddUnit : public FuncUnit
{
public:
    AddUnit(int regAssignment, int inputA, int inputB) : FuncUnit(regAssignment, inputA, inputB)
    {

    }

    ~AddUnit()
    {

    }
};


class PassThrough : public FuncUnit
{
public:
    PassThrough(int regAssignment, int inputA) : FuncUnit(regAssignment, inputA)
    {

    }

    ~PassThrough()
    {
        
    }


private:

};


class MultUnit : public FuncUnit
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
    AGULayer(int nAddUnits, int nMultUnits) : maxAddUnit(nAddUnits), maxMultUnit(nMultUnits), usedAddUnit(0), usedMultUnit(0)
    {

    }

    ~AGULayer()
    {

    }


    int MapAddUnit(AddUnit* unit)
    {
        int mapping = usedAddUnit;
        usedAddUnit++;
        assert(usedAddUnit <= maxAddUnit);
        MapFuncUnit(unit);
        return mapping;
    }


    int MapMultUnit(MultUnit* unit)
    {
        int mapping = usedMultUnit;
        usedMultUnit++;
        assert(usedMultUnit <= maxMultUnit);
        MapFuncUnit(unit);
        return mapping;
    }

    void MapFuncUnit(FuncUnit* unit)
    {
        inputRouting.push_back(unit);
    }


    int getNextAddUnit() const
    {
        return usedAddUnit;
    }

    int getNextMultUnit() const
    {
        return usedMultUnit;
    }

    std::vector<FuncUnit*> inputRouting;
private:
    int maxAddUnit;
    int maxMultUnit;
    int usedAddUnit;
    int usedMultUnit;

};


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
            LayerRouting.push_back(new AGULayer(layerAddUnits, layerMultUnits));
    }

    ~OutStmtRouting()
    {

    }

    int RequestAddUnit(int layer, int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerRouting.size());

        auto& routing = LayerRouting[layer];
        return routing->MapAddUnit(new AddUnit(routing->getNextAddUnit(), inputMapA, inputMapB));
    }

    int RequestMultUnit(int layer,  int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerRouting.size());
        auto& routing = LayerRouting[layer];
        return routing->MapMultUnit(new MultUnit(routing->getNextMultUnit(), inputMapA, inputMapB));
    }


    int GetNodeFuncUnitMapping(ASTNode* node)
    {
        if (OpFuncUnitMapping.find(node) != OpFuncUnitMapping.end())
            return OpFuncUnitMapping[node]; 
        return -1;
    }

    void MapNodeFuncUnit(ASTNode* node, int unit)
    {
        OpFuncUnitMapping.insert(std::make_pair(node, unit));
    }
    
private:
    std::unordered_map<ASTNode*, int> OpFuncUnitMapping;
    std::vector<AGULayer*> LayerRouting;
    int LayerAddUnitCount;
    int LayerMultUnitCount;
    
};



class ResourceAllocation{
public:
	static void* build(ResourceAnalysis* ra, AGUHardwareStat* hwStat){
		ResourceAllocation * resourceAlloc = new ResourceAllocation;
		if (!resourceAlloc) return nullptr;

        if (!hwStat->CheckMeetHardwareConstaints(ra->GetResources()))
        {
            std::cerr << "Configuration uses too many hardware resources.\n";
            exit(-1);
        }




        
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

    OutStmtRouting* GetCurrentOutStatement() const
    {
        assert(OutStatementRouting.size());
        return OutStatementRouting[OutStatementRouting.size()-1];
    }

private:

    // [OutStmtNode # -> [Layer # -> N]]
    std::vector<OutStmtRouting*> OutStatementRouting;


    std::unordered_map<std::string, int> idForLoopRegMap;
    std::vector<LoopReg> loopRegisters;
    ResourceAnalysis* rsrcAnalysis;
	ResourceAllocation() 
	{

	}
};

}







#endif