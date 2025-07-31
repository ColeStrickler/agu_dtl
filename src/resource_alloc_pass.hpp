#ifndef RSRC_ALLOC_PASS_HPP
#define RSRC_ALLOC_PASS_HPP
#include "resource_analysis_pass.hpp"
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include "util.hpp"


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


#define USED_OUTSTMT_REG 0xf01


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
struct LoopReg
{
    int init_value;
    int increment_condition;
    int reg_num;
    Magic hwDivMagic;
};



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


            we will make each cell have 4 valid outs a byte wide
        */

        bytesCell = 4;
        bytesLayer = bytesCell * (nAdd + nMult + nPassThrough);

        bytesOutStatement = (nLayers+1) * bytesLayer; // layer at beginning at at the end
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


    inline uint64_t GetLoopRegsOffset() const
    {
        return nOutStatements*(nLayers+1)*(nLayerPassThrough+nLayerMultUnits+nLayerAddUnits)*bytesCell;
    }

    inline uint64_t GetLoopIncRegsOffset(uint32_t byteWidth) const
    {
        return GetLoopRegsOffset() + (nForLoopRegisters*byteWidth);
    }

    inline uint64_t GetConstantRegsOffset(uint32_t byteWidth) const
    {
        return GetLoopIncRegsOffset(byteWidth) + (nForLoopRegisters*byteWidth);
    }



    void DoForLoopWrite(uint64_t baseAddress, LoopReg& reg, uint32_t byte_width)
    {
        
        uint64_t addr = baseAddress + GetLoopRegsOffset() + reg.reg_num*byte_width;
        printf("DoForLoopWrite()1 0x%x, 0x%x\n", baseAddress, GetLoopRegsOffset() + reg.reg_num*byte_width);
        uint32_t write_value_ = static_cast<uint32_t>(reg.init_value);
        WRITE_UINT32(addr, write_value_);

        addr = baseAddress + GetLoopIncRegsOffset(byte_width) + reg.reg_num*byte_width; // these should align
        write_value_ = static_cast<uint32_t>(reg.increment_condition);
         printf("DoForLoopWrite()1 0x%x, 0x%x\n", baseAddress, GetLoopIncRegsOffset(byte_width) + reg.reg_num*byte_width);
        WRITE_UINT32(addr, write_value_);
    }


    /*
        We will return both the inc register write and the for loop write
    */
    std::string PrintForLoopWrite(uint64_t baseAddress, LoopReg& reg, uint32_t byte_width)
    {
        uint64_t addr_ = baseAddress + GetLoopRegsOffset() + reg.reg_num*byte_width;
        std::string addr = to_hex(addr_);



        uint64_t write_value_ = reg.init_value;
        std::string write_value = to_hex(write_value_);

        // ret to loop register
        std::string ret = "\nWRITE_UINT32(" + addr + "," + write_value + ");";


        addr_ = baseAddress + GetLoopIncRegsOffset(byte_width) + reg.reg_num*byte_width; // these should align
        addr = to_hex(addr_);
        write_value = to_hex(static_cast<uint64_t>(reg.increment_condition));
        ret += "\nWRITE_UINT32(" + addr + "," + write_value + ");";
        return ret;
    }

    void DoConstRegWrite(uint64_t baseAddress, int constRegNum, int constRegvalue, uint32_t byte_width)
    {
       
        uint64_t addr_ = baseAddress + GetConstantRegsOffset(byte_width) + constRegNum*byte_width;
         printf("DoConstRegWrite() 0x%x\n", addr_);
        assert(byte_width == 4);
        WRITE_UINT32(addr_, constRegvalue);
    }

    std::string PrintConstRegWrite(uint64_t baseAddress, int constRegNum, int constRegvalue, uint32_t byte_width)
    {
        uint64_t addr_ = baseAddress + GetConstantRegsOffset(byte_width) +constRegNum*byte_width;
        std::string addr = to_hex(addr_);



        uint64_t write_value_ = constRegvalue;
        std::string write_value = to_hex(write_value_);

        // ret to loop register
        std::string ret = "WRITE_UINT32(" + addr + "," + write_value + ");";
        return ret;
    }


    void DoControlWrite(uint64_t baseAddress, int numOutStatement, int layer, int inRegNumber, int outRegNumber)
    {
        
        if (outRegNumber == 255)
            return;
        assert(outRegNumber < __UINT8_MAX__);

        unsigned int layerByteOffset = layer * bytesLayer;
        unsigned int cellByteOffset = inRegNumber * bytesCell;
        unsigned int outStatementOffset = numOutStatement * bytesOutStatement;

         auto hash_str = std::to_string(baseAddress) + "_" +
           std::to_string(layerByteOffset) + "_" +
           std::to_string(cellByteOffset) + "_" +
           std::to_string(outStatementOffset);


        int cell_index = VarOutMap[hash_str];
        uint64_t offset = layerByteOffset + cellByteOffset + outStatementOffset + cell_index;
        printf("DoControlWrite() 0x%x +0x%x\n", baseAddress, offset);
        VarOutMap[hash_str]++;
        assert(VarOutMap[hash_str] <= bytesCell); // this maps to maxVarOut
        WRITE_UINT8(baseAddress+offset, static_cast<unsigned char>(outRegNumber));
    }

    std::string PrintControlWrite(uint64_t baseAddress, int numOutStatement, int layer, int inRegNumber, int outRegNumber)
    {
        if (outRegNumber == 255)
            return "";
        

        unsigned int layerByteOffset = layer * bytesLayer;
        unsigned int cellByteOffset = inRegNumber * bytesCell;
        unsigned int outStatementOffset = numOutStatement * bytesOutStatement;

        assert(outRegNumber < __UINT8_MAX__);
        auto hash_str = std::to_string(baseAddress) + "_" +
           std::to_string(layerByteOffset) + "_" +
           std::to_string(cellByteOffset) + "_" +
           std::to_string(outStatementOffset);


        int cell_index = VarOutMap[hash_str];
        uint64_t offset = layerByteOffset + cellByteOffset + outStatementOffset + cell_index;
        std::string addr = to_hex(baseAddress + offset);
        VarOutMap[hash_str]++;
        assert(VarOutMap[hash_str] <= bytesCell); // this maps to maxVarOut


        printf("0x%x, 0x%x, 0x%x, 0x%x\n", layerByteOffset, cellByteOffset, outStatementOffset, cell_index);
        printf("0x%x nOut %d, layer %d, inReg %d, outReg %d 0x%x\n", baseAddress, numOutStatement, layer, inRegNumber, outRegNumber, offset);
        std::string write_value = std::to_string(static_cast<unsigned char>(outRegNumber));

        return "WRITE_UINT8(" + addr + ", " + write_value + ");";  
    }

    std::unordered_map<std::string, int> VarOutMap;



    bool CheckMeetHardwareConstaints(DTLResources* rsrc) const
    {
        bool ret = (rsrc->nAddNeeded <= nLayerAddUnits);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nAddNeeded, nLayerAddUnits);
            std::cerr << "(rsrc->nAddNeeded <= nLayerAddUnits)\n"; goto end;
        }
        ret = (rsrc->nMultNeeded <= nLayerMultUnits);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nMultNeeded, nLayerMultUnits);
            std::cerr << "(rsrc->nMultNeeded <= nLayerMultUnits)\n"; goto end;
        }
        ret = (rsrc->ForLoopsNeeded <= nForLoopRegisters);
        if (!ret) {
            printf("%d <= %d\n", rsrc->ForLoopsNeeded, nForLoopRegisters);
            std::cerr << "(rsrc->ForLoopsNeeded <= nForLoopRegisters)\n"; goto end;
        }
        ret = (rsrc->nConstsNeeded <= nConstRegisters);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nConstsNeeded, nConstRegisters);
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
            printf("%d <= %d\n", rsrc->nPassThrough, nLayerPassThrough);
            std::cerr << "(rsrc->nPassThrough <= nLayerPassThrough)\n"; goto end;
        }
        ret = (rsrc->nOutStatements <= nOutStatements);
        if (!ret) {
            printf("%d <= %d\n", rsrc->nOutStatements, nOutStatements);
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





class FuncUnit
{
public:

    /*
        Used for passthrough
    */
    FuncUnit(int regAssignment, int inputA) : RegAssignment(regAssignment), InputA(inputA), InputB(255)
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
    void DoControlWrites(uint64_t baseaddr, int numOutStmt, int layer, AGUHardwareStat* hwstat);

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
        assert(layer >= 0 && layer < LayerCount+1);
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
        while(m < LayerCount+1)
        {
            LayerRouting[m] = new AGULayer(LayerAddUnitCount, LayerMultUnitCount, LayerPassThruCount);
            unit = RequestPassThrough(m, unit);
            m++;
        }
        
        //for (auto& e: LayerRouting)





    }
    
    std::string PrintDigraph(const std::string& file) const;
    std::string PrintControlWrites(uint64_t baseaddr, int numoutstatement) const;
    void DoControlWrites(uint64_t baseaddr, int numoutstatement) const;

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
		resourceAlloc->CalculateForLoopMagicValues();

        
        //ra->ast->resourceAllocation(resourceAlloc);
        return resourceAlloc;
		//return resourceAnalysis->GetResources();
	}


    void CalculateForLoopMagicValues()
    {
        /*
            We traverse backwards, as the inner loops are placed at the end of this
        */

        uint32_t d = rsrcAnalysis->GetResources()->nOutStatements; // the first divisor is # of outstatements
        
        for (int i = loopRegisters.size()-1; i >= 0; i--)
        {
            auto loopReg = loopRegisters[i];
            auto magicnums = magicu(d);
            loopReg.hwDivMagic.add_indicator = magicnums.add_indicator;
            loopReg.hwDivMagic.M = magicnums.M;
            loopReg.hwDivMagic.s = magicnums.s;


            /*
                This is equivalent to computing the stride for each loop counter
                as seen in the dtu_stateless_algo
            */
            d *= loopReg.increment_condition;
        }


    }


	void AllocLoopRegister(int initVal, int maxVal)
    {
        // we map backwards
        loopRegisters.push_back({initVal, maxVal, (rsrcAnalysis->GetResources()->ForLoopsNeeded - 1 - (int)loopRegisters.size())});
    }


    /*
        We now know which for loop register is mapped to this ID


        may need to check the routing of these
    */
    void MapForLoopReg(std::string idName)
    {
        auto forloopsneeded = rsrcAnalysis->GetResources()->ForLoopsNeeded;
        idForLoopRegMap[idName] =  hwStat->nConstRegisters + (forloopsneeded - loopRegisters.size()-1);
        ReverseidForLoopRegMap[hwStat->nConstRegisters + (forloopsneeded - loopRegisters.size()-1)] = idName;
        //printf("For Loop reg %s --> %d\n", idName.c_str(), (forloopsneeded - loopRegisters.size()-1));
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
        //printf("a %d, b %d\n", a, b);
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


    void PrintInitStateRegisters(const std::string& file, uint64_t baseaddr);
    void DoInitStateRegisters(uint64_t baseAddr);


    void DoControlWrites(uint64_t baseaddr)
    {
        for (int i = 0; i < OutStatementRouting.size(); i++)
        {
            auto& outstmt = OutStatementRouting[i];
            outstmt->DoControlWrites(baseaddr, i);
        }
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