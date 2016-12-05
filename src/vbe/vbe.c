#include <vbe/vbe.h>

#include <lib/stdio.h>

#define SERIAL_VBE_OUTPUT 0

// define our structure
typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;
 
// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);

typedef struct _mode_list_node{
	uint16_t mode;
	struct _mode_list_noode* next;
} mode_list_node_t;

typedef struct _mode_list{
	uint32_t count;
	mode_list_node_t* head;
} mode_list_t;

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