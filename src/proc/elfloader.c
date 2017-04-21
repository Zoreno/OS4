#include <proc/elfloader.h>

#include <elf/elf.h>

#include <lib/stdio.h>
#include <lib/stdarg.h>
#include <vfs/file_system.h>
#include <mm/physmem.h>
#include <mm/virtmem.h>

#define MIN_NUM_BLOCKS(size, blockSize) (((size) + (blockSize) - 1) / (blockSize))

#define IMAGE_OK 0
#define IMAGE_NOT_OK -1

#define PRINT_DEBUG 1

#if PRINT_DEBUG == 1
#define DEBUG_PRINTF(...) printf(__VA_ARGS__);
#else
#define DEBUG_PRINTF(...)
#endif 

void mapSegment(uint32_t vaddr, uint32_t size);
int verifyImage(char* buffer);

void mapSegment(uint32_t vaddr, uint32_t size)
{
	uint32_t numBlocks = MIN_NUM_BLOCKS(size, 0x1000);

	for(int i = 0; i < numBlocks; ++i)
	{
		// TODO: Keep track of these so they can be free'd later.
		uint32_t physAddr = pmmngr_alloc_block();

		// TODO: Check flags of header and map accordingly.
		vmmngr_mapPhysicalAddress(
			vmmngr_get_directory(),
			vaddr + 0x1000*i, 
			physAddr, 
			I86_PTE_WRITABLE | I86_PTE_PRESENT | I86_PTE_USER);
	}
}

int verifyImage(char* buffer)
{
	Elf32_Ehdr* ehdr = (Elf32_Ehdr*) buffer;

	if(!is_elf(ehdr))
	{
		return IMAGE_NOT_OK;
	}

	if(ehdr->e_ident[EI_CLASS] != ELFCLASS32)
	{
		return IMAGE_NOT_OK;
	}

	if(ehdr->e_ident[EI_DATA] != ELFDATA2LSB)
	{
		return IMAGE_NOT_OK;
	}

	if(ehdr->e_type != ET_EXEC)
	{
		return IMAGE_NOT_OK;
	}

	if(ehdr->e_machine != EM_386)
	{
		return IMAGE_NOT_OK;
	}

	if(ehdr->e_version != EV_CURRENT)
	{
		return IMAGE_NOT_OK;
	}

	return IMAGE_OK;
}

typedef struct _StackInfo
{
	uint32_t size;
	uint32_t start;
	uint32_t end;
} StackInfo;

void readSections(Elf32_Ehdr* hdr, StackInfo* sInfo)
{
	// Store byte pointer for accurate pointer calculations.
	uint8_t* h_addr = (uint8_t*) hdr;

	// Find the section string table
	uint32_t sectionStringTableIndex = hdr->e_shstrndx;

	Elf32_Shdr* sectionStringTableSection = 
		(Elf32_Shdr*)(h_addr + hdr->e_shoff + sectionStringTableIndex*hdr->e_shentsize);

	char* sectionStringTable = (char*)(h_addr + sectionStringTableSection->sh_offset);

	// Map out all sections we are interested in
	uint32_t symtabSectionIndex = 0;
	uint32_t strtabSectionIndex = 0;
	
	for(int i = 0; i < hdr->e_shnum; ++i)
	{
		Elf32_Shdr* shdr = (Elf32_Shdr*)(h_addr + hdr->e_shoff + i*hdr->e_shentsize);

		if(!shdr->sh_name)
			continue;

		if(strcmp(&sectionStringTable[shdr->sh_name], ".strtab") == 0)
			strtabSectionIndex = i;

		if(strcmp(&sectionStringTable[shdr->sh_name], ".symtab") == 0)
			symtabSectionIndex = i;
	}

	if(!strtabSectionIndex)
	{
		DEBUG_PRINTF("NO STRTAB\n");
		return;
	}	

	// Find STRTAB
	Elf32_Shdr* strtabSection = 
		(Elf32_Shdr*)(h_addr + hdr->e_shoff + strtabSectionIndex*hdr->e_shentsize);	

	char* strTab = (char*)(h_addr + strtabSection->sh_offset);

	// Find SYMTAB
	Elf32_Shdr* symtabSection = 
		(Elf32_Shdr*)(h_addr + hdr->e_shoff + symtabSectionIndex*hdr->e_shentsize);

	DEBUG_PRINTF("Symtab:\n");
	DEBUG_PRINTF("Size: %i\n", symtabSection->sh_size);
	DEBUG_PRINTF("Offset: %i\n", symtabSection->sh_offset);
	DEBUG_PRINTF("Entry Size: %i\n", symtabSection->sh_entsize);
	uint32_t num_entries = symtabSection->sh_size / symtabSection->sh_entsize;
	DEBUG_PRINTF("Entries: %i\n", num_entries);
	for(int j = 0; j < num_entries; ++j)
	{
		Elf32_Sym* sym = 
			(Elf32_Sym*)(h_addr + symtabSection->sh_offset + j*symtabSection->sh_entsize);
		//DEBUG_PRINTF("[Entry %i] Name: %s, Address: %#x\n", 
		//	j, &strTab[sym->st_name], sym->st_value);

		if(strcmp(&strTab[sym->st_name], "__OS4_stack_size__") == 0)
			sInfo->size = sym->st_value;

		if(strcmp(&strTab[sym->st_name], "__OS4_stack_start__") == 0)
			sInfo->start = sym->st_value;

		if(strcmp(&strTab[sym->st_name], "__OS4_stack_end__") == 0)
			sInfo->end = sym->st_value;
	}
}

ElfImage loadELF(const char* filePath)
{
	FILE file;
	FS_ERROR e;

	ElfImage ret = {0};

	e = fs_open_file(&file, filePath, 0);

	if(e != 0)
	{
		printf("Could not open file: %s\n", fs_err_str(e));
		fs_close_file(&file);
		return ret;
	}

	uint32_t buffer_size = MIN_NUM_BLOCKS(file.fileLength, 512) * 512;

	char* buffer = (char*)kmalloc(buffer_size);

	int sectors_read = 0;

	while (!file.eof)
	{

		e = fs_read_file(&file, buffer + 512*sectors_read, 0);
		++sectors_read;

	}

	fs_close_file(&file);

	// Executabe header is always first.
	Elf32_Ehdr* ehdr = (Elf32_Ehdr*) buffer;

	StackInfo sInfo = {0};

	readSections(ehdr, &sInfo);

	if(verifyImage(buffer) != IMAGE_OK)
	{
		kfree(buffer);

		printf("Image is not in supported ELF format\n");
		return ret;
	}

	for(int i = 0; i < ehdr->e_phnum; ++i)
	{
		Elf32_Phdr* phdr = (Elf32_Phdr*)(buffer + ehdr->e_phoff + i*ehdr->e_phentsize);

		if(phdr->p_type == PT_LOAD)
		{
			// Map the segment
			mapSegment(phdr->p_vaddr, phdr->p_memsz);

			// Copy program to block.
			memcpy((void*)phdr->p_vaddr, buffer + phdr->p_offset, phdr->p_memsz);
		}
	}

	EntryFunc entry = (EntryFunc)ehdr->e_entry;

	// Free the buffer with the image now that the loadable segments is loaded.
	kfree(buffer);

	ret.valid = 1;
	ret.entry = entry;
	ret.stackSize = sInfo.size;
	ret.stackStart = sInfo.start;
	ret.stackEnd = sInfo.end;
	return ret;
}

