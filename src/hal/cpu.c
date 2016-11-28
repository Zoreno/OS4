#include <hal/cpu.h>
#include <hal/gdt.h>
#include <hal/idt.h>

static void cpuid(int code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d);

static inline void cpuid(int code, uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d)
{
    asm volatile ( "cpuid" : "=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d): "0"(code));
}

int i86_cpu_initialize(){
	i86_gdt_initialize();
	i86_idt_initialize(0x8);

	return 0;
}

void i86_cpu_shutdown(){
	// Do nothing yet.
}

const char* i86_cpu_get_vendor(){
	static char vendor[13] = {0};

	union{
		uint32_t reg;
		char byte[4]; 
	}eax;

	union{
		uint32_t reg;
		char byte[4]; 
	}ebx;

	union{
		uint32_t reg;
		char byte[4]; 
	}ecx;

	union{
		uint32_t reg;
		char byte[4]; 
	}edx;

	cpuid(0, &(eax.reg), &(ebx.reg), &(ecx.reg), &(edx.reg));

	for(int i = 0; i < 4; ++i)
	{
		vendor[i] = ebx.byte[i];
		vendor[i+4] = edx.byte[i];
		vendor[i+8] = ecx.byte[i];
	}

	return (const char*) vendor;

}