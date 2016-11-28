#ifndef _BPB_H
#define _BPB_H

#include <lib/stdint.h>

/**
*	bios paramater block
*/
typedef struct {

	uint8_t 	OEMName[8];
	uint16_t 	bytesPerSector;
	uint8_t		sectorsPerCluster;
	uint16_t 	reservedSectors;
	uint8_t		numberOfFats16;
	uint16_t	numDirEntries;
	uint16_t	numSectors;
	uint8_t		media;
	uint16_t	sectorsPerFat16;
	uint16_t	sectorsPerTrack;
	uint16_t	headsPerCyl;
	uint32_t	hiddenSectors;
	uint32_t	longSectors;

}__attribute__((packed)) bios_param_block_t;

/**
*	bios paramater block extended attributes
*/
typedef struct {

	uint32_t	sectorsPerFat32;
	uint16_t	flags;
	uint16_t	version;
	uint32_t	rootCluster;
	uint16_t	infoCluster;
	uint16_t	backupBoot;
	uint16_t	reserved[12];

}__attribute__((packed)) bios_ext_param_block_t;

/**
*	Boot sector. First sector of the disk.
*/
typedef struct {

	uint8_t 				jmp[3]; // Jump instruction. Ignore for all other purposes.
	bios_param_block_t 		bpb;
	bios_ext_param_block_t  bpbExt;
	uint8_t 				filler[436]; // Rest of boot sector
	
}__attribute__((packed)) boot_sector_t;

#endif