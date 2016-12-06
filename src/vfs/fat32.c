#include <vfs/fat32.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <ata/ata.h>

FILESYSTEM _FSysFat;

boot_sector_t _bootSector = {0};

uint32_t first_data_sector = 0;

#define BOOT_SECTOR_NUMBER 0

//===================================================================
// FAT structure function prototypes
//===================================================================

// Reads the FAT at entry and return the value
uint32_t read_FAT_entry(uint32_t entry);

// Writes the passed in value to the entry
void write_FAT_entry(uint32_t entry, uint32_t value);

uint32_t get_cluster_chain_length(uint32_t startingCluster);

//===================================================================
// FAT directory function prototypes
//===================================================================

FS_ERROR FAT_look_in_directory(PFILE file, uint32_t startingCluster, const char* filePath, uint32_t flags);

//===================================================================
// FAT structure function implementations
//===================================================================

uint32_t read_FAT_entry(uint32_t entry){
	uint32_t first_FAT_sector = _bootSector.bpb.reservedSectors;

	// Not initialized
	if(!first_FAT_sector){
		return FAT_CLUSTER_BAD; // Bad sector
	}

	// 128 entries per sector (512/4)
	uint32_t fat_sector_num = entry / 128;
	uint32_t fat_sector_offset = entry % 128;

	// Allocate a buffer the size of a sector
	uint32_t* buffer = (uint32_t*)kmalloc(512);

	// Read the sector containprinting the requested value
	ide_read_sectors(0,1, first_FAT_sector + fat_sector_num, 0, buffer);

	// Store the 28 lowest bits
	uint32_t value = buffer[fat_sector_offset] & 0x7FFFFFFF;

	// Free the buffer
	kfree(buffer);

	return value;
}

void write_FAT_entry(uint32_t entry, uint32_t value){

	uint32_t first_FAT_sector = _bootSector.bpb.reservedSectors;

	// Not initialized
	if(!first_FAT_sector){
		return;
	}

	// 128 entries per sector (512/4)
	uint32_t fat_sector_num = entry / 128;
	uint32_t fat_sector_offset = entry % 128;

	// Allocate a buffer the size of a sector
	uint32_t* buffer = (uint32_t*)kmalloc(512);

	// Read the sector containing the requested value
	ide_read_sectors(0,1, first_FAT_sector + fat_sector_num, 0, buffer);

	uint32_t prev_value = buffer[fat_sector_offset];

	// Store the 28 lowest bits of the new number and conserve the high 
	// 4 bits of the prev value
	buffer[fat_sector_offset] = (value & 0x7FFFFFFF) + (prev_value & 0x80000000);

	// Write the new contents to disk
	ide_write_sectors(0,1, first_FAT_sector + fat_sector_num, 0, buffer);

	// Free the buffer
	kfree(buffer);
}

uint32_t get_cluster_chain_length(uint32_t startingCluster){
	uint32_t count = 1;

	uint32_t current_cluster = startingCluster;

	while(read_FAT_entry(current_cluster) != FAT_CLUSTER_EOC){
		count++;

		if(read_FAT_entry(current_cluster) == FAT_CLUSTER_BAD)
			return 0;

		current_cluster = read_FAT_entry(current_cluster);
	}

	return count;
}

//===================================================================
// FAT directory function implementations
//===================================================================

// test.txt
// test/folder/file.dat

#define CLUSTER(low, high)((uint32_t)low + (((uint32_t)high)<<16))

FS_ERROR FAT_look_in_directory(PFILE file, uint32_t startingCluster, const char* filePath, uint32_t flags){

	// Start with parsing the filePath

	char fileName[100] = {0};
	char restOfPath[100] = {0};
	int folder = 0;
	char* p = strchr(filePath, '/');

	if(!p){
		// No occurance of '/', must be a file

		// Copy full path(should only contain fileName)
		strcpy(fileName, filePath);
	} else {
		// Occurance of '/', must be a directory
		folder = 1;

		// Get content of filePath before '/'
		strncpy(fileName, filePath, (size_t)(p-filePath-1));

		++p;

		// Get rest of filePath
		strcpy(restOfPath, p);
	}

	// Parse Directory structure
	uint32_t chain_length = get_cluster_chain_length(startingCluster);

	// Allocate buffer
	DIR_Ent_t* buffer = (DIR_Ent_t*)kmalloc(512*chain_length);

	uint32_t current_cluster = startingCluster;

	for(uint32_t i = 0; i < chain_length; ++i){

		ide_read_sectors(0,1, first_data_sector + current_cluster, 0, buffer + i*16);

		// TODO check for bad cluster

		current_cluster = read_FAT_entry(current_cluster);
	}

	char current_long_name[100] = {0};
	int hasLongName = 0;
	for(int i = 0; i < 16*chain_length; ++i){
		if(buffer[i].DIR_Name[0] == 0xE5){
			hasLongName = 0;
			memset(current_long_name, 0, 100);
		} else if(buffer[i].DIR_Name[0] == 0x00){
			
			break;
		}else if(has_attr(buffer[i].DIR_Attr, ATTR_LONG_NAME)){

			
			// TODO checksum
			LDIR_Ent_t* long_name_entry = (LDIR_Ent_t*)&buffer[i];

			uint8_t ordinal = long_name_entry->LDIR_Ord & ~(0x40);

			char name[14] = {0};
			size_t pos = 0;
			for(int i = 0; i < 5; ++i){
				name[pos++] = (char)long_name_entry->LDIR_Name1[i];
			}
			for(int i = 0; i < 6; ++i){
				name[pos++] = (char)long_name_entry->LDIR_Name2[i];
			}
			for(int i = 0; i < 2; ++i){
				name[pos++] = (char)long_name_entry->LDIR_Name3[i];
			}
			// Copy name to buffer
			memcpy(current_long_name+13*(ordinal-1), name, 13);
			hasLongName = 1;
		} else if(has_attr(buffer[i].DIR_Attr, ATTR_DIRECTORY)){

			// We found a folder, but are looking for a file
			if(!folder){
				hasLongName = 0;
				memset(current_long_name, 0, 100);
				continue;
			}

			// If we parsed a long file name, 
			if(hasLongName){
				if(strcmp(current_long_name, fileName) == 0){
					return FAT_look_in_directory(
						file, 
						CLUSTER(buffer[i].DIR_FstClusLO ,buffer[i].DIR_FstClusHI),
						restOfPath,  
						flags);
				}
			} else {
				char entry_name[12] = {0};
				memcpy(entry_name, &buffer[i].DIR_Name, 11);

				if(strncmp(entry_name, fileName, 11) == 0){
					return FAT_look_in_directory(
						file, 
						CLUSTER(buffer[i].DIR_FstClusLO ,buffer[i].DIR_FstClusHI),
						restOfPath,  
						flags);
				}
			}

			hasLongName = 0;
			memset(current_long_name, 0, 100);
		} else { // FILE
			// We found a file, but are looking for a folder

			if(folder){
				hasLongName = 0;
				memset(current_long_name, 0, 100);
				continue;
			}

			if(hasLongName){
				if(strcmp(current_long_name, fileName) == 0){
					// Found file

					// Fill in file info
					strcpy(file->name, fileName);
					file->id = 0;
					file->currentCluster = CLUSTER(buffer[i].DIR_FstClusLO ,buffer[i].DIR_FstClusHI);
					file->eof = 0;
					file->fileLength = buffer[i].DIR_FileSize;

					file->flags = FS_FILE;

					return FSE_GOOD;
				}
			} else {
				char entry_name[12] = {0};
				memcpy(entry_name, &buffer[i].DIR_Name, 11);

				if(strncmp(entry_name, fileName, 11) == 0){
					// Found file

					strcpy(file->name, fileName);
					file->id = 0;
					file->currentCluster = CLUSTER(buffer[i].DIR_FstClusLO ,buffer[i].DIR_FstClusHI);
					file->eof = 0;
					file->fileLength = buffer[i].DIR_FileSize;

					file->flags = FS_FILE;

					return FSE_GOOD;
				}
			}
			hasLongName = 0;
			memset(current_long_name, 0, 100);
		}
	}

	return FSE_FILE_NOT_FOUND;
}

//===================================================================
// FAT file system function implementations
//===================================================================

FS_ERROR FAT_initialize(){
	ide_read_sectors(0, 1, BOOT_SECTOR_NUMBER, 0, &_bootSector);

	first_data_sector = _bootSector.bpb.reservedSectors 
			+ _bootSector.bpbExt.sectorsPerFat32*_bootSector.bpb.numberOfFats16 - 2;

	
	strcpy(_FSysFat.name, "FAT32");

	_FSysFat.f_open = FAT_fopen;
	_FSysFat.f_close = FAT_fclose;
	_FSysFat.f_read = FAT_fread;
	_FSysFat.f_write = FAT_fwrite;

	fs_register(&_FSysFat, 'a');


	return FSE_GOOD;
}

FS_ERROR FAT_fopen(PFILE file, const char* filePath, uint32_t flags){
	FAT_look_in_directory(file, _bootSector.bpbExt.rootCluster, filePath, flags);
}

FS_ERROR FAT_fclose(PFILE file){

}

FS_ERROR FAT_fread(PFILE file, void* buffer, size_t length){
	if(!file){
		return FSE_BAD_FILE;
	}

	// Read one sector
	ide_read_sectors(0, 1, first_data_sector + file->currentCluster, 0, buffer);

	uint32_t nextCluster = read_FAT_entry(file->currentCluster);

	if(nextCluster == FAT_CLUSTER_EOC){
		file->eof = 1;
		return FSE_EOF;
	}

	if(nextCluster == FAT_CLUSTER_FREE){
		file->eof = 1;
		return FSE_FILE_CORRUPT;
	}

	file->currentCluster = nextCluster;

	return FSE_GOOD;
}

FS_ERROR FAT_fwrite(PFILE file, const void* buffer, size_t length){

}