#include "dtl_api.hpp"
#include "util.hpp"
#define ERR(msg) (std::cerr << msg << std::endl)



DTL::API::API(uint64_t realBackingStart, uint64_t realBackingSize) : m_ConfigBitmap(0)
{
    ReadHardwareInfo();


    assert(is_power_of_two(realBackingSize) && realBackingSize%Megabyte == 0);
    assert(realBackingSize > Megabyte);
    m_BuddyAllocator = new BuddyAllocator(realBackingSize);



}

DTL::API::API(AGUHardwareStat *hwStat, uint64_t realBackingStart, uint64_t realBackingSize) 
    : ControlBaseAddress(DTU_CONFIG_BASE), hwStat(hwStat), m_ConfigBitmap(0)
{

    assert(is_power_of_two(realBackingSize) && realBackingSize%Megabyte == 0);
    assert(realBackingSize > Megabyte);
    m_BuddyAllocator = new BuddyAllocator(realBackingSize);
}

DTL::API::~API() {
    
}

void DTL::API::ReadHardwareInfo()
{
    std::cerr << "DTL::API::ReadHardwareInfo() not yet implemented" << std::endl;
    assert(false);
}

bool DTL::API::CompileAndProgramHardware(const std::string &dtlProgram) 
{
    if (!Compile(dtlProgram))
        return false;
    return ProgramHardware();
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
    writeTokenStream("./aguconfig", "tokens.out");


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
    root = static_cast<DTL::ProgramNode*>(DTL::ASTTransformPass::Transform(root));
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

    return true;
}

bool DTL::API::ProgramHardware()
{
    
    ralloc->DoInitStateRegisters(ControlBaseAddress);
    printf("FinishInitStateRegs\n");
    ralloc->DoControlWrites(ControlBaseAddress);
    printf("Finished Control Writes\n");
    return true;
}

DTL::EphemeralRegion *DTL::API::AllocEphemeralRegion(uint64_t size_needed)
{

    /*
        We allocate a region offset from the buddy allocator
    */
    uint64_t region_offset = AllocateRegion(size_needed);
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
    auto ephemeral = new EphemeralRegion(region_offset, next_power_of_two(size_needed), config, m_RealBackingStart, hwStat);
    return ephemeral;
}

uint64_t DTL::API::GetError()
{
    return m_ErrorCode;
}

uint64_t DTL::API::AllocateRegion(uint64_t size) 
{
  assert(size < m_RealBackingSize);
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
}

void DTL::API::SetError(uint64_t Error)
{
    m_ErrorCode = Error;
}

void DTL::API::SetControlBaseAddr(uint64_t newControlBaseAddr)
{
    ControlBaseAddress = newControlBaseAddr;
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
    m_RootNode = new BuddyNode(region_size, 0, nullptr);
}

void DTL::BuddyAllocator::FreeNode(uint64_t offset) 
{
    BuddyNode* node = FindNode(offset, m_RootNode);
    assert(node != nullptr);
    
}

uint64_t DTL::BuddyAllocator::AllocNode(uint64_t size_needed) {
  uint64_t AllocatedOffset = FindAndAllocFreeNode(size_needed, m_RootNode);
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
                return check_left;
            return FindAndAllocFreeNode(size_needed, currentNode->GetRight());
        }
    }
    else
    {
        uint64_t check_left = FindAndAllocFreeNode(size_needed, currentNode->GetLeft());
        if (check_left != BUDDY_ALLOC_FAILURE)
            return check_left;
        return FindAndAllocFreeNode(size_needed, currentNode->GetRight());
    }
    return BUDDY_ALLOC_FAILURE;
}

DTL::BuddyNode *DTL::BuddyAllocator::FindNode(uint64_t offset, BuddyNode* currNode) 
{
    if (currNode == nullptr)
        return nullptr;

    auto currNodeOffset = currNode->GetTrackedOffset();
    assert(offset >= currNodeOffset); // we should never be less than
    

    if (offset == currNodeOffset)
    {
        if (!currNode->isFree() && currNode->GetLeft() == nullptr && currNode->GetRight() == nullptr)
            return currNode;
        else
        {
            // the left should have the same offset based on BuddyNode::Split() semantics
            return FindNode(offset, currNode->GetLeft()); 
        }
    }

    if (offset > currNodeOffset)
    {
        // the right should always have a greater offset based on BuddyNode::Split() semantics
        return FindNode(offset, currNode->GetRight());
    }


    assert(false);
    return nullptr;
}

DTL::EphemeralRegion::EphemeralRegion(uint64_t region_offset, uint64_t region_size, int config_num, uint64_t physical_backing_start, AGUHardwareStat* hwStat)
    : m_RegionOffset(region_offset), m_ConfigNum(config_num), m_PhysBackingStart(physical_backing_start), m_RegionSize(region_size), hwStat(hwStat)
{
    m_Regionfd = open_fd("/dev/mem");
    assert(m_Regionfd != -1);
    m_DTURuntimeDriverfd = open_fd(DEVICE_FILE);


    m_DTUConfigRegion = mmap(NULL, DTU_CONFIG_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_Regionfd, DTU_CONFIG_BASE);
    assert(m_DTUConfigRegion != nullptr);



    /*
        We just allocate a region of adequate size and then remap it

        we do not use mmap callback through driver fd because we need to write out the config to the driver,
        if we add this functionality to the driver we can use the mmap callback instead of the remap ioctl
     */
    m_EphemeralRegionAccess = mmap(NULL,
                                    region_size,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    m_Regionfd,
                                    0);
    assert(m_EphemeralRegionAccess != nullptr);
    int syncret = Sync();
    if (syncret != 0)
    {
        printf("DTL::EphemeralRegion::EphemeralRegion() %d\n", syncret);
    }
}

DTL::EphemeralRegion::~EphemeralRegion()
{
    munmap(m_EphemeralRegionAccess, m_RegionSize);
    close(m_Regionfd);
}

int DTL::EphemeralRegion::Sync()
{
    RemapPARequest req;
    req.u_VA = m_EphemeralRegionAccess;
    int ret = ioctl(m_DTURuntimeDriverfd, IOCTL_REMAP_PA, &req);
    if (ret == 0) // if successful, update config mapping
    {
        m_CurrentEphemeralPhysicalAddr = (uint64_t)req.u_NewPA;
        UPDATE_CONFIG_PHYSMAP(m_DTUConfigRegion, m_ConfigNum, hwStat->nMaxConfigs, m_CurrentEphemeralPhysicalAddr);
    }


    return ret;
}

void *DTL::EphemeralRegion::GetHeadlessRegion() {
  return m_EphemeralRegionAccess;
}

uint8_t DTL::EphemeralRegion::GuardedRead_8(uint64_t offset)
{
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

int DTL::EphemeralRegion::open_fd(const std::string& path) {
  int fd = open(path.c_str(), O_RDWR | O_SYNC);
  if (fd == -1) {
    printf("Can't open /dev/mem.\n");
    exit(0);
  }
  return fd;
}
