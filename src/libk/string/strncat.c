#include <lib/string.h>

char* strncat(char* dest, const char* src, size_t n){
	strncpy(dest + strlen(dest), src, n);
	return dest;
}