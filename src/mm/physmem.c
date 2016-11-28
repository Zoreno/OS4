/** @file physmem.c
 *  @brief Physical memory manager.
 *
 *	Implementation of the physical memory manager done with bitmap-based
 *	approach.
 *
 *  @author Joakim Bertils
 */

#include <mm/physmem.h>

#include <lib/string.h>
#include <lib/stdio.h>

#define PMMNGR_BLOCKS_PER_BYTE 8
#define PMMNGR_BLOCK_SIZE 4096
#define PMMNGR_BLOCK_ALIGN PMMNGR_BLOCK_SIZE

static uint32_t _mmngr_memory_size = 0;
static uint32_t _mmngr_used_blocks = 0;
static uint32_t _mmngr_max_blocks = 0;
static uint32_t* _mmngr_memory_map = 0;

// 4GB Physical address space. 32 4k blocks per entry
//static uint32_t _mmngr_memory_map[0xFFFFFFFF/(4096*32)] = {0};

//===================================================================
// Bitmap manipulation
//===================================================================

void mmap_set(const int bit);
void mmap_unset(const int bit);
int mmap_test(const int bit);
int mmap_first_free();
int mmap_first_free_s(size_t size);

/*
	
	_mmngr_memory_map:

	|            Entry 0               |             Entry 1               |etc.
	00000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000 

	Entry contains 32 bits, each representing a block of memory. Total number of
	bits is equal to the number of 4kb blocks the kernel has detected

*/

#define BITS_PER_ENTRY 32

void mmap_set(const int bit){
	_mmngr_memory_map[bit/BITS_PER_ENTRY] |= (1<<(bit%BITS_PER_ENTRY));
}

void mmap_unset(const int bit){
	_mmngr_memory_map[bit/BITS_PER_ENTRY] &= ~(1<<(bit%BITS_PER_ENTRY));
}

int mmap_test(const int bit){
	return _mmngr_memory_map[bit/BITS_PER_ENTRY] & (1<<(bit%BITS_PER_ENTRY));
}

int mmap_first_free(){

	for(uint32_t entry = 0; 
		entry < pmmngr_get_block_count() / BITS_PER_ENTRY; 
		++entry){

		// If the entry is full, go to the next one
		if(_mmngr_memory_map[entry] == 0xFFFFFFFF)
			continue;

		// Check each bit in the entry
		for(int bit = 0; bit < BITS_PER_ENTRY;  ++bit){
			if(!(_mmngr_memory_map[entry] & (1<<bit))){
				return entry * BITS_PER_ENTRY + bit;
			}
		}
	}

	return -1;
}

int mmap_first_free_s(size_t size){

	uint32_t firstFree = mmap_first_free();

	// We could not find any free blocks.
	if(firstFree == -1)
		return -1;

	// Don't allocate if there is nothing to allocate
	if(size == 0)
		return -1;

	// If we only want to allocate one block, we can pick first free
	if(size == 1)
		return firstFree;

	// Number of free contiguous blocks found.
	uint32_t free = 0;

	// First free bit of chain
	uint32_t first_bit_in_chain = firstFree;

	// Loop through each bit from firstFree to end of memory
	for(int bit = firstFree; bit < pmmngr_get_block_count(); ++bit){
		// If bit is set (memory not free)
		if(mmap_test(bit)){
			// Reset counter
			free = 0;

			// Next bit is potential new start of chain.
			first_bit_in_chain = bit + 1;
		} else {
			// Bit is free. Increase counter
			++free;
		}

		// If we found enough bits, we are done
		if(free == size)
			return first_bit_in_chain;
	}

	//We went through all blocks in memory and did not find a chain
	return -1;
}

//===================================================================
// Physical memory manager
//===================================================================


void pmmngr_init(size_t memsize, physical_addr bitmap){

	_mmngr_memory_size = memsize;
	_mmngr_memory_map = (uint32_t*)bitmap;
	_mmngr_max_blocks = (pmmngr_get_memory_size()*1024)/PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks = _mmngr_max_blocks;

	printf("PMM Bitmap Size: %i\n", pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE);

	//memset(
	//	_mmngr_memory_map,  
	//	0xFF, 
	//	pmmngr_get_block_count() / PMMNGR_BLOCKS_PER_BYTE); 

	int entries = pmmngr_get_block_count() / BITS_PER_ENTRY;

	for(int i = 0; i < entries; ++i){
		
		_mmngr_memory_map[i] = 0xFFFFFFFF;
		//printf("%p\n", &_mmngr_memory_map[i]);
	}

	printf("PMM init. with bitmap at: %#p, (Entries: %i)\n",_mmngr_memory_map, entries);
}

void pmmngr_init_region(physical_addr base, size_t size){

	int align = (base - 0xC0000000)/ PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;

	printf("[PHYSMEM] Initiating region with size: %i\n", size);
	
	while(blocks-->=0){
		//printf("[PHYSMEM] Align: %#x, Blocks:%i\n", align, blocks);
		mmap_unset(align++);
		_mmngr_used_blocks--;
	}

	//asm volatile("xchgw %bx, %bx");

	mmap_set(0);
}

void pmmngr_deinit_region(physical_addr base, size_t size){

	int align = (base - 0xC0000000)/ PMMNGR_BLOCK_SIZE;
	int blocks = size / PMMNGR_BLOCK_SIZE;

	//printf("[PHYSMEM] Deinitiating region\n");

	for(;blocks>=0; blocks--){
		mmap_set(align++);
		_mmngr_used_blocks++;
	}
}

void* pmmngr_alloc_block(){

	// Check if we have memory left
	if(pmmngr_get_block_count() <= 0){
		printf("[PHYSMEM] No memory left(1)\n");
		return 0;
	}
		

	int frame = mmap_first_free();

	// Out of memory
	if(frame == -1){
		printf("\n[PHYSMEM] No memory left(2)\n");
		return 0;
	}
		

	// Mark block as used
	mmap_set(frame);

	physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks++;

	// Return address of block start
	return (void*) addr;
}

void* pmmngr_alloc_block_z(){

	// Check if we have memory left
	if(pmmngr_get_block_count() <= 0)
		return 0;

	int frame = mmap_first_free();

	// Out of memory
	if(frame == -1)
		return 0;

	// Mark block as used
	mmap_set(frame);

	physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks++;

	memset(addr, 0x00, PMMNGR_BLOCK_SIZE);

	// Return address of block start
	return (void*) addr;
}

void pmmngr_free_block(void* p){

	// Calculate frame of the address
	physical_addr addr = (physical_addr)p;
	int frame = addr / PMMNGR_BLOCK_SIZE;

	// Mark as free
	mmap_unset(frame);

	_mmngr_used_blocks--;
}

void* pmmngr_alloc_blocks(size_t size){

	// Out of memory
	if(pmmngr_get_block_count() <= size)
		return 0;

	int frame = mmap_first_free_s(size);

	// Out of memory
	if(frame == -1)
		return 0;

	for(uint32_t i = 0; i < size; ++i){
		mmap_set(frame + i);
	}

	physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks += size;

	return (void*) addr;
}

void* pmmngr_alloc_blocks_z(size_t size){

	// Out of memory
	if(pmmngr_get_block_count() <= size)
		return 0;

	int frame = mmap_first_free_s(size);

	// Out of memory
	if(frame == -1)
		return 0;

	for(uint32_t i = 0; i < size; ++i){
		mmap_set(frame + i);
	}

	physical_addr addr = frame * PMMNGR_BLOCK_SIZE;
	_mmngr_used_blocks += size;

	memset(addr, 0x00, PMMNGR_BLOCK_SIZE * size);

	return (void*) addr;
}

void pmmngr_free_blocks(void* p, size_t size){

	// Calculate start frame from the address
	physical_addr addr = (physical_addr)p;
	int frame = addr / PMMNGR_BLOCK_SIZE;

	for(uint32_t i = 0; i < size; ++i){
		mmap_unset(frame + i);
	}

	_mmngr_used_blocks -= size;
}

size_t pmmngr_get_memory_size(){
	return _mmngr_memory_size;
}

uint32_t pmmngr_get_block_count(){
	return _mmngr_max_blocks;
}

uint32_t pmmngr_get_use_block_count(){
	return _mmngr_used_blocks;
}

uint32_t pmmngr_get_free_block_count(){
	return _mmngr_max_blocks - _mmngr_used_blocks;
}

uint32_t pmmngr_get_block_size(){
	return PMMNGR_BLOCK_SIZE;
}

void pmmngr_paging_enable(int b){

	uint32_t cr0_reg;

	// Fetch data in cr0
	asm volatile ("mov %%cr0, %0" : "=r"(cr0_reg));

	
	if(b){ // If we should enable paging
		cr0_reg |= 0x80000000;
	} else {// If we should disable paging
		cr0_reg &= 0x7FFFFFFF;
	}

	// Store new data in cr0
	asm volatile ("mov %0, %%cr0" :: "r"(cr0_reg));
}

int pmmngr_is_paging(){

	uint32_t cr0_reg;

	// Fetch data in cr0
	asm volatile ("mov %%cr0, %0" : "=r"(cr0_reg));

	return (cr0_reg & 0x80000000) ? 1 : 0;
}

void pmmngr_load_PBDR(physical_addr addr){
	//asm volatile ("mov %0, %%cr3" :: "r"(addr));

	asm volatile ("movl %0,%%eax"::"r"(addr):"%eax"); // Move addr to eax
    asm volatile ("movl %%eax,%%cr3":::"%eax"); // move eax to cr3
}

physical_addr pmmngr_get_PBDR(){
	physical_addr PBDR;

	// Fetch data in cr3
	asm volatile ("mov %%cr3, %0" : "=r"(PBDR));

	return PBDR;
}