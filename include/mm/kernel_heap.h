#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <lib/stdint.h>

#define HEAP_ALIGNMENT_MASK 0x00FFFFFF

#define BIT(i) (1<<i)

#define HEAP_WITHIN_PAGE BIT(24)
#define HEAP_WITHIN_64K BIT(25)
#define HEAP_CONTINUOUS BIT(31)

// Returns the current end of the heap
void* heap_get_current_end();

// Initiates the kernel heap
void inik_kernel_heap();

// Allocates a chunk of memory at least 'size' bytes large.
void* kernel_malloc(size_t size);

// Allocates a chunk of memory at least 'size' bytes large, aligned at a
// multiplier of 'alignment'
void* kernel_malloc_a(size_t size, uint32_t alignment);

// Allocates a chunk of memory at least 'size' bytes large with a comment 
// stating it's use
void* kernel_malloc_c(size_t size, const char* comment);

// Allocates a chunk of memory at least 'size' bytes large, aligned at a
// multiplier of 'alignment' with a comment stating it's use
void* kernel_malloc_ac(size_t size, uint32_t alignment, const char* comment);

// Frees the memory chunk beginning at addr
void kernel_free(void* addr); 

#endif