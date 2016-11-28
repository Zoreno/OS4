#pragma once

#include <lib/stdint.h>

typedef struct
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint32_t year;

}time_t;

// Initiates the RTC with century register from ACPI
void initRTC(uint8_t centReg);


// Reads the current RTC state
void readRTC(time_t* timePtr);