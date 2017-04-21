#ifndef _TSS_H
#define _TSS_H

#include <lib/stdint.h>

/**
*	Task-State Segment struct.
*/
typedef struct 
{
	/**
	*	Previous task segment selector
	*/
	uint16_t prevTss;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res1;

	/**
	*	Stack pointer for ring 0.
	*/
	uint32_t esp0;

	/**
	*	Stack segment selector for ring 0.
	*/
	uint16_t ss0;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res2;

	/**
	*	Stack pointer for ring 1.
	*/
	uint32_t esp1;

	/**
	*	Stack segment selector for ring 1.
	*/
	uint16_t ss1;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res3;

	/**
	*	Stack pointer for ring 2.
	*/
	uint32_t esp2;

	/**
	*	Stack segment selector for ring 2.
	*/
	uint32_t ss2;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res4;

	/**
	*	CR3 Control register. Contains the base physical address of the page
	*	directory to be used by the stack.
	*/
	uint32_t cr3;

	/**
	*	State of EIP register prior to task switch.
	*/
	uint32_t eip;

	/**
	*	State of EFLAGS register prior to task switch.
	*/
	uint32_t eflags;

	/**
	*	State of EAX register prior to task switch.
	*/
	uint32_t eax;

	/**
	*	State of ECX register prior to task switch.
	*/
	uint32_t ecx;

	/**
	*	State of EDX register prior to task switch.
	*/
	uint32_t edx;

	/**
	*	State of EBX register prior to task switch.
	*/
	uint32_t ebx;

	/**
	*	State of ESP register prior to task switch.
	*/
	uint32_t esp;

	/**
	*	State of EBP register prior to task switch.
	*/
	uint32_t ebp;

	/**
	*	State of ESI register prior to task switch.
	*/
	uint32_t esi;

	/**
	*	State of EDI register prior to task switch.
	*/
	uint32_t edi;

	/**
	*	State of ES segment selector prior to task switch.
	*/
	uint16_t es;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res5;

	/**
	*	State of CS segment selector prior to task switch.
	*/
	uint16_t cs;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res6;

	/**
	*	State of SS segment selector prior to task switch.
	*/
	uint16_t ss;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res7;

	/**
	*	State of DS segment selector prior to task switch.
	*/
	uint16_t ds;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res8;

	/**
	*	State of FS segment selector prior to task switch.
	*/
	uint16_t fs;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res9;

	/**
	*	State of GS segment selector prior to task switch.
	*/
	uint16_t gs;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res10;

	/**
	*	Segment selector for the task's LDT.
	*/
	uint16_t ldt;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res11;

	/**
	*	Debug Trap Flag.
	*/
	uint16_t trap : 1;

	/**
	*	Reserved. Should be zero.
	*/
	uint16_t res12 : 15;

	/**
	*	I/O map base address field. 16-bit offset from TSS to I/O permissions
	*	map.
	*/
	uint16_t iomap;

} __attribute__((packed)) tss_entry;


void tss_set_stack(uint16_t kernelSS, uint32_t kernelESP);
void install_tss(uint32_t idx, uint16_t kernelSS, uint32_t kernelESP);

#endif
