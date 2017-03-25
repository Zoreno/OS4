#include <elf/elf.h>


int is_elf(Elf32_Ehdr* hdr)
{
	if(hdr->e_ident[EI_MAG0] != ELFMAG0)
		return 0;
	if(hdr->e_ident[EI_MAG1] != ELFMAG1)
		return 0;
	if(hdr->e_ident[EI_MAG2] != ELFMAG2)
		return 0;
	if(hdr->e_ident[EI_MAG3] != ELFMAG3)
		return 0;

	return 1;
}

unsigned long elf_hash(const unsigned char* name)
{
	unsigned long h = 0;
	unsigned long g;

	while(*name)
	{
		h = (h << 4) + *name++;
		if(g = h & 0xF0000000)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}