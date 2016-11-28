#include <lib/string.h>

char* strcat(char* dest, const char* src){
	// Copy source string to the end of the existing dest string
	strcpy(dest + strlen(dest), src);
	return dest;
}