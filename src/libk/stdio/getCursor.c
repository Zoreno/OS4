#include <lib/stdio.h>
#include <monitor/monitor.h>

void getCursor(uint32_t* x, uint32_t* y){
	monitor_getCursor(x, y);
}