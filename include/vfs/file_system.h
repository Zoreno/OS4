#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

typedef enum{

	FSE_GOOD = 0,

	FSE_DEVICE_NOT_PRESENT,

	FSE_BAD_FILE,
	FSE_FILE_NOT_FOUND,
	FSE_DIR_NOT_FOUND,
	FSE_INVALID_FILE_NAME,

	FSE_FILE_CORRUPT,

	FSE_EOF

} FS_ERROR;

typedef struct{

	char name[32];
	uint32_t flags;
	uint32_t fileLength;
	uint32_t id;
	uint32_t eof;
	uint32_t position;
	uint32_t currentCluster;
	uint32_t deviceID;

} FILE;

typedef FILE* PFILE;

typedef struct{

	char name[8];

	FS_ERROR (*p_install)(void);

	FS_ERROR (*f_open)	(PFILE file, const char* fileName, uint8_t flags);
	FS_ERROR (*f_close)	(PFILE file);

	FS_ERROR (*f_read)	(PFILE file, void* buffer, size_t length);
	FS_ERROR (*f_write)	(PFILE file, const void* buffer, size_t length);
	
} FILESYSTEM;

typedef FILESYSTEM* PFILESYSTEM;

#define FS_FILE 0
#define FS_DIRECTORY 1
#define FS_INVALID 2

#define IS_INVALID(file) (file.flags == FS_INVALID)
#define IS_DIRECTORY(file) ((file.flags & FS_DIRECTORY) == FS_DIRECTORY)

#endif