#ifndef RSRC_ALLOC_PASS_HPP
#define RSRC_ALLOC_PASS_HPP
#include "resource_analysis_pass.hpp"
#include <vector>
namespace DTL
{


class AGUHardwareStat
{
public:
    AGUHardwareStat(int nAdd, int nMult, int nLayers, int nConst, int nForLoop, int nPassThrough, int nOutStatements) :\
        nLayerAddUnits(nAdd), nLayerMultUnits(nMult), nConstRegisters(nConst), nForLoopRegisters(nForLoop),\
        nLayers(nLayers), nLayerPassThrough(nPassThrough), nOutStatements(nOutStatements)
    {

    }

    ~AGUHardwareStat()
    {

    }

    bool CheckMeetHardwareConstaints(DTLResources* rsrc) const
    {
        bool ret = (rsrc->nAddNeeded <= nLayerAddUnits);
        if (!ret) {
            std::cerr << "(rsrc->nAddNeeded <= nLayerAddUnits)\n"; goto end;
        }
        ret = (rsrc->nMultNeeded <= nLayerMultUnits);
        if (!ret) {
            std::cerr << "(rsrc->nMultNeeded <= nLayerMultUnits)\n"; goto end;
        }
        ret = (rsrc->ForLoopsNeeded <= nForLoopRegisters);
        if (!ret) {
            std::cerr << "(rsrc->ForLoopsNeeded <= nForLoopRegisters)\n"; goto end;
        }
        ret = (rsrc->nConstsNeeded <= nConstRegisters);
        if (!ret) {
            std::cerr << "(rsrc->nConstsNeeded <= nConstRegisters)\n"; goto end;
        }
        ret = (rsrc->nLayersNeeded <= nLayers);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nLayersNeeded, nLayers);
            std::cerr << "(rsrc->nLayersNeeded <= nLayers)\n"; goto end;
        }
        ret = (rsrc->nPassThrough <= nLayerPassThrough); 
        if (!ret) {
            std::cerr << "(rsrc->nPassThrough <= nLayerPassThrough)\n"; goto end;
        }
        ret = (rsrc->nOutStatements <= nOutStatements);
        if (!ret) {
            std::cerr << "(rsrc->nOutStatements <= nOutStatements)\n"; goto end;
        }
    end:
        return ret;
    }

    int nLayerAddUnits;
    int nLayerMultUnits;
    int nLayers;
    int nConstRegisters;
    int nForLoopRegisters;
    int nLayerPassThrough;
    int nOutStatements;

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
    AGULayer(int nAddUnits, int nMultUnits, int nPassThru) : maxAddUnit(nAddUnits), maxMultUnit(nMultUnits), maxPassThrough(nPassThru), \
        usedPassThrough(0), usedAddUnit(0), usedMultUnit(0)
    {

    }

    ~AGULayer()
    {

    }


    int MapPassThrough(PassThrough* unit)
    {
        int mapping = usedPassThrough;
        usedPassThrough++;
        assert(usedPassThrough <= maxPassThrough);
        MapFuncUnit(unit);
        return mapping;
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

    int getNextPassThrough() const
    {
        return usedPassThrough;
    }

    std::vector<FuncUnit*> inputRouting;
private:
    int maxAddUnit;
    int maxMultUnit;
    int maxPassThrough;
    int usedAddUnit;
    int usedMultUnit;
    int usedPassThrough;
};


/*
    We will add method onto this such that if we need less layers
    than the actual number of layers, then we just pass the finished
    value through to the end
*/
class OutStmtRouting
{
public:
    OutStmtRouting(int layerAddUnits, int layerMultUnits, int nPassThru, int nLayers) : \
        LayerAddUnitCount(layerAddUnits), LayerMultUnitCount(layerMultUnits), LayerPassThruCount(nPassThru)
    {
        for (int i = 0; i < nLayers; i++)
            LayerRouting.push_back(new AGULayer(layerAddUnits, layerMultUnits, nPassThru));
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

    int RequestPassThrough(int layer, int inputMapA)
    {
        assert(layer >= 0 && layer < LayerRouting.size());
        auto& routing = LayerRouting[layer];
        return routing->MapPassThrough(new PassThrough(routing->getNextPassThrough(), inputMapA));
    }


    /*
        We may want to do this in a slightly different way such that the indexes are contiguous,
        rather than overlapping
    */
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
    int LayerPassThruCount;
};



class ResourceAllocation{
public:
	static void* build(ResourceAnalysis* ra, AGUHardwareStat* hwStat)
    {
		ResourceAllocation * resourceAlloc = new ResourceAllocation;
		if (!resourceAlloc) return nullptr;
        resourceAlloc->hwStat = hwStat;

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

        ra->ast->resourceAllocation(resourceAlloc, 0);
		

        
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

    int ForLoopIDToMapping(std::string idName)
    {
        auto it = idForLoopRegMap.find(idName);

        // should always be mapped
        assert(it != idForLoopRegMap.end());

        return it->second;
    }


    OutStmtRouting* GetCurrentOutStatement() const
    {
        assert(OutStatementRouting.size());
        return OutStatementRouting[OutStatementRouting.size()-1];
    }

    void NewOutStatement()
    {
        assert(hwStat->nOutStatements <= OutStatementRouting.size());
        OutStatementRouting.push_back(new OutStmtRouting(hwStat->nLayerAddUnits, hwStat->nLayerMultUnits, hwStat->nLayerPassThrough, hwStat->nLayers));
    }

    
    void MultUnit(int layer, ASTNode* multNode, ASTNode* fromA, ASTNode* fromB)
    {
        auto currentOut = GetCurrentOutStatement();

        int a = currentOut->GetNodeFuncUnitMapping(fromA);
        int b = currentOut->GetNodeFuncUnitMapping(fromB);

        /*
            This should never occur. All units should be mapped somewhere. 
        */
        assert(a != -1 && b != -1);
        int unit = currentOut->RequestMultUnit(layer, a, b);
        currentOut->MapNodeFuncUnit(multNode, unit);
    }

    void AddUnit(int layer, ASTNode* plusNode, ASTNode* fromA, ASTNode* fromB)
    {
        auto currentOut = GetCurrentOutStatement();

        int a = currentOut->GetNodeFuncUnitMapping(fromA);
        int b = currentOut->GetNodeFuncUnitMapping(fromB);

        /*
            This should never occur. All units should be mapped somewhere. 
        */
        assert(a != -1 && b != -1);
        int unit = currentOut->RequestAddUnit(layer, a, b);
        currentOut->MapNodeFuncUnit(plusNode, unit);
    }

    void PassThru(int layer, ASTNode* plusNode, ASTNode* fromA)
    {
        auto currentOut = GetCurrentOutStatement();
        int a = currentOut->GetNodeFuncUnitMapping(fromA);

        /*
            This should never occur. All units should be mapped somewhere. 
        */
        assert(a != -1);
        int unit = currentOut->RequestPassThrough(layer, a);
        currentOut->MapNodeFuncUnit(plusNode, unit);
    }

    void SetAnswer(ASTNode* fromA)
    {

    }

    AGUHardwareStat* hwStat;
    ResourceAnalysis* rsrcAnalysis;

private:

    // [OutStmtNode # -> [Layer # -> N]]
    std::vector<OutStmtRouting*> OutStatementRouting;
    

    std::unordered_map<std::string, int> idForLoopRegMap;
    std::vector<LoopReg> loopRegisters;
	ResourceAllocation() 
	{

	}
};

}







#endif