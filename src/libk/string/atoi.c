#include <lib/string.h>

int atoi(const char* str){

	return (int) strtol(str, 0, 10);
}