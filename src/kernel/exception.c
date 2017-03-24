#include <kernel/exception.h>
#include <kernel/panic.h>

void divide_by_zero_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Divide by 0");
	for(;;);
}

void single_step_trap (
	unsigned int cs, 
	unsigned int eip, 
	unsigned int eflags
	){
	kernel_panic("Single step");
	for(;;);
}

void nmi_trap (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("NMI trap");
	for(;;);
}

void breakpoint_trap (
	unsigned int cs, 
    unsigned int eip,
    unsigned int eflags
    ){
	//kernel_panic("Breakpoint trap");
	//for(;;);
}

void overflow_trap (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Overflow trap");
	for(;;);
}


void bounds_check_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Bounds check fault");
	for(;;);
}

void invalid_opcode_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Invalid OP-code. [%#(10)p] [%#(10)p] [%#(10)p]", cs, eip, eflags);
	for(;;);
}

void no_device_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Device not found");
	for(;;);
}

void double_fault_abort (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Double fault");
	for(;;);
}

void invalid_tss_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Invalid TSS");
	for(;;);
}

void no_segment_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Invalid segment");
	for(;;);
}


void stack_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Stack fault");
	for(;;);
}

void general_protection_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("General protection fault");
	for(;;);
}

void page_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	asm volatile ("cli");
	asm volatile ("sub $4, %ebp");

	int faultAddr;

	asm volatile ("mov %%cr2, %0" : "=r"(faultAddr));

	kernel_panic("Page fault at %#0(6)p:%#0(10)p: referenced memory at %#0(10)p(flags: %#x)", 
		cs, err, faultAddr, eip);

	//asm volatile ("popal; leave; iret");
	
	asm volatile ("cli");
	asm volatile ("hlt");
}

void fpu_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("FPU fault");
	for(;;);
}

void alignment_check_fault (
	unsigned int cs, 
	unsigned int err,
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Alignment check fault");
	for(;;);
}

void machine_check_abort (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("Machine check abort");
	for(;;);
}

void simd_fpu_fault (
	unsigned int cs, 
    unsigned int eip, 
    unsigned int eflags
    ){
	kernel_panic("SIMD FPU fault");
	for(;;);
}