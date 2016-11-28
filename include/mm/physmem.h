/** @file physmem.h
 *  @brief Function prototypes for the physical memory manager.
 *
 *  Defines the interface for the physical memory manager.
 *
 *  @author Joakim Bertils
 */

#ifndef _PHYSMEM_H
#define _PHYSMEM_H

#include <lib/stdint.h>

/** @brief Type definition of a physical address
 *
 *  An physical address is represented as 32 bit unsigned integer, which is the
 *	natural representation of an address on a 32 bit computer. This type
 *	definition serves mainly as a distinction between physical and virtual
 *	addresses.
 */
typedef uint32_t physical_addr;


/** @brief Initializes the physical memory manager
 *
 *  Initializes the physical memory manager with given memory size and a 
 *	location where the bitmap is going to be stored. When the manager is
 *	initialized, all memory will be treated as used until we initialize a
 *	region where we have available memory to use.
 *
 *  @param memsize 		Size of memory
 *	@param bitmap		Address where the PMM can store the bitmap
 *  @return The input character
 */
void pmmngr_init(size_t memsize, physical_addr bitmap);


/** @brief Initializes a region for the physical memory manager to use.
 *
 *  Marks a given region as available for use by the PMM. The start address and
 *	the size are rounded down to nearest multiple of the block size used in
 *	the manager.
 *
 *  @param base		Base address of the region
 *  @param size 	Size of the region to be initiated.
 */
void pmmngr_init_region(physical_addr base, size_t size);

/** @brief Deinitializes a region for the physical memory manager not to use.
 *
 *  Marks a given region as unavailable for use by the PMM. The start address 
 *	and the size are rounded down to nearest multiple of the block size used in
 *	the manager.
 *
 *  @param base		Base address of the region
 *  @param size 	Size of the region to be deinitiated.
 */
void pmmngr_deinit_region(physical_addr base, size_t size);

/** @brief Allocates a block of memory
 *
 *	Allocates a block from the memory available in the PMM.	If the allocation
 *	fails, the returned value will be 0.
 *
 *	@return 		Address of start of allocated block if successful, else 0.
 */
void* pmmngr_alloc_block();

/** @brief Allocates a zeroed block of memory
 *
 *	Allocates a block from the memory available in the PMM and sets all bytes 
 *  in memory range to 0. If the allocation fails, the returned value will be 0.
 *
 *	@return 		Address of start of allocated block if successful, else 0.
 */
void* pmmngr_alloc_block_z();

void pmmngr_free_block(void* p);

/** @brief Allocates a series of blocks of memory
 *
 *	Allocates given number of blocks contiguous in physical memory. If the
 *	allocation fails, the returned value will be 0.
 *
 *	@return 		Address of start of allocated blocks if successful, else 0.
 */
void* pmmngr_alloc_blocks(size_t size);

/** @brief Allocates a series of zeroed blocks of memory
 *
 *	Allocates given number of blocks contiguous in physical memory and sets all
 *	bytes in memory range to 0. If the allocation fails, the returned value 
 *  will be 0.
 *
 *	@return 		Address of start of allocated blocks if successful, else 0.
 */
void* pmmngr_alloc_blocks_z(size_t size);

void pmmngr_free_blocks(void* p, size_t size);

size_t pmmngr_get_memory_size();

uint32_t pmmngr_get_block_count();

uint32_t pmmngr_get_use_block_count();

uint32_t pmmngr_get_free_block_count();

uint32_t pmmngr_get_block_size();

void pmmngr_paging_enable(int b);

int pmmngr_is_paging();

void pmmngr_load_PBDR(physical_addr addr);

physical_addr pmmngr_get_PBDR();

#endif