/**
* @file serial.h
* @author Joakim Bertils
* @date 26 July 2016
* @brief File containing interface of serial communication routines.
*
* Serial Communication through COM1-COM4 com ports.
*/

#ifndef _SERIAL_H
#define _SERIAL_H

#include <lib/stdint.h>

/**
* Enum containing ID values for COM-ports
*/
typedef enum
{
	/**
	* COM1 ID value
	*/
	COM1 = 0,

	/**
	* COM2 ID value
	*/
	COM2 = 1,

	/**
	* COM3 ID value
	*/
	COM3 = 2,

	/**
	* COM4 ID value
	*/
	COM4 = 3,

	/**
	* Number of com-ports this driver can manage.
	*/
	NUMBER_OF_COMPORTS
} COM_port;

/**
* Enum containing baud rate division register values.
*
* Baud rate is a measurement of the speed of the transfer. Transfer speed
* should match the other device's speed.
*/
typedef enum
{
	SERIAL_BAUD_115200 = 1,
	SERIAL_BAUD_57600 = 2,
	SERIAL_BAUD_38400 = 3,
	SERIAL_BAUD_19200 = 6,
	SERIAL_BAUD_9600 = 12,
	SERIAL_BAUD_4800 = 24,
	SERIAL_BAUD_2400 = 48,
	SERIAL_BAUD_1200 = 96,
	SERIAL_BAUD_600 = 192,
	SERIAL_BAUD_300 = 384,
	SERIAL_BAUD_220 = 524,
	SERIAL_BAUD_110 = 1047,
	SERIAL_BAUD_50 = 2304,
} serial_baud_rate;

/**
* Enum containing valid parity modes. Parity mode should match the other
* device's parity mode.
*/
typedef enum
{
	/**
	* No parity will be used.
	*/
	SERIAL_PARITY_NONE,

	/**
	* Sum of all bits should be odd.
	*/
	SERIAL_PARITY_ODD,

	/**
	* Sum of all bits should be even.
	*/
	SERIAL_PARITY_EVEN,

	/**
	* Parity is always 1.
	*/
	SERIAL_PARITY_MARK,

	/**
	* Parity is always 0.
	*/
	SERIAL_PARITY_SPACE
} serial_parity_mode;

/**
* Initiate the serial communication through a port with given parameters.
*
* @param port		Port to configure
* @param baud_rate	Baud rate for the transfer
* @param data_bits	Data bits per transfer. Valid values are 5, 6, 7 and 8.
* @param stop_bits	Stop bits per transfer. Valid values are 1 and 2.
* @param parity		Parity mode.
*/
void init_serial(
	COM_port port, 
	serial_baud_rate baud_rate, 
	uint8_t data_bits, 
	uint8_t stop_bits, 
	serial_parity_mode parity);

/**
* Checks whether new data has been recieved.
*
* @param	port	Port to check.
*
* @return			Non-zero if new data has been recieved.
*/
int serial_recieved(COM_port port);

/**
* Read one byte of incoming data. Blocks until data is recieved.
*
* @param	port	Port to read.
*
* @return			Byte recieved.
*/
char serial_recieve_data(COM_port port);

/**
* Checks whether port is ready to send data.
*
* @param	port	Port to check.
*
* @return			Non-zero if data can be sent.
*/
int serial_is_transmit_ready(COM_port port);

/**
* Sends one byte of data.
*
* @param	port	Port to send over.
* @param	data	Data to be sent.
*/
void serial_send_data(COM_port port, char data);

#endif