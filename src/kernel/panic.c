#include <kernel/panic.h>
#include <hal/hal.h>
#include <lib/stdarg.h>
#include <lib/stdio.h>

void kernel_panic (const char* fmt, ...){

	disable();

	va_list args;
	
	char buf[256] = {0};

	va_start(args, fmt);

	vsprintf(buf, fmt, args);

	va_end(args);

	printf("[KERNEL_PANIC]: %s\n", buf);
}
