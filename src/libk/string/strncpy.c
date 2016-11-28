#include <lib/string.h>

char* strncpy(char* dest, const char* src, size_t n){

	char* s1_p = dest;
	while((*dest++ = *src++) && n-- > 0);
	*dest = '\0'; // Null terminate
	return s1_p;
}