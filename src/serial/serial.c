/**
* @file serial.c
* @author Joakim Bertils
* @date 26 July 2016
* @brief File containing implementation of serial communication routines.
*
* Serial Communication through COM1-COM4 com ports.
*
* @todo expand
*/

#include <serial/serial.h>

#include <hal/hal.h>
#include <lib/stdarg.h>
#include <lib/string.h>

#include <lib/stdio.h>

static uint8_t _COM1_initiated = 0;
static uint8_t _COM2_initiated = 0;
static uint8_t _COM3_initiated = 0;
static uint8_t _COM4_initiated = 0;

/**
* Base address of COM1 com port.
*/
#define COM1_BASE 0x3F8

/**
* Base address of COM2 com port.
*/
#define COM2_BASE 0x2F8

/**
* Base address of COM3 com port.
*/
#define COM3_BASE 0x3E8

/**
* Base address of COM4 com port.
*/
#define COM4_BASE 0x2E8

// Serial communication register addresses
#define SERIAL_DATA_REG(base) (base)
#define SERIAL_INT_EN_REG(base) (base + 1)
#define SERIAL_INT_ID_FIFO_CTRL_REG(base) (base + 2)
#define SERIAL_LINE_CTRL_REG(base) (base + 3)
#define SERIAL_MODEM_CTRL_REG(base) (base + 4)
#define SERIAL_LINE_STATUS_REG(base) (base + 5)
#define SERIAL_MODEM_STATUS_REG(base) (base + 6)
#define SERIAL_SCRATCH_REG(base) (base + 7)

// Serial communication registers if DLAB bit in LCR is 1
#define SERIAL_BAUD_RATE_LOW(base) (base)
#define SERIAL_BAUD_RATE_HIGH(base) (base + 1)

// Line control register masks
#define SERIAL_LCR_DLAB_MASK 0x80

// Interrupt enable register masks
#define SERIAL_IER_DATA_AVAILABLE_MASK 0x01
#define SERIAL_IER_TRANSMITTER_EMPRY_MASK 0x02
#define SERIAL_IER_BREAK_ERROR_MASK 0x04
#define SERIAL_IER_STATUS_CHANGE_MASK 0x08

#define SERIAL_IER_DISABLE_INTS 0x00

// Interrupt identification register masks
#define SERIAL_IIR_INT_PENDING_MASK 0x01
#define SERIAL_IIR_INT_SOURCE_MASK 0x06

#define SERIAL_IIR_STATUS_CHANGE_INT 0b0000
#define SERIAL_IIR_TRANSMITTER_EMPTY_INT 0b0100
#define SERIAL_IIR_LINE_STATUS_INT 0b0110
#define SERIAL_IIR_DATA_AVAILABLE_INT 0b1000
#define SERIAL_IIR_ERROR_ON_BREAK_INT 0b1100

#define SERIAL_IIR_64_BYTE_FIFO_ENABLED_MASK 0x10
#define SERIAL_IIR_FIFO_MODE_MASK 0xC0

#define SERIAL_IIR_NO_FIFO 0x00
#define SERIAL_IIR_FIFO_ENABLED 0xC0

// Modem control register masks
#define SERIAL_MCR_DATA_TERMINAL_READY_MASK 0x01
#define SERIAL_MCR_REQUEST_TO_SEND_MASK 0x02
#define SERIAL_MCR_OUT_1_MASK 0x04
#define SERIAL_MCR_INT_ENABLE_MASK 0x08
#define SERIAL_MCR_LOOPBACK_MODE_MASK 0x10

// Line status register masks
#define SERIAL_LSR_DATA_AVAILABLE_MASK 0x01
#define SERIAL_LSR_OVERRUN_ERROR_MASK 0x02
#define SERIAL_LSR_PARITY_ERROR_MASK 0x04
#define SERIAL_LSR_FRAMING_ERROR_MASK 0x08
#define SERIAL_LSR_BREAK_INT_MASK 0x10
#define SERIAL_LSR_TRANSMIT_READY_MASK 0x20
#define SERIAL_LSR_TRANSMIT_SR_EMPTY_MASK 0x40

// Modem status register masks
#define SERIAL_MSR_CLEAR_TO_SEND_CHANGED_MASK 0x01
#define SERIAL_MSR_DATA_SET_READY_MASK 0x02
#define SERIAL_MSR_TRAILING_EDGE_OF_RING_IND_MASK 0x04
#define SERIAL_MSR_DATA_CARRIER_DETECT_CHANGED_MASK 0x08
#define SERIAL_MSR_CLEAR_TO_SEND_MASK 0x10
#define SERIAL_MSR_DATA_SET_READY_MASK 0x20
#define SERIAL_MSR_RING_INDICATOR_MASK 0x40
#define SERIAL_MSR_DATA_CARRIER_DETECT_MASK 0x80

/**
* Layout of the Line Control Register
*/
typedef struct _LineCtrlReg
{
	/**
	* Data bits value.
	*
	* Defined values are:
	*
	* 00: 5 data bits
	* 01: 6 data bits
	* 10: 7 data bits
	* 11: 8 data bits
	*/
	uint8_t data_bits : 2;

	/**
	* Stop bits value.
	*
	* Defined values are:
	*
	* 0: 1 stop bit
	* 1: 2 stop bits
	*/
	uint8_t stop_bits : 1;

	/**
	* Parity mode value
	*
	* Defined values are:
	*
	* 000: No parity
	* 001: Odd parity
	* 011: Even parity
	* 101: Mark parity
	* 111: Space parity
	*/
	uint8_t parity_mode : 3;

	/**
	* Reserved.
	*/
	uint8_t reserved : 1;

	/**
	* Divisor Latch.
	*
	* When 1, port (base) and (base + 1) are mapped to divisor value registers
	* (low byte) and (high byte) instead.
	*/
	uint8_t DLAB_bit : 1;

} LineControlReg_t;

void init_serial(COM_port port, serial_baud_rate baud_rate, uint8_t data_bits, uint8_t stop_bits, serial_parity_mode parity)
{

	// Determine port
	uint16_t port_base;
	switch (port)
	{
	case COM1:
		port_base = COM1_BASE;
		break;
	case COM2:
		port_base = COM2_BASE;
		break;
	case COM3:
		port_base = COM3_BASE;
		break;
	case COM4:
		port_base = COM4_BASE;
		break;
	default:
		printf("\nERROR: Invalid port");
		// ERROR: Invalid port
		return;
	}

	// Disable all interrupts
	outportb(SERIAL_INT_EN_REG(port_base), SERIAL_IER_DISABLE_INTS);

	// Enable DLAB to be able to send divisor value
	outportb(SERIAL_LINE_CTRL_REG(port_base), SERIAL_LCR_DLAB_MASK);

	// Set baud rate divisor
	uint16_t baud = (uint16_t)baud_rate;

	printf("Baud rate divisor: %x\n", baud);

	outportb(SERIAL_BAUD_RATE_LOW(port_base), baud & 0x00FF);
	outportb(SERIAL_BAUD_RATE_HIGH(port_base), (baud & 0xFF00) >> 8);

	// Set line data
	LineControlReg_t line_ctrl_data;

	switch (data_bits)
	{
	case 5:
		line_ctrl_data.data_bits = 0;
		break;
	case 6:
		line_ctrl_data.data_bits = 1;
		break;
	case 7:
		line_ctrl_data.data_bits = 2;
		break;
	case 8:
		line_ctrl_data.data_bits = 3;
		break;
	default:
		//ERROR, invalid settings
		printf("\nERROR: Invalid data bit setting");
		return;
	}

	switch (stop_bits)
	{
	case 1:
		line_ctrl_data.stop_bits = 0;
		break;
	case 2:
		line_ctrl_data.stop_bits = 1;
	default:
		//ERROR, invalid settings
		printf("\nERROR: Invalid stop bit setting");
		return;
	}

	switch (parity)
	{
	case SERIAL_PARITY_NONE:
		line_ctrl_data.parity_mode = 0b000;
		break;
	case SERIAL_PARITY_ODD:
		line_ctrl_data.parity_mode = 0b001;
		break;
	case SERIAL_PARITY_EVEN:
		line_ctrl_data.parity_mode = 0b011;
		break;
	case SERIAL_PARITY_MARK:
		line_ctrl_data.parity_mode = 0b101;
		break;
	case SERIAL_PARITY_SPACE:
		line_ctrl_data.parity_mode = 0b111;
		break;
	default:
		//ERROR, invalid settings
		printf("\nERROR: Invalid parity setting");
		return;
	}

	// Disable DLAB
	line_ctrl_data.DLAB_bit = 0;

	uint8_t line_ctrl_data_byte;
	
	memcpy(&line_ctrl_data_byte, &line_ctrl_data, 1);

	outportb(SERIAL_LINE_CTRL_REG(port_base), line_ctrl_data_byte);

	// Enable FIFO, clear, 14 byte threshold
	outportb(SERIAL_INT_ID_FIFO_CTRL_REG(port_base), SERIAL_IIR_FIFO_ENABLED | SERIAL_IIR_LINE_STATUS_INT | SERIAL_IIR_INT_PENDING_MASK);

	// IRQ enabled RTS/DSR set
	outportb(SERIAL_MODEM_CTRL_REG(port_base), SERIAL_MCR_DATA_TERMINAL_READY_MASK | SERIAL_MCR_REQUEST_TO_SEND_MASK | SERIAL_MCR_INT_ENABLE_MASK);

	switch (port)
	{
	case COM1:
		_COM1_initiated = 1;
		break;
	case COM2:
		_COM2_initiated = 1;
		break;
	case COM3:
		_COM3_initiated = 1;
		break;
	case COM4:
		_COM4_initiated = 1;
		break;
	default:
		// ERROR: Invalid port
		// Should not happen
		return;
	}
}

int serial_recieved(COM_port port)
{
	switch (port)
	{
	case COM1:
		return (inportb(SERIAL_LINE_STATUS_REG(COM1_BASE)) & SERIAL_LSR_DATA_AVAILABLE_MASK);
	case COM2:
		return (inportb(SERIAL_LINE_STATUS_REG(COM2_BASE)) & SERIAL_LSR_DATA_AVAILABLE_MASK);
	case COM3:
		return (inportb(SERIAL_LINE_STATUS_REG(COM3_BASE)) & SERIAL_LSR_DATA_AVAILABLE_MASK);
	case COM4:
		return (inportb(SERIAL_LINE_STATUS_REG(COM4_BASE)) & SERIAL_LSR_DATA_AVAILABLE_MASK);
	default:
		// ERROR: Invalid port
		return 1;
	}
}

char serial_recieve_data(COM_port port)
{
	// TODO check if initiated
	while (!serial_recieved(port));

	switch (port)
	{
	case COM1:
		return (inportb(SERIAL_DATA_REG(COM1_BASE)));
	case COM2:
		return (inportb(SERIAL_DATA_REG(COM2_BASE)));
	case COM3:
		return (inportb(SERIAL_DATA_REG(COM3_BASE)));
	case COM4:
		return (inportb(SERIAL_DATA_REG(COM4_BASE)));
	default:
		// ERROR: Invalid port
		return 0;
	}
}

int serial_is_transmit_ready(COM_port port)
{
	switch (port)
	{
	case COM1:
		return (inportb(SERIAL_LINE_STATUS_REG(COM1_BASE)) & SERIAL_LSR_TRANSMIT_READY_MASK);
	case COM2:
		return (inportb(SERIAL_LINE_STATUS_REG(COM2_BASE)) & SERIAL_LSR_TRANSMIT_READY_MASK);
	case COM3:
		return (inportb(SERIAL_LINE_STATUS_REG(COM3_BASE)) & SERIAL_LSR_TRANSMIT_READY_MASK);
	case COM4:
		return (inportb(SERIAL_LINE_STATUS_REG(COM4_BASE)) & SERIAL_LSR_TRANSMIT_READY_MASK);
	default:
		// ERROR: Invalid port
		return 1;
	}
}

void serial_send_data(COM_port port, char data)
{
	// TODO check if initiated

	while (!serial_is_transmit_ready(port));

	switch (port)
	{
	case COM1:
		outportb(SERIAL_DATA_REG(COM1_BASE), data);
	case COM2:
		outportb(SERIAL_DATA_REG(COM2_BASE), data);
	case COM3:
		outportb(SERIAL_DATA_REG(COM3_BASE), data);
	case COM4:
		outportb(SERIAL_DATA_REG(COM4_BASE), data);
	default:
		// ERROR: Invalid port
		return;
	}
}