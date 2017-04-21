#ifndef _ELFLOADER_H
#define _ELFLOADER_H

#include <lib/stdint.h>

typedef int(*EntryFunc)(void);

typedef struct _ElfImage
{
	uint32_t valid;
	EntryFunc entry;

	uint32_t stackSize;
	uint32_t stackStart;
	uint32_t stackEnd;
} ElfImage;

ElfImage loadELF(const char* filePath);

#endif
