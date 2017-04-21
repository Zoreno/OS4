#include <hal/gdt.h>
#include <lib/string.h>
#include <lib/stdio.h>

typedef struct {
	uint16_t	m_limit;
	uint32_t	m_base;
} __attribute__((packed)) gdtr;

gdt_descriptor _gdt[MAX_DESCRIPTORS];
gdtr _gdtr;

void gdt_set_descriptor(
	uint32_t i, 
	uint64_t base, 
	uint64_t limit, 
	uint8_t access, 
	uint8_t gran){

	if(i > MAX_DESCRIPTORS)
		return;

	// Set memory to 0
	memset((void*)&_gdt[i],0,sizeof(gdt_descriptor));

	// set limit and base addresses
	_gdt[i].baseLo	= (uint16_t)(base & 0xffff);
	_gdt[i].baseMid	= (uint8_t)((base >> 16) & 0xff);
	_gdt[i].baseHi	= (uint8_t)((base >> 24) & 0xff);
	_gdt[i].limit	= (uint16_t)(limit & 0xffff);

	// set flags and granularity bytes
	_gdt[i].flags = access;
	_gdt[i].granularity = (uint8_t)((limit >> 16) & 0x0f);
	_gdt[i].granularity |= gran & 0xf0;
}

gdt_descriptor* i86_gdt_get_descriptor(int i){
	if(i > MAX_DESCRIPTORS)
		return 0;

	return &_gdt[i];
}

int i86_gdt_initialize(){

	// set up gdtr
	_gdtr.m_limit = (uint16_t)((sizeof(gdt_descriptor) * MAX_DESCRIPTORS)-1);
	_gdtr.m_base = (uint32_t)&_gdt[0];

	// set null descriptor
	gdt_set_descriptor(0, 0, 0, 0, 0);

	// set default code descriptor
	gdt_set_descriptor (1,0,0xffffffff,
		I86_GDT_DESC_READWRITE |
		I86_GDT_DESC_EXEC_CODE |
		I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY,

		I86_GDT_GRAN_4K | 
		I86_GDT_GRAN_32BIT | 
		I86_GDT_GRAN_LIMITHI
		);

	// set default data descriptor
	gdt_set_descriptor (2,0,0xffffffff,
		I86_GDT_DESC_READWRITE |
		I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY,

		I86_GDT_GRAN_4K | 
		I86_GDT_GRAN_32BIT | 
		I86_GDT_GRAN_LIMITHI
		);

	// set default user code descriptor
	gdt_set_descriptor (3,0,0xffffffff,
		I86_GDT_DESC_READWRITE |
		I86_GDT_DESC_EXEC_CODE |
		I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY |
		I86_GDT_DESC_DPL,

		I86_GDT_GRAN_4K | 
		I86_GDT_GRAN_32BIT | 
		I86_GDT_GRAN_LIMITHI
		);

	// set default user data descriptor
	gdt_set_descriptor (4,0,0xffffffff,
		I86_GDT_DESC_READWRITE |
		I86_GDT_DESC_CODEDATA |
		I86_GDT_DESC_MEMORY |
		I86_GDT_DESC_DPL,

		I86_GDT_GRAN_4K | 
		I86_GDT_GRAN_32BIT | 
		I86_GDT_GRAN_LIMITHI
		);

	// install gdtr
	gdt_flush ((uint32_t)(&_gdtr));

	return 0;
}