#include <lib/string.h>

char* strchr(const char* str, char c){

	do{
		if(*str == c)
			return (char*) str;
	} while(*str++);

	return 0;
}