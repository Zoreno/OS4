#include <string.h>

void* memchr(const void* str, int c, size_t n)
{
	unsigned char ch = (unsigned char) c;

	const unsigned char* string = (const unsigned char*) str;

	for(size_t i = 0; i < n; ++n)
	{
		if(!string[i])
			break;

		if(string[i] == ch)
			return &string[i];
	}

	return NULL;
}