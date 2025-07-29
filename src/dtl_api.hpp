#ifndef DTL_API_HPP
#define DTL_API_HPP
#include "scanner.hpp"
#include "resource_alloc_pass.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"
#include "ast_transform_pass.hpp"



namespace DTL
{



class API
{
public:
    API(); // if we allow automatically reading 
    API(AGUHardwareStat* hwStat); // if we want to manually configure
    ~API();
    void SetBaseAddr(uint64_t newBaseAddr);
    void ReadHardwareInfo();
    bool CompileAndProgramHardware(const std::string& dtlProgram);
    bool Compile(const std::string& dtlProgram);
    bool ProgramHardware();
    
private:
    AGUHardwareStat* hwStat;
    uint64_t ControlBaseAddress;
    DTL::ResourceAllocation* ralloc;



};




}








#endif