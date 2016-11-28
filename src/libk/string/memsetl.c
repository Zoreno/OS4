#include <lib/string.h>

void* memsetl(void* bufptr, uint32_t value, size_t dwords){
	uint32_t* buf = (uint32_t*) bufptr;
	for(size_t i = 0; i < dwords; ++i){
		buf[i] = value;
	}
	return bufptr;
}
