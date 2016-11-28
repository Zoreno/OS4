/**
* @file floppy_disk.h
* @author Joakim Bertils
* @date 9 July 2016
* @brief File containing interface of floppy disk driver.
*
* Floppy disk driver using DMA for read and write.
*/

#ifndef _FLOPPY_H
#define _FLOPPY_H

#include <lib/stdint.h>

/**
* Set DMA address
*
* @param	addr	New DMA address.
*/
void floppy_disk_set_dma(const int addr);

/**
* Install floppy driver with IRQ
*
* @param	irq		IRQ to use.
*/
void floppy_disk_install(const int irq);

/**
* Set working drive.
*
* @param	drive	Drive to use.
*/
void floppy_disk_set_working_drive(const uint8_t drive);

/**
* Get working drive
*
* @return			Current working drive.
*/
const uint8_t floppy_disk_get_working_drive();

/**
* Reads a sector from floppy and stores it in DMA buffer.
*
* @param	sectorLBA	Linear block address for sector to read.
* @return				Address of memory block where result is stored.
*/
const uint8_t* floppy_disk_read_sector(const int sectorLBA);

/**
* Writes a sector to floppy.
*
* @param	buffer		Buffer containing data to store.
* @param	sectorLBA	Linear block address for sector to write.
*/
void floppy_disk_write_sector(const uint8_t* buffer, const int sectorLBA);

/**
* Convert LBA (Linear block address) to CHS (head,track,sector)
*
* @param	lba			LBA value to be converted.
* @param	head		Pointer to where head info will be stored.
* @param	track		Pointer to where track info will be stored.
* @param	sector		Pointer to where sector info will be stored.
*/
void floppy_disk_lba_to_chs(const int lba, int* head, int* track, int* sector);

#endif