#ifndef DTL_API_HPP
#define DTL_API_HPP
#include "scanner.hpp"
#include "resource_alloc_pass.hpp"
#include "name_analysis.hpp"
#include "type_analysis.hpp"
#include "ast_transform_pass.hpp"
#include "util.hpp"

// System Headers
#include <fcntl.h> 
#include <sys/mman.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define Kilobyte            1024
#define Megabyte            Kilobyte*1024

namespace DTL
{



#define DEVICE_FILE "/dev/DTU-RUNTIME-DRIVER"
#define IOCTL_NEW_PA        _IO('a', 1)
#define IOCTL_REMAP_PA      _IO('a', 2)
typedef struct RemapPARequest
{
    void* u_VA;
    void* u_NewPA;
}RemapPARequest;



#define DTU_CONFIG_SIZE 0x1000
#define DTU_CONFIG_BASE 0x3000000ULL
#define AGU_CONFIG_BASE 0x4000000ULL
#define AGU_CONFIG_RST 0xf00


#define AGU_CONFIG_OFFSET(config) (config*0x1000)
#define DTU_UNCACHED_REGION_ADDR 0x180000000ULL
#define DTU_UNCACHED_REGION_SIZE 0x10000000ULL



// this is the only place we should have to update
#define UPDATE_CONFIG_PHYSMAP(base, config, maxConfigs, EphemeralConfigPhysStart) (WRITE_UINT64(base+(config*0x8)+(2*maxConfigs*0x8)+0x400, EphemeralConfigPhysStart))
#define UPDATE_CONFIG_SIZE(base, config, maxConfigs, size) (WRITE_UINT64(base+(config*0x8)+(maxConfigs*0x8)+0x400, size))
#define UPDATE_CONFIG_START(base, config, maxConfigs, size) (WRITE_UINT64(base+(config*0x8)+0x400, size))









class EphemeralRegion
{
public:
    EphemeralRegion(uint64_t region_offset, uint64_t region_size, int config_num, uint64_t physical_backing_start, AGUHardwareStat* hwStat);
    ~EphemeralRegion();

    /*
        We want methods for: access, delete, locking, update

        We want to enable access by index and by offset
    */

    // Maybe define some macros? not sure how we want to do this yet

    /*
        This method remaps the va->pa region and ensures that region pulls in latest data
    */


   /*
        We need to add methods to write to the uncached backing of these regions and
        to enable teardown

   
   */


    int Sync(); //

    void* GetHeadlessReadRegion(); // get raw pointer to ephemeral region, no bounds checking, writes allowed
    void* GetHeadlessWriteregion();

    uint64_t GetRegionOffset() {return m_RegionOffset; }

    void GuardedWrite_8(uint64_t offset, uint8_t data);
    uint8_t GuardedRead_8(uint64_t offset);
    uint16_t GuardedRead_16(uint64_t offset);
    uint32_t GuardedRead_32(uint64_t offset);
    uint64_t GuardedRead_64(uint64_t offset);

    float GuardedRead_Float(uint64_t offset);

    inline int GetConfig() {return m_ConfigNum;}

private:
    uint64_t m_RegionOffset;
    uint64_t m_PhysBackingStart;
    uint64_t m_RegionSize;
    int m_ConfigNum;
    void* m_EphemeralRegionAccess;
    void* m_UncachedRegionAccess;
    int m_Regionfd;
    int m_DTURuntimeDriverfd;
    void* m_DTUConfigRegion;
    AGUHardwareStat* hwStat;


    
    uint64_t m_CurrentEphemeralPhysicalAddr;

    //int open_fd(const std::string& path);

    


};


/*
    want to do something like this:
    
    dtu_api = new API();

    EphemeralRegion* region = dtu_api->AllocateEphemeralRegion(size = 0x100000)
    dtu_api->CompileAndProgramHardware(region, dtlProgram); // bind program to region


    float* ephemeral_raw = region->GetHeadlessRegion(); // access region raw pointer

    for (int i = 0; i < region->GetRegionSize(); i += increment_size)
    {
        read ephemeral_raw[i]
    }


    ///// OR WITH BOUNDS CHECKING //////
    for (int i = 0; i < nRows; i += increment_size)
    {
        region->GuardedRead(i*row_size);
    }


    // 

    // now update region
    for (int i = 0; i<nWrites; i++)
    {
        *((uint64_t)ephemeral_raw +  write_offset) = write_data;
        ////// OR /////////
        region->GuardedWrite(write_offset, write_data); // we can have some flagging semantics that notify readers we have performed writes
    }

    region->Sync(); // now all new packed cache lines will be updated


    for (int i = 0; i < nRows; i += increment_size)
    {
        region->GuardedRead(i*row_size);
    }


    We can keep synchronization primitives light. Just simple update flags, and let users wrap around it that best fits their paradigm




*/

class BuddyNode
{
public:
    BuddyNode(uint64_t TrackedSize, uint64_t TrackedOffset, BuddyNode* parent);
    ~BuddyNode();

    void Split();
    BuddyNode* GetLeft();
    BuddyNode* GetRight();
    void SetInUse();
    void SetFree();
    bool isFree();
    bool isRoot();
    void Coalesce();

    void DebugPrint();

    uint64_t GetTrackedOffset();
    uint64_t GetTrackedSize();

private:
    uint64_t m_TrackedOffset;
    uint64_t m_TrackedSize;
    bool m_IsFree;
    BuddyNode* m_LeftChild;
    BuddyNode* m_RightChild;
    BuddyNode* m_Parent;
};


#define BUDDY_ALLOC_FAILURE     UINT64_MAX
#define CONFIG_ALLOC_FAILURE    (UINT64_MAX-1ULL)


class BuddyAllocator
{
public:
    BuddyAllocator(uint64_t region_size);
    ~BuddyAllocator();
    void FreeNode(uint64_t offset);  
    uint64_t AllocNode(uint64_t size_needed);


    void DebugPrintTree();



    
private:
    uint64_t FindAndAllocFreeNode(uint64_t size_needed, BuddyNode* currentNode);
    BuddyNode* FindNode(uint64_t offset, BuddyNode* currNode);

    uint64_t m_Size;
    BuddyNode* m_RootNode;


};


class API
{
public:
    API(uint64_t realBackingStart = 0x170000000UL, uint64_t realBackingSize = 0x10000000UL); // if we allow automatically reading 
    API(AGUHardwareStat* hwStat, uint64_t realBackingStart = 0x170000000UL, uint64_t realBackingSize = 0x10000000UL); // if we want to manually configure
    ~API();

    /*
        Should be a mmap'ed address being passed in
    */
    void SetAGUControlRegionBaseAddr(uint64_t newBaseAddr);
    void ReadHardwareInfo();
    bool CompileAndProgramHardware(const std::string& dtlProgram, EphemeralRegion* region);
    bool Compile(const std::string& dtlProgram);
    bool ProgramHardware(EphemeralRegion* region);

    void ResetConfig(int config);

    void DebugPrintAllocator();
    
    /*
        We want to track allocations of the ephemeral region/support multiple configurations

        We want to track each configuration and have handles to each

        We need an API to remap phys addr based on the handle

        We probably want some sort of concurrency support
    */

    EphemeralRegion* AllocEphemeralRegion(uint64_t size_needed);
    void FreeEphemeralRegion(EphemeralRegion* ephemeralRegion);

    uint64_t GetError();
    AGUHardwareStat* GetHWStat();
private:
    uint64_t AllocateRegion(uint64_t size);
    int AllocateConfig();
    void FreeConfig(int config);
    void SetError(uint64_t Error);
    uint64_t m_ConfigBitmap; // config 
    uint64_t m_ReceivedPAFromDriver;
    uint64_t m_RealBackingStart;
    uint64_t m_RealBackingSize;
    BuddyAllocator* m_BuddyAllocator;
    AGUHardwareStat* hwStat;
    uint64_t AGUControlRegionBaseAddress; // will index off this
    DTL::ResourceAllocation* ralloc; // possibly move this to the ephemeral region? 


    int m_AGUControlRegionfd;

    uint64_t m_ErrorCode;
};




};








#endif