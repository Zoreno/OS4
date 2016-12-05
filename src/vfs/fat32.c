#include <vfs/fat32.h>
#include <lib/stdio.h>
#include <lib/string.h>
#include <ata/ata.h>
#include <vfs/bpb.h>

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

FS_ERROR FAT_parse_directory(uint32_t startingCluster);

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

	// Read the sector containing the requested value
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
		printf("current_cluster: %i\n", current_cluster);

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

FS_ERROR FAT_parse_directory(uint32_t startingCluster){

	uint32_t chain_length = get_cluster_chain_length(startingCluster);

	// Allocate buffer
	DIR_Ent_t* buffer = (DIR_Ent_t*)kmalloc(512*chain_length);

	uint32_t current_cluster = startingCluster;

	for(uint32_t i = 0; i < chain_length; ++i){

		ide_read_sectors(0,1, first_data_sector + current_cluster, 0, buffer + i*16);

		// TODO check for bad cluster

		current_cluster = read_FAT_entry(current_cluster);
	}

	for(int i = 0; i < 16*chain_length; ++i){
		if(buffer[i].DIR_Name[0] == 0xE5){
			printf("Entry %i: FREE\n", i);
		} else if(buffer[i].DIR_Name[0] == 0x00){
			printf("Entry %i: FREE(last)\n", i);
			break;
		} else if(has_attr(buffer[i].DIR_Attr, ATTR_LONG_NAME)){
			LDIR_Ent_t* long_name_entry = (LDIR_Ent_t*)&buffer[i];
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
				printf("Entry %i: %s[LONG_NAME]\n", i, name);
		} else if(has_attr(buffer[i].DIR_Attr, ATTR_DIRECTORY)){
			char entry_name[12] = {0};
			memcpy(entry_name, &buffer[i].DIR_Name, 11);
			printf("Entry %i: %s [DIR]\n", i, entry_name);
		} else {
			char entry_name[12] = {0};
			memcpy(entry_name, &buffer[i].DIR_Name, 11);
			printf("Entry %i: %(11)s %i bytes [FILE]\n", i, entry_name, buffer[i].DIR_FileSize);
		}
	}

	return FSE_GOOD;
}

//===================================================================
// FAT file system function implementations
//===================================================================

FS_ERROR FAT_initialize(){
	ide_read_sectors(0, 1, BOOT_SECTOR_NUMBER, 0, &_bootSector);

	first_data_sector = _bootSector.bpb.reservedSectors 
			+ _bootSector.bpbExt.sectorsPerFat32*_bootSector.bpb.numberOfFats16 - 2;

	FAT_parse_directory(_bootSector.bpbExt.rootCluster);

	return FSE_GOOD;
}

FS_ERROR FAT_fopen(PFILE file, const char* filePath, uint8_t flags){

}

FS_ERROR FAT_fclose(PFILE file){

}

FS_ERROR FAT_fread(PFILE file, void* buffer, size_t length){

}

FS_ERROR FAT_fwrite(PFILE file, const void* buffer, size_t length){

}