#include <stdio.h>

void puts(const char* str)
{

	const char* string = str;
	int pos = 0;

	while(string[pos])
		putchar(string[pos++]);

	return;
}
