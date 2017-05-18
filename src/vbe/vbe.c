#include <vbe/vbe.h>

#include <lib/stdio.h>
#include <mm/virtmem.h>
#include <mm/physmem.h>

#define SERIAL_VBE_OUTPUT 0

// Regs struct used to manipulate registers for bios calls.
typedef struct _regs16_t
{
	uint16_t di;
	uint16_t si;
	uint16_t bp;
	uint16_t sp;
	
	uint16_t bx;
	uint16_t dx;
	uint16_t cx;
    uint16_t ax;

	uint16_t gs;
	uint16_t fs;
	uint16_t es;
	uint16_t ds;
	uint16_t eflags;

} __attribute__((packed)) regs16_t;

typedef struct _regs8_t
{
	// 16 bit regs we are not interested in now.
	uint16_t di;
	uint16_t si;
	uint16_t bp;
	uint16_t sp;

	// BX reg
	uint8_t bh;
	uint8_t bl;

	// DX reg
	uint8_t dh;
	uint8_t dl;

	// CX reg
	uint8_t ch;
	uint8_t cl;

	// AX reg
	uint8_t ah;
	uint8_t al;

	// Other 16 bit resisters we are not interested in.
	uint16_t gs;
	uint16_t fs;
	uint16_t es;
	uint16_t ds;
	uint16_t eflags;
} __attribute__((packed)) regs8_t;

typedef enum _VESA_ERROR
{
	VESA_SUCCESS = 0,
	VESA_FAILED = 1,
	VESA_NOT_SUPPORTED = 2,
	VESA_INVALID = 3
} VESA_ERROR;

// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);

typedef struct _mode_list_node{
	uint16_t mode;
	struct _mode_list_node* next;
} mode_list_node_t;

typedef struct _mode_list{
	uint32_t count;
	mode_list_node_t* head;
} mode_list_t;

static vesa_rendering_context_t current_context = {0};

static mode_list_t mode_list = {0, 0, 0};

void mode_list_push(uint16_t mode);
void mode_list_pop();
uint16_t mode_list_get(uint32_t i);
void mode_list_clear();

void mode_list_push(uint16_t mode){

	// Alloc new node and insert data
	mode_list_node_t* newNode = kmalloc(sizeof(mode_list_node_t));
	newNode->mode = mode; 
	newNode->next = 0;

	// Advance to end

	if(mode_list.head == 0){
		mode_list.head = newNode;
		return;
	}

	mode_list_node_t* curNode = mode_list.head;

	while(curNode->next != 0){
		curNode = curNode->next;
	}

	// Insert into end of list

	curNode->next = newNode;
	mode_list.count++;
}

void mode_list_pop(){

	if(mode_list.head == 0)
		return;

	mode_list_node_t* curNode = mode_list.head;
	mode_list_node_t* prevNode = 0;

	// if curNode->next is equal to 0, it is the last node, and thus the node we
	// want to remove;
	while(curNode->next != 0){
		prevNode = curNode;
		curNode = curNode->next;
	}

	// Free curNode
	prevNode->next = 0;
	kfree(curNode);

	mode_list.count--;
}

uint16_t mode_list_get(uint32_t i){
	if(i >= mode_list.count)
		return 0xFFFF;

	mode_list_node_t* curNode = mode_list.head;
	for(int j = 0; j < i; ++j){
		curNode = curNode->next;
	}

	return curNode->mode;
}

void mode_list_clear(){
	// Not efficient but works
	while(mode_list.count){
		mode_list_pop();
	}
}

void vesa_get_video_info(){
		
		vbe_info_t vbe_info;

		strncpy(&vbe_info.signature[0], "VBE2", 4);

		vesa_get_controller_info(&vbe_info);

		char SIG[5] = {0};
		char REF[5] = {'V','E','S','A',0};

		memcpy(SIG, &vbe_info.signature[0], 4);

		if(strcmp(SIG, REF)){
			return; // Signature does not match
		}
#if SERIAL_VBE_OUTPUT
		serial_printf(COM1, "\n=================================================");
		serial_printf(COM1, "\n==VBE INFO=======================================");
		serial_printf(COM1, "\n=================================================\n");

		serial_printf(COM1, "Version: %i.%i\n", vbe_info.version >> 8, vbe_info.version & 0xFF);

		serial_printf(COM1, "Video memory: %i\n", vbe_info.video_memory);

		uint16_t* modes = (uint16_t*)REAL_PTR(vbe_info.video_modes);

		for(int i = 0; modes[i] != 0xFFFF; ++i){

			mode_list_push(modes[i]);
		}

		serial_printf(COM1, "\n=================================================");

		for(int i = 0; i < mode_list.count; ++i){
			uint16_t mode = mode_list_get(i);
			vbe_mode_info_t mode_info;
			vesa_get_mode_info(mode, &mode_info);

			serial_printf(COM1, "Video Mode %#x:\n", mode);
			if(mode_info.attributes & 0x80){
				serial_printf(COM1, "LFB\n");
			}
			serial_printf(COM1, "Pitch: %i\n", mode_info.pitch);
			serial_printf(COM1, "Width: %i\n", mode_info.width);
			serial_printf(COM1, "Height: %i\n", mode_info.height);
			serial_printf(COM1, "Planes: %i\n", mode_info.planes);
			serial_printf(COM1, "Bits per pixel: %i\n", mode_info.bpp);
			serial_printf(COM1, "Banks: %i\n", mode_info.banks);
			serial_printf(COM1, "Memory model: %i\n", mode_info.memory_model);
			serial_printf(COM1, "Bank Size: %i\n", mode_info.bank_size);
			serial_printf(COM1, "Framebuffer addr: %x\n", mode_info.framebuffer);
			serial_printf(COM1, "Red mask: %i\n", mode_info.red_mask);
			serial_printf(COM1, "Green mask: %i\n", mode_info.green_mask);
			serial_printf(COM1, "Blue mask: %i\n", mode_info.blue_mask);
			serial_printf(COM1, "Red position: %i\n", mode_info.red_position);
			serial_printf(COM1, "Green position: %i\n", mode_info.green_position);
			serial_printf(COM1, "Blue position: %i\n", mode_info.blue_position);
			serial_printf(COM1, "Off screen mem off: %i\n", mode_info.off_screen_mem_off);
			serial_printf(COM1, "Off screen mem size: %i\n", mode_info.off_screen_mem_size);
			serial_printf(COM1, "\n");
			serial_printf(COM1, "\n=================================================");
		}

#endif
}

int vesa_get_controller_info(vbe_info_t* info){
	regs16_t regs;

	uint32_t info_ptr = (uint32_t) info;

	vbe_info_t* temp = 0x1000;

	regs.ax = CMD_GET_CONTROLLER_INFO;
	regs.di = 0;
	regs.es = 0x100;

	int32(0x10, &regs);

	if(regs.ax!=0x004F){
		printf("Error getting VBE controller info\n");
		return 1;
	} 


	memcpy(info, temp, sizeof(vbe_info_t));
	return 0;
}

int vesa_get_mode_info(const uint16_t mode, vbe_mode_info_t* info){
	regs16_t regs;

	uint32_t info_ptr = (uint32_t) info;

	vbe_mode_info_t* temp = 0x1000;

	regs.cx = mode;
	regs.ax = CMD_GET_MODE_INFO;
	regs.di = 0;
	regs.es = 0x100;

	int32(0x10, &regs);

	if(regs.ax!=0x004F){
		printf("Error getting VBE controller info\n");
		return 1;
	} 

	memcpy(info, temp, sizeof(vbe_info_t));
	return 0;
}

int vesa_set_vbe_mode(uint16_t mode)
{
	serial_printf(COM1, "Switching to mode %#x\n", mode);
	union
	{
		regs16_t r16;
		regs8_t r8;
	} regs;

	regs.r16.ax = CMD_SET_VBE_MODE;
	regs.r16.bx = mode;
	
	int32(0x10, &regs);

	if(regs.r8.ah != 0x4F)
	{
		serial_printf(COM1, "Function not supported.\n");
		return VESA_NOT_SUPPORTED;
	}

	if(regs.r8.al != VESA_SUCCESS)
	{
		serial_printf(COM1, "VESA ERROR: %i\n", (int)regs.r8.al);
		return (int)regs.r8.al;
	}

	serial_printf(COM1, "Mode switch successful!\n");
	
	// Mode change successful. Save relevant info.

	vbe_mode_info_t mode_info;

	if(vesa_get_mode_info(mode, &mode_info))
	{
		return VESA_FAILED;
	}

	current_context.mode_number = mode;
	current_context.pitch = mode_info.pitch;
	current_context.width = mode_info.width;
	current_context.height = mode_info.height;
	current_context.bits_per_pixel = mode_info.bpp;
	current_context.framebuffer = mode_info.framebuffer;

	serial_printf(COM1, "Info gathered\n");
	
#define PAGE_SIZE 4096
	
	uint32_t buf = (uint32_t)current_context.framebuffer;
	
	// Identity map vid buffer
    for(size_t i = 0;
		i < (current_context.pitch*current_context.height);
		i+=PAGE_SIZE)
	{
    	vmmngr_mapPhysicalAddress(
			vmmngr_get_directory(),
			buf + i, // Virtual Address
			buf + i, // Physical Address
			I86_PTE_PRESENT | I86_PTE_WRITABLE);
    }

	serial_printf(COM1, "Buffer mapped\n");

	serial_printf(COM1, "Mode %#x\n", current_context.mode_number);
	serial_printf(COM1, "Pitch %i\n", current_context.pitch);
	serial_printf(COM1, "Width %i\n", current_context.width);
	serial_printf(COM1, "Height %i\n", current_context.height);
	serial_printf(COM1, "BPP %i\n", current_context.bits_per_pixel);
	serial_printf(COM1, "Framebuffer %#x\n", current_context.framebuffer);

	return (int)VESA_SUCCESS;
}

int vesa_get_vbe_mode(uint16_t* mode)
{
	*mode = current_context.mode_number;

	return VESA_SUCCESS;
}

int vesa_restore_terminal_mode()
{
	serial_printf(COM1, "Restoring terminal mode\n");
	regs16_t regs;
    // switch to 80x25x16 text mode
    regs.ax = 0x0003;
    int32(0x10, &regs);

   	// null the context descriptor
	memset(&current_context, 0, sizeof(vesa_rendering_context_t));

	return VESA_SUCCESS;
}

void vesa_put_pixel(vesa_pixel_t pixel, uint32_t x, uint32_t y)
{
	if(!current_context.mode_number)
		return;

	serial_printf(COM1, "Put Pixel\n");

	current_context.framebuffer[x + current_context.width*y] = pixel;
}

void vesa_fill_rect(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height)
{
	uint32_t cur_x;
	uint32_t max_x = x + width;
	uint32_t max_y = y + height;

	// Clamp x and y coordinate to screen so we don't write outside window.
	if(max_x > current_context.width)
		max_x = current_context.width;

	if(max_y > current_context.height)
		max_y = current_context.height;

	// This can be sped up.
	for(;y < max_y; ++y)
	{
		for(cur_x = x; cur_x < max_x; ++cur_x)
		{
			current_context.framebuffer[cur_x + current_context.width*y] = pixel;
		}
	}
}

//===================================================================
// BGA - Bochs graphics adapter
//===================================================================

#define VBE_DISPI_IOPORT_INDEX		0x01CE
#define VBE_DISPI_IOPORT_DATA		0x01CF

#define VBE_DISPI_INDEX_ID			0x00
#define VBE_DISPI_INDEX_XRES		0x01
#define VBE_DISPI_INDEX_YRES		0x02
#define VBE_DISPI_INDEX_BPP			0x03
#define VBE_DISPI_INDEX_ENABLE		0x04
#define VBE_DISPI_INDEX_BANK		0x05
#define VBE_DISPI_INDEX_VIRT_WIDTH	0x06
#define VBE_DISPI_INDEX_VIRT_HEIGHT	0x07
#define VBE_DISPI_INDEX_X_OFFSET    0x08
#define VBE_DISPI_INDEX_Y_OFFSET	0x09

#define VBE_DISPI_ID0				0xB0C0
#define VBE_DISPI_ID1				0xB0C1
#define VBE_DISPI_ID2				0xB0C2
#define VBE_DISPI_ID3				0xB0C3
#define VBE_DISPI_ID4				0xB0C4
#define VBE_DISPI_ID5				0xB0C5

#define VBE_DISPI_DISABLED			0x00
#define VBE_DISPI_ENABLED			0x01
#define VBE_DISPI_LFB_ENABLED		0x40
#define VBE_DISPI_NOCLEARMEM		0x80

typedef struct _vba_rendering_context_t
{
	uint16_t width;
	uint16_t height;
	uint16_t bpp;
	uint32_t framebuffer;
} vba_rendering_context_t;

static vba_rendering_context_t vbe_current_context = {0};

void vbe_bochs_write(uint16_t index, uint16_t value)
{
	outportw(VBE_DISPI_IOPORT_INDEX, index);
	outportw(VBE_DISPI_IOPORT_DATA, value);
}

void vbe_bochs_set_gfx(uint16_t width, uint16_t height, uint16_t bpp)
{
	vbe_bochs_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
	vbe_bochs_write(VBE_DISPI_INDEX_XRES, width);
	vbe_bochs_write(VBE_DISPI_INDEX_YRES, height);
	vbe_bochs_write(VBE_DISPI_INDEX_BPP, bpp);
	vbe_bochs_write(VBE_DISPI_INDEX_ENABLE,
					VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

	vbe_current_context.width = width;
	vbe_current_context.height = height;
	vbe_current_context.bpp = bpp;
	vbe_current_context.framebuffer = (uint32_t)0xE0000000;
	
	unsigned int buf_size = width*height*bpp;
	for(unsigned int c = 0; c < buf_size; c += 0x1000)
	{
		vmmngr_mapPhysicalAddress(
			vmmngr_get_directory(),
			vbe_current_context.framebuffer + c,
			vbe_current_context.framebuffer + c,
			3);
	}
}

void vbe_clear_screen(vesa_pixel_t pixel)
{
	vesa_pixel_t* lfb = (vesa_pixel_t*) vbe_current_context.framebuffer;

	for(uint32_t c = 0; c < vbe_current_context.width*vbe_current_context.height; ++c)
	{
		lfb[c] = pixel;
	}
}

void vbe_fill_rect(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	uint32_t width,
	uint32_t height)
{
	vesa_pixel_t* lfb = (vesa_pixel_t*) vbe_current_context.framebuffer;

	uint32_t max_x = x + width;
	uint32_t max_y = y + height;
	
	// Clamp right border to screen
	if(max_x > vbe_current_context.width)
	{
		max_x = vbe_current_context.width;
	}

	// Clamp bottom border
	if(max_y > vbe_current_context.height)
	{
		max_y = vbe_current_context.height;
	}

	uint32_t cur_x;

	for(;y < max_y; ++y)
	{
		for(cur_x = x; cur_x < max_x; ++cur_x)
		{
			lfb[cur_x + vbe_current_context.width*y] = pixel;
		}
	}
}

void vbe_put_pixel(vesa_pixel_t pixel, uint32_t x, uint32_t y)
{
	vesa_pixel_t* lfb = (vesa_pixel_t*) vbe_current_context.framebuffer;

	lfb[x + vbe_current_context.width*y] = pixel;
}

static uint32_t char_width = 8;
static uint32_t char_height = 8;

// Taken from https://github.com/dhepper/font8x8 2017-04-29
static uint8_t font8x8_basic[128][8] =
{
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
    { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
    { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
    { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
    { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
    { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
    { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
    { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
    { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
    { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
    { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
    { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
    { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
    { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
    { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
    { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
    { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
    { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
    { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
    { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
    { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
    { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
    { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
    { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
    { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (//)
    { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
    { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
    { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
    { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
    { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
    { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
    { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
    { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
    { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
    { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
    { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
    { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
    { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
    { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
    { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
    { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
    { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
    { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
    { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
    { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
    { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
    { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
    { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
    { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
    { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
    { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
    { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
    { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
    { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
    { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
    { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
    { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
    { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
    { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
    { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
    { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
    { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
    { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
    { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
    { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
    { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
    { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
    { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
    { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
    { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
    { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
    { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
    { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
    { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
    { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
    { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
    { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
    { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
    { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
    { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
    { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
    { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
    { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
    { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
    { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
    { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
    { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};

void vbe_put_char(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	char c)
{
	uint8_t* character = font8x8_basic[c];
	
	for(uint32_t cy = 0; cy < char_height; ++cy)
	{
		for(uint32_t cx = 0; cx < char_width; ++cx)
		{
			if(*(character + (char_width - cy - 1)) & (1 << cx))
			{			
				vbe_put_pixel(pixel, x + cx, y - cy);
			}
		}
	}
}

void vbe_print_string(
	vesa_pixel_t pixel,
	uint32_t x,
	uint32_t y,
	char* str)
{
	uint32_t strpos = 0;
	uint32_t xpos = x;
	uint32_t ypos = y;

	// While we have'nt reached the end of the string
	while(str[strpos])
	{
		// Extract one char
		char c = str[strpos++];

		// Handle special characters
		switch(c)
		{
			// If we got a new line char, reset x position and advance y position.
		case '\n':
			xpos = x;
			ypos += char_height;
			break;
		default:
			vbe_put_char(pixel, xpos, ypos, c);
			xpos += char_width;
			break;
		}
	}
}

uint16_t vbe_get_width()
{
	return vbe_current_context.width;
}

uint16_t vbe_get_height()
{
	return vbe_current_context.height;
}

uint16_t vbe_get_bpp()
{
	return vbe_current_context.bpp;
}

vesa_pixel_t* vbe_get_buffer()
{
	return vbe_current_context.framebuffer;
}
