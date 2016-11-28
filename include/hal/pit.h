#ifndef _PIT_H
#define _PIT_H

#include <lib/stdint.h>

#define		I86_PIT_OCW_MASK_BINCOUNT		1		//00000001
#define		I86_PIT_OCW_MASK_MODE			0xE		//00001110
#define		I86_PIT_OCW_MASK_RL				0x30	//00110000
#define		I86_PIT_OCW_MASK_COUNTER		0xC0	//11000000

#define		I86_PIT_OCW_BINCOUNT_BINARY		0		//0
#define		I86_PIT_OCW_BINCOUNT_BCD		1		//1

#define		I86_PIT_OCW_MODE_TERMINALCOUNT	0		//0000
#define		I86_PIT_OCW_MODE_ONESHOT		0x2		//0010
#define		I86_PIT_OCW_MODE_RATEGEN		0x4		//0100
#define		I86_PIT_OCW_MODE_SQUAREWAVEGEN	0x6		//0110
#define		I86_PIT_OCW_MODE_SOFTWARETRIG	0x8		//1000
#define		I86_PIT_OCW_MODE_HARDWARETRIG	0xA		//1010

#define		I86_PIT_OCW_RL_LATCH			0			//000000
#define		I86_PIT_OCW_RL_LSBONLY			0x10		//010000
#define		I86_PIT_OCW_RL_MSBONLY			0x20		//100000
#define		I86_PIT_OCW_RL_DATA				0x30		//110000

#define		I86_PIT_OCW_COUNTER_0			0		//00000000
#define		I86_PIT_OCW_COUNTER_1			0x40	//01000000
#define		I86_PIT_OCW_COUNTER_2			0x80	//10000000

void i86_pit_send_command(uint8_t cmd);
void i86_pit_send_data(uint16_t data, uint8_t counter);
uint8_t i86_pit_read_data(uint16_t counter);
uint32_t i86_pit_set_tick_count(uint32_t i);
uint32_t i86_pit_get_tick_count();
void i86_pit_start_counter(uint32_t freq, uint8_t counter, uint8_t mode);
void i86_pit_initialize();
int i86_pit_is_initialized();

#endif