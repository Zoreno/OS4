#include <hal/dma.h>

void dma_set_mode(uint8_t channel, uint8_t mode){

	int dma = (channel < 4) ? 0 : 1;
	int chan = (dma == 0) ? channel : channel - 4;

	dma_mask_channel(channel);
	outportb((channel < 4) ? DMA0_MODE_REG : DMA1_MODE_REG, chan | mode);
	dma_unmask_all(dma);	
}

void dma_set_read(uint8_t channel){

	dma_set_mode (channel,	DMA_MODE_READ_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

void dma_set_write(uint8_t channel){

	dma_set_mode (channel,
		DMA_MODE_WRITE_TRANSFER | DMA_MODE_TRANSFER_SINGLE);
}

void dma_set_address(uint8_t channel, uint8_t low, uint8_t high){

	if(channel > 8)
		return;

	uint16_t port = 0;

	switch(channel){

		case 0:
			port = DMA0_CH0_ADDR_REG;
			break;
		case 1:
			port = DMA0_CH1_ADDR_REG;
			break;
		case 2:
			port = DMA0_CH2_ADDR_REG;
			break;
		case 3:
			port = DMA0_CH3_ADDR_REG;
			break;
		case 4:
			port = DMA1_CH4_ADDR_REG;
			break;
		case 5:
			port = DMA1_CH5_ADDR_REG;
			break;
		case 6:
			port = DMA1_CH6_ADDR_REG;
			break;
		case 7:
			port = DMA1_CH7_ADDR_REG;
			break;
		default:
			return; // Should never happen
	}

	outportb(port, low);
	outportb(port, high);
}

void dma_set_count(uint8_t channel, uint8_t low, uint8_t high){

	if(channel > 8)
		return;

	uint16_t port = 0;

	switch(channel){

		case 0:
			port = DMA0_CH0_COUNT_REG;
			break;
		case 1:
			port = DMA0_CH1_COUNT_REG;
			break;
		case 2:
			port = DMA0_CH2_COUNT_REG;
			break;
		case 3:
			port = DMA0_CH3_COUNT_REG;
			break;
		case 4:
			port = DMA1_CH4_COUNT_REG;
			break;
		case 5:
			port = DMA1_CH5_COUNT_REG;
			break;
		case 6:
			port = DMA1_CH6_COUNT_REG;
			break;
		case 7:
			port = DMA1_CH7_COUNT_REG;
			break;
		default:
			return; // Should never happen
	}

	outportb(port, low);
	outportb(port, high);	
}

void dma_mask_channel(uint8_t channel){

	if(channel < 5){

		outportb(DMA0_SINGLE_CHAN_MASK_REG, (1 << (channel-1)));
	} else {
		outportb(DMA1_SINGLE_CHAN_MASK_REG, (1 << (channel-5)));
	}
}

void dma_unmask_channel(uint8_t channel){

	if(channel < 5){

		outportb(DMA0_SINGLE_CHAN_MASK_REG, channel);
	} else {
		outportb(DMA1_SINGLE_CHAN_MASK_REG, channel);
	}
}

void dma_reset_flipflop(int dma){

	if(dma < 2)
		return;

	if(dma == 0){

		outportb(DMA0_CLEARBYTE_FLIPFLOP_REG, 0xFF);
	} else {

		outportb(DMA1_CLEARBYTE_FLIPFLOP_REG, 0xFF);
	}
}

void dma_reset(int dma){

	outportb(DMA0_TEMP_REG, 0xFF);
}

void dma_set_external_page_register(uint8_t reg, uint8_t val){
	
	if (reg > 14)
		return;

	unsigned short port = 0;
	switch ( reg ) {

		case 1: {port = DMA_PAGE_CHAN1_ADDRBYTE2; break;}
		case 2: {port = DMA_PAGE_CHAN2_ADDRBYTE2; break;}
		case 3: {port = DMA_PAGE_CHAN3_ADDRBYTE2; break;}
		case 4: {return;}//! nothing should ever write to register 4
		case 5: {port = DMA_PAGE_CHAN5_ADDRBYTE2; break;}
		case 6: {port = DMA_PAGE_CHAN6_ADDRBYTE2; break;}
		case 7: {port = DMA_PAGE_CHAN7_ADDRBYTE2; break;}
	}

	outportb(port, val);
}

void dma_unmask_all(int dma){

	outportb(DMA1_UNMASK_ALL_REG, 0xFF);
}