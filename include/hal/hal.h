#ifndef _HAL_H
#define _HAL_H

#include <lib/stdint.h>

int hal_initialize();
int hal_shutdown();

void interruptdone(uint32_t intno);
void sound(uint32_t frequency);

uint8_t inportb(uint16_t port);
void outportb(uint16_t port, uint8_t value);
uint16_t inportw(uint16_t port);
void outportw(uint16_t port, uint16_t value);
uint32_t inportl(uint16_t port);
void outportl(uint16_t port, uint32_t value);

void enable();
void disable();

typedef void(*irq_vect)(void);

void setvect(int intno, irq_vect);

void (*getvect(int intno))();

const char* get_cpu_vendor();

int get_tick_count();

#endif