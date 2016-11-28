/**
* @file acpi.cpp
* @author Joakim Bertils
* @date 9 July 2016
* @brief File containing implementation of ACPI driver
*
* ACPI driver for shutting down computer.
*/

#include <acpi/acpi.h>

#define ACPI_DEBUG 0

#include <lib/stdint.h>
#include <lib/string.h>
#include <lib/stdio.h>
#include <hal/hal.h>
#include <mm/virtmem.h>

extern void sleep (int ms);

// TODO kommentera doxygen.
// TODO implementera parsers för fler tables, främst MADT.

// http://www.acpi.info/DOWNLOADS/ACPI_5_Errata%20A.pdf

uint32_t* SMI_CMD;
uint8_t ACPI_ENABLE;
uint8_t ACPI_DISABLE;
uint32_t* PM1a_CNT;
uint32_t* PM1b_CNT;
uint16_t SLP_TYPa;
uint16_t SLP_TYPb;
uint16_t SLP_EN;
uint16_t SCI_EN;
uint8_t PM1_CNT_LEN;
uint8_t centuryRegister;

/**
* Root System Description Pointer (RSDP)
*
* Structure providing the location of the RSDT or XSDT
*/
typedef struct
{
	/**
	* Signature.
	*
	* Should be "RSD PTR "
	*/
	char Signature[8];

	/**
	* Checksum of the fields defined in ACPI 1.0 spec. Sum of all bytes must be
	* 0.
	*/
	uint8_t Checksum;

	/**
	* An OEM-supplied string that identifies the OEM.
	*/
	char OEMID[6];

	/**
	* Revision of the structure. ACPI 1.0 value is 0. Current value
	* should be 2.
	*/
	uint8_t Revision;

	/**
	* 32 bit physical address of the RSDT.
	*/
	uint32_t* RsdtAddress;

	/**
	* Length of the table including the header.
	*/
	uint32_t Length;

	/**
	* 64 bit physical address of the XSDT.
	*/
	uint64_t XSDTAddress;

	/**
	* Checksum of the entire extended(ACPI 2.0+) table. Sum of all bytes should
	* be 0.
	*/
	uint8_t ExtendedChecksum;

	/**
	* Reserved.
	*/
	char reserved[3];
} RSDPDescriptor;


/**
* Generic Address Structure.
*
* Uniform way of describing register locations. Describes registers within
* tables in ACPI.
*/
typedef struct
{
	/**
	* Address Space ID.
	*
	* Defined values are:
	*
	* 0				System Memory
	* 1				System I/O
	* 2				PCI Configuration Space
	* 3				Embedded Controller
	* 4				SMBus
	* 5 to 0x09		Reserved
	* 0x0A			Platform Communications Channel (PCC)
	* 0x0B to 0x7E	Reserved
	* 0x7F			Functional Fixed Hardware
	* 0x80 to 0xBF	Reserved
	* 0xC0 to 0xFF	OEM Defined
	*/
	uint8_t AddressSpace;

	/**
	* Size in bits of given register.
	*/
	uint8_t BitWidth;

	/**
	* Bit offset of given register.
	*/
	uint8_t BitOffset;

	/**
	* Specifies access size.
	*
	* Defined values are:
	*
	* 0 Undefined (legacy reasons)
	* 1 Byte access
	* 2 Word access
	* 3 Dword access
	* 4 QWord access
	*/
	uint8_t AccessSize;

	/**
	* 64 bit address of register in given address space.
	*/
	uint64_t Address;
} GenericAddressStructure;

/**
* Standard ACPI table header. Makes up the first 36 bytes of most ACPI
* tables.
*/
typedef struct
{
	/**
	* 4 character ascii string of table identifier.
	*/
	char Signature[4];

	/**
	* Length of the entire table in bytes, including header.
	*/
	uint32_t Length;

	/**
	* Revision of the structure.
	*/
	uint8_t revision;

	/**
	* Checksum for the entire table. All bytes must add to 0 for the table
	* to be considered valid.
	*/
	uint8_t checksum;

	/**
	* An OEM-supplied string that identifies the OEM.
	*/
	char OEMID[6];

	/**
	* An OEM-supplied string that the OEM uses to identify the particular data
	* table.
	*/
	uint64_t OEMTableID;

	/**
	* An OEM-supplied revision number.
	*/
	uint32_t OEMRevision;

	/**
	* Vendor ID of ultility that created the table.
	*/
	uint32_t CreatorID;

	/**
	* Revision of the utility that created the table.
	*/
	uint32_t CreatorRevision;
} ACPIheader;

//========================================================================
// RSDT
//========================================================================

/**
* Root System Description Table
*
* Contains pointers to other system description tables.
*
* Header signature must be "RSDT"
*/
typedef struct
{
	/**
	* Standard ACPI header.
	*/
	ACPIheader header;

	/**
	* First table entry.
	*
	* Number of entries is determined by size of table.
	*/
	uint32_t tables[1];
} RSDT;

//========================================================================
// XSDT
//========================================================================

/**
* Extended System Description Table
*
* Contains pointers to other system description tables.
*
* Header signature must be "XSDT"
*/
typedef struct
{
	/**
	* Standard ACPI header.
	*/
	ACPIheader header;

	/**
	* First table entry.
	*
	* Number of entries is determined by size of table.
	*/
	uint64_t tables[1];
} XSDT;

//========================================================================
// FADT
//========================================================================


/**
* Fixed System Description Table
*
* Defines various fixed hardware ACPI information vital to an ACPI-compatible
* OS.
*
* Header signature must be "FACP"
*/
typedef struct
{
	/**
	* Standard ACPI header.
	*/
	ACPIheader header;

	/**
	* Physical memory address of the FACS
	*/
	uint32_t facs;

	/**
	* Physical memory address of the DSDT
	*/
	uint32_t dsdt;

	/**
	* Reserved
	*/
	uint8_t  Reserved;

	/**
	* Preferred power management profile.
	*
	* Field Values:
	* 0 Unspecified
	* 1 Desktop
	* 2 Mobile
	* 3 Workstation
	* 4 Enterprise Server
	* 5 SOHO Server
	* 6 Appliance PC
	* 7 Performance Server
	* 8 Tablet
	*
	* >8 Reserved
	*/
	uint8_t  PreferredPowerManagementProfile;

	/**
	* System vector of the SCI interrupt.
	*/
	uint16_t SCI_Interrupt;

	/**
	* System port address of the SMI command port.
	*/
	uint32_t* SMI_CommandPort;

	/**
	* The value to write to SMI_CommandPort to disable SMI ownership of
	* ACPI hardware registers.
	*/
	uint8_t  AcpiEnable;

	/**
	* The value to write to SMI_CommandPort to re-enable SMI ownership of
	* ACPI hardware registers.
	*/
	uint8_t  AcpiDisable;

	/**
	* The value to write to SMI_CommandPort to enter S4BIOS state.
	*/
	uint8_t  S4BIOS_REQ;

	/**
	* If non-zero, this field contains the value OSPM writes to the
	* SMI_CommandPort register to assume processor performance state
	* control responibility.
	*/
	uint8_t  PSTATE_Control;

	/**
	* System port address of the PM1a Event Register Block.
	*/
	uint32_t PM1aEventBlock;

	/**
	* System port address of the PM1b Event Register Block.
	*/
	uint32_t PM1bEventBlock;

	/**
	* System port address of the PM1a Control Register Block.
	*/
	uint32_t* PM1aControlBlock;

	/**
	* System port address of the PM1b Control Register Block.
	*/
	uint32_t* PM1bControlBlock;

	/**
	* System port address of the PM2 Control Register Block.
	*/
	uint32_t PM2ControlBlock;

	/**
	* System port address of the Power Management Timer Control
	* Register Block.
	*/
	uint32_t PMTimerBlock;

	/**
	* System port address of the General Purpose Event 0 Register Block.
	*/
	uint32_t GPE0Block;

	/**
	* System port address of the General Purpose Event 1 Register Block.
	*/
	uint32_t GPE1Block;

	/**
	* Number of bytes decoded by PM1 Event Block.
	*/
	uint8_t  PM1EventLength;

	/**
	* Number of bytes decoded by PM1 Control Block.
	*/
	uint8_t  PM1ControlLength;

	/**
	* Number of bytes decoded by PM2 Control Block.
	*/
	uint8_t  PM2ControlLength;

	/**
	* Number of bytes decoded by PM Timer Block.
	*/
	uint8_t  PMTimerLength;

	/**
	* Number of bytes decoded by GPE0 Block.
	*/
	uint8_t  GPE0Length;

	/**
	* Number of bytes decoded by GPE1 Block.
	*/
	uint8_t  GPE1Length;

	/**
	* Offset within ACPI GPE where GPE1 based events begin.
	*/
	uint8_t  GPE1Base;

	/**
	* If non-zero, this field contains the value OSPM writes to the
	* SMI_CommandPort register to indicate OS support for the _CST object
	* and C states Changed notification.
	*/
	uint8_t  CStateControl;

	/**
	* Worst case latency to exit a C2 state.
	*/
	uint16_t WorstC2Latency;

	/**
	* Worst case latency to exit a C3 state.
	*/
	uint16_t WorstC3Latency;

	/**
	* Number of flush strides that need to be read to completely flush
	* dirty lines from any processor's memory caches.
	*/
	uint16_t FlushSize;

	/**
	* Cache line with in bytes of processor memory caches.
	*/
	uint16_t FlushStride;

	/**
	* Offset of duty cycle setting in P_CNT register.
	*/
	uint8_t  DutyOffset;

	/**
	* With of duty cycke setting in P_CNT register
	*/
	uint8_t  DutyWidth;

	/**
	* RTC CMOS RMA index to the day-of-month alarm value.
	*/
	uint8_t  DayAlarm;

	/**
	* RTC CMOS RMA index to the month alarm value.
	*/
	uint8_t  MonthAlarm;

	/**
	* RTC CMOS RMA index to the century of data value.
	*/
	uint8_t  Century;

	/**
	* Boot architecture flags.
	*/
	uint16_t BootArchitectureFlags;

	/**
	* Reserved
	*/
	uint8_t  Reserved2;

	/**
	* Fixed feature flags.
	*/
	uint32_t Flags;

	/**
	* Address to reset register
	*/
	GenericAddressStructure ResetReg;

	/**
	* Indicates the value to write to ResetReg port ti reset system.
	*/
	uint8_t  ResetValue;

	/**
	* Reserved.
	*/
	uint8_t  Reserved3[3];

	/**
	* 64bit pointer to FACS
	*/
	uint64_t                X_facs;

	/**
	* 64bit pointer to DSDT
	*/
	uint64_t                X_dsdt;

	/**
	* System port address of the PM1a Event Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PM1aEventBlock;

	/**
	* System port address of the PM1b Event Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PM1bEventBlock;

	/**
	* System port address of the PM1a Control Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PM1aControlBlock;

	/**
	* System port address of the PM1b Control Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PM1bControlBlock;

	/**
	* System port address of the PM2 Control Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PM2ControlBlock;

	/**
	* System port address of the Power Management Timer Control
	* Register Block. 64 bit pointer.
	*/
	GenericAddressStructure X_PMTimerBlock;

	/**
	* System port address of the General Purpose Event 0 Register Block.
	* 64 bit pointer.
	*/
	GenericAddressStructure X_GPE0Block;

	/**
	* System port address of the General Purpose Event 1 Register Block.
	* 64 bit pointer.
	*/
	GenericAddressStructure X_GPE1Block;

	/**
	* Address of the Sleep register. 64 bit pointer.
	*/
	GenericAddressStructure SLEEP_CONTROL_REG;

	/**
	* Address of the Sleep status register. 64 bit pointer.
	*/
	GenericAddressStructure SLEEP_STATUS_REG;
}  FADT;

//========================================================================
// DSDT
//========================================================================


/**
* Differentiated System Description Table.
* 
* Signature must be "DSDT"
*/
typedef struct
{
	/**
	* ACPI standard header.
	*/
	ACPIheader header;

	/**
	* First byte of definition block.
	* 
	* Size can be calculated from length in header.
	*/
	uint8_t definitionBlock[1];
} DSDT;

//========================================================================
// SSDT
//========================================================================


/**
* Secondary System Description Table.
*
* Signature must be "SSDT"
*/
typedef struct
{
	/**
	* ACPI standard header.
	*/
	ACPIheader header;

	/**
	* First byte of definition block.
	*
	* Size can be calculated from length in header.
	*/
	uint8_t definitionBlock[1];
} SSDT;

//========================================================================
// MADT
//========================================================================


/**
* Bit field for MADT flags entry.
*/
typedef struct
{
	/**
	* One indicates that then system also has a PC-AT compatible
	* dual-8259 setup.
	*/
	uint32_t PCAT_COMPAT : 1;

	/**
	* Reserved.
	*/
	uint32_t Reserved : 31;
} MADT_FLAGS;

/**
* Header for the MADT entries.
*/
typedef struct
{
	/**
	* APIC structure type.
	* 
	* Defined values are:
	* 
	* 0			Processor local APIC
	* 1			I/O APIC
	* 2			Interrupt Source Override
	* 3			Non-maskable Interrupt Source
	* 4			Local APIC NMI
	* 5			Local APIC Address Override
	* 6			I/O SAPIC
	* 7			Local SAPIC
	* 8			Platform Interrupt Sources
	* 9			Processor Local x2APIC
	* 0xA		Local x2APIC NMI
	* 0xB		GIC
	* 0xC		GICD
	* 0xD-0x7F	Reserved.
	* 0x80-0xFF	Reserved for OEM use.
	*/
	uint8_t type;

	/**
	* Length of entry.
	*/
	uint8_t length;
}  MADT_ENTRY_HEADER;

/**
* Bit field for MADT local apic entry flags.
*/
typedef struct 
{
	/**
	* If zero, this processor is unusable.
	*/
	uint32_t enabled : 1;

	/**
	* Reserved.
	*/
	uint32_t Reserved : 31;
}MADT_ENTRY_LOCAL_APIC_FLAGS;

/**
* Processor Local APIC Structure
*/
typedef struct 
{
	/**
	* MADT entry standard header.
	* 
	* Type:		0
	* Length:	8
	*/
	MADT_ENTRY_HEADER header;

	/**
	* Processor ID for which this processor is listed in the ACPI
	* Processor declaration operator.
	*/
	uint8_t ProcessorID;

	/**
	* The processor's local APIC ID.
	*/
	uint8_t ApicID;

	/**
	* Local APIC flags.
	*/
	MADT_ENTRY_LOCAL_APIC_FLAGS flags;
}MADT_ENTRY_LOCAL_APIC;

/**
* IO APIC Structure
*/
typedef struct
{
	/**
	* MADT entry standard header.
	*
	* Type:		1
	* Length:	12
	*/
	MADT_ENTRY_HEADER header;

	/**
	* I/O APIC ID.
	*/
	uint8_t IO_APIC_ID;

	/**
	* Reserved.
	*/
	uint8_t Reserved;

	/**
	* 32 bit physical address to access this I/O APIC.
	*/
	uint32_t IO_APIC_ADDRESS;

	/**
	* Global system interrupt number where this I/O APIC's interrupt
	* input starts.
	*/
	uint32_t GlobalSystemInterruptBase;
} MADT_ENTRY_IO_APIC;

/**
* Bit field for MPS INTI Flags
*/
typedef struct
{
	/**
	* Polarity of APIC I/O input signals.
	* 
	* Defined values are:
	* 
	* 00	Conforms to specs of bus
	* 01	Active High
	* 10	Reserved
	* 11	Active Low
	*/
	uint16_t polarity : 2;

	/**
	* Trigger Mode of APIC I/O input signals.
	*
	* Defined values are:
	*
	* 00	Conforms to specs of bus
	* 01	Edge Triggered
	* 10	Reserved
	* 11	Level Triggered
	*/
	uint16_t triggerMode : 2;

	/**
	* Reserved.
	*/
	uint16_t Reserved : 12;
} MADT_ENTRY_MPS_INTI_FLAGS;

/**
* Interrupt Source Override Structure.
*/
typedef struct 
{
	/**
	* MADT entry standard header.
	*
	* Type:		2
	* Length:	10
	*/
	MADT_ENTRY_HEADER header;

	/**
	* Bus. Must be 0.
	*/
	uint8_t bus;

	/**
	* Bus-relative interrupt source.
	*/
	uint8_t source;

	/**
	* The global system interrupt that this bus-relative interrupt
	* source will signal.
	*/
	uint32_t GlobalSystemInterrupt;

	/**
	* MPS INTI flags.
	*/
	MADT_ENTRY_MPS_INTI_FLAGS Flags;
} MADT_ENTRY_INT_OVERRIDE;

// TODO fixa resten av MADT entries

/**
* Multiple APIC Description Table.
*
* Signature must be "APIC".
*/
typedef struct
{
	/**
	* ACPI Standard Header.
	*/
	ACPIheader header;

	/**
	* 32 bit physical address at which each processor can access its
	* local interrupt controller.
	*/
	uint32_t localApicAddr;

	/**
	* Multiple APIC flags.
	*/
	MADT_FLAGS flags;
	
	/**
	* First MADT entry.
	*/
	MADT_ENTRY_HEADER firstEntry[1];
} MADT;

//========================================================================
// SBST
//========================================================================


// Smart Battery Table.
typedef struct
{
	ACPIheader header;
	uint32_t warning_energy_level;
	uint32_t low_energy_level;
	uint32_t critical_energy_level;
} SBST;

//========================================================================
// ECDT
//========================================================================


// Embedded Controller Boot Resources Table.
typedef struct
{
	ACPIheader header;

	GenericAddressStructure EC_CONTROL;
	GenericAddressStructure EC_DATA;
	uint32_t UID;
	uint8_t GPE_BIT;
	char EC_ID[1]; // Actually bigger. Null terminated ASCII string.
} ECDT;

//========================================================================
// FACS
//========================================================================

/**
* Bit field for FACS flags entry.
*/
typedef struct
{
	/**
	* Indicates whether the platform supports S4_BIOS_REQ.
	*/
	uint32_t S4BIOS_F : 1;

	/**
	* Indicates that the platform firmware supports 64bit execution
	* enviroment for the waking vector.
	*/
	uint32_t WAKE_UP_64BIT_SUPPORTED_F : 1;

	/**
	* Reserved.
	*/
	uint32_t Reserved : 30;
} FACS_FLAGS;

/**
* Bit field for FACS OSPM flags entry.
*/
typedef struct
{
	/**
	* OSPM sets this bit to indicate that the XFirmwareWakingVector 
	* requires a 64bit execution enviroment.
	*/
	uint32_t WAKE_64BIT_F : 1;

	/**
	* Reserved.
	*/
	uint32_t Reserved : 31;
} FACS_OSPM_FLAGS;

/**
* Bit field for FACS global lock entry.
*/
typedef struct
{
	/**
	* Non-zero indicates that a request for ownership is pending.
	*/
	uint32_t Pending : 1;

	/**
	* Non-zero indicates that the global lock is owned.
	*/
	uint32_t Owned : 1;

	/**
	* reserved.
	*/
	uint32_t Reserved : 30;
} FACS_GLOBAL_LOCK;

/**
* Firmware ACPI Control Structure
*
* Signature must be "FACS"
*/
typedef struct
{
	/**
	* Signature. Must be "FACS"
	*/
	char Signature[4];

	/**
	* Length in bytes of entire table.
	*/
	uint32_t Length;

	/**
	* System's hardware signature at last boot.
	*/
	uint32_t HardwareSignature;

	/**
	* 32 bit address field where OSPM puts it's waking vector.
	*/
	uint32_t FirmwareWakingVector;

	/**
	* Global lock used to synchronize access to shared hardware resources.
	*/
	FACS_GLOBAL_LOCK GlobalLock;

	/**
	* Firmware control structure flags.
	*/
	FACS_FLAGS Flags;

	/**
	* 64 bit address field where OSPM puts it's waking vector.
	*/
	uint64_t XFirmwareWakingVector;

	/**
	* Version of this table.
	*/
	uint8_t Version;

	/**
	* Reserved.
	*/
	uint8_t Reserved1[3];

	/**
	* OSPM enabled firmware control structure flags.
	*/
	FACS_OSPM_FLAGS OSPMFlags;

	/**
	* Reserved.
	*/
	uint8_t Reserved2[24];
} FACS;

//**********************************************

// Check if the passed in pointer points to a valid RSDPtr
// Returns the address to the RSDT if valid, otherwise 0.
unsigned int* acpiCheckRSDPtr(unsigned int* ptr);

// Searches the memory for a valid RSDP and returns the RSDT Ptr if found,
// otherwise 0.
unsigned int* acpiGetRSDPtr(void);

// Checks the ptr for correct signature and checksum.
int acpiCheckHeader(ACPIheader* ptr, char* sig);

// Enables ACPI
int acpiEnable(void);

// Parse FADT table
void parseFADT(FADT* fadt);

// Calculate checksum.
uint8_t acpiCalculateChecksum(ACPIheader* header);

//**********************************************

unsigned int* acpiCheckRSDPtr(unsigned int* ptr)
{
	char* sig = "RSD PTR ";

	RSDPDescriptor* rsdp = (RSDPDescriptor*) ptr;

	// Check signature
	if (memcmp(sig, rsdp, 8) == 0)
	{

		// Check the checksum
		uint8_t* bptr = (uint8_t*)ptr;

		uint8_t checksum = 0;

		for (int i = 0; i < sizeof(RSDPDescriptor); ++i)
		{
			//This line have to be here...
			asm ("nop");


			uint8_t term = *bptr;
			checksum += term;

			bptr++;
		}

		if (checksum == 0)
		{
			// Return the address to the XSDT
			return (unsigned int*)rsdp->XSDTAddress;
		}
	}

	// The ptr does not point to a valid RSDP
	return 0;
}

unsigned int* acpiGetRSDPtr(void)
{
	unsigned int* addr;
	unsigned int* xsdt;

	// Start by checking 0x000E0000 to 0x00100000 on every 
	for (addr = (unsigned int*)0x000E0000; (int)addr < 0x00100000; addr += 0x10 / sizeof(addr))
	{
		xsdt = acpiCheckRSDPtr(addr);

		// If we got non-zero return, we got a valid RSDT 
		if (xsdt != 0)
		{
			return xsdt;
		}
	}

	// Check at 0x40:0x0E (Extended BIOS data area)
	int ebda = *((short *)0x40E);
	ebda = ebda * 0x10 & 0x000FFFFF; // Transform to linear address

	for (addr = (unsigned int*)ebda; (int)addr < ebda + 1024; addr += 0x10 / sizeof(addr))
	{
		xsdt = acpiCheckRSDPtr(addr);

		// If we got non-zero return, we got a valid RSDT 
		if (xsdt != 0)
		{
			return xsdt;
		}
	}

	// We did not find it.
	return 0;
}

uint8_t acpiCalculateChecksum(ACPIheader* header)
{
	volatile uint8_t sum = 0;

	for (int i = 0; i < header->Length; ++i)
	{
		asm ("nop");

		sum += ((uint8_t*)header)[i];
	}

	return sum;
}

int acpiCheckHeader(ACPIheader* header, char* sig)
{
	// Map first page.
	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)header, (uint32_t)header, I86_PTE_PRESENT);

	// Compare the signature
	if (memcmp(header->Signature, sig, 4) != 0)
	{
		//vmmngr_unmapPhysicalAddress(vmmngr_get_directory(), (uint32_t)header);
		return -1;
	}

	int len = header->Length;

	//vmmngr_unmapPhysicalAddress(vmmngr_get_directory(), (uint32_t)header);

	// Map the rest of table
	for (int i = 0; i < (len / 0x1000) + 1; ++i)
	{
		vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)header + i * 0x1000, (uint32_t)header + i * 0x1000, I86_PTE_PRESENT);
	}

	if (acpiCalculateChecksum(header) == 0)
	{
		return 0;
	}


	return -1;
}

int acpiEnable(void)
{

	// Check if acpi is enabled
	if ((inportw((unsigned int)PM1a_CNT) & SCI_EN) == 0)
	{
		// Check if acpi can be enabled
		if (SMI_CMD != 0 && ACPI_ENABLE != 0)
		{
			// Send acpi enable command
			outportb((unsigned int)SMI_CMD, ACPI_ENABLE);

			// Wait for a maximum of 3 seconds
			int i;
			for (i = 0; i < 300; ++i)
			{
				if ((inportw((unsigned int)PM1a_CNT) & SCI_EN) == 1)
					break;

				sleep(10);
			}

			// If we did not count to 300, we exited early and 
			// this means we enabled ACPI.
			if (i < 300)
			{
				return 0;
			}
			else
			{
				printf("ACPI could not be enabled\n");
				return -1;
			}
		}
		else
		{
			printf("No known way to enable ACPI\n");
			return -1;
		}
	}
	else
	{
		// Already enabled. 
		return 0;
	}
}

//*************************************

void parseFADT(FADT* fadt)
{
	// Map the fadt table
	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)fadt, (uint32_t)fadt, I86_PTE_PRESENT);

	// Get century register for CMOS.
	centuryRegister = fadt->Century;

	DSDT* dsdt = (DSDT*)fadt->dsdt;

	// Check dsdt
	if (acpiCheckHeader(&dsdt->header, "DSDT") == 0)
	{

		// Search fpr S5 Addr.
		char* S5Addr = (char*)dsdt->definitionBlock;

		// Extract length of the dsdt and remove header length
		int dsdtLength = dsdt->header.Length - sizeof(ACPIheader);

		// Search through the dsdt
		while (0 < dsdtLength--)
		{
			// Look for the \_S5
			if (memcmp(S5Addr, "_S5_", 4) == 0)
			{
				break;
			}
			S5Addr++;
		}
		if (dsdtLength > 0)
		{
			// Check for valid AML structure
			if ((*(S5Addr - 1) == 0x08 || (*(S5Addr - 2) == 0x08 && *(S5Addr - 1) == '\\')) && *(S5Addr + 4) == 0x12)
			{

				// Calculate PkgLength size
				S5Addr += 5;
				S5Addr += ((*S5Addr & 0xC0) >> 6) + 2;

				//Extract data from structure

				if (*S5Addr == 0x0A)
				{
					S5Addr++; // Skip byteprefix
				}

				SLP_TYPa = *(S5Addr) << 10;
				S5Addr++;

				if (*S5Addr == 0x0A)
				{
					S5Addr++; // Skip byteprefix
				}

				SLP_TYPb = *(S5Addr) << 10;

				SMI_CMD = fadt->SMI_CommandPort;

				ACPI_ENABLE = fadt->AcpiEnable;
				ACPI_DISABLE = fadt->AcpiDisable;

				PM1a_CNT = fadt->PM1aControlBlock;
				PM1b_CNT = fadt->PM1bControlBlock;

				PM1_CNT_LEN = fadt->PM1ControlLength;

				SLP_EN = 1 << 13;
				SCI_EN = 1;


			}
			else
			{
				printf("S5 parse error\n");
			}
		}
		else
		{
			printf("S5 not present\n");
		}

	}
	else
	{
		printf("DSDT invalid\n");
	}
}

int initAcpi()
{
	unsigned int* ptr = acpiGetRSDPtr();

	// Map page
	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)ptr, (uint32_t)ptr, I86_PTE_PRESENT);
	XSDT* xsdt = (XSDT*) ptr;

	// Check if the ptr we got points to a valid RSDT
	if ((ptr != 0) && (acpiCheckHeader(&xsdt->header, "XSDT") == 0))
	{

		int entries = (xsdt->header.Length - sizeof(xsdt->header)) / 8;

		// Iterate all tables

		for (uint32_t i = 0; i < entries; ++i)
		{
			ACPIheader* h = (ACPIheader*) xsdt->tables[i];

			vmmngr_mapPhysicalAddress(vmmngr_get_directory(), (uint32_t)h, (uint32_t)h, I86_PTE_PRESENT);

			char sign[5];

			memcpy(sign, h->Signature, 4);

			sign[4] = 0;

			printf("[ACPI] Found table: %s\n", sign);

			if (acpiCheckHeader(h, "FACP") == 0)
			{
				FADT* fadt = (FADT*) h;

				parseFADT(fadt);
			}

		}
	}
	return 0;
}

void acpiPowerOff()
{
	// check if ACPI shutdown is possible
	if (SCI_EN == 0)
	{
		return;
	}

	acpiEnable();

	outportw((unsigned int)PM1a_CNT, SLP_TYPa | SLP_EN);
	if (PM1b_CNT != 0)
	{
		outportw((unsigned int)PM1b_CNT, SLP_TYPb | SLP_EN);
	}

	printf("APCI poweroff failed.");
}

uint8_t acpiGetCenturyRegister()
{
	return centuryRegister;
}