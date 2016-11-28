#ifndef _IDT_H
#define _IDT_H

#include <lib/stdint.h>

#define I86_MAX_INTERRUPTS		256

#define I86_IDT_DESC_BIT16		0x06
#define I86_IDT_DESC_BIT32		0x0E
#define I86_IDT_DESC_RING1		0x40
#define I86_IDT_DESC_RING2		0x20
#define I86_IDT_DESC_RING3		0x60
#define I86_IDT_DESC_PRESENT	0x80

// IRQ handler typedef
typedef void (* I86_IRQ_HANDLER)(void);

typedef struct {

	uint16_t	baseLo;
	uint16_t	sel;
	uint8_t		reserved;
	uint8_t		flags;
	uint16_t 	baseHi;
}__attribute__((packed)) idt_descriptor;

idt_descriptor* i86_get_ir(uint32_t i);

extern void idt_flush(uint32_t idt_ptr);

int i86_install_ir(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER);

int i86_idt_initialize(uint16_t codeSel);

#endif