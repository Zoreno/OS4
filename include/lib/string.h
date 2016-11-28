#ifndef _STRING_H
#define _STRING_H

#include "size_t.h"
#include "stdint.h"
#include "va_list.h"
#include "ctype.h"

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define clamp(minimum, x, maximum) (max(min(x, maximum), minimum))

// Memory management
int memcmp(const void* aptr, const void* bptr, size_t bytes);
void* memcpy(void* dest, const void* src, size_t bytes);
void* memmove(void* dest, const void* src, size_t bytes);
void* memset(void* bufptr, uint8_t value, size_t bytes);
void* memsetl(void* bufptr, uint32_t value, size_t dwords);

// String manupulation
size_t strlen(const char* str);
char* strcpy(char* dest, const char* src);
char* strcat(char* dest, const char* src); 
char* strchr(const char* str, char c);
char* strncat(char* dest, const char* src, size_t n); // NI
char* strncpy(char* dest, const char* src, size_t n);
int strcmp (const char* str1, const char* str2);
int strcicmp(const char* str1, const char* str2);
int strncmp(const char* str1, const char* str2, size_t n);
int strncicmp(const char* str1, const char* str2, size_t n);

// Int to array (c-string)
void itoa(uint32_t i, uint32_t base, char* buf);
void itoa_s(int32_t i, uint32_t base, char* buf);

// Array to Int
int atoi(const char* str);

// String to long
long strtol(const char* nptr, char** endptr, int base);
unsigned long strtoul(const char* nptr, char** endptr, int base);

// String formatting
int vsprintf(char* str, const char* format, va_list ap);

// Memory allocation
void* kmalloc(size_t size);
void* kmalloc_a(size_t size, size_t alignment);
void* kmalloc_c(size_t size, const char* comment);
void* kmalloc_ac(size_t size, size_t alignment, const char* comment);
void kfree(void* addr);

#endif
