#ifndef _GDT_H
#define _GDT_H

#include <lib/stdint.h>

// Maximum number of descriptors
#define MAX_DESCRIPTORS 		3U

#define I86_GDT_DESC_ACCESS		0x0001
#define I86_GDT_DESC_READWRITE	0x0002
#define I86_GDT_DESC_EXPANSION	0x0004
#define I86_GDT_DESC_EXEC_CODE	0x0008
#define I86_GDT_DESC_CODEDATA	0x0010
#define I86_GDT_DESC_DPL		0x0060
#define I86_GDT_DESC_MEMORY		0x0080

#define I86_GDT_GRAN_LIMITHI	0x0F

#define I86_GDT_GRAN_OS			0x10
#define I86_GDT_GRAN_32BIT		0x40
#define I86_GDT_GRAN_4K			0x80

typedef struct {

	uint16_t 	limit;
	uint16_t	baseLo;
	uint8_t		baseMid;
	uint8_t		flags;
	uint8_t		granularity;
	uint8_t 	baseHi;

} __attribute__((packed)) gdt_descriptor;

void gdt_set_descriptor(
	uint32_t i, 
	uint64_t base, 
	uint64_t limit, 
	uint8_t access, 
	uint8_t granularity);

extern void gdt_flush(uint32_t gdt_ptr);

gdt_descriptor* i86_gdt_get_descriptor(int i);

int i86_gdt_initialize();

#endif