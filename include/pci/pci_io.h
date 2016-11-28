/**
* @file pci_io.h
* @author Joakim Bertils
* @date 21 November 2016
* @brief File containing interface for pci I/O routines.
*
* Routines for accessing PCI registers.
*/

#ifndef _PCI_IO_H
#define _PCI_IO_H 

#include <lib/stdint.h>

/**
* @brief Reads a 8 bit PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
*
* @return		Data in register.
*/
uint8_t pci_read_b(uint32_t id, uint32_t reg);

/**
* @brief Reads a 16 bit PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
*
* @return		Data in register.
*/
uint16_t pci_read_w(uint32_t id, uint32_t reg);

/**
* @brief Reads a 32 bit PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
*
* @return		Data in register.
*/
uint32_t pci_read_l(uint32_t id, uint32_t reg);

/**
* @brief Writes a 8 bit value to a PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
* @param data	Data to write
*/
void pci_write_b(uint32_t id, uint32_t reg, uint8_t data);

/**
* @brief Writes a 16 bit value to a PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
* @param data	Data to write
*/
void pci_write_w(uint32_t id, uint32_t reg, uint16_t data);

/**
* @brief Writes a 32 bit value to a PCI register.
*
* @param id		Device ID.
* @param reg	Register to read.
* @param data	Data to write
*/
void pci_write_l(uint32_t id, uint32_t reg, uint32_t data);

#endif