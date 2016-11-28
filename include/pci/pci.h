#include <lib/stdint.h>

#include <pci/pci_device.h>

pci_device_list_t* device_list = 0;

typedef struct _PciBAR
{
	union
	{
		void* address;
		uint16_t port;
	};

	uint64_t size;
	uint32_t flags;
} PciBAR_t;

void pciInit();