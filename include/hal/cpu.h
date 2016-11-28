#ifndef _CPU_H
#define _CPU_H

#include <lib/stdint.h>

#include "regs.h"

int i86_cpu_initialize();
void i86_cpu_shutdown();

// TODO: expand for all CPUID data
const char* i86_cpu_get_vendor();

#endif