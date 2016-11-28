/** @file dma.h
 *  @brief Function prototypes for the dma interface
 *
 *  Defines the interface for the Direct Memory Access which allows 
 *	peripherals to write directly to the RAM. Used primarly for the floppy
 *	disk driver
 *
 *  @author Joakim Bertils
 *  @bug No known bugs.
 */

// TODO: Finish commenting the types and functions

#ifndef _DMA_H
#define _DMA_H

#include <lib/stdint.h>

/**
 *	Maxmimum number of DMA channels
 */
#define DMA_MAX_CHANNELS 16

/**
 *	Number of channels per DMA controller
 */
#define DMA_CHANNELS_PER_DMAC 8

//===================================================================
//	DMA0 Registers
//===================================================================

/**
 *	Port numbers for DMA0 address and count registers
 */
typedef enum {

	/**
 	*	Channel 0 Address Register
 	*/
	DMA0_CH0_ADDR_REG = 0,

	/**
 	*	Channel 0 Count Register
 	*/
	DMA0_CH0_COUNT_REG = 1,

	/**
 	*	Channel 1 Address Register
 	*/
	DMA0_CH1_ADDR_REG = 2,

	/**
 	*	Channel 1 Count Register
 	*/
	DMA0_CH1_COUNT_REG = 3,

	/**
 	*	Channel 2 Address Register
 	*/
	DMA0_CH2_ADDR_REG = 4,

	/**
 	*	Channel 2 Count Register
 	*/
	DMA0_CH2_COUNT_REG = 5,

	/**
 	*	Channel 3 Address Register
 	*/
	DMA0_CH3_ADDR_REG = 6,

	/**
 	*	Channel 3 Count Register
 	*/
	DMA0_CH3_COUNT_REG = 7,

} DMA0_CHANNEL_IO_REGISTERS;

/**
 *	Generic registers belonging to DMA0.
 */
typedef enum {

	/**
 	*	DMA0 Status Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Transfer complete channel 0
 	*	1 			Transfer complete channel 1
 	*	2 			Transfer complete channel 2
 	*	3 			Transfer complete channel 3
 	*	4 			DMA Request pending channel 0
 	*	5 			DMA Request pending channel 1
 	*	6 			DMA Request pending channel 2
 	*	7 			DMA Request pending channel 3
 	*	
 	*/
	DMA0_STATUS_REG = 0x08,

	/**
 	*	DMA0 Command Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Memory to memory transfer
 	*	1 			Channel 0 address hold
 	*	2 			Controller Enable
 	*	3 			Compressed Timing
 	*	4 			Normal Priority
 	*	5 			Extended Write Selection
 	*	6 			DMA request active low
 	*	7 			DMA Acknowledge active high (DACK)
 	*	
 	*/
	DMA0_COMMAND_REG = 0x08,

	/**
 	*	DMA0 Request Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Channel Select
 	*	1 			Channel Select
 	*	2 			Set request bit
 	*	3 			Unused
 	*	4 			Unused
 	*	5 			Unused
 	*	6 			Unused
 	*	7 			Unused
 	*	
 	*/
	DMA0_REQUEST_REG = 0x09,
	DMA0_SINGLE_CHAN_MASK_REG = 0x0A,
	DMA0_MODE_REG = 0x0B,
	DMA0_CLEARBYTE_FLIPFLOP_REG = 0x0C,
	DMA0_TEMP_REG = 0x0D,
	DMA0_MASTER_CLEAR_REG = 0x0D,
	DMA0_CLEAR_MASK_REG = 0x0E,
	DMA0_MASK_REG = 0x0F

} DMA0_GENERIC_IO_REGISTERS;



//===================================================================
//	DMA1 Registers
//===================================================================

/**
 *	Port numbers for DMA1 address and count registers
 */
typedef enum {

	/**
 	*	Channel 4 Address Register
 	*/
	DMA1_CH4_ADDR_REG = 0xC0,

	/**
 	*	Channel 4 Count Register
 	*/
	DMA1_CH4_COUNT_REG = 0xC2,

	/**
 	*	Channel 5 Address Register
 	*/
	DMA1_CH5_ADDR_REG = 0xC4,

	/**
 	*	Channel 5 Count Register
 	*/
	DMA1_CH5_COUNT_REG = 0xC6,

	/**
 	*	Channel 6 Address Register
 	*/
	DMA1_CH6_ADDR_REG = 0xC8,

	/**
 	*	Channel 6 Count Register
 	*/
	DMA1_CH6_COUNT_REG = 0xCA,

	/**
 	*	Channel 7 Address Register
 	*/
	DMA1_CH7_ADDR_REG = 0xCC,

	/**
 	*	Channel 7 Count Register
 	*/
	DMA1_CH7_COUNT_REG = 0xCE,

} DMA1_CHANNEL_IO_REGISTERS;

/**
 *	Generic registers belonging to DMA1.
 */
typedef enum {

	/**
 	*	DMA1 Status Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Transfer complete channel 0
 	*	1 			Transfer complete channel 1
 	*	2 			Transfer complete channel 2
 	*	3 			Transfer complete channel 3
 	*	4 			DMA Request pending channel 0
 	*	5 			DMA Request pending channel 1
 	*	6 			DMA Request pending channel 2
 	*	7 			DMA Request pending channel 3
 	*	
 	*/
	DMA1_STATUS_REG = 0xD0,

	/**
 	*	DMA1 Command Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Memory to memory transfer
 	*	1 			Channel 0 address hold
 	*	2 			Controller Enable
 	*	3 			Compressed Timing
 	*	4 			Normal Priority
 	*	5 			Extended Write Selection
 	*	6 			DMA request active low
 	*	7 			DMA Acknowledge active high (DACK)
 	*	
 	*/
	DMA1_COMMAND_REG = 0xD0,

	/**
 	*	DMA1 Request Register
 	*
 	*	Format: 
 	*	
 	*	Bit(s):		Data:
 	*
 	*	0 			Channel Select
 	*	1 			Channel Select
 	*	2 			Set request bit
 	*	3 			Unused
 	*	4 			Unused
 	*	5 			Unused
 	*	6 			Unused
 	*	7 			Unused
 	*	
 	*/
	DMA1_REQUEST_REG = 0xD2,
	DMA1_SINGLE_CHAN_MASK_REG = 0xD4,
	DMA1_MODE_REG = 0xD6,
	DMA1_CLEARBYTE_FLIPFLOP_REG = 0xD8,
	DMA1_INTER_REG = 0xDA,
	DMA1_UNMASK_ALL_REG = 0xDC,
	DMA1_MASK_REG = 0xDE

} DMA1_GENERIC_IO_REGISTERS;

//===================================================================
//	Other DMA Registers
//===================================================================

enum DMA_PAGE_REG {

	DMA_PAGE_EXTRA0 = 0x80, //! Also diagnostics port
	DMA_PAGE_CHAN2_ADDRBYTE2 = 0x81,
	DMA_PAGE_CHAN3_ADDRBYTE2 = 0x82,
	DMA_PAGE_CHAN1_ADDRBYTE2 = 0x83,
	DMA_PAGE_EXTRA1 = 0x84,
	DMA_PAGE_EXTRA2 = 0x85,
	DMA_PAGE_EXTRA3 = 0x86,
	DMA_PAGE_CHAN6_ADDRBYTE2 = 0x87,
	DMA_PAGE_CHAN7_ADDRBYTE2 = 0x88,
	DMA_PAGE_CHAN5_ADDRBYTE2 = 0x89,
	DMA_PAGE_EXTRA4 = 0x8c,
	DMA_PAGE_EXTRA5 = 0x8d,
	DMA_PAGE_EXTRA6 = 0x8e,
	DMA_PAGE_DRAM_REFRESH = 0x8f //!no longer used in new PCs
};

enum DMA_MODE_REG_MASK {

	DMA_MODE_MASK_SEL = 3,

	DMA_MODE_MASK_TRA = 0xc,
	DMA_MODE_SELF_TEST = 0,
	DMA_MODE_READ_TRANSFER =4,
	DMA_MODE_WRITE_TRANSFER = 8,

	DMA_MODE_MASK_AUTO = 0x10,
	DMA_MODE_MASK_IDEC = 0x20,

	DMA_MODE_MASK = 0xc0,
	DMA_MODE_TRANSFER_ON_DEMAND= 0,
	DMA_MODE_TRANSFER_SINGLE = 0x40,
	DMA_MODE_TRANSFER_BLOCK = 0x80,
	DMA_MODE_TRANSFER_CASCADE = 0xC0
};

enum DMA_CMD_REG_MASK {

	DMA_CMD_MASK_MEMTOMEM = 1,
	DMA_CMD_MASK_CHAN0ADDRHOLD = 2,
	DMA_CMD_MASK_ENABLE = 4,
	DMA_CMD_MASK_TIMING = 8,
	DMA_CMD_MASK_PRIORITY = 0x10,
	DMA_CMD_MASK_WRITESEL = 0x20,
	DMA_CMD_MASK_DREQ = 0x40,
	DMA_CMD_MASK_DACK = 0x80
};

//===================================================================
//	DMA Function Prototypes
//===================================================================

void dma_set_mode(uint8_t channel, uint8_t mode);

void dma_set_read(uint8_t channel);

void dma_set_write(uint8_t channel);

void dma_set_address(uint8_t channel, uint8_t low, uint8_t high);

void dma_set_count(uint8_t channel, uint8_t low, uint8_t high);

void dma_mask_channel(uint8_t channel);

void dma_unmask_channel(uint8_t channel);

void dma_reset_flipflop(int dma);

void dma_reset(int dma);

void dma_set_external_page_register(uint8_t reg, uint8_t val);

void dma_unmask_all(int dma);

#endif