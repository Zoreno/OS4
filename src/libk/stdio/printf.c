#include <lib/stdio.h>
#include <lib/string.h>
#include <lib/stdarg.h>

#include <monitor/monitor.h>

int printf(const char* format, ...){
	// Allocate a buffer
	char buf[256] = {0};

	va_list ap;

	va_start(ap, format);

	vsprintf(buf, format, ap);

	monitor_puts(buf);

	va_end(ap);
}