#include <monitor/monitor.h>

#include <lib/stdint.h>
#include <sync/mutex.h>

//===================================================================
// Implementation specific data structures
//===================================================================

static mutex_t mon_mutex;

// Data structure for each entry (character) in the VGA text mode array
typedef struct {

	// Character in ascii code
	uint8_t character;

	// Foreground color
	uint8_t front_color : 4;

	// Background color
	uint8_t back_color : 4;

} character_entry_t;

//===================================================================
// Private forward declarations
//===================================================================

uint32_t offset(uint32_t x, uint32_t y);

void scroll();

//===================================================================
// Implementation variables
//===================================================================

// Pointer to start of video memory
character_entry_t* VIDMEM = (character_entry_t*) 0x000B8000;

// Current X-position of the cursor
static uint32_t cursor_x = 0;

// Current Y-position of the cursor
static uint32_t cursor_y = 1;

// Current background color
static uint8_t background_color = VGA_COLOR_BLACK;

// Current foreground color
static uint8_t foreground_color = VGA_COLOR_MAGENTA;

//===================================================================
// Private function implementations
//===================================================================

// Calculates the offset to the entry at (x,y) from video memory start.
inline uint32_t offset(uint32_t x, uint32_t y) {

	return (x + (y * MONITOR_WIDTH));

}

void scroll(){
	if(cursor_y>=25){
		int i;
		for(i = 1; i < (80*24);++i){
			asm volatile("nop");
			VIDMEM[i] = VIDMEM[i+80];
		}
		for(i; i < (80*25);++i){
			asm volatile("nop");
			VIDMEM[i] = (character_entry_t){' ',foreground_color, background_color};
		}
		cursor_y = 24;
	}
}

//===================================================================
// Interface functions
//===================================================================

void monitor_putch(char c) {

	

	// Sort out special characters

	if(c == 0)
		return;

	if(c == '\n' || c == '\r'){
		cursor_x = 0;
		++cursor_y;
		return;
	}

	// Line wrap if needed
	if(cursor_x >= MONITOR_WIDTH){
		
		cursor_x = 0;
		++cursor_y;

	}

	if (cursor_y >= 25)
		scroll ();

	// Put ascii characters
	VIDMEM[offset(cursor_x++, cursor_y)] = 
		(character_entry_t){c,foreground_color, background_color};
}

void monitor_puts(const char* s){

	asm volatile ("cli");
	lock(&mon_mutex);

	while(*s){

		monitor_putch(*(s++));
		
	}

	unlock(&mon_mutex);
	asm volatile ("sti");

}

void monitor_clear(){
	for(int32_t i = 0; i < (MONITOR_WIDTH*MONITOR_HEIGHT); ++i){
		VIDMEM[i] = (character_entry_t){' ',foreground_color, background_color};
	}

	cursor_x = 0;
	cursor_y = 1;
}

void monitor_setFColor(vga_color_t fc){
	foreground_color = fc;
}

void monitor_setBColor(vga_color_t bc){
	background_color = bc;
}

void monitor_setCursor(uint32_t x, uint32_t y){
	cursor_x = x;
	cursor_y = y;
}

void monitor_getCursor(uint32_t* x, uint32_t* y){
	*x = cursor_x;
	*y = cursor_y;
}

//===================================================================
// End of file
//===================================================================
