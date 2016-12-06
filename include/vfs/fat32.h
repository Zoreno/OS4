#ifndef _FAT32_H
#define _FAT32_H

#include <lib/stdint.h>
#include <vfs/bpb.h>
#include <vfs/file_system.h>

typedef struct {

	// Leading signature
	// Value 0x41615252
	uint32_t FSI_LeadSig;

	// Reserved. Do not use
	uint8_t FSI_reserved1[480];

	// Another signature
	// Value 0x61417272
	uint32_t FSI_StrucSig;

	// Last known free cluster count on volume
	// If value is 0xFFFFFFFF, we must compute the value ourself.
	uint32_t FSI_Free_Count;

	// Indicates the cluster number where the driver should start looking for
	// free clusters.
	// If value is 0xFFFFFFFF, we must compute the value ourself.
	uint32_t FSI_Next_Free;

	// Reserved. Do not use
	uint8_t FSI_reserved2[12];

	// Trailing signature
	// Value 0xAA550000
	uint32_t FSI_TrailSig;

} __attribute__((packed)) FAT32_FSInfo_t;

#define ATTR_READ_ONLY	0x01
#define ATTR_HIDDEN		0x02
#define ATTR_SYSTEM		0x04
#define ATTR_VOLUME_ID	0x08
#define ATTR_DIRECTORY	0x10
#define ATTR_ARCHIVE	0x20
#define ATTR_LONG_NAME  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_LONG_NAME_MASK (ATTR_LONG_NAME | ATTR_DIRECTORY | ATTR_ARCHIVE)

#define has_attr(dir_attr, attr) ((dir_attr & attr) == attr)

typedef struct {

	// 2-second count (0-29)
	uint16_t seconds : 5;

	// Minutes (0-59)
	uint16_t minutes : 6;

	// Hours (0-23)
	uint16_t hours : 5;
} __attribute__((packed)) DIR_time_entry_t;

typedef struct {

	// Day of month (1-31)
	uint16_t day : 5;

	// Month 1 = January (1-12)
	uint16_t month : 4;

	// Year from 1980, 0 = 1980 (0-127)
	uint16_t year : 7;
} __attribute__((packed)) DIR_date_entry_t;

// Directory entry struct
typedef struct {

	// Short Name
	char DIR_Name[11];

	// Directory attributes
	uint8_t DIR_Attr;

	// Reserved for windows NT
	uint8_t DIR_NTRes;

	// Millisecond stamp at file creation time
	uint8_t DIR_CrtTimeTenth;

	// Time file was created
	DIR_time_entry_t DIR_CrtTime;

	// Date file was created
	DIR_date_entry_t DIR_CrtDate;

	// Last access date
	DIR_date_entry_t DIR_LstAccDate;

	// High word of entry's first cluster number
	uint16_t DIR_FstClusHI;

	// Time file was last written to.
	DIR_time_entry_t DIR_WrtTime;

	// Date file was last written to.
	DIR_date_entry_t DIR_WrtDate;

	// Low word of entry's first cluster number
	uint16_t DIR_FstClusLO;

	// File size in bytes
	uint32_t DIR_FileSize;

} __attribute__((packed)) DIR_Ent_t;

// Long name directory entry
typedef struct {
	uint8_t LDIR_Ord;
	short LDIR_Name1[5];
	uint8_t LDIR_Attr;
	uint8_t LDIR_Type;
	uint8_t Chksum;
	short LDIR_Name2[6];
	uint16_t LDIR_FstClustLO;
	short LDIR_Name3[2];
} __attribute__((packed)) LDIR_Ent_t;

#define FAT_CLUSTER_FREE 0x00000000
#define FAT_CLUSTER_BAD 0xFFFFFFF7
#define FAT_CLUSTER_EOC 0x0FFFFFF8

FS_ERROR FAT_initialize();

FS_ERROR FAT_fopen(PFILE file, const char* filePath, uint32_t flags);

FS_ERROR FAT_fclose(PFILE file);

FS_ERROR FAT_fread(PFILE file, void* buffer, size_t length);

FS_ERROR FAT_fwrite(PFILE file, const void* buffer, size_t length);

#endif