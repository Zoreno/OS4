/**
* @file acpi.h
* @author Joakim Bertils
* @date 9 July 2016
* @brief File containing interface of ACPI driver
*
* ACPI driver for shutting down computer.
*
* @todo expand
*/

#ifndef _ACPI_H
#define _ACPI_H

#include <lib/stdint.h>

/** 
* Initiate the ACPI structure.
*
* @return		0 if successfull.
*/
int initAcpi();


/**
* Shut down the computer.
* 
* @note			Requires ACPI to be initiated.
*/
void acpiPowerOff();

/**
* Get the CMOS century register.
*
* @return		Century register address.
*/
uint8_t acpiGetCenturyRegister();

#endif