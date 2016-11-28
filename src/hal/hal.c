#include <hal/hal.h>

#include <hal/cpu.h>
#include <hal/idt.h>
#include <hal/pic.h>
#include <hal/pit.h>

#include <lib/stdio.h>

int hal_initialize(){
	i86_cpu_initialize();
	i86_pic_initialize(0x20, 0x28);
	i86_pit_initialize();
	i86_pit_start_counter (100,I86_PIT_OCW_COUNTER_0, I86_PIT_OCW_MODE_SQUAREWAVEGEN);

	enable();

	return 0;
}

int hal_shutdown(){
	i86_cpu_shutdown();
	return 0;
}

void interruptdone(uint32_t intno){
	// insure its a valid hardware irq
	if (intno > 16)
		return;

	// test if we need to send end-of-interrupt to second pic
	if (intno >= 8)
		i86_pic_send_command (I86_PIC_OCW2_MASK_EOI, 1);

	// always send end-of-interrupt to primary pic
	i86_pic_send_command (I86_PIC_OCW2_MASK_EOI, 0);
}

void sound(uint32_t frequency){
	outportb (0x61, 3 | (unsigned char)(frequency<<2) );
}


uint8_t inportb(uint16_t port){
	uint8_t ret;
    asm volatile ( "in %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outportb(uint16_t port, uint8_t value){
	asm volatile ( "out %0, %1" : : "a"(value), "Nd"(port) );
}

uint16_t inportw(uint16_t port){
	uint16_t ret;
    asm volatile ( "in %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outportw(uint16_t port, uint16_t value){
	asm volatile ( "out %0, %1" : : "a"(value), "Nd"(port) );
}

uint32_t inportl(uint16_t port){
	uint32_t ret;
    asm volatile ( "in %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

void outportl(uint16_t port, uint32_t value){
	asm volatile ( "out %0, %1" : : "a"(value), "Nd"(port) );
}

void enable(){
	asm volatile("sti");
}

void disable(){
	asm volatile("cli");
}

void setvect(int intno, irq_vect vect){
	i86_install_ir (intno, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32,
		0x8, vect);
}

void (*getvect(int intno))(){
	// get the descriptor from the idt
	idt_descriptor* desc = i86_get_ir (intno);
	if (!desc)
		return 0;

	// get address of interrupt handler
	uint32_t addr = desc->baseLo | (desc->baseHi << 16);

	// return interrupt handler
	I86_IRQ_HANDLER irq = (I86_IRQ_HANDLER)addr;
	return irq;
}

const char* get_cpu_vendor(){
	return i86_cpu_get_vendor();
}

int get_tick_count(){
	return i86_pit_get_tick_count();
}