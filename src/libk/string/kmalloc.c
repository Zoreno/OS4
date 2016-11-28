#include <lib/string.h>

#include <mm/kernel_heap.h>

void* kmalloc(size_t size){
	return kernel_malloc(size);
}

void* kmalloc_a(size_t size, size_t alignment){
	return kernel_malloc_a(size, alignment);
}

void* kmalloc_c(size_t size, const char* comment){
	return kernel_malloc_c(size, comment);
}

void* kmalloc_ac(size_t size, size_t alignment, const char* comment){
	return kernel_malloc_ac(size, alignment, comment);
}

void kfree(void* addr){
	kernel_free(addr);
}
