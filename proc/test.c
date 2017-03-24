#include "test.h"

void do_syscall()
{
	asm volatile ("push %eax");
	asm volatile ("push %ebx");
	asm volatile ("push %ecx");
	asm volatile ("push %edx");
	asm volatile ("mov $0xD, %eax");
	asm volatile ("mov $0x37, %ebx");
	asm volatile ("mov $0xFF, %ecx");
	asm volatile ("mov $0xFE, %edx");
	asm volatile ("int $0x80" ::: "eax", "ebx", "ecx", "edx");
	asm volatile ("pop %edx");
	asm volatile ("pop %ecx");
	asm volatile ("pop %ebx");
	asm volatile ("pop %eax");
}
