#ifndef RSRC_ALLOC_PASS_HPP
#define RSRC_ALLOC_PASS_HPP
#include "resource_analysis_pass.hpp"
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
namespace DTL
{

#define WRITE_BOOL(addr, value)(*(bool*)(addr) = value)
#define WRITE_UINT8(addr, value)(*(uint8_t*)(addr) = value)
#define WRITE_UINT16(addr, value)(*(uint16_t*)(addr) = value)
#define WRITE_UINT32(addr, value)(*(uint32_t*)(addr) = value)
#define WRITE_UINT64(addr, value)(*(uint64_t*)(addr) = value)

#define READ_BOOL(addr)(*(bool*)(addr))
#define READ_UINT8(addr)(*(uint8_t*)(addr))
#define READ_UINT16(addr)(*(uint16_t*)(addr))
#define READ_UINT32(addr)(*(uint32_t*)(addr))
#define READ_UINT64(addr)(*(uint64_t*)(addr))

inline int log2ceil(int n) {
    if (n <= 0) {
        // Handle error or return a specific value for non-positive input
        return 0; // Or throw an exception
    }
    return static_cast<int>(std::ceil(std::log2(static_cast<double>(n))));
}

template<typename T>
T divceil(T a, T b) {
    if (b == 0) throw std::invalid_argument("division by zero");

    if ((a ^ b) > 0) // same sign
        return (a + b - 1) / b;
    else
        return a / b;
}
inline std::string to_hex(uint64_t val) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::setw(16) << std::setfill('0') << val;
    return ss.str();
}

class AGUHardwareStat
{
public:
    AGUHardwareStat(int nAdd, int nMult, int nLayers, int nConst, int nForLoop, int nPassThrough, int nOutStatements) :\
        nLayerAddUnits(nAdd), nLayerMultUnits(nMult), nConstRegisters(nConst), nForLoopRegisters(nForLoop),\
        nLayers(nLayers), nLayerPassThrough(nPassThrough), nOutStatements(nOutStatements)
    {

        /*
            1 config per out statement,
            1 routing config per layer,
            nMult, nAdd, nPassThrough per layer


            we will make each cell a byte wide
        */

        bytesCell = 1;
        bytesLayer = bytesCell * (nAdd + nMult * nPassThrough);

        bytesOutStatement = nLayers * bytesLayer;
        totalConfigRegionBits = nOutStatements * bytesOutStatement;


    }

    ~AGUHardwareStat()
    {

    }

    /*
        [AddUnits, MultUnits, PassThru]
    */
    void WriteConfig(uint64_t baseAddress, int numOutStatement, int layer, int inRegNumber, int outRegNumber)
    {
        unsigned int layerByteOffset = layer * bytesLayer;
        unsigned int cellByteOffset = inRegNumber * bytesCell;
        unsigned int outStatementOffset = numOutStatement * bytesOutStatement;

        assert(outRegNumber < __UINT8_MAX__);

        /*
            We call each in->out routing config a "Cell".

            Each Cell is 1 byte
        */

        WRITE_UINT8(baseAddress + layerByteOffset + cellByteOffset + outStatementOffset, static_cast<unsigned char>(outRegNumber));
    }


    std::string PrintControlWrite(uint64_t baseAddress, int numOutStatement, int layer, int inRegNumber, int outRegNumber)
    {
        unsigned int layerByteOffset = layer * bytesLayer;
        unsigned int cellByteOffset = inRegNumber * bytesCell;
        unsigned int outStatementOffset = numOutStatement * bytesOutStatement;

        assert(outRegNumber < __UINT8_MAX__);

        std::string addr = to_hex(baseAddress + layerByteOffset + cellByteOffset + outStatementOffset);
        std::string write_value = std::to_string(static_cast<unsigned char>(outRegNumber));

        return "WRITE_UINT8(" + addr + ", " + write_value + ");";  
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

        /*
            Our current convention is to use the last layer to route PassThru0 to the output,
            so we need an extra layer to do this successfully. We can probably do this in a better way
            later on
        */
        ret = (rsrc->nLayersNeeded + 1 <= nLayers);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nLayersNeeded+1, nLayers);
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
    int bytesCell;
    int bytesLayer;
    int bytesOutStatement;
    int totalConfigRegionBits;
    int bits_outStatement;

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

    virtual std::string toString(int layer) = 0;
    int RegAssignment;
    int InputA;
    int InputB;
private:



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

    virtual std::string toString(int layer) override;

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

    virtual std::string toString(int layer) override;
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

    virtual std::string toString(int layer) override;
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
        return maxAddUnit + maxMultUnit + mapping;
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
        return maxAddUnit + mapping;
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
        return maxAddUnit+ usedMultUnit;
    }

    int getNextPassThrough() const
    {
        return maxAddUnit + maxMultUnit + usedPassThrough;
    }
    std::vector<FuncUnit*> inputRouting;

    std::string PrintControlWrites(uint64_t baseaddr, int numOutStmt, int layer, AGUHardwareStat* hwstat);

    std::string PrintDigraph(int layer) const;
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
    OutStmtRouting(int layerAddUnits, int layerMultUnits, int nPassThru, int nLayers, AGUHardwareStat* hwStat) : \
        LayerAddUnitCount(layerAddUnits), LayerMultUnitCount(layerMultUnits), LayerPassThruCount(nPassThru), LayerCount(nLayers), hwStat(hwStat)
    {
        
    }

    ~OutStmtRouting()
    {

    }


    bool PrintDigraph(const std::string& file);


    
    void CreateLayerIfNeeded(int layer)
    {
        auto it = LayerRouting.find(layer);
        if (it == LayerRouting.end())
            LayerRouting.insert(std::make_pair(layer, new AGULayer(LayerAddUnitCount, LayerMultUnitCount, LayerPassThruCount)));
    }


    int RequestAddUnit(int layer, int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerCount);
        CreateLayerIfNeeded(layer);
        auto& routing = LayerRouting[layer];
        return routing->MapAddUnit(new AddUnit(routing->getNextAddUnit(), inputMapA, inputMapB));
    }

    int RequestMultUnit(int layer,  int inputMapA, int inputMapB)
    {
        assert(layer >= 0 && layer < LayerCount);
        CreateLayerIfNeeded(layer);
        auto& routing = LayerRouting[layer];
        return routing->MapMultUnit(new MultUnit(routing->getNextMultUnit(), inputMapA, inputMapB));
    }

    int RequestPassThrough(int layer, int inputMapA)
    {
        assert(layer >= 0 && layer < LayerCount);
        CreateLayerIfNeeded(layer);
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
            return OpFuncUnitMapping[node].first; 
        return -1;
    }

    int GetNodeLayerMapping(ASTNode* node)
    {
        if (OpFuncUnitMapping.find(node) != OpFuncUnitMapping.end())
            return OpFuncUnitMapping[node].second; 
        return -1;
    }

    void MapNodeFuncUnit(ASTNode* node, int unit, int layer)
    {
        OpFuncUnitMapping.insert(std::make_pair(node, std::make_pair(unit, layer)));
    }


    void SetAnswer(ASTNode* node)
    {
        auto it = OpFuncUnitMapping.find(node);
        assert(it != OpFuncUnitMapping.end());
        int unit = it->second.first;
        int layer = it->second.second;
        /*
            The answer may or may not fit exactly onto the hardware. We may use less layers than there
            actually are, and if this is the case we need to map the answer to pass throughs for the unused layers
        */
        int nLayers = LayerRouting.size();
        std::map<int, AGULayer*> newLayerRouting;
        int m = 0;
        for (int i = nLayers-1; i >= 0; i--)
        {
            newLayerRouting[m] = LayerRouting[i];
            m++;
        }
        LayerRouting = newLayerRouting;
        while(m < LayerCount)
        {
            LayerRouting[m] = new AGULayer(LayerAddUnitCount, LayerMultUnitCount, LayerPassThruCount);
            unit = RequestPassThrough(m, unit);
            m++;
        }
        
        //for (auto& e: LayerRouting)





    }
    
    std::string PrintDigraph(const std::string& file) const;
    std::string PrintControlWrites(uint64_t baseaddr, int numoutstatement) const;

private:
    std::unordered_map<ASTNode*, std::pair<int, int>> OpFuncUnitMapping;
    AGUHardwareStat* hwStat;

    // layer# -> AGULayer*
    std::map<int, AGULayer*> LayerRouting;
    //std::vector<AGULayer*> LayerRouting;
    int LayerAddUnitCount;
    int LayerMultUnitCount;
    int LayerPassThruCount;
    int LayerCount;
};



class ResourceAllocation{
public:
	static ResourceAllocation* build(ResourceAnalysis* ra, AGUHardwareStat* hwStat)
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
        return resourceAlloc;
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
        idForLoopRegMap[idName] =  hwStat->nConstRegisters + loopRegisters.size();
        ReverseidForLoopRegMap[hwStat->nConstRegisters + loopRegisters.size()] = idName;
    }

    int ForLoopIDToMapping(std::string idName)
    {
        auto it = idForLoopRegMap.find(idName);

        // should always be mapped

        assert(it != idForLoopRegMap.end());

        return it->second;
    }

    std::string ReverseForLoopIDToMapping(int forLoopRegNum)
    {
        auto it = ReverseidForLoopRegMap.find(forLoopRegNum);

        // should always be mapped

        assert(it != ReverseidForLoopRegMap.end());

        return it->second;
    }


    OutStmtRouting* GetCurrentOutStatement() const
    {
        assert(OutStatementRouting.size());
        return OutStatementRouting[OutStatementRouting.size()-1];
    }

    void NewOutStatement()
    {
        assert(OutStatementRouting.size() <= hwStat->nOutStatements);
        OutStatementRouting.push_back(new OutStmtRouting(hwStat->nLayerAddUnits, hwStat->nLayerMultUnits, hwStat->nLayerPassThrough, hwStat->nLayers, hwStat));
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
        currentOut->MapNodeFuncUnit(multNode, unit, layer);
    }

    void AddUnit(int layer, ASTNode* plusNode, ASTNode* fromA, ASTNode* fromB)
    {
        auto currentOut = GetCurrentOutStatement();

        int a = currentOut->GetNodeFuncUnitMapping(fromA);
        int b = currentOut->GetNodeFuncUnitMapping(fromB);
        printf("a %d, b %d\n", a, b);
        /*
            This should never occur. All units should be mapped somewhere. 
        */
        assert(a != -1 && b != -1);
        int unit = currentOut->RequestAddUnit(layer, a, b);
        currentOut->MapNodeFuncUnit(plusNode, unit, layer);
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
        currentOut->MapNodeFuncUnit(plusNode, unit, layer);
    }

    void SetAnswer(ASTNode* fromA)
    {
        auto currentOut = GetCurrentOutStatement();
        currentOut->SetAnswer(fromA);
    }


    void PrintDigraph(int outStatementNum, const std::string& file)
    {
        OutStatementRouting[outStatementNum]->PrintDigraph(file);
    }

    void PrintControlWrites(const std::string& file, uint64_t baseaddr)
    {
        std::ofstream outfile(file);  // open for writing
        if (!outfile.is_open()) {
            std::cerr << "Failed to open file.\n";
            return;
        }

        std::string out;
        for (int i = 0; i < OutStatementRouting.size(); i++)
        {
            auto& outstmt = OutStatementRouting[i];
            printf("here\n");
            out += outstmt->PrintControlWrites(baseaddr, i);
        }

        outfile << out;
        outfile.close();


        /*
            We still need to set up a way to write for loop and constant registers
        */
    }



    AGUHardwareStat* hwStat;
    ResourceAnalysis* rsrcAnalysis;

private:

    // [OutStmtNode # -> [Layer # -> N]]
    std::vector<OutStmtRouting*> OutStatementRouting;
    
    std::unordered_map<int, std::string> ReverseidForLoopRegMap;
    std::unordered_map<std::string, int> idForLoopRegMap;
    std::vector<LoopReg> loopRegisters;
	ResourceAllocation() 
	{

	}
};

}







#endif