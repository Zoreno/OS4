#include <lib/string.h>

void* memmove(void* dest, const void* src, size_t bytes){
	uint8_t* destPtr = (uint8_t*) dest;
	const uint8_t* srcPtr = (const uint8_t*) src;

	if(destPtr < srcPtr){
		for(size_t i = 0; i < bytes; ++i){
			destPtr[i] = srcPtr[i];
		}
	}else{ 	// 	If destination is after source in memory, we revese copying 
			//	to ensure that memory is not overwritten before it is copied.
		for(size_t i = bytes; i != 0; --i){
			destPtr[i-1] = srcPtr[i-1];
		}
	}
	return dest;
}