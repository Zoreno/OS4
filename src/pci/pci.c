#include <pci/pci.h>

#include <hal/hal.h>
#include <pci/pci_io.h>
#include <lib/string.h>
#include <lib/stdio.h>

#define PCI_MAKE_ID(bus,dev,func) (((bus) << 16) | ((dev) << 11) | ((func) << 8))

// PCI Configuration Registers
#define PCI_CONFIG_VENDOR_ID			0x00
#define PCI_CONFIG_HEADER_TYPE          0x0e

void pciCheckDevice(uint32_t bus, uint32_t dev, uint32_t func);

void pciCheckDevice(uint32_t bus, uint32_t dev, uint32_t func)
{
	uint32_t id = PCI_MAKE_ID(bus, dev, func);

	// No existing device
	if (pci_read_w(id, PCI_CONFIG_VENDOR_ID) == 0xFFFF)
	{
		return;
	}

	// Make sure there is a list
	if(device_list == 0){
		// Allocate head
		device_list = kmalloc(sizeof(pci_device_list_t));
		device_list->next = 0; // Since we are not guaranteed empty pages.

		// read info into the first node.
		pci_read_device_info(id, &device_list->dev_info);
	} else {
		// get next node
		pci_device_list_t* cur_node = device_list;

		//  Loop to the end
		while(cur_node->next != 0){
			cur_node = cur_node->next;
		}

		// Allocate a new node
		cur_node->next = kmalloc(sizeof(pci_device_list_t));

		cur_node = cur_node->next;

		cur_node->next = 0;

		// read info into the current node node.
		pci_read_device_info(id, &cur_node->dev_info);
	}
}

void pciInit()
{
	for (uint32_t bus = 0; bus < 256; ++bus)
	{
		for (uint32_t dev = 0; dev < 32; ++dev)
		{
			uint32_t baseID = PCI_MAKE_ID(bus, dev, 0);

			uint8_t headerType = pci_read_b(baseID, PCI_CONFIG_HEADER_TYPE);

			uint32_t funcCount = headerType & 0x80 ? 8 : 1;

			for (uint8_t func = 0; func < funcCount; ++func)
			{
				pciCheckDevice(bus, dev, func);
			}
		}
	}

	// Send out all read info to serial output for reference

	serial_printf(COM1, "\n==========================================\n");
	serial_printf(COM1,   "============ PCI Device List =============\n");
	serial_printf(COM1,   "==========================================\n\n");

	pci_device_list_t* cur_node = device_list;

	while(cur_node != 0){

		PciDeviceInfo_t* dev = &cur_node->dev_info;

		serial_printf(COM1, "Device Name: %s\n", pci_device_name(dev->vendorID, dev->deviceID));
		serial_printf(COM1, "Class Name: %s\n", pci_class_name(dev->classCode, dev->subClass, dev->progIntf));

		serial_printf(COM1, "Vendor ID: %#x\n", dev->vendorID);
		serial_printf(COM1, "Device ID: %#x\n", dev->deviceID);
		serial_printf(COM1, "Command: %#x\n", dev->command_w); // TODO Print bit for bit
		serial_printf(COM1, "Status: %#x\n", dev->status_w); // TODO Print bit for bit
		serial_printf(COM1, "Revision ID: %#x\n", dev->revisionID);
		serial_printf(COM1, "Prog IF: %#x\n", dev->progIntf);
		serial_printf(COM1, "Subclass: %#x\n", dev->subClass);
		serial_printf(COM1, "Class Code: %#x\n", dev->classCode);
		serial_printf(COM1, "Cache line size: %#x\n", dev->cacheLineSize);
		serial_printf(COM1, "Latency Timer: %#x\n", dev->latencyTimer);
		serial_printf(COM1, "Header Type: %#x\n", dev->headerType);
		serial_printf(COM1, "BIST: %#x\n", dev->BIST);
		serial_printf(COM1, "\n");

		switch(dev->headerType & 0x7F){
			case 0:
				for(int i = 0; i < 6; ++i)
					serial_printf(COM1, "BAR[%i]: %#x\n", i, dev->type0.BaseAddresses[i]);

				serial_printf(COM1, "CIS: %#x\n", dev->type0.CIS);
				serial_printf(COM1, "SubVendorID: %#x\n", dev->type0.SubVendorID);
				serial_printf(COM1, "SubSystemID: %#x\n", dev->type0.SubSystemID);
				serial_printf(COM1, "ROMBaseAddress: %#x\n", dev->type0.ROMBaseAddress);
				serial_printf(COM1, "CapabilitiesPtr: %#x\n", dev->type0.CapabilitiesPtr);
				serial_printf(COM1, "InterruptLine: %#x\n", dev->type0.InterruptLine);
				serial_printf(COM1, "InterruptPin: %#x\n", dev->type0.InterruptPin);
				serial_printf(COM1, "MinimumGrant: %#x\n", dev->type0.MinimumGrant);
				serial_printf(COM1, "MaximumLatency: %#x\n", dev->type0.MaximumLatency);
				break;
			case 1:
				break;
			case 2:
				break;
			default:
				serial_printf(COM1, "Invalid header type\n");
				break;
		}

		serial_printf(COM1, "\n==========================================\n\n");

		cur_node = cur_node->next; // Advance
	}
}