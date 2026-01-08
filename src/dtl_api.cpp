#include "dtl_api.hpp"
#include "util.hpp"
#include <algorithm>
#include <cmath>


#define ERR(msg) (std::cerr << msg << std::endl)


static int open_fd(const std::string& path) {
  int fd = open(path.c_str(), O_RDWR | O_SYNC);
  if (fd == -1) {
    printf("Can't open /dev/mem.\n");
    exit(0);
  }
  return fd;
}
#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PFN_MASK ((1ULL << 55) - 1)


static uint64_t va_to_pa(void *va) {
    uint64_t value;
    int pagemap_fd;
    off_t offset = ((uintptr_t)va / PAGE_SIZE) * sizeof(uint64_t);

    pagemap_fd = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap_fd < 0) {
        perror("open");
        return 0;
    }

    if (pread(pagemap_fd, &value, sizeof(value), offset) != sizeof(value)) {
        perror("pread");
        close(pagemap_fd);
        return 0;
    }

    close(pagemap_fd);

    if (!(value & (1ULL << 63))) {
        // page not present
        return 0;
    }

    uint64_t pfn = value & PFN_MASK;
    return (pfn << PAGE_SHIFT) | ((uintptr_t)va & (PAGE_SIZE - 1));
}




DTL::API::API(uint64_t realBackingStart, uint64_t realBackingSize) : m_ConfigBitmap(0)
{
    ReadHardwareInfo();


    assert(is_power_of_two(realBackingSize) && realBackingSize%Megabyte == 0);
    assert(realBackingSize > Megabyte);
    m_BuddyAllocator = new BuddyAllocator(realBackingSize);



}

DTL::API::API(AGUHardwareStat *hwStat, uint64_t realBackingStart, uint64_t realBackingSize) 
    :  hwStat(hwStat), \
        m_ConfigBitmap(0), m_RealBackingStart(realBackingStart), m_RealBackingSize(realBackingSize)
{

    SetError(0);

    assert(is_power_of_two(realBackingSize) && realBackingSize%Megabyte == 0);
    assert(realBackingSize > Megabyte);
    m_BuddyAllocator = new BuddyAllocator(realBackingSize);

    m_AGUControlRegionfd = open_fd("/dev/mem");
    if (m_AGUControlRegionfd == -1)
    {
        printf("Could not open /dev/mem\n");
        SetError(-1);
        return;
    }



    auto control_region_size = hwStat->nMaxConfigs*0x1000;
    printf("AGUControlRegionSize: 0x%x\n", control_region_size);
    AGUControlRegionBaseAddress = (uint64_t)mmap(NULL, control_region_size, PROT_READ | PROT_WRITE, MAP_SHARED, m_AGUControlRegionfd, AGU_CONFIG_BASE);
    if (AGUControlRegionBaseAddress == (uint64_t)MAP_FAILED)
    {
        printf("DTL::API::API() could not map AGUControlRegionBaseAddress\n");
        SetError(-1);
        return;
    }

}

DTL::API::~API() {
    
}

void DTL::API::ReadHardwareInfo()
{
    std::cerr << "DTL::API::ReadHardwareInfo() not yet implemented" << std::endl;
    assert(false);
}

bool DTL::API::CompileAndProgramHardware(const std::string &dtlProgram, EphemeralRegion* region) 
{
    if (!Compile(dtlProgram))
        return false;
    return ProgramHardware(region);
}

static void writeTokenStream(const char * inPath, const char * outPath){
	std::ifstream inStream(inPath);
	if (!inStream.good()){
		std::string msg = "Bad input stream";
		msg += inPath;
		throw new DTL::InternalError(msg.c_str());
	}
	if (outPath == nullptr){
		std::string msg = "No tokens output file given";
		throw new DTL::InternalError(msg.c_str());
	}

	DTL::Scanner scanner(&inStream);
	if (strcmp(outPath, "--") == 0){
		scanner.outputTokens(std::cout);
	} else {
		std::ofstream outStream(outPath);
		if (!outStream.good()){
			std::string msg = "Bad output file ";
			msg += outPath;
			throw new DTL::InternalError(msg.c_str());
		}
		scanner.outputTokens(outStream);
		outStream.close();
	}
}


bool DTL::API::Compile(const std::string &dtlProgram)
{
    /*
        May want to handle memory management here differently to prevent leaks!
    */


    std::istringstream input(dtlProgram);
    DTL::ProgramNode * root = nullptr;
    DTL::Scanner scanner(&input);
	DTL::Parser parser(scanner, &root);
    //parser.set_debug_level(1);  // Turn on debugging
    int err = parser.parse();
	if (err != 0){ printf("parse() errCode: %d\n", err); return false; }

    auto na = DTL::NameAnalysis::build(root);
	if (na == nullptr)
	{
		ERR("Failed Name Analysis");
		return false;
	}
	auto ta = DTL::TypeAnalysis::build(na);
	if (ta == nullptr)
	{
        ERR("Failed Type Analysis");
        return false;
    }
    root = static_cast<DTL::ProgramNode*>(DTL::DTLOptimizer::OptimizeStatic(root, DTL::DTL_OPTLEVEL::OPTMAX));
    root = static_cast<DTL::ProgramNode*>(DTL::ASTTransformPass::Transform(root, DTL_OPT_MAX));
    if (root == nullptr)
    {
        ERR("ASTTransformPass() failed");
        return false;
    }
		
	auto ra = DTL::ResourceAnalysis::build(root, hwStat);
    if (ra == nullptr)
    {
        ERR("ResourceAnalysis Failed");
        return false;
    }
    ra->GetResources()->GetNeededResourceStats();

    ralloc = DTL::ResourceAllocation::build(ra, hwStat);
    if (ralloc == nullptr)
    {
        ERR("ResourceAllocation failed\n");
        return false;
    }

    hwStat->VarOutMap.clear(); // otherwise we we will get subsequent collisions

    return true;
}


bool DTL::API::ProgramHardware(EphemeralRegion* region)
{
    // because we map each config region in the TLRegisterNode I think we can just calculate a new base offset
    uint64_t config_n_base = AGUControlRegionBaseAddress + AGU_CONFIG_OFFSET(region->GetConfig());

    //printf("DTU_CONFIG_OFFSET(0x%x)\n", DTU_CONFIG_OFFSET(region->GetConfig()));
    ralloc->DoInitStateRegisters(config_n_base);
   // printf("FinishInitStateRegs\n");
    ralloc->DoControlWrites(config_n_base);
  //  printf("Finished Control Writes\n");
    return true;
}

void DTL::API::ResetConfig(int config)
{
    uint64_t config_n_rst = AGUControlRegionBaseAddress + AGU_CONFIG_OFFSET(config) + AGU_CONFIG_RST;
    WRITE_BOOL(config_n_rst, true);
    while(READ_BOOL(config_n_rst) != true){};
    WRITE_BOOL(config_n_rst, false);
}

DTL::EphemeralRegion *DTL::API::AllocEphemeralRegion(uint64_t size_needed)
{

    /*
        We allocate a region offset from the buddy allocator
    */
    uint64_t region_offset = AllocateRegion(size_needed);
    printf("DTL::API::AllocEphemeralRegion() region_offset 0x%x\n", region_offset);
    if (region_offset == BUDDY_ALLOC_FAILURE)
    {
        SetError(BUDDY_ALLOC_FAILURE);
        return nullptr;
    }

    int config = AllocateConfig();
    if (config == -1)
    {
        SetError(CONFIG_ALLOC_FAILURE);
        return nullptr;
    }
    printf("DTL::API::AllocEphemeralRegion() config %d\n", config);
    auto ephemeral = new EphemeralRegion(region_offset, next_power_of_two(size_needed), config, m_RealBackingStart, hwStat);
    return ephemeral;
}

void DTL::API::FreeEphemeralRegion(EphemeralRegion *ephemeralRegion)
{
    m_BuddyAllocator->FreeNode(ephemeralRegion->GetRegionOffset());
    FreeConfig(ephemeralRegion->GetConfig());
    delete ephemeralRegion;
}

uint64_t DTL::API::GetError()
{
    return m_ErrorCode;
}

DTL::AGUHardwareStat *DTL::API::GetHWStat() { return hwStat; }

uint64_t DTL::API::AllocateRegion(uint64_t size) 
{
  assert(size <= m_RealBackingSize);
  uint64_t received_size = next_power_of_two(size);
  return m_BuddyAllocator->AllocNode(size);
}

/*
    Sets a bit in the config bitmap, returns -1 on error
*/
int DTL::API::AllocateConfig() {
    for (int i = 0; i < 64; i++)
    {
        if (((m_ConfigBitmap >> i) & 0x1ULL) == 0)
        {
            m_ConfigBitmap |= (1ULL << i);
            printf("Allocating config %d\n", i);
            return i;
        }
    }
    return -1;
}

void DTL::API::FreeConfig(int config)
{
    assert(config >= 0 && config < 64);
    assert(((m_ConfigBitmap >> config) & 0x1ULL) != 0);


    m_ConfigBitmap &= (~(1ULL << config));
    ResetConfig(config);
}

void DTL::API::SetError(uint64_t Error)
{
    m_ErrorCode = Error;
}

void DTL::API::SetAGUControlRegionBaseAddr(uint64_t newControlBaseAddr)
{
    AGUControlRegionBaseAddress = newControlBaseAddr;
}

DTL::BuddyNode::BuddyNode(uint64_t TrackedSize, uint64_t TrackedOffset, BuddyNode* parent) 
    : m_TrackedSize(TrackedSize), m_TrackedOffset(TrackedOffset), m_Parent(parent)
{
    SetFree();

    m_LeftChild = nullptr;
    m_RightChild = nullptr;
}

DTL::BuddyNode::~BuddyNode() 
{

}

void DTL::BuddyNode::Split()
{
    auto new_size = m_TrackedSize/2;
    m_LeftChild = new BuddyNode(new_size, m_TrackedOffset, this);
    m_RightChild = new BuddyNode(new_size, m_TrackedOffset + new_size, this);
}

DTL::BuddyNode *DTL::BuddyNode::GetLeft() 
{
    return m_LeftChild;
}

DTL::BuddyNode *DTL::BuddyNode::GetRight() 
{
    return m_RightChild;
}

void DTL::BuddyNode::SetInUse() 
{
    m_IsFree = false;
}

void DTL::BuddyNode::SetFree() 
{
    m_IsFree = true;
}

bool DTL::BuddyNode::isFree() 
{
    return m_IsFree;
}

bool DTL::BuddyNode::isRoot() 
{ 
    m_Parent == nullptr;
}

void DTL::BuddyNode::Coalesce()
{
   // printf("node 0x%llx coalescing!\n", m_TrackedOffset);

    auto left = GetLeft();
    auto right = GetRight();

    bool left_good = left != nullptr && left->isFree();
    bool right_good = right != nullptr && right->isFree();

    if (left_good && right_good)
    {
        delete right;
        delete left;
        SetFree();
        if (!isRoot())
        {
            m_Parent->Coalesce();
        }
    }
}

uint64_t DTL::BuddyNode::GetTrackedOffset() 
{
    return m_TrackedOffset;
}

uint64_t DTL::BuddyNode::GetTrackedSize() 
{
    return m_TrackedSize;
}

DTL::BuddyAllocator::BuddyAllocator(uint64_t region_size) 
{
    auto size = region_size;
    if (!is_power_of_two(region_size))
    {
        size = next_power_of_two(region_size);
    }

    printf("Root size: 0x%llx\n", size);
    m_RootNode = new BuddyNode(size, 0, nullptr);
}

void DTL::BuddyAllocator::FreeNode(uint64_t offset) 
{
    BuddyNode* node = FindNode(offset, m_RootNode);
    assert(node != nullptr);

    node->SetFree();
    node->Coalesce();
    
}

uint64_t DTL::BuddyAllocator::AllocNode(uint64_t size_needed) {

    auto size  = std::max(size_needed, (uint64_t)(0x100000ULL));
    if (!is_power_of_two(size))
        size = next_power_of_two(size);

  uint64_t AllocatedOffset = FindAndAllocFreeNode(size, m_RootNode);
  return AllocatedOffset;
}

uint64_t DTL::BuddyAllocator::FindAndAllocFreeNode(uint64_t size_needed, BuddyNode *currentNode) 
{
    if (currentNode == nullptr)
        return BUDDY_ALLOC_FAILURE;

    auto currentNodeSize = currentNode->GetTrackedSize();
    if (currentNodeSize < size_needed)
        return BUDDY_ALLOC_FAILURE;


    if (currentNode->isFree())
    {
        if (next_power_of_two(size_needed) == currentNodeSize)
        {
            currentNode->SetInUse();

            return currentNode->GetTrackedOffset();
        }
        else
        {
            currentNode->Split();
            uint64_t check_left = FindAndAllocFreeNode(size_needed, currentNode->GetLeft());
            if (check_left != BUDDY_ALLOC_FAILURE)
            {
                currentNode->SetInUse();
                return check_left;
            }
            uint64_t check_right = FindAndAllocFreeNode(size_needed, currentNode->GetRight());
            if (check_right != BUDDY_ALLOC_FAILURE)
                currentNode->SetInUse();
            return check_right;
        }
    }
    else
    {
        uint64_t check_left = FindAndAllocFreeNode(size_needed, currentNode->GetLeft());
        if (check_left != BUDDY_ALLOC_FAILURE)
        {
            currentNode->SetInUse();
            return check_left;
        }
        uint64_t check_right = FindAndAllocFreeNode(size_needed, currentNode->GetRight());
        if (check_right != BUDDY_ALLOC_FAILURE)
            currentNode->SetInUse();
        return check_right;
    }
    return BUDDY_ALLOC_FAILURE;
}

DTL::BuddyNode *DTL::BuddyAllocator::FindNode(uint64_t offset, BuddyNode* currNode) 
{
    if (currNode == nullptr)
        return nullptr;

    auto currNodeOffset = currNode->GetTrackedOffset();
    if(offset > currNodeOffset && offset > currNodeOffset + currNode->GetTrackedSize()) // we should never be less than
    {
        return nullptr; 
    }   
   // printf("here\n");
    if (offset == currNodeOffset)
    {
        auto left = currNode->GetLeft();
        auto right = currNode->GetRight();


        if (!currNode->isFree() && left == nullptr && right == nullptr)
            return currNode;
        else if (!currNode->isFree() && left != nullptr && right != nullptr && left->isFree() && right->isFree())
        {
            return currNode;
        }
        else
        {
            // the left should have the same offset based on BuddyNode::Split() semantics
            return FindNode(offset, currNode->GetLeft()); 
        }
    }

    if (offset >= currNodeOffset + currNode->GetTrackedSize()/2)
    {
        // the right should always have a greater offset based on BuddyNode::Split() semantics
        return FindNode(offset, currNode->GetRight());
    }
    else
        return FindNode(offset, currNode->GetLeft());


    assert(false);
    return nullptr;
}

DTL::EphemeralRegion::EphemeralRegion(uint64_t region_offset, uint64_t region_size, int config_num, uint64_t physical_backing_start, AGUHardwareStat* hwStat)
    : m_RegionOffset(region_offset), m_ConfigNum(config_num), m_PhysBackingStart(physical_backing_start), m_RegionSize(region_size), hwStat(hwStat)
{
    m_Regionfd = open_fd("/dev/mem");
    assert(m_Regionfd != -1);
    m_DTURuntimeDriverfd = open_fd(DEVICE_FILE);



    /*
        Will want to make this per config ?



        We need the DTU not the AGU config, because the DTU config is where the address indirection comes from
    */
    m_DTUConfigRegion = mmap(NULL, DTU_CONFIG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_Regionfd, DTU_CONFIG_BASE);
    assert(m_DTUConfigRegion != nullptr && m_DTUConfigRegion != MAP_FAILED);


    UPDATE_CONFIG_SIZE(m_DTUConfigRegion, m_ConfigNum, hwStat->nMaxConfigs, m_RegionSize);

    printf("m_PhysBackingStart 0x%llx, region_offset 0x%x\n, region_size 0x%x\n", m_PhysBackingStart, region_offset, region_size);
    UPDATE_CONFIG_START(m_DTUConfigRegion, m_ConfigNum, hwStat->nMaxConfigs, m_RegionOffset + m_PhysBackingStart);
    /*
        We just allocate a region of adequate size and then remap it

        we do not use mmap callback through driver fd because we need to write out the config to the driver,
        if we add this functionality to the driver we can use the mmap callback instead of the remap ioctl
     */
    m_EphemeralRegionAccess = mmap(NULL,
                                    region_size,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS,
                                    -1,
                                    0);
    assert(m_EphemeralRegionAccess != nullptr && m_EphemeralRegionAccess != MAP_FAILED);
    if(Sync() == -1)
    {
        printf("Failed to initialize EphemeralRegion()\n");
        return;
    }

    




    assert(m_EphemeralRegionAccess != nullptr && m_EphemeralRegionAccess != MAP_FAILED);

    //m_CurrentEphemeralPhysicalAddr = va_to_pa(m_EphemeralRegionAccess);
    //printf("Got m_EphemeralRegionAccess PA = 0x%llx\n", m_CurrentEphemeralPhysicalAddr);
    //UPDATE_CONFIG_PHYSMAP(m_DTUConfigRegion, m_ConfigNum, hwStat->nMaxConfigs, m_CurrentEphemeralPhysicalAddr);


    //int syncret = Sync();
    //if (syncret != 0)
    //{
    //    printf("DTL::EphemeralRegion::EphemeralRegion() %d\n", syncret);
    //}



    m_UncachedRegionAccess = mmap(NULL, DTU_UNCACHED_REGION_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, m_Regionfd, region_offset + DTU_UNCACHED_REGION_ADDR);
    assert(m_UncachedRegionAccess != nullptr && m_UncachedRegionAccess != MAP_FAILED);






}

DTL::EphemeralRegion::~EphemeralRegion()
{
    munmap(m_EphemeralRegionAccess, m_RegionSize);
    munmap(m_UncachedRegionAccess, DTU_UNCACHED_REGION_SIZE);
    munmap(m_DTUConfigRegion, DTU_CONFIG_SIZE);
    close(m_Regionfd);
    close(m_DTURuntimeDriverfd);
}



int DTL::EphemeralRegion::Sync() {
  RemapPARequest req;
  req.u_VA = m_EphemeralRegionAccess;
  int ret = ioctl(m_DTURuntimeDriverfd, IOCTL_REMAP_PA, &req);
  if (ret == 0) // if successful, update config mapping
  {
    m_CurrentEphemeralPhysicalAddr = (uint64_t)req.u_NewPA;
    UPDATE_CONFIG_PHYSMAP(m_DTUConfigRegion,
                          m_ConfigNum,
                          hwStat->nMaxConfigs,
                          m_CurrentEphemeralPhysicalAddr);
    printf("new PA 0x%llx\n", m_CurrentEphemeralPhysicalAddr);
  } else
    printf("DTL::EphemeralRegion::Sync() ioctl(IOCTL_REMAP_PA) failed!\n");

  return ret;
}

void *DTL::EphemeralRegion::GetHeadlessReadRegion() {
  return m_EphemeralRegionAccess;
}

void *DTL::EphemeralRegion::GetHeadlessWriteregion()
{
    return m_UncachedRegionAccess;
}

void DTL::EphemeralRegion::GuardedWrite_8(uint64_t offset, uint8_t data)
{
    assert(offset < DTU_UNCACHED_REGION_SIZE);
    WRITE_UINT8(m_UncachedRegionAccess + offset, data);
}

uint8_t DTL::EphemeralRegion::GuardedRead_8(uint64_t offset) {
  assert(offset < m_RegionSize);
  return *((uint16_t *)m_EphemeralRegionAccess);
}

uint16_t DTL::EphemeralRegion::GuardedRead_16(uint64_t offset) {
  assert(offset < m_RegionSize);
  return *((uint16_t *)m_EphemeralRegionAccess);
}

uint32_t DTL::EphemeralRegion::GuardedRead_32(uint64_t offset) {
  assert(offset < m_RegionSize);
  return *((uint32_t *)m_EphemeralRegionAccess);
}

uint64_t DTL::EphemeralRegion::GuardedRead_64(uint64_t offset) {
  assert(offset < m_RegionSize);
  return *((uint64_t *)m_EphemeralRegionAccess);
}

float DTL::EphemeralRegion::GuardedRead_Float(uint64_t offset) {
  assert(offset < m_RegionSize);
  return *((float *)m_EphemeralRegionAccess);
}
