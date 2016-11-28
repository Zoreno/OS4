#include <lib/stdio.h>

#include <lib/string.h>

void serial_puts(COM_port port, const char* str)
{
	if (!str)
		return;

	for (size_t i = 0; i < strlen(str); i++)
		serial_putch(port, str[i]);
}