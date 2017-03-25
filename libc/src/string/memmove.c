#include <string.h>

void* memmove_forward(unsigned char* dest, const unsigned char* src, size_t n)
{
	for(size_t i = 0; i < n; ++i)
	{
		dest[i] = src[i];
	}
}

void* memmove_backward(unsigned char* dest, const unsigned char* src, size_t n)
{
	for(size_t i = n - 1; i >= 0; --i)
	{
		dest[i] = src[i];
	}
}

void* memmove(void* dest, const void* src, size_t n)
{
	unsigned char* destPtr = (unsigned char*) dest;
	const unsigned char* srcPtr = (const unsigned char*) src;

	if(destPtr < srcPtr)
	{
		memmove_forward(destPtr, srcPtr, n);
	}
	else if(destPtr > srcPtr)
	{
		memmove_backward(destPtr, srcPtr, n);
	}

	return dest;
}