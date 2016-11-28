#include <lib/stdio.h>

#include <lib/string.h>
#include <lib/stdarg.h>

int serial_printf(COM_port port, const char* format, ...){
	// Allocate a buffer
	char buf[256] = {0};

	va_list ap;

	va_start(ap, format);

	vsprintf(buf, format, ap);

	serial_puts(port, buf);

	va_end(ap);
}