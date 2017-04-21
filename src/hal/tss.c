#include <hal/tss.h>
#include <hal/gdt.h>

#include <lib/stdio.h>

static tss_entry TSS;

extern void flush_tss(uint16_t sel);

void tss_set_stack(uint16_t kernelSS, uint32_t kernelESP)
{
	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
}

void install_tss(uint32_t idx, uint16_t kernelSS, uint32_t kernelESP)
{
	uint32_t base = (uint32_t)&TSS;

	gdt_set_descriptor(
		idx, 
		base, 
		base + sizeof(tss_entry),
		I86_GDT_DESC_ACCESS|I86_GDT_DESC_EXEC_CODE|I86_GDT_DESC_DPL|I86_GDT_DESC_MEMORY,
		0);

	memset((void*)&TSS, 0, sizeof(tss_entry));

	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
	TSS.cs = 0x0B;
	TSS.ss = 0x13;
	TSS.es = 0x13;
	TSS.ds = 0x13;
	TSS.ds = 0x13;
	TSS.fs = 0x13;

	flush_tss(idx * sizeof(gdt_descriptor));
}
