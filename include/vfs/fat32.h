#ifndef _FAT32_H
#define _FAT32_H

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

#endif