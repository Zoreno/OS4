#include <mm/kernel_heap.h>

#include <lib/string.h>
#include <lib/stdio.h> // For debugging purposes

#include <mm/physmem.h>
#include <mm/virtmem.h>

#define PLACEMENT_BEGIN   0xD0000000U
#define PLACEMENT_END     0xD0200000U

#define KERNEL_HEAP_START 0xD0200000U
#define KERNEL_HEAP_END 0xE0000000U

#define PAGE_SIZE 4096U

typedef struct
{
	uint32_t size;
	uint32_t number;
	uint8_t reserved;
	char comment[21];
} __attribute__((packed)) region_t;

static region_t* regions = 0;
static uint32_t regionCount = 0;
static uint32_t regionMaxCount = 0;
static uint32_t firstFreeRegion = 0;
static void* firstFreeAddr = (void*)KERNEL_HEAP_START;
static const uint8_t* HEAP_START = (const uint8_t*)KERNEL_HEAP_START;
static uint32_t heapSize = 0;
static const uint32_t HEAP_MIN_GROWTH = 0x10000;

uint32_t alignUp(uint32_t val, uint32_t alignment);

uint32_t alignDown(uint32_t val, uint32_t alignment);

void* pmalloc(size_t size, uint32_t alignment);

int heap_grow(size_t size, uint8_t* heapEnd, int continuous);

void* kmalloc_imp(size_t size, uint32_t alignment, const char* comment);

uint32_t alignUp(uint32_t val, uint32_t alignment)
{
	// Sanity Check
	if (!alignment)
	{
		return val;
	}

	--alignment;

	return (val + alignment) & ~alignment;
}

uint32_t alignDown(uint32_t val, uint32_t alignment)
{
	// Sanity Check
	if (!alignment)
	{
		return val;
	}

	return val & ~(alignment - 1);
}

void* pmalloc(size_t size, uint32_t alignment)
{
	static uint8_t* nextPlacement = (uint8_t*)PLACEMENT_BEGIN;

	size = alignUp(size, 4);

	uint8_t* currPlacement = (uint8_t*)alignUp((uint32_t)nextPlacement, alignment);

	if (((uint32_t)currPlacement + size) > PLACEMENT_END)
	{
		return 0;
	}

	nextPlacement = currPlacement + size;

	return currPlacement;
}

int heap_grow(size_t size, uint8_t* heapEnd, int continuous)
{
	//printf("\n[heap_grow] Size:%i, Heap End: %#p, Continuous: %i", size, heapEnd, continuous);

	if ((regionCount > 0) && regions[regionCount - 1].reserved && (regionCount >= regionMaxCount))
	{
		//printf("\nError1");
		return 0;
	}

	uint32_t offset = 0;

	while (offset < size)
	{
		physical_addr addr = pmmngr_alloc_block();

		vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)heapEnd + offset, (uint32_t)addr, I86_PTE_PRESENT | I86_PTE_WRITABLE);

		//printf("\nMapping %#p", heapEnd + offset);

		offset += PAGE_SIZE;
	}

	//printf("\nSize: %i", size);

	if ((regionCount > 0) && !regions[regionCount - 1].reserved)
	{
		regions[regionCount - 1].size += size;
	}
	else
	{

		//printf("\nSize: %i", size);

		//printf("\n Region address: %#p", regions);

		regions[regionCount].reserved = 0;
		regions[regionCount].size = size;
		regions[regionCount].number = 0;

		//printf("\nNew Region: %i", regionCount);
		//printf("\nRegion reserved: %i",(uint32_t) regions[regionCount].reserved);
		//printf("\nRegion Size: %i", (uint32_t) regions[regionCount].size);
		//printf("\nRegion Number: %i", (uint32_t) regions[regionCount].number);

		++regionCount;
	}

	heapSize += size;
	return 1;
}

void* heap_get_current_end()
{
	return (void*)(HEAP_START + heapSize);
}

void init_kernel_heap()
{
	// Map placement area

	uint32_t i = PLACEMENT_BEGIN;

	for (i; i < PLACEMENT_END; i += PAGE_SIZE)
	{
		vmmngr_mapPhysicalAddress(vmmngr_get_directory(), i, (uint32_t)pmmngr_alloc_block(), I86_PTE_PRESENT | I86_PTE_WRITABLE);
	}

	//printf("Placement mapping done.\n");

	regions = (region_t*)pmalloc(0, 0);

	regionCount = 0;
	regionMaxCount = (PLACEMENT_END - (uint32_t)regions) / sizeof(region_t);
}

void* kmalloc_imp(size_t size, uint32_t alignment, const char* comment)
{
	//printf("\n[kmalloc] Size:%i, Alignment: %i, Comment: %s", size, alignment, comment);

	static uint32_t consecutiveNumber = 0;

	size_t within = 0xFFFFFFFF;

	if (alignment & HEAP_WITHIN_PAGE)
	{
		within = PAGE_SIZE;
	}
	else if (alignment & HEAP_WITHIN_64K)
	{
		within = 0x10000;
	}

	int continuous = (alignment & HEAP_CONTINUOUS) ? 1 : 0;

	alignment &= HEAP_ALIGNMENT_MASK;

	// Check if heap is set up.

	if (regions == 0)
	{
		//printf("\nError2");
		return (pmalloc(size, alignment));
	}

	size = alignUp(size, 0);

	int foundFree = 0;

	uint8_t* regionAddress = (uint8_t*)firstFreeAddr;

	for (uint32_t i = firstFreeRegion; i < regionCount;++i)
	{
		//printf("\nRegion Loop: %i", i);
		if (regions[i].reserved)
		{
			//printf("\nFound Free");
			foundFree = 1;
		}
		else if (!foundFree)
		{
			//printf("\nNot Found Free");
			firstFreeRegion = i;
			firstFreeAddr = regionAddress;
		}

		uint8_t* alignedAddress = (uint8_t*)alignUp((uintptr_t)regionAddress, alignment);
		uintptr_t additionalSize = (uintptr_t)alignedAddress - (uintptr_t)regionAddress;

		// Check whether this region is free, big enough and fits page requirements
		if (!regions[i].reserved && (regions[i].size >= size + additionalSize) &&
			(within - (uintptr_t)regionAddress%within >= additionalSize))
		{
			//printf("\nFound Free, big and page");
			// Check if the region consists of continuous physical memory if required
			if (continuous)
			{
				int iscontinuous = 1;
				for (uint32_t virt1 = (uint32_t)alignDown((uintptr_t)alignedAddress, PAGE_SIZE); (uintptr_t)((uint32_t)virt1 + PAGE_SIZE) <= (uintptr_t)(alignedAddress + size); virt1 += PAGE_SIZE)
				{
					uintptr_t phys1 = (uintptr_t)vmmngr_getPhysicalAddress(vmmngr_get_directory(), (uint32_t)virt1);
					uintptr_t phys2 = (uintptr_t)vmmngr_getPhysicalAddress(vmmngr_get_directory(), (uint32_t)virt1 + PAGE_SIZE);
					if (phys1 + PAGE_SIZE != phys2)
					{
						iscontinuous = 0;
						break;
					}
				}
				if (!iscontinuous)
					continue;
			}

			// We will split up this region ...
			// +--------------------------------------------------------+
			// |                      Current Region                    |
			// +--------------------------------------------------------+
			//
			// ... into three, the first and third remain free,
			// while the second gets reserved, and its address is returned
			//
			// +------------------+--------------------------+----------+
			// | Before Alignment | Aligned Destination Area | Leftover |
			// +------------------+--------------------------+----------+

			// Split the pre-alignment area
			if (alignedAddress != regionAddress)
			{
				// Check whether we are able to expand
				if (regionCount >= regionMaxCount)
				{
					//printf("\nError3");
					return (0);
				}

				//printf("\nMoving regions");

				// Move all following regions ahead to get room for a new one
				memmove(regions + i + 1, regions + i, (regionCount - i) * sizeof(region_t));

				++regionCount;

				// Setup the regions
				regions[i].size = alignedAddress - regionAddress;

				regions[i].reserved = 0;

				regions[i + 1].size -= regions[i].size;

				// "Aligned Destination Area" becomes the "current" region
				regionAddress += regions[i].size;

				i++;
			}

			// Split the leftover
			if (regions[i].size > size + additionalSize)
			{

				//printf("\nSplitting leftover");
				// Check whether we are able to expand
				if (regionCount + 1 > regionMaxCount)
				{
					//printf("\nError4");
					return (0);
				}

				//printf("\nMoving regions 2");

				// Move all following regions ahead to get room for a new one
				memmove(regions + i + 2, regions + i + 1, (regionCount - i - 1)*sizeof(region_t));

				++regionCount;

				// Setup the regions
				regions[i + 1].size = regions[i].size - size;
				regions[i + 1].reserved = 0;
				regions[i + 1].number = 0;

				regions[i].size = size;
			}

			//printf("\nSet region to reserved and return");

			// Set the region to "reserved" and return its address
			regions[i].reserved = 1;
			strncpy(regions[i].comment, comment, 20);
			regions[i].comment[20] = 0;
			regions[i].number = ++consecutiveNumber;

			return (regionAddress);

		} //region is free and big enough

		regionAddress += regions[i].size;
	}

	// There is nothing free, try to expand the heap
	uint32_t sizeToGrow = max(HEAP_MIN_GROWTH, alignUp(size * 3 / 2, PAGE_SIZE));

	int success = heap_grow(sizeToGrow, (uint8_t*)((uintptr_t)HEAP_START + heapSize), continuous);

	if (!success)
	{
		//printf("\nError5");
		return (0);
	}

	// Now there should be a region that is large enough
	return kmalloc_imp(size, alignment, comment);
}

void* kernel_malloc(size_t size){
	return kmalloc_imp(size, 0, "None");
}

void* kernel_malloc_a(size_t size, uint32_t alignment){
	return kmalloc_imp(size, alignment, "None");
}

void* kernel_malloc_c(size_t size, const char* comment){
	return kmalloc_imp(size, 0, comment);
}

void* kernel_malloc_ac(size_t size, uint32_t alignment, const char* comment){
	return kmalloc_imp(size, alignment, comment);
}

void kernel_free(void* addr)
{
	if (addr == 0)
	{
		return;
	}

	// Walk the regions and find the correct one
	uint8_t* regionAddress = (uint8_t*)HEAP_START;
	for (uint32_t i = 0; i < regionCount; i++)
	{
		if (regionAddress == addr && regions[i].reserved)
		{
			regions[i].number = 0;
			regions[i].reserved = 0; // free the region

										 // Check for a merge with the next region
			if ((i + 1 < regionCount) && !regions[i + 1].reserved)
			{
				// Adjust the size of the now free region
				regions[i].size += regions[i + 1].size; // merge

														// Move all following regions back by one
				memmove(regions + i + 1, regions + i + 2, (regionCount - 2 - i)*sizeof(region_t));

				--regionCount;
			}

			// Check for a merge with the previous region
			if (i > 0 && !regions[i - 1].reserved)
			{
				// Adjust the size of the previous region
				regions[i - 1].size += regions[i].size; // merge

														// Move all following regions back by one
				memmove(regions + i, regions + i + 1, (regionCount - 1 - i)*sizeof(region_t));

				--regionCount;
			}

			if (i < firstFreeRegion)
			{
				firstFreeRegion = i;
				firstFreeAddr = regionAddress;
			}
			return;
		}

		regionAddress += regions[i].size;
	}
}