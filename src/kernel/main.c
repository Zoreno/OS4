
#include <lib/stdio.h>
#include <lib/string.h>
#include <hal/hal.h>
#include <hal/idt.h>
#include <hal/tss.h>
#include <kernel/exception.h>
#include <kernel/multiboot.h>
#include <lib/size_t.h>
#include <mm/physmem.h>
#include <mm/virtmem.h>
#include <mm/kernel_heap.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include <floppy/floppy.h>
#include <serial/serial.h>
#include <ata/ata.h>
#include <cmos/cmos_time.h>

#include <vfs/bpb.h>
#include <vfs/fat32.h>
#include <vfs/file_system.h>
#include <vbe/vbe.h>

#include <elf/elf.h>

#include <proc/elfloader.h>

#include <proc/task.h>

#include <gui/window.h>
#include <gui/rendering_context.h>
#include <gui/desktop.h>

/*

	Known Bugs:
		[FATAL] Freeze on floppy disk read,

	Other notes:

*/

void sleep (int ms);

#define CHECK_FLAG(flags,bit)	((flags) & (1 << (bit)))

extern uint32_t end;

char* strMemoryTypes[] = {

	{"None"},
	{"Available"},			//memory_region.type==1
	{"Reserved"},			//memory_region.type==2
	{"ACPI Reclaim"},		//memory_region.type==3
	{"ACPI NVS Memory"}		//memory_region.type==4
};

extern void enable_A20();
void int32_test(uint16_t mode);
void read_FAT();

// define our structure
typedef struct __attribute__ ((packed)) {
    unsigned short di, si, bp, sp, bx, dx, cx, ax;
    unsigned short gs, fs, es, ds, eflags;
} regs16_t;
 
// tell compiler our int32 function is external
extern void int32(unsigned char intnum, regs16_t *regs);


vesa_pixel_t magenta = {0xFF, 0x00, 0xFF, 0xFF}; // BGRA
vesa_pixel_t red = {0x00, 0x00, 0xFF, 0xFF};
vesa_pixel_t green = {0x00, 0xFF, 0x00, 0xFF};
vesa_pixel_t blue = {0xFF, 0x00, 0x00, 0xFF};
vesa_pixel_t black = {0x00, 0x00, 0x00, 0xFF};
vesa_pixel_t white = {0xFF, 0xFF, 0xFF, 0xFF};

// int32 test
void int32_test(uint16_t mode)
{
   	vbe_bochs_set_gfx(1024, 768, 32);

	vesa_pixel_t clear_color = {0xFF,0xFF,0xFF,0xFF};
	
	vbe_clear_screen(clear_color);
	
	vbe_fill_rect(blue, 100, 200, 300, 400);

	vbe_put_char(magenta, 16, 16, 'A');
	vbe_put_pixel(red, 16, 16);

	vbe_print_string(black, 32, 32, "Hello World!\nHello new line!");
	
	for(;;);
}

Desktop* desktop = 0;

uint16_t dbg_mouse_x = 0;
uint16_t dbg_mouse_y = 0;
uint8_t dbg_mouse_button = 0;

int my_clamp(int value, int low, int high)
{
	if(value > high)
	{
		return high;
	}

	if(value < low)
	{
		return low;
	}

	return value;
}

void gui_mouse_moved_handler(mouse_moved_event_t* ev)
{
	dbg_mouse_x = (uint16_t)my_clamp(dbg_mouse_x + ev->dx, 0, 1024);
	dbg_mouse_y = (uint16_t)my_clamp(dbg_mouse_y - ev->dy, 0, 768);

	Desktop_ProcessMouse(desktop, dbg_mouse_x, dbg_mouse_y, dbg_mouse_button);
}

void gui_mouse_button_handler(mouse_button_event_t* ev)
{
	if(ev->button == MOUSE_BUTTON_LEFT)
	{
	    dbg_mouse_button = ev->action == MOUSE_ACTION_PRESS ? 1 : 0; 
	}

	Desktop_ProcessMouse(desktop, dbg_mouse_x, dbg_mouse_y, dbg_mouse_button);
}

void gui_test()
{
	uint16_t width = 600;
	uint16_t height = 480;
	
	vbe_bochs_set_gfx(width, height, 32);

	vesa_pixel_t clear_color = {0xFF,0xFF,0xFF,0xFF};
	
	vbe_clear_screen(clear_color);

	Rendering_Context* context = Rendering_Context_New(0,0,0);
	context->buffer = (uint32_t) vbe_get_buffer();
	context->width = vbe_get_width();
	context->height = vbe_get_height();

	desktop = Desktop_Create(context);

    Desktop_Create_Window(desktop, 10, 10, 300, 200);
	Desktop_Create_Window(desktop, 100, 150, 400, 400);
	Desktop_Create_Window(desktop, 200, 100, 200, 600);

	register_mouse_moved_handler(gui_mouse_moved_handler);
	register_mouse_button_handler(gui_mouse_button_handler);

	Desktop_Paint(desktop);

	for(;;);
}

void read_FAT(){
	uint32_t sectornum = 0;

	uint8_t* buffer =(uint8_t*) kmalloc(512);
	if(!buffer){
		printf("Could not allocate buffer!\n");
		return;
	}

	ide_read_sectors(0, 1, sectornum, 0, buffer);

	boot_sector_t bootSector = {0};

	memcpy(&bootSector, buffer, 512);

	serial_printf(COM1, "\nBios Parameter Block:\n");

	char OEM[9] = {0};
	memcpy(OEM, &bootSector.bpb.OEMName[0], 8);
	serial_printf(COM1, "OEM Name: %s\n", OEM);

	serial_printf(COM1, "Bytes per sector: %i\n", bootSector.bpb.bytesPerSector);
	serial_printf(COM1, "Sectors per cluster: %i\n", bootSector.bpb.sectorsPerCluster);
	serial_printf(COM1, "Reserved sectors: %i\n", bootSector.bpb.reservedSectors);
	serial_printf(COM1, "Number of FATs (16): %i\n", bootSector.bpb.numberOfFats16);
	serial_printf(COM1, "Number of dir entries: %i\n", bootSector.bpb.numDirEntries);
	serial_printf(COM1, "Media: %i\n", bootSector.bpb.media);
	serial_printf(COM1, "Sectors per FAT (16): %i\n", bootSector.bpb.sectorsPerFat16);
	serial_printf(COM1, "Sectors per track: %i\n", bootSector.bpb.sectorsPerTrack);
	serial_printf(COM1, "Heads per cylinder: %i\n", bootSector.bpb.headsPerCyl);
	serial_printf(COM1, "Hidden sectors: %i\n", bootSector.bpb.hiddenSectors);
	serial_printf(COM1, "Long sectors: %i\n", bootSector.bpb.longSectors);
	serial_printf(COM1, "Sectors per FAT (32): %i\n", bootSector.bpbExt.sectorsPerFat32);
	serial_printf(COM1, "Flags: %i\n", bootSector.bpbExt.flags);
	serial_printf(COM1, "Version: %i\n", bootSector.bpbExt.version);
	serial_printf(COM1, "Root cluster: %i\n", bootSector.bpbExt.rootCluster);
	serial_printf(COM1, "Info cluster: %i\n", bootSector.bpbExt.infoCluster);
	serial_printf(COM1, "Backup Boot: %i\n", bootSector.bpbExt.backupBoot);

	uint32_t first_data_sector = bootSector.bpb.reservedSectors + bootSector.bpbExt.sectorsPerFat32*bootSector.bpb.numberOfFats16;

	kfree(buffer);
}

extern void syscall_interrupt_handler();



void my_moved_handler(mouse_moved_event_t* ev)
{
	printf("Mouse Moved\n");
}

void my_button_handler(mouse_button_event_t* ev)
{
	printf("Mouse Button %i, %i\n", ev->button, ev->action);
}

void my_scroll_handler(mouse_scroll_event_t* ev)
{
	printf("Mouse Scroll %i, %i\n", ev->horizontal, ev->vertical);
}

void init(multiboot_info_t* mb_ptr, unsigned int esp){

	//asm volatile("xchgw %bx,%bx");

	clearScreen();

	//printf("Multiboot at %#p\n", mb_ptr);
	
	printf("OS kernel booting (%s)...\nPlease wait...\n", __FUNCTION__);

	printf("Initiating COM1 serial port\n");

	init_serial(COM1, SERIAL_BAUD_115200, 8, 1, SERIAL_PARITY_NONE);

	printf("Initializing HAL\n");

	hal_initialize();

	printf("HAL initialization done!\n");
	
	printf("Installing keyboard\n");

	keyboard_install (33);

	keyboard_set_autorepeat(0x10, 1);

	keyboard_disable();
	
	printf("Installing mouse\n");

	// This causes the keyboard to stop working due to a
	// dodgy PS/2 controller.
	
	mouse_install();

	keyboard_enable();

	setvect (0,(void (*)(void))divide_by_zero_fault, 0);
	setvect (1,(void (*)(void))single_step_trap, 0);
	setvect (2,(void (*)(void))nmi_trap, 0);
	setvect (3,(void (*)(void))breakpoint_trap, 0);
	setvect (4,(void (*)(void))overflow_trap, 0);
	setvect (5,(void (*)(void))bounds_check_fault, 0);
	setvect (6,(void (*)(void))invalid_opcode_fault, 0);
	setvect (7,(void (*)(void))no_device_fault, 0);
	setvect (8,(void (*)(void))double_fault_abort, 0);
	setvect (10,(void (*)(void))invalid_tss_fault, 0);
	setvect (11,(void (*)(void))no_segment_fault, 0);
	setvect (12,(void (*)(void))stack_fault, 0);
	setvect (13,(void (*)(void))general_protection_fault, 0);
	setvect (14,(void (*)(void))page_fault, 0);
	setvect (16,(void (*)(void))fpu_fault, 0);
	setvect (17,(void (*)(void))alignment_check_fault, 0);
	setvect (18,(void (*)(void))machine_check_abort, 0);
	setvect (19,(void (*)(void))simd_fpu_fault, 0);

	setvect(0x80, (void(*)(void))syscall_interrupt_handler, I86_IDT_DESC_RING3);

	//printf("CPU Vendor: %s\n", get_cpu_vendor());

	//clearScreen();

	uint32_t memSize = 1024 + mb_ptr->mem_lower + mb_ptr->mem_upper;

	//printf("Memory size: %i kB (%i MB)\n", memSize, memSize/1024);

	uint32_t kernel_end = (uint32_t)(&end);

	//printf("End: %#p\n", kernel_end);

	uint32_t kernel_size = kernel_end - 0xC0100000;

	//printf("Kernel size: %#i\n", kernel_size);

	pmmngr_init(memSize, 0xC0000000 + kernel_size);

	// Calculate number of mmap entries
	size_t entries = mb_ptr->mmap_length / sizeof(memory_map_t);
	
	memory_map_t* mem_map = (memory_map_t*)(mb_ptr->mmap_addr+0xC0000000);

	//printf("Mem_map: %#p\n", mem_map);

	for(int i = 0; i < entries; ++i){
		/*printf("[Entry %i] Size: %i, addr_high: %p, addr_low: %p\n, length_high: %i, length_low: %i, type: %s\n",
			i,
			mem_map[i].size,
			mem_map[i].base_addr_high,
			mem_map[i].base_addr_low,
			mem_map[i].length_high,
			mem_map[i].length_low,
			strMemoryTypes[mem_map[i].type]);*/
		

		if(mem_map[i].type == 1){
			pmmngr_init_region(mem_map[i].base_addr_low + 0xC0000000, mem_map[i].length_low);
			//printf("Initializing region: A:%#p, S:%i\n", mem_map[i].base_addr_low  + 0xC0000000, mem_map[i].length_low);
		}
	}

	pmmngr_deinit_region(0xC0100000, kernel_size);
	pmmngr_deinit_region(0xC0001000, 0x4000); // For VESA

	//printf ("\npmm regions initialized: %i allocation blocks; used or reserved blocks: %i\nfree blocks: %i\n",
	//	pmmngr_get_block_count (),  pmmngr_get_use_block_count (), pmmngr_get_free_block_count () );

	printf("Initializing VMM\n");

	vmmngr_initialize();

	printf("Initializing ACPI\n");

	initAcpi();

	printf("Initiating CMOS\n");

	initRTC(acpiGetCenturyRegister());

	printf("Initializing kernel heap\n");

	init_kernel_heap();

	printf("Initializing PCI\n");

	pciInit();
	
#if 0
	printf("Initializing floppy driver\n");

	printf("Setting working drive\n");
	// Set floppy drive to 0
	floppy_disk_set_working_drive(0);

	printf("Installing IRQ\n");

	// Install floppy disk on IR 38 (IRQ 6)
	floppy_disk_install(38);
#endif

	// TODO: This should use values from PCI
	ide_initialize(0x1F0, 0x3F6, 0x170, 0x376, 0x000);

	// Initialize FAT

	printf("Initializing FAT\n");

	FAT_initialize();

	// Fetch video information
	vesa_get_video_info();

	read_FAT();
	
	install_tss (5,0x10,esp);

	register_mouse_moved_handler(my_moved_handler);
	register_mouse_button_handler(my_button_handler);
	register_mouse_scroll_handler(my_scroll_handler);

	printf("Kernel initialization done!\n");
}

void sleep (int ms) {

	int ticks = ms + get_tick_count ();
	while (ticks > get_tick_count ())
		;
}

void cmd () {

	printf ("\n> ");
}

void get_cmd (char* buf, int n) {

	cmd ();

	KEYCODE key = KEY_UNKNOWN;
	int	BufChar;

	//! get command string
	int i=0;
	while ( i < n ) {

		//! buffer the next char
		BufChar = 1;

		//! grab next char
		key = getch ();


		if(!isprint(keyboard_key_to_ascii(key)))
			BufChar = 0;

		//! end of command if enter is pressed
		if (key==KEY_RETURN)
			break;

		switch(key){
			case KEY_LSHIFT:
			case KEY_RSHIFT:
			case KEY_LCTRL:
			case KEY_RCTRL:
				continue;
		}

		//! backspace
		if (key==KEY_BACKSPACE) {

			//! dont buffer this char
			BufChar = 0;

			if (i > 0) {

				//! go back one char
				unsigned y, x;
				getCursor (&x, &y);
				if (x>0)
					setCursor (--x, y);
				else {
					//! x is already 0, so go back one line
					y--;
					x = 80; // TODO: Do not hardcode
				}

				//! erase the character from display
				putch (' ');
				setCursor (x, y);

				//! go back one char in cmd buf
				i--;
			}
		}

		//! only add the char if it is to be buffered
		if (BufChar) {

			//! convert key to an ascii char and put it in buffer
			char c = keyboard_key_to_ascii (key);
			if (c != 0) { //insure its an ascii char

				putch (c);
				buf [i++] = c;
			}
		}
	}

	//! null terminate the string
	buf [i] = '\0';

	putch('\n');
}

typedef struct{

	uint32_t useful_value;
	uint32_t useless_value;

} test_struct_t;

int run_cmd (char* cmd_buf) {

	//! exit command
	if (strcmp (cmd_buf, "exit") == 0) {
		return 1;
	}

	//! clear screen
	else if (strcmp (cmd_buf, "cls") == 0) {
		clearScreen();
	}

	else if (strcmp (cmd_buf, "time") == 0) {
		time_t currentTime;

		readRTC(&currentTime);

		
		printf("\nCurrent time and date:\n%0(2)i:%0(2)i:%0(2)i %0(2)i/%0(2)i/%0(4)i\n\n", 
		currentTime.hour, 
		currentTime.minute, 
		currentTime.second, 
		currentTime.day, 
		currentTime.month, 
		currentTime.year);
	}

	//! Test video mode change
	else if (strcmp (cmd_buf, "vid") == 0) {
		uint32_t videoMode = 0;
		/*
		char vidModeBuf[16];

		printf("Enter Video Mode: \n");
		get_cmd(vidModeBuf, 16);
		videoMode = strtol(vidModeBuf, 0, 16);

		if(videoMode == 0)
			return;
		*/
		// This does a video mode switch
		// Do not enable until graphics mode driver and serial debug
		// output driver is developed.
		int32_test((uint16_t)videoMode);
	}

	else if (strcmp(cmd_buf, "heap") == 0) {
		printf("\nStarting kernel heap test.");
		printf("\nAllocating int");

		int* i = kmalloc(sizeof(int));

		printf("\nAddress: %#(10)p", i);

		printf("\nWriting to int");

		*i = 4;

		printf("\nNew value: %i", *i);

		printf("\nAllocating struct");

		test_struct_t* p = kmalloc(sizeof(test_struct_t));

		printf("\nAddress: %#(10)p", p);

		printf("\nWriting to struct");

		p->useful_value = 10;

		printf("\nNew value: %i", p->useful_value);

		printf("\nFreeing int");

		kfree(i);

		printf("\nFreeing struct");

		kfree(p);

	}

	//! help
	else if (strcmp (cmd_buf, "help") == 0) {

		printf("\nHelp text...");
#if 0
		serial_printf(COM1, "%c", 'c');
		serial_printf(COM1, "\n%i", 4);
		serial_printf(COM1, "\n%(5)i", 4);
		serial_printf(COM1, "\n%*i", 5, 4);
		serial_printf(COM1, "\n%(5)i", 123456);
		serial_printf(COM1, "\n%*i", 5, 123456);
		serial_printf(COM1, "\n%0(5)i", 4);
		serial_printf(COM1, "\n%-(5)i", 4);
#endif
	}
	else if(strcmp (cmd_buf, "readfloppybuggy") == 0) {
		uint32_t sectornum = 0;
		char sectornumbuf[4];
		const uint8_t* sector = 0;

		printf("Enter sector num: \n");
		get_cmd(sectornumbuf, 5);
		sectornum = atoi(sectornumbuf);

		printf("Sector %i contents:\n\n", sectornum);

		sector = floppy_disk_read_sector(sectornum);

		if(sector != 0){

			for(int c = 0; c < 512; c+=128){
				for(int i = 0; i < 128; ++i){
					printf("0x%p ", sector[c+i]);
				}

				printf("Press any key to continue\n");

				getch();
			}
		}
		else{

			printf("\nError reading from disk");
		}
		printf("\nDone\n");
	}

	else if(strcmp (cmd_buf, "proc") == 0){
		char* filePath = "proc.elf";

		printf("\nReading file %s\n", filePath);

		FILE file;
		FS_ERROR e;

		e = fs_open_file(&file, filePath, 0);

		if(e != 0)
		{
			printf("Could not open file: %s\n", fs_err_str(e));
			fs_close_file(&file);
			return;
		}


		// Make sure that we do have enough space in buffer.
		char* buffer = (char*)kmalloc((file.fileLength) + 512);

		int sectors_read = 0;

		while(!file.eof){
			e = fs_read_file(&file, buffer + 512*sectors_read, 0);
			++sectors_read;
		}	

		fs_close_file(&file);

		// Executabe header is always first.
		Elf32_Ehdr* ehdr = (Elf32_Ehdr*) buffer;

		if(is_elf(ehdr))
		{
			printf("Is Elf\n");
		}
		else
		{
			printf("Is not Elf\n");
		}

		if(ehdr->e_type == ET_EXEC)
		{
			printf("Is executable\n");
		}

		printf("%i\n", ehdr->e_shoff);

		printf("%i\n", ehdr->e_shnum);

		for(int i = 0; i < ehdr->e_shnum; ++i)
		{
			Elf32_Shdr* shdr = (Elf32_Shdr*) (buffer + ehdr->e_shoff + i*ehdr->e_shentsize);

			serial_printf(COM1, "Section header %i:", i);

			serial_printf(COM1, "Type: %i\n", shdr->sh_type);

		}

		printf("%i\n", ehdr->e_phoff);
		printf("%i\n", ehdr->e_phnum);

		for(int i = 0; i < ehdr->e_phnum; ++i)
		{
			Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + ehdr->e_phoff + i*ehdr->e_phentsize);

			if(phdr->p_type == PT_LOAD)
			{
				printf("Mem needed: %i bytes\n", phdr->p_memsz);

				// Map a new memory block to requested virtual address.
				vmmngr_mapPhysicalAddress(vmmngr_get_directory(), phdr->p_vaddr, (uint32_t)pmmngr_alloc_block(), I86_PTE_WRITABLE | I86_PTE_PRESENT);
				
				// Copy program to block.
				memcpy((void*)phdr->p_vaddr, buffer + phdr->p_offset, phdr->p_memsz);
			}

		}

		typedef int(*EntryFunc)(void);

		EntryFunc entry = (EntryFunc)ehdr->e_entry;

		int returnCode = entry();

		printf("Returncode: %#x", returnCode);

		// Free buffer when we are done.
		kfree(buffer);
	}

	else if (strcmp(cmd_buf, "newproc") == 0)
	{
		ElfImage img = loadELF("proc.elf");

		if(img.valid)
		{		
			EntryFunc entry = img.entry;

			printf("Stack size: %i\n", img.stackSize);
		
			int returnCode = entry();

			printf("Returncode: %#x", returnCode);
		}

	}

	else if (strcmp(cmd_buf, "process") == 0)
	{
		createProcess("proc.elf", 0);
		//executeProcess();
	}

	else if (strcmp(cmd_buf, "gui") == 0)
	{
		gui_test();
	}

	else if(strcmp (cmd_buf, "readfile") == 0){

		char filePath[100] = {0};

		printf("\nEnter path:");
		get_cmd(filePath, 100);
		printf("\n");

		FILE file;
		FS_ERROR e;
		
		e = fs_open_file(&file, filePath, 0);

		if(e!=0){
			printf("File open error: %s\n", fs_err_str(e));
			fs_close_file(&file);
			return;
		}

		char* buffer = (char*) kmalloc(512);
		while(!file.eof){
			e = fs_read_file(&file, buffer, 0);

			if(e != 0){
				printf("File read error: %s\n", fs_err_str(e));
			}

			for(int i = 0; i < 512; ++i){
				printf("%c", buffer[i]);
			}

			getch();
		}

		fs_close_file(&file);
	}

	else if(strcmp (cmd_buf, "write") == 0){
		return; // We do not want to write anything yet.


		uint32_t sectornum = 0;

		uint8_t* buffer =(uint8_t*) kmalloc(512);

		if(!buffer){
			printf("Could not allocate buffer!\n");
			return;
		}

		memset(buffer,0x00, 512);

		ide_write_sectors(0, 1, sectornum, 0, buffer);
	}

	//! invalid command
	else {
		printf ("\nUnkown command");
	}

	return 0;
}

void run () {

	char cmd_buf [100];

	while (1) {

		//! get command
		get_cmd (cmd_buf, 98);

		//! run command
		if (run_cmd (cmd_buf) == 1)
			break;
	}
}

void time_updater()
{
	while(1)
	{
		asm volatile ("cli");
		time_t currentTime;

		readRTC(&currentTime);

		uint32_t x, y;
	
		getCursor(&x,&y);

		setCursor(0,0);

		printf("Current time and date:%0(2)i:%0(2)i:%0(2)i %0(2)i/%0(2)i/%0(4)i", 
		currentTime.hour, 
		currentTime.minute, 
		currentTime.second, 
		currentTime.day, 
		currentTime.month, 
		currentTime.year);

		setCursor(x,y);	

		asm volatile ("sti");

		thread_sleep(10);
	}
}

void idle_func()
{
	
	printf("Welcome to the new improved (multitasking) OS4 kernel.\n \
		 Developed by Joakim Bertils.\n");

	

	Thread* time_updater_thread = createThread(getKernelProcess(), time_updater, 1);

	run();

	acpiPowerOff();

	for(;;);
}


/** kernel_main:
 *  The C entry point of the kernel.
 *  
 *	@return 	0 when done
 */
int kernel_main(unsigned int ebx, unsigned int eax, unsigned int esp)
{
	multiboot_info_t* mb_ptr = (multiboot_info_t*) (ebx);

	init(mb_ptr, esp);

	clearScreen();

	initialize_scheduler();

	Thread* idleThread = createThread(getKernelProcess(), idle_func, 1);

	thread_execute(idleThread);

	//printf("Welcome to OS4 kernel text based GUI. Developed by Joakim Bertils.\n");
	//run ();

	//printf("\nExit command recieved\n");

	for(;;);

	return 0;
}
