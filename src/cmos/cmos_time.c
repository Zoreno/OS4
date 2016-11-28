#include <cmos/cmos_time.h>

#include <hal/hal.h>

#define CURRENT_YEAR 2016

int centuryRegister = 0x00;

uint8_t second;
uint8_t minute;
uint8_t hour;
uint8_t day;
uint8_t month;
uint32_t year;

// Registers
enum {

	cmos_address = 0x70,
	cmos_data = 0x71
};

enum {

	REGISTER_SECOND = 0x00,
	REGISTER_MINUTE = 0x02,
	REGISTER_HOUR = 0x04,
	REGISTER_DAY = 0x07,
	REGISTER_MONTH = 0x08,
	REGISTER_YEAR = 0x09,

	REGISTER_B = 0x0B
};

int get_update_in_progress_flag() {

	// Send command
	outportb(cmos_address, 0x0A);

	// Update in progress flag is bit 7
	return inportb(cmos_data) & 0x80;
}

uint8_t get_RTC_register(uint8_t reg){

	// Send address to register
	outportb(cmos_address, reg);

	// Return data.
	return inportb(cmos_data);
}

void initRTC(uint8_t centReg){

	centuryRegister = centReg;
}

void readRTC(time_t* timePtr){

	uint8_t century = 0;
	uint8_t last_second;
	uint8_t last_minute;
	uint8_t last_hour;
	uint8_t last_day;
	uint8_t last_month;
	uint8_t last_year;
	uint8_t last_century;

	uint8_t registerB;

	// Wait for the cmos to be ready
	while (get_update_in_progress_flag());

	// Get data from CMOS
	second = get_RTC_register(REGISTER_SECOND);

	minute = get_RTC_register(REGISTER_MINUTE);

	hour = get_RTC_register(REGISTER_HOUR);

	day = get_RTC_register(REGISTER_DAY);

	month = get_RTC_register(REGISTER_MONTH);

	year = get_RTC_register(REGISTER_YEAR);

	if (centuryRegister != 0) {

		century = get_RTC_register(centuryRegister);
	}

	// Read until values dont change
	do {
		// Save values for later comparison
		last_second = second;

		last_minute = minute;

		last_hour = hour;

		last_day = day;

		last_month = month;

		last_year = year;

		last_century = century;

		// Wait for the cmos to be ready
		while (get_update_in_progress_flag());

		// Get data from CMOS
		second = get_RTC_register(REGISTER_SECOND);

		minute = get_RTC_register(REGISTER_MINUTE);

		hour = get_RTC_register(REGISTER_HOUR);

		day = get_RTC_register(REGISTER_DAY);

		month = get_RTC_register(REGISTER_MONTH);

		year = get_RTC_register(REGISTER_YEAR);
	} while (
		(last_second != second) ||
		(last_minute != minute) ||
		(last_hour != hour) ||
		(last_day != day) ||
		(last_month != month) ||
		(last_year != year) ||
		(last_century != century)
		);

	registerB = get_RTC_register(REGISTER_B);

	// Do we need to convert BCD to binary values?
	if (!(registerB & 0x04)) {

		second = (second & 0x0F) + ((second / 16) * 10);
		minute = (minute & 0x0F) + ((minute / 16) * 10);
		hour = ((hour & 0x0F) + (((hour & 0x70) / 16) * 10) | (hour & 0x80));
		day = (day & 0x0F) + ((day / 16) * 10);
		month = (month & 0x0F) + ((month / 16) * 10);
		year = (year & 0x0F) + ((year / 16) * 10);
		
		if (centuryRegister != 0){

			century = (century & 0x0F) + ((century / 16) * 10);
		}
	}

	// Convert to 24h clock
	if (!(registerB & 0x02) && (hour & 0x80)){

		hour = ((hour & 0x7F) + 12) % 24;
	}

	// Calculate 4 digit year

	if (centuryRegister != 0){

		year += century * 100;
	}
	else {

		year += (CURRENT_YEAR / 100) * 100;

		if (year < CURRENT_YEAR){

			year += 100;
		}
	}

	// Fill in return values
	timePtr->second = second;
	timePtr->minute = minute;
	timePtr->hour = hour;
	timePtr->day = day;
	timePtr->month = month;
	timePtr->year = year;
}