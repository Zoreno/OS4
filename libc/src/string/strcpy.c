#include <string.h>

char* strcpy(char* s1, const char* s2)
{
	for(size_t i = 0; i <= strlen(s2); ++i)
	{
		s1[i] = s2[i];
	}

	return s1;
}
