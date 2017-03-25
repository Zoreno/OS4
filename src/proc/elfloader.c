#include <proc/elfloader.h>

#include <elf/elf.h>

#include <lib/stdio.h>
#include <vfs/file_system.h>
#include <mm/physmem.h>
#include <mm/virtmem.h>

#define MIN_NUM_BLOCKS(size, blockSize) (((size) + (blockSize) - 1) / (blockSize))

#define IMAGE_OK 0
#define IMAGE_NOT_OK -1

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
			I86_PTE_WRITABLE | I86_PTE_PRESENT);
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

EntryFunc loadELF(const char* filePath)
{
	FILE file;
	FS_ERROR e;

	e = fs_open_file(&file, filePath, 0);

	if(e != 0)
	{
		printf("Could not open file: %s\n", fs_err_str(e));
		fs_close_file(&file);
		return 0;
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

	if(verifyImage(buffer) != IMAGE_OK)
	{
		kfree(buffer);

		printf("Image is not in supported ELF format\n");
		return 0;
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

	// Return entry point.
	return entry;
}

