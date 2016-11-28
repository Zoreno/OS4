#include <lib/string.h>

int strncicmp(const char* str1, const char* str2, size_t n){

	// Sanity check
	if(n == 0) 
		return 0;

	for(;*str1 && (n > 1) && (tolower(*str1++) == tolower(*str2++));--n);
	return (tolower(*str1) - tolower(*str2));
}