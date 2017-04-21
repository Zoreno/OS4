#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include "size_t.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define NULL (0)

void *memchr(const void* str, int c, size_t n);
int memcmp(const void* str1, const void* str2, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memset(void* str, int c, size_t n);

size_t strlen(const char* str);
char* strcpy(char* s1, const char* s2);
char* strncpy(char* s1, const char* s2, size_t n);

#if defined(__cplusplus)
} /* extern "C" */
#endif

#endif
