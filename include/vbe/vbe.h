#ifndef _VBE_H
#define _VBE_H

#include <lib/stdint.h>

// http://www.phatcode.net/res/221/files/vbe20.pdf

#define CMD_GET_CONTROLLER_INFO 	0x4F00
#define CMD_GET_MODE_INFO			0x4F01

/**
*	The resulting struct from the CMD_GET_CONTROLLER_INFO bios int.
*/
typedef struct {

	char signature[4];			// must be "VESA" to indicate valid VBE support
	uint16_t version;			// VBE version; high byte is major version, low byte is minor version
	uint32_t oem;				// segment:offset pointer to OEM
	uint32_t capabilities;		// bitfield that describes card capabilities
	uint32_t video_modes;		// segment:offset pointer to list of supported video modes
	uint16_t video_memory;		// amount of video memory in 64KB blocks
	uint16_t software_rev;		// software revision
	uint32_t vendor;			// segment:offset to card vendor string
	uint32_t product_name;		// segment:offset to card model name
	uint32_t product_rev;		// segment:offset pointer to product revision
	char reserved[222];			// reserved for future expansion
	char oem_data[256];			// OEM BIOSes store their strings in this area

} __attribute__ ((packed)) vbe_info_t;

/**
*	The resulting struct from the CMD_GET_MODE_INFO bios int.
*/
typedef struct {

	uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
	uint8_t window_a;			// deprecated
	uint8_t window_b;			// deprecated
	uint16_t granularity;		// deprecated; used while calculating bank numbers
	uint16_t window_size;
	uint16_t segment_a;
	uint16_t segment_b;
	uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
	uint16_t pitch;				// number of bytes per horizontal line
	uint16_t width;				// width in pixels
	uint16_t height;			// height in pixels
	uint8_t w_char;				// unused...
	uint8_t y_char;				// ...
	uint8_t planes;
	uint8_t bpp;				// bits per pixel in this mode
	uint8_t banks;				// deprecated; total number of banks in this mode
	uint8_t memory_model;
	uint8_t bank_size;			// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
	uint8_t image_pages;
	uint8_t reserved0;
 
	uint8_t red_mask;
	uint8_t red_position;
	uint8_t green_mask;
	uint8_t green_position;
	uint8_t blue_mask;
	uint8_t blue_position;
	uint8_t reserved_mask;
	uint8_t reserved_position;
	uint8_t direct_color_attributes;
 
	uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
	uint8_t reserved1[206];

} __attribute__ ((packed)) vbe_mode_info_t;



#define REAL_PTR(seg_off) ((seg_off >> 0x10) * 0x10 + (seg_off & 0xFFFF))


void get_video_info();
int vesa_get_controller_info(vbe_info_t* info);
int vesa_get_mode_info(const uint16_t mode, vbe_mode_info_t* info);

#endif