#include <lib/stdio.h>

void serial_putch(COM_port port, const char c)
{
	if (!c)
		return;
	serial_send_data(port, c);
}