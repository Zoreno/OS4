#include <string.h>

void* memset(void* str, int c, size_t n)
{
	unsigned char* string = (unsigned char*) str;
	unsigned char ch = (unsigned char) c;

	for(size_t i = 0; i < n; ++i)
	{
		string[i] = ch;
	}

	return str;
}