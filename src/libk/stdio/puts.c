#include <lib/stdio.h>

#include <monitor/monitor.h>

int puts(const char* s){
	monitor_puts(s);
}