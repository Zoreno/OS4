#ifndef _ELFLOADER_H
#define _ELFLOADER_H

typedef int(*EntryFunc)(void);

EntryFunc loadELF(const char* filePath);

#endif
