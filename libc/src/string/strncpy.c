#include <string.h>

char* strncpy(char* s1, const char* s2, size_t n)
{
	size_t i = 0;

	size_t s2len = strlen(s2);

	size_t limit = s2len > n ? n : s2len;

	for(; i < s2len ; ++i)
	{
		s1[i] = s2[i];
	}

	for(; i < n; ++i)
	{
		s1[i] = '\0';
	}

	return s1;
}
