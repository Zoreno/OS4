#include <lib/string.h>

int strcicmp(const char* str1, const char* str2){
	while((*str1) && (tolower(*str1++) == tolower(*str2++)));
	return (tolower(*str1) - tolower(*str2));
}