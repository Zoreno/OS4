#include <lib/string.h>

int strncmp(const char* str1, const char* str2, size_t n){

	// Sanity check
	if(n == 0) 
		return 0;

	for(;*str1 && (n > 1) && (*str1++ == *str2++);--n);
	return (*str1 - *str2);
}