#include <lib/stdio.h>

#include <monitor/monitor.h>

int putch(char c){
	monitor_putch(c);
}