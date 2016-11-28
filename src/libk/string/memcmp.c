#include <lib/string.h>

int memcmp(const void* aptr, const void* bptr, size_t bytes){
	const uint8_t* a = (const uint8_t*) aptr;
	const uint8_t* b = (const uint8_t*) bptr;

	for(size_t i = 0; i < bytes; ++i){
		if(a[i] < b[i])
			return -1;
		else if(b[i] < a[i])
			return 1;
	}
	return 0;
}