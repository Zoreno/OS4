#include <lib/string.h>

void* memset(void* bufptr, uint8_t value, size_t bytes){
	uint8_t* buf = (uint8_t*) bufptr;
	for(size_t i = 0; i < bytes; ++i){
		buf[i] = value;
	}
	return bufptr;
}
