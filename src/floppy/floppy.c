/**
* @file floppy_disk.cpp
* @author Joakim Bertils
* @date 9 July 2016
* @brief File containing implementation of the floppy disk driver.
*
* Floppy disk driver using DMA for read and write.
*/

#include <floppy/floppy.h>

#include <hal/hal.h>
#include <lib/string.h>
#include <lib/stdio.h>

/** Floppy disk IO ports
* Enum containing the IO port addresses of floppy disk registers.
*/
typedef enum 
{
	/**
	* Digital output register.
	*/
	FLOPPY_DISK_DOR = 0x3f2,

	/**
	* Main status register.
	*/
	FLOPPY_DISK_MSR = 0x3f4,

	/**
	* FIFO register.
	*/
	FLOPPY_DISK_FIFO = 0x3f5,

	/**
	* Control Register.
	*/
	FLOPPY_DISK_CTRL = 0x3f7
} FLOPPY_DISK_IO;

/** Floppy disk command byte mask
* Enum containing the lower bits mask of floppy disk command byte.
*/
typedef enum 
{
	/**
	* FDC Command: Read Track
	*/
	FDC_CMD_READ_TRACK = 2,

	/**
	* FDC Command: Specify
	*/
	FDC_CMD_SPECIFY = 3,

	/**
	* FDC Command: Check stat
	*/
	FDC_CMD_CHECK_STAT = 4,

	/**
	* FDC Command: Write sector
	*/
	FDC_CMD_WRITE_SECT = 5,

	/**
	* FDC Command: Read Sector
	*/
	FDC_CMD_READ_SECT = 6,

	/**
	* FDC Command: Calibrate
	*/
	FDC_CMD_CALIBRATE = 7,

	/**
	* FDC Command: Check int.
	*/
	FDC_CMD_CHECK_INT = 8,

	/**
	* FDC Command: Format track.
	*/
	FDC_CMD_FORMAT_TRACK = 0xd,

	/**
	* FDC Command: Seek
	*/
	FDC_CMD_SEEK = 0xf
} FLOPPY_DISK_CMD;


/** Floppy disk command byte mask
* Enum containing the higher bits mask of floppy disk command byte.
*/
typedef enum 
{
	/**
	* FDC Command: Skip mode
	*/
	FDC_CMD_EXT_SKIP = 0x20,

	/**
	* FDC Command: Density mode
	*/
	FDC_CMD_EXT_DENSITY = 0x40,

	/**
	* FDC Command: MultiTrack Operation.
	*/
	FDC_CMD_EXT_MULTITRACK = 0x80
} FLOPPY_DISK_CMD_EXT;


/** Floppy disk digital output register mask
* Enum containing descriptions of each bit in the Digital output register.
*/
typedef enum 
{
	/**
	* Drive 0
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE0 = 0,

	/**
	* Drive 1
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE1 = 1,

	/**
	* Drive 2
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE2 = 2,

	/**
	* Drive 3
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE3 = 3,

	/**
	* Reset
	*/
	FLOPPY_DISK_DOR_MASK_RESET = 4,

	/**
	* DMA mode
	*/
	FLOPPY_DISK_DOR_MASK_DMA = 8,

	/**
	* Drive 0 motor.
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE0_MOTOR = 0x10,

	/**
	* Drive 1 motor.
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE1_MOTOR = 0x20,

	/**
	* Drive 2 motor.
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE2_MOTOR = 0x40,

	/**
	* Drive 3 motor.
	*/
	FLOPPY_DISK_DOR_MASK_DRIVE3_MOTOR = 0x80
} FLOPPY_DISK_DOR_MASK;


/** Floppy disk main status register mask
* Enum containing descriptions of each bit in the main status register.
*/
typedef enum 
{
	/**
	* FDD 0 busy in seek mode
	*/
	FLOPPY_DISK_MSR_MASK_DRIVE0_POS_MODE = 1,

	/**
	* FDD 1 busy in seek mode
	*/
	FLOPPY_DISK_MSR_MASK_DRIVE1_POS_MODE = 2,

	/**
	* FDD 2 busy in seek mode
	*/
	FLOPPY_DISK_MSR_MASK_DRIVE2_POS_MODE = 4,

	/**
	* FDD 3 busy in seek mode
	*/
	FLOPPY_DISK_MSR_MASK_DRIVE3_POS_MODE = 8,

	/**
	* Read or write command in progress.
	*/
	FLOPPY_DISK_MSR_MASK_BUSY = 0x10,

	/**
	* Direct memory access (DMA) mode
	*/
	FLOPPY_DISK_MSR_MASK_DMA = 0x20,

	/**
	* Direction of data.
	*/
	FLOPPY_DISK_MSR_MASK_DATAIO = 0x40,

	/**
	* Data register ready for transfer.
	*/
	FLOPPY_DISK_MSR_MASK_DATAREG = 0x80
} FLOPPY_DISK_MSR_MASK;


/** Floppy disk status port 0 register mask
* Enum containing descriptions of each bit in the status port 0 register.
*/
typedef enum 
{
	/**
	* Drive 0
	*/
	FLOPPY_DISK_ST0_MASK_DRIVE0 = 0,

	/**
	* Drive 1
	*/
	FLOPPY_DISK_ST0_MASK_DRIVE1 = 1,

	/**
	* Drive 2
	*/
	FLOPPY_DISK_ST0_MASK_DRIVE2 = 2,

	/**
	* Drive 3
	*/
	FLOPPY_DISK_ST0_MASK_DRIVE3 = 3,

	/**
	* Head active
	*/
	FLOPPY_DISK_ST0_MASK_HEAD_ACTIVE = 4,

	/**
	* Not ready
	*/
	FLOPPY_DISK_ST0_MASK_NOT_READY = 8,

	/**
	* Unit check.
	*/
	FLOPPY_DISK_ST0_MASK_UNIT_CHECK = 0x10,

	/**
	* Seek end.
	*/
	FLOPPY_DISK_ST0_MASK_SEEK_END = 0x20,

	/**
	* INTCODE
	*/
	FLOPPY_DISK_ST0_MASK_INTCODE = 0xC0
} FLOPPY_DISK_ST0_MASK;

/** Floppy disk status port 1 register mask
* Enum containing descriptions of each bit in the status port 1 register.
*/
typedef enum 
{
	/**
	* Missing address
	*/
	FLOPPY_DISK_ST1_MASK_MISS_ADDR = 1,

	/**
	* Not writable
	*/
	FLOPPY_DISK_ST1_MASK_NOT_WRITABLE = 2,

	/**
	* No data
	*/
	FLOPPY_DISK_ST1_MASK_NO_DATA = 4,

	/**
	* Buffer under/overrun
	*/
	FLOPPY_DISK_ST1_MASK_UNDER_OVER_RUN = 0x10,

	/**
	* Data error
	*/
	FLOPPY_DISK_ST1_MASK_DATA_ERR = 0x20,

	/**
	* End of cylinder.
	*/
	FLOPPY_DISK_ST1_MASK_END_OF_CYL = 0x80,
} FLOPPY_DISK_ST1_MASK;

/** Floppy disk status port 2 register mask
* Enum containing descriptions of each bit in the status port 2 register.
*/
typedef enum 
{
	/**
	* Missing data address mark
	*/
	FLOPPY_DISK_ST2_MASK_MISS_DATA_ADDR_MARK = 1,

	/**
	* Bad cylinder
	*/
	FLOPPY_DISK_ST2_MASK_BAD_CYL = 2,

	/**
	* Wrong cylinder
	*/
	FLOPPY_DISK_ST2_MASK_WRONG_CYL = 0x10,

	/**
	* Data error in data field.
	*/
	FLOPPY_DISK_ST2_MASK_DATA_ERR = 0x20,

	/**
	* Control mark.
	*/
	FLOPPY_DISK_ST2_MASK_CTRL_MARK = 0x40
} FLOPPY_DISK_ST2_MASK;

/** Floppy disk INTCODE mask
* Enum containing descriptions of each combinations of bit in the INTCODE.
*/
typedef enum 
{

	/**
	* Normal
	*/
	FLOPPY_DISK_ST0_TYPE_NORMAL = 0,

	/**
	* Abnormal error
	*/
	FLOPPY_DISK_ST0_TYPE_ABNORMAL_ERR = 1,

	/**
	* Invalid error
	*/
	FLOPPY_DISK_ST0_TYPE_INVALID_ERR = 2,

	/**
	* Not ready
	*/
	FLOPPY_DISK_ST0_TYPE_NOT_READY = 3
} FLOPPY_DISK_ST0_INTCODE_TYPE;


/** Floppy disk GAP3 length
* Enum containing space between sector on physical.
*/
typedef enum 
{
	/**
	* GAP length for standard floppy
	*/
	FLOPPY_DISK_GAP3_LENGTH_STD = 42,

	/**
	* GAP length for 5.14 floppy
	*/
	FLOPPY_DISK_GAP3_LENGTH_5_14 = 32,

	/**
	* GAP length for 3.5 floppy
	*/
	FLOPPY_DISK_GAP3_LENGTH_3_5 = 27
} FLOPPY_DISK_GAP3_LENGTH;

/** Floppy disk Bytes per sector
* Enum containing bytes per sector values for commands.
*/
typedef enum
{
	/**
	* 128 bytes per sector
	*/
	FLOPPY_DISK_DTL_128 = 0,

	/**
	* 256 bytes per sector
	*/
	FLOPPY_DISK_DTL_256 = 1,

	/**
	* 512 bytes per sector
	*/
	FLOPPY_DISK_DTL_512 = 2,

	/**
	* 1024 bytes per sector
	*/
	FLOPPY_DISK_DTL_1024 = 3
} FLOPPY_DISK_SECTOR_DTL;

/** Read/Write return value.
* Struct containing return value from read/write operations
*/
typedef struct _RW_RETURN
{
	/**
	* Status register 0
	*/
	uint8_t st0;

	/**
	* Status register 1
	*/
	uint8_t st1;

	/**
	* Status register 2
	*/
	uint8_t st2;

	/**
	* Current cylinder
	*/
	uint8_t currentCylinder;

	/**
	* Current head.
	*/
	uint8_t currentHead;

	/**
	* Sector number
	*/
	uint8_t sectorNumber;

	/**
	* Sector size
	*/
	uint8_t sectorSize;

} rw_return_t;

/** Floppy Disk Return Error Codes
* Enumeration containing error codes representing floppy disk failures.
*/
typedef enum
{
	/**
	* Operation completed successfully.
	*/
	FLOPPY_GOOD = 0,

	/**
	* Operation not completed successfully
	*/
	FLOPPY_BAD = -1,

	/**
	* Drive number is invalid.
	*/
	FLOPPY_INVALID_DRIVE = -2
} FLOPPY_DISK_ERROR;


// ===========
// Constants
// ===========

/**
* Floppy disk controller IRQ.
*/
const int FLOPPY_IRQ = 6;

/**
* Sectors per track.
*/
const int FLOPPY_SECTORS_PER_TRACK = 18;

/**
* Floppy disk DMA channel.
*/
const int FDC_DMA_CHANNEL = 2;

// ============
// Private data
// ============

/**
* Current working drive.
*/
static uint8_t _CurrentDrive = 0;

/**
* Floppy disk IRQ status.
*/
static volatile uint8_t _FloppyDiskIRQ = 0;

/**
* Start of DMA buffer.
* Spans a total of 64k
*/
int DMA_BUFFER = 0x1000;

// =====================================
// Private function forward declarations
// =====================================

/**
* Initiate floppy DMA
*
* @param	buffer		DMA buffer address
* @param	length		Length of buffer.
* @return				1 if successfull
*/
int dma_initialize_floppy(const uint8_t* buffer, const uint32_t length);

/**
* Reads floppy disk status
*
* @return				MSR content
*/
const uint8_t floppy_disk_read_status();

/**
* Write to floppy disk Digital Output Register
*
* @param	val		Value to write.
*/
void floppy_disk_write_dor(const uint8_t val);

/**
* Send command to floppy disk controller
*
* @param	cmd		Command to send.
*/
void floppy_disk_send_command(const uint8_t cmd);

/**
* Get data from floppy disk controller.
*
* @return			Floppy disk controller data.
*/
const uint8_t floppy_disk_read_data();

/**
* Write to configuration control register.
*
* @param	val		Data to write.
*/
void floppy_disk_write_ccr(const uint8_t val);

/**
* Wait for floppy irq to fire.
*/
void floppy_disk_wait_irq();

/**
* Floppy IRQ handler.
*/
void i86_floppy_irq();

/**
* Check interrupt status command.
*
* @param	st0		Status register 0
* @param	cyl		Cylinder.
*/
void floppy_disk_check_int(uint32_t* st0, uint32_t* cyl);

/**
* Turns the floppy motor on/off
*
* @param	b		New value for floppy motor.
*/
void floppy_disk_set_motor(int b);

/**
* Configure drive.
*
* @param	stepr	Steprate
* @param	loadt	Load time
* @param	unloadt Unload time
* @param	dma		Whether to use DMA
*/
void floppy_disk_configure_drive(const uint32_t stepr, const uint32_t loadt, const uint32_t unloadt, int dma);

/**
* Calibrates the drive.
*
* @see @FLOPPY_DISK_ERROR
*
* @param	drive	Drive to calibrate.
* @return			Error code
*/
const FLOPPY_DISK_ERROR floppy_disk_calibrate(const uint32_t drive);

/**
* Disable controller.
*/
void floppy_disk_disable_controller();

/**
* Enable controller.
*/
void floppy_disk_enable_controller();

/**
* Reset controller.
*/
void floppy_disk_reset();

/**
* Read a sector and store data in DMA buffer.
*
* @param	head	Head to read from.
* @param	track	Track to read from.
* @param	sector	Sector to read from.
*/
void floppy_disk_read_sector_imp(const uint8_t head, const uint8_t track, const uint8_t sector);

/**
* Write DMA buffer data to a sector.
*
* @param	head	Head to read to.
* @param	track	Track to read to.
* @param	sector	Sector to read to.
*/
void floppy_disk_write_sector_imp(const uint8_t head, const uint8_t track, const uint8_t sector);

/**
* Seek to a given track
*
* @param	cyl		Cylinder to seek to.
* @param	head	Head to seek to.
* @return			Error code
*/
const FLOPPY_DISK_ERROR floppy_disk_seek(const uint32_t cyl, const uint32_t head);

// =====================================
// Private functions
// =====================================

int dma_initialize_floppy(const uint8_t* buffer, const uint32_t length)
{
	union 
	{
		uint8_t byte[4];//Lo[0], Mid[1], Hi[2]
		unsigned long l;
	}a, c;

	a.l = (unsigned)buffer;
	c.l = (unsigned)length - 1;

	//Check for buffer issues
	if ((a.l >> 24) || (c.l >> 16) || (((a.l & 0xffff) + c.l) >> 16)) 
	{
		printf("[FDC]DMA buffer error\n");
		return 0;
	}

	dma_reset(1);
	dma_mask_channel(FDC_DMA_CHANNEL);//Mask channel 2
	dma_reset_flipflop(1);//Flipflop reset on DMA 1

	dma_set_address(FDC_DMA_CHANNEL, a.byte[0], a.byte[1]);//Buffer address
	dma_reset_flipflop(1);//Flipflop reset on DMA 1

	dma_set_count(FDC_DMA_CHANNEL, c.byte[0], c.byte[1]);//Set count
	dma_set_read(FDC_DMA_CHANNEL);

	dma_unmask_all(1);//Unmask channel 2

	return 1;
}

const uint8_t floppy_disk_read_status()
{
	// Return main status register
	return inportb(FLOPPY_DISK_MSR);
}

void floppy_disk_write_dor(const uint8_t val)
{
	// Write to the DOR
	outportb(FLOPPY_DISK_DOR, val);
}

void floppy_disk_send_command(const uint8_t cmd)
{
	// Wait until data register is ready

	printf("[FDC]Trying to send command %0#(4)x... ", cmd);

	for (int i = 0; i < 500;++i)
	{
		if (floppy_disk_read_status() & FLOPPY_DISK_MSR_MASK_DATAREG)
		{
			printf("Done!\n", cmd);
			return outportb(FLOPPY_DISK_FIFO, cmd);
		}
	}
	printf("Failed!\n");
}

const uint8_t floppy_disk_read_data()
{
	// Wait until data register is ready

	for (int i = 0; i < 500;++i)
	{
		if (floppy_disk_read_status() & FLOPPY_DISK_MSR_MASK_DATAREG)
		{
			return inportb(FLOPPY_DISK_FIFO);
		}
	}
}

void floppy_disk_write_ccr(const uint8_t val)
{
	// Write to configuration control register

	outportb(FLOPPY_DISK_CTRL, val);
}

void floppy_disk_wait_irq()
{
	printf("Waiting...\n");
	while (_FloppyDiskIRQ == 0);
	_FloppyDiskIRQ = 0;
}

void i86_floppy_irq()
{
	asm volatile ("pushal");
	asm volatile ("cli");

	printf("[FDC]IRQ fired\n");

	//! irq fired
	_FloppyDiskIRQ = 1;

	//! tell hal we are done
	interruptdone(FLOPPY_IRQ);

	asm volatile ("sti");
	asm volatile ("popal; leave; iret");
}

void floppy_disk_check_int(uint32_t* st0, uint32_t* cyl)
{
	floppy_disk_send_command(FDC_CMD_CHECK_INT);

	*st0 = floppy_disk_read_data();
	*cyl = floppy_disk_read_data();
}

void floppy_disk_set_motor(int b)
{
	// Sanity check

	if (_CurrentDrive > 3)
	{
		return;
	}

	uint8_t motor = 0;

	// Get motor from drive
	switch (_CurrentDrive)
	{
	case 0:
		motor = FLOPPY_DISK_DOR_MASK_DRIVE0_MOTOR;
		break;
	case 1:
		motor = FLOPPY_DISK_DOR_MASK_DRIVE1_MOTOR;
		break;
	case 2:
		motor = FLOPPY_DISK_DOR_MASK_DRIVE2_MOTOR;
		break;
	case 3:
		motor = FLOPPY_DISK_DOR_MASK_DRIVE3_MOTOR;
		break;
	}

	// Turn on or off motor
	if (b)
	{
		floppy_disk_write_dor(_CurrentDrive | motor | FLOPPY_DISK_DOR_MASK_RESET | FLOPPY_DISK_DOR_MASK_DMA);
		sleep(50);
	}
	else
	{
		floppy_disk_write_dor(FLOPPY_DISK_DOR_MASK_RESET | FLOPPY_DISK_DOR_MASK_DMA);
		sleep(50);
	}

	sleep(20);
}

void floppy_disk_configure_drive(const uint32_t stepr, const uint32_t loadt, const uint32_t unloadt, int dma)
{
	uint8_t data = 0;

	floppy_disk_send_command(FDC_CMD_SPECIFY);

	data = ((stepr & 0x0F) << 4) | (unloadt & 0x0F);
	floppy_disk_send_command(data);

	data = (loadt) << 1 | (dma) ? 1 : 0;
	floppy_disk_send_command(data);
}

const FLOPPY_DISK_ERROR floppy_disk_calibrate(const uint32_t drive)
{
	uint32_t st0;
	uint32_t cyl;

	// Sanity check

	if (drive >= 4)
	{
		return FLOPPY_INVALID_DRIVE;
	}

	// Turn on motor

	floppy_disk_set_motor(1);

	for (int i = 0; i < 10; ++i)
	{
		floppy_disk_send_command(FDC_CMD_CALIBRATE);
		floppy_disk_send_command(drive);
		floppy_disk_wait_irq();
		floppy_disk_check_int(&st0, &cyl);

		// Check if cylinder 0 was found
		if (!cyl)
		{
			floppy_disk_set_motor(0);
			return FLOPPY_GOOD;
		}
	}

	floppy_disk_set_motor(0);
	return FLOPPY_BAD;
}

void floppy_disk_disable_controller()
{
	floppy_disk_write_dor(0);
}

void floppy_disk_enable_controller()
{
	floppy_disk_write_dor(FLOPPY_DISK_DOR_MASK_RESET | FLOPPY_DISK_DOR_MASK_DMA);
}

void floppy_disk_reset()
{
	uint32_t st0;
	uint32_t cyl;

	floppy_disk_disable_controller();

	floppy_disk_enable_controller();

	floppy_disk_wait_irq();

	// Send CHECK INT/SENSE INTERRUPT command to all drives

	for (int i = 0; i < 4; ++i)
	{
		floppy_disk_check_int(&st0, &cyl);
	}

	// Transfer speed 500 kb/s

	floppy_disk_write_ccr(0);

	// Pass mechanincal info.
	// Steprate = 3ms
	// load time = 16ms
	// unload time = 240ms
	// Use DMA
	floppy_disk_configure_drive(3, 16, 240, 1);

	// Calibrate disk
	floppy_disk_calibrate(_CurrentDrive);
}

void floppy_disk_read_sector_imp(const uint8_t head, const uint8_t track, const uint8_t sector)
{
	uint32_t st0;
	uint32_t cyl;

	// Set DMA to read
	dma_initialize_floppy((uint8_t*)DMA_BUFFER, 512);
	dma_set_read(FDC_DMA_CHANNEL);

	_FloppyDiskIRQ = 0;


	// Read in a sector
	floppy_disk_send_command(
		FDC_CMD_READ_SECT| 
		FDC_CMD_EXT_MULTITRACK |
		FDC_CMD_EXT_SKIP |
		FDC_CMD_EXT_DENSITY);

	floppy_disk_send_command((head << 2) | _CurrentDrive);
	floppy_disk_send_command(track);
	floppy_disk_send_command(head);
	floppy_disk_send_command(sector);
	floppy_disk_send_command(FLOPPY_DISK_DTL_512);
	floppy_disk_send_command(((sector + 1) >= FLOPPY_SECTORS_PER_TRACK) ? FLOPPY_SECTORS_PER_TRACK : sector + 1);
	floppy_disk_send_command(FLOPPY_DISK_GAP3_LENGTH_3_5);
	floppy_disk_send_command(0xFF); // End of command

	printf("Waiting for READ irq fire\n");

	floppy_disk_wait_irq();

	printf("Got READ irq\n");

	//Return byte 0 : ST0
	//Return byte 1 : ST1
	//Return byte 2 : ST2
	//Return byte 3 : Current cylinder
	//Return byte 4 : Current head
	//Return byte 5 : Sector number
	//Return byte 6 : Sector size

	rw_return_t res;

	res.st0 = floppy_disk_read_data();
	res.st1 = floppy_disk_read_data();
	res.st2 = floppy_disk_read_data();
	res.currentCylinder = floppy_disk_read_data();
	res.currentHead = floppy_disk_read_data();
	res.sectorNumber = floppy_disk_read_data();
	res.sectorSize = floppy_disk_read_data();

	// Let FDC know we handled interrupt.
	floppy_disk_check_int(&st0, &cyl);

	static uint32_t readCount = 0;

	//monitor_printf("\n%i",++readCount);
}

void floppy_disk_write_sector_imp(const uint8_t head, const uint8_t track, const uint8_t sector)
{
	uint32_t st0;
	uint32_t cyl;

	// Set DMA to write
	dma_initialize_floppy((uint8_t*)DMA_BUFFER, 512);
	dma_set_write(FDC_DMA_CHANNEL);

	// Read in a sector
	floppy_disk_send_command(
		FDC_CMD_WRITE_SECT |
		FDC_CMD_EXT_MULTITRACK |
		FDC_CMD_EXT_SKIP |
		FDC_CMD_EXT_DENSITY);
	floppy_disk_send_command((head << 2) | _CurrentDrive);
	floppy_disk_send_command(track);
	floppy_disk_send_command(head);
	floppy_disk_send_command(sector);
	floppy_disk_send_command(FLOPPY_DISK_DTL_512);
	floppy_disk_send_command(((sector + 1) >= FLOPPY_SECTORS_PER_TRACK) ? FLOPPY_SECTORS_PER_TRACK : sector + 1);
	floppy_disk_send_command(FLOPPY_DISK_GAP3_LENGTH_3_5);
	floppy_disk_send_command(0xFF); // End of command

	floppy_disk_wait_irq();

	//Return byte 0 : ST0
	//Return byte 1 : ST1
	//Return byte 2 : ST2
	//Return byte 3 : Current cylinder
	//Return byte 4 : Current head
	//Return byte 5 : Sector number
	//Return byte 6 : Sector size

	rw_return_t res;

	res.st0 = floppy_disk_read_data();
	res.st1 = floppy_disk_read_data();
	res.st2 = floppy_disk_read_data();
	res.currentCylinder = floppy_disk_read_data();
	res.currentHead = floppy_disk_read_data();
	res.sectorNumber = floppy_disk_read_data();
	res.sectorSize = floppy_disk_read_data();

	// Let FDC know we handled interrupt.
	floppy_disk_check_int(&st0, &cyl);
}

const FLOPPY_DISK_ERROR floppy_disk_seek(const uint32_t cyl, const uint32_t head)
{
	uint32_t st0;
	uint32_t cyl0;

	// Sanity check

	if (_CurrentDrive >= 4)
	{
		return FLOPPY_INVALID_DRIVE;
	}

	for (int i = 0; i < 10; ++i)
	{
		_FloppyDiskIRQ = 0;


		floppy_disk_send_command(FDC_CMD_SEEK);
		floppy_disk_send_command((head) << 2 | _CurrentDrive);
		floppy_disk_send_command(cyl);



		printf("Waiting for SEEK irq fire\n");

		// Wait for results
		floppy_disk_wait_irq();

		printf("Got SEEK irq\n");

		floppy_disk_check_int(&st0, &cyl0);

		// Have we found the cylinder?
		if (cyl == cyl0)
		{
			return FLOPPY_GOOD;
		}
	}
	return FLOPPY_BAD;
}

// =====================================
// Public functions
// =====================================

void floppy_disk_set_dma(const int addr) 
{
	DMA_BUFFER = addr;
}

void floppy_disk_install(const int irq)
{
	// Install IRQ handler
	setvect(irq, i86_floppy_irq);

	// Reset floppy disk
	floppy_disk_reset();

	// Set drive information
	floppy_disk_configure_drive(13, 1, 0xF, 1);
}

void floppy_disk_set_working_drive(const uint8_t drive)
{
	if (drive < 4)
	{
		_CurrentDrive = drive;
	}
}

const uint8_t floppy_disk_get_working_drive()
{
	return _CurrentDrive;
}

const uint8_t* floppy_disk_read_sector(const int sectorLBA)
{
	// Sanity check

	if (_CurrentDrive >= 4)
	{
		return 0;
	}

	// Convert LBA sector to CHS
	int head = 0;
	int track = 0;
	int sector = 1;
	floppy_disk_lba_to_chs(sectorLBA, &head, &track, &sector);


	//Turn on motor and seek track
	floppy_disk_set_motor(1);
	if (floppy_disk_seek(track, head) != 0)
	{
		return 0;
	}

	// Read sector and turn motor off
	floppy_disk_read_sector_imp(head, track, sector);
	floppy_disk_set_motor(0);

	// Return ptr to dma buffer (maybe we should malloc a buffer and place it 
	// there)
	return (const uint8_t*)DMA_BUFFER;
}

void floppy_disk_write_sector(const uint8_t* buffer, const int sectorLBA)
{
	// Sanity check
	if (_CurrentDrive >= 4)
	{
		return;
	}

	// Copy data to dma
	memcpy((void*)DMA_BUFFER, buffer, 512);

	// Convert LBA sector to CHS
	int head = 0;
	int track = 0;
	int sector = 1;
	floppy_disk_lba_to_chs(sectorLBA, &head, &track, &sector);


	//Turn on motor and seek track
	floppy_disk_set_motor(1);
	if (floppy_disk_seek(track, head) != 0)
	{
		return;
	}

	// Write sector and turn motor off
	floppy_disk_write_sector_imp(head, track, sector);
	floppy_disk_set_motor(0);

	return;
}

void floppy_disk_lba_to_chs(const int lba, int* head, int* track, int* sector)
{
	*head = (lba % (FLOPPY_SECTORS_PER_TRACK * 2)) / (FLOPPY_SECTORS_PER_TRACK);
	*track = lba / (FLOPPY_SECTORS_PER_TRACK * 2);
	*sector = lba % FLOPPY_SECTORS_PER_TRACK + 1;
}