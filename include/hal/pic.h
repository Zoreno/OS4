#ifndef _PIC_H
#define _PIC_H

#include <lib/stdint.h>

#define		I86_PIC_IRQ_TIMER			0
#define		I86_PIC_IRQ_KEYBOARD		1
#define		I86_PIC_IRQ_SERIAL2			3
#define		I86_PIC_IRQ_SERIAL1			4
#define		I86_PIC_IRQ_PARALLEL2		5
#define		I86_PIC_IRQ_DISKETTE		6
#define		I86_PIC_IRQ_PARALLEL1		7

#define		I86_PIC_IRQ_CMOSTIMER		0
#define		I86_PIC_IRQ_CGARETRACE		1
#define		I86_PIC_IRQ_AUXILIARY		4
#define		I86_PIC_IRQ_FPU				5
#define		I86_PIC_IRQ_HDC				6

#define		I86_PIC_OCW2_MASK_L1		1		//00000001
#define		I86_PIC_OCW2_MASK_L2		2		//00000010
#define		I86_PIC_OCW2_MASK_L3		4		//00000100
#define		I86_PIC_OCW2_MASK_EOI		0x20	//00100000
#define		I86_PIC_OCW2_MASK_SL		0x40	//01000000
#define		I86_PIC_OCW2_MASK_ROTATE	0x80	//10000000

#define		I86_PIC_OCW3_MASK_RIS		1		//00000001
#define		I86_PIC_OCW3_MASK_RIR		2		//00000010
#define		I86_PIC_OCW3_MASK_MODE		4		//00000100
#define		I86_PIC_OCW3_MASK_SMM		0x20	//00100000
#define		I86_PIC_OCW3_MASK_ESMM		0x40	//01000000
#define		I86_PIC_OCW3_MASK_D7		0x80	//10000000

uint8_t i86_pic_read_data(uint8_t picNum);
void i86_pic_send_data(uint8_t data, uint8_t picNum);
void i86_pic_send_command(uint8_t cmd, uint8_t picNum);
void i86_pic_mask_irq(uint8_t irqmask, uint8_t picNum);
void i86_pic_initialize(uint8_t base0, uint8_t base1);

#endif