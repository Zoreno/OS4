#include <lib/stdint.h>
#include <lib/stdio.h>
#include <proc/task.h>

#define DEBUG_SYSCALL 1

void syscall_handler(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx)
{
#if DEBUG_SYSCALL
	printf("EAX: %#x, EBX: %#x, ECX: %#x, EDX: %#x\n", eax, ebx, ecx, edx);
#endif

	switch(eax)
	{
	case 0:
	{
		putch((char)ebx);
	}
	break;
	case 1:
	{
		TerminateProcess((int)ebx);
	}
	break;
	default:
		printf("Invalid syscall instruction");
	}
}
