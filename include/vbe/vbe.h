#ifndef _VBE_H
#define _VBE_H

#include <lib/stdint.h>

// http://www.phatcode.net/res/221/files/vbe20.pdf

#define CMD_GET_CONTROLLER_INFO 	0x4F00
#define CMD_GET_MODE_INFO			0x4F01
#define CMD_SET_VBE_MODE			0x4F02

/**
*	The resulting struct from the CMD_GET_CONTROLLER_INFO bios int.
*/
typedef struct {

	/**
	 * The info struct signature. Must be "VESA".
	 */
	char signature[4];

	/**
	 * VBE version. High byte is major version. Low byte is minor version.
	 */
	uint16_t version;

	/**
	 * Segment:offset pointer to OEM.
	 */
	uint32_t oem;

	/**
	 * Bitfield that describes card capabilities.
	 */
	uint32_t capabilities;

	/**
	 * Segment:offset pointer to list of supported video modes.
	 */
	uint32_t video_modes;

	/**
	 * Amount of video memory in 64KB blocks.
	 */
	uint16_t video_memory;

	/**
	 * Software revision.
	 */
	uint16_t software_rev;

	/**
	 * Segment:offset pointer to card vendor string.
	 */
	uint32_t vendor;

	/**
	 * Segment:offset pointer to card model name.
	 */
	uint32_t product_name;

	/**
	 * Segment:offset pointer to product revision.
	 */
	uint32_t product_rev;

	/**
	 * Reserved for furure expansion.
	 */
	char reserved[222];

	/**
	 * OEM bioses store their strings in this area.
	 */
	char oem_data[256];

} __attribute__ ((packed)) vbe_info_t;

/**
 * Protected mode information block
 *
 * Struct should be located somewhere within the first 32kb of bios image.
 */
typedef struct _vesa_pm_info_block_t
{
	/**
	 * Special signature to identify the struct in memory.
	 */
	char signature[4];

	/**
	 * Offset of the protected mode entry point from the start of the BIOS image.
	 */
	uint32_t entry_point;

	/**
	 * Offset of the function for initializing the protected mode BIOS code.
	 */
	uint32_t pm_initialize;

	/**
	 * Protected mode bios data selector.
	 */
	uint32_t bios_data_sel;

	/**
	 * Selector for A0000h memory area.
	 */
	uint32_t A0000_sel;

	/**
	 * Selector for B0000h memory area.
	 */
	uint32_t B0000_sel;

	/**
	 * Selector for B8000h memory area.
	 */
	uint32_t B8000_sel;

	/**
	 * Protected mode bios data selector.
	 */
	uint32_t code_seg_sel;

	/**
	 * Flag to indicate whether bios code is running in protected mode.
	 */
	uint8_t in_protect_mode;

	/**
	 * Checksum to verify the validite the struct.
	 */
	uint8_t checksum;
	
} __attribute__((packed)) vesa_pm_info_block_t;

#define VBE_MODE_ATTR_HARDWARE_SUPPORT 	(1<<0)
#define VBE_MODE_ATTR_TTY_OUTPUT		(1<<2)
#define VBE_MODE_ATTR_MONOCHROME		(1<<3)
#define VBE_MODE_ATTR_MODE				(1<<4)
#define VBE_MODE_ATTR_VGA_COMP			(1<<5)
#define VBE_MODE_ATTR_VGA_COMP_MEM		(1<<6)
#define VBE_MODE_ATTR_LIN_FRAME_BUF		(1<<7)

#define VBE_MODE_WINATTR_RELOC			(1<<0)
#define VBE_MODE_WINATTR_READABLE		(1<<1)
#define VBE_MODE_WINATTR_WRITEABLE		(1<<2)

/**
*	The resulting struct from the CMD_GET_MODE_INFO bios int.
*/
typedef struct {

	/**
	 * Describes important characteristics of the graphics mode.
	 *
	 * Bit 7 indicates linear frame buffer.
	 */
	uint16_t attributes;

	/**
	 * Describes characteristics of the CPU windowing scheme.
	 */
	uint8_t window_a;

	/**
	 * Describes characteristics of the CPU windowing scheme.
	 */
	uint8_t window_b;

	/**
	 * Desctibes the smallest boundary on which the window can be placed
	 * in the frame buffer memory.
	 */
	uint16_t granularity;

	/**
	 * Specifies the size of the window in KB.
	 */
	uint16_t window_size;

	/**
	 * Specifies the segment address where the window is located in CPU 
	 * address space.
	 */
	uint16_t segment_a;

	/**
	 * Specifies the segment addrese where the window is located in CPU
	 * address space.
	 */
	uint16_t segment_b;

	/**
	 * Specifies offset to VBE memory windowing function.
	 */
	uint32_t win_func_ptr;

	/**
	 * Number of bytes per horizontal line.
	 */
	uint16_t pitch;

	/**
	 * Width in pixels.
	 */
	uint16_t width;

	/**
	 * Height in pixels.
	 */
	uint16_t height;
	uint8_t w_char;
	uint8_t y_char;
	uint8_t planes;

	/**
	 *  Bits per pixel.
	 */
	uint8_t bpp;
	uint8_t banks;
	uint8_t memory_model;
	uint8_t bank_size;
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

   	/**
	 * Physical address of the memory mapped frame buffer.
	 */
	uint32_t framebuffer;
	uint32_t off_screen_mem_off;
	uint16_t off_screen_mem_size;
	uint8_t reserved1[206];

} __attribute__ ((packed)) vbe_mode_info_t;

#define REAL_PTR(seg_off) ((seg_off >> 0x10) * 0x10 + (seg_off & 0xFFFF))

/**
 * Struct representing a pixel in 32 bpp mode.
 */
typedef struct _vesa_pixel_t
{
	/**
	 * Blue color channel
	 */
	uint8_t blue;

	/**
	 * Green color channel
	 */
	uint8_t green;

	/**
	 * Red color channel
	 */
	uint8_t red;

	/**
	 * Reserved padding byte for now.
	 */
	uint8_t reserved;
} __attribute__ ((packed)) vesa_pixel_t;

/**
 * OS4 representation of a video mode.
 */
typedef struct _vesa_rendering_context_t
{
	/**
	 * VESA mode number.
	 */
	uint16_t mode_number;

	/**
	 * Size in bytes of one horizontal line.
	 */
	uint32_t pitch;

	/**
	 * Width in pixels of framebuffer
	 */
	uint32_t width;

	/**
	 * Height in pixels of framebuffer
	 */
	uint32_t height;

	/**
	 * Number of bits needed to represent one pixel
	 */
	uint32_t bits_per_pixel;

	/**
	 * Physical address of memory mapped framebuffer.
	 */
	vesa_pixel_t* framebuffer;
} vesa_rendering_context_t;

void get_video_info();
int vesa_get_controller_info(vbe_info_t* info);
int vesa_get_mode_info(const uint16_t mode, vbe_mode_info_t* info);

int vesa_set_vbe_mode(uint16_t mode);
int vesa_get_vbe_mode(uint16_t* mode);
int vesa_restore_terminal_mode();

void vesa_put_pixel(vesa_pixel_t pixel, uint32_t x, uint32_t y);

void vesa_fill_rect(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height);

//==========================================================================
// BGA - Bochs graphics adapter
//==========================================================================

void vbe_bochs_set_gfx(
	uint16_t width,
    uint16_t height,
    uint16_t bpp);

void vbe_clear_screen(
	vesa_pixel_t pixel);

void vbe_fill_rect(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height);

void vbe_put_pixel(
	vesa_pixel_t pixel,
    uint32_t x,
    uint32_t y);

void vbe_put_char(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	char c);

void vbe_print_string(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	char* str);

uint16_t vbe_get_width();

uint16_t vbe_get_height();

uint16_t vbe_get_bpp();

vesa_pixel_t* vbe_get_buffer();

#endif
