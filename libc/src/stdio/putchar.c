#include <stdio.h>

void putchar(char c)
{
	// Push frame on stack
	asm volatile ("pushal");

	// Make sure that only the last 8 bits are set.
	unsigned int ch = c & 0x000000FF;

	// Move instruction 0 into eax
	asm volatile ("mov $0, %eax");

	// Move char code into ebx.
	asm volatile ("mov %0, %%ebx" :: "g"(ch));

	// Move 0 into ecx
	asm volatile ("mov $0, %ecx");

	// Move 0 into edx
	asm volatile ("mov $0, %edx");

	// Do the interrupt
	asm volatile ("int $0x80");

	// Restore frame
	asm volatile ("popal");
}

