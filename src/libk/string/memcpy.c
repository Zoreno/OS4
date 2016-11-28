#include <lib/string.h>

void* memcpy(void* dest, const void* src, size_t bytes){
	uint8_t* destPtr = (uint8_t*) dest;
	const uint8_t* srcPtr = (const uint8_t*) src;

	for(size_t i = 0; i < bytes; ++i){
		destPtr[i] = srcPtr[i];
	}

	return dest;
}