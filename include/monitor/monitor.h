// OS text mode terminal

#ifndef _MONITOR_H
#define _MONITOR_H

#include <lib/stdint.h>

#define MONITOR_WIDTH 80U
#define MONITOR_HEIGHT 25U

// VGA color values
typedef enum vga_color {
	VGA_COLOR_BLACK = 			0,
	VGA_COLOR_BLUE = 			1,
	VGA_COLOR_GREEN = 			2,
	VGA_COLOR_CYAN = 			3,
	VGA_COLOR_RED = 			4,
	VGA_COLOR_MAGENTA = 		5,
	VGA_COLOR_BROWN = 			6,
	VGA_COLOR_LIGHT_GREY = 		7,
	VGA_COLOR_DARK_GREY = 		8,
	VGA_COLOR_LIGHT_BLUE = 		9,
	VGA_COLOR_LIGHT_GREEN = 	0xA,
	VGA_COLOR_LIGHT_CYAN = 		0xB,
	VGA_COLOR_LIGHT_RED = 		0xC,
	VGA_COLOR_LIGHT_MAGENTA = 	0xD,
	VGA_COLOR_LIGHT_BROWN =		0xE,
	VGA_COLOR_WHITE = 			0xF,
} vga_color_t;


// Text output
void monitor_putch(char c);
void monitor_puts(const char* s);

// Clear
void monitor_clear();

// Color manipulation
void monitor_setFColor(vga_color_t fc);
void monitor_setBColor(vga_color_t bc);

void monitor_setCursor(uint32_t x, uint32_t y);
void monitor_getCursor(uint32_t* x, uint32_t* y);

#endif
