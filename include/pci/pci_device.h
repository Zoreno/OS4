/**
* @file pci_device.h
* @author Joakim Bertils
* @date 3 September 2016
* @brief File containing interface of the PCI device abstraction
*
* @see http://wiki.osdev.org/PCI
*/

#ifndef _PCI_DEVICE_H
#define _PCI_DEVICE_H

#include <lib/stdint.h>

/**
* PCI Device Command Register Layout
*
* Provides control over a device's ablility to generate and respond to 
* PCI cycles. Where the only functionality guaranteed to be supported by 
* all devices is, when a 0 is written to this register, the device is 
* disconnected from the PCI bus for all accesses except Configuration Space
* access.
*/
typedef struct _CommandField
{
	/**
	* If set to 1 the device can respond to I/O Space accesses; otherwise,
	* the device's response is disabled.
	*/
	uint16_t IOSpace : 1;

	/**
	* If set to 1 the device can respond to Memory Space accesses; 
	* otherwise, the device's response is disabled.
	*/
	uint16_t MemorySpace : 1;

	/**
	* If set to 1 the device can behave as a bus master; otherwise,
	* the device can not generate PCI accesses.
	*/
	uint16_t BusMaster : 1;

	/**
	* If set to 1 the device can monitor Special Cycle operations;
	* otherwise, the device will ignore them.
	*/
	uint16_t SpecialCycles : 1;

	/**
	* If set to 1 the device can generate the Memory Write and 
	* Invalidate command; otherwise, the Memory Write command must be used.
	*/
	uint16_t MemWriteAndInvEnable : 1;

	/**
	* If set to 1 the device does not respond to palette register writes 
	* and will snoop the data; otherwise, the device will trate palette write 
	* accesses like all other accesses.
	*/
	uint16_t VGAPaletteSnoop : 1;

	/**
	* If set to 1 the device will take its normal action when a parity error is
	* detected; otherwise, when an error is detected, the device will set bit 15
	* of the Status register (Detected Parity Error Status Bit), but will not 
	* assert the PERR# (Parity Error) pin and will continue operation as normal.
	*/
	uint16_t ParityErrorResponse : 1;

	/**
	* Reserved
	*/
	uint16_t reserved : 1;

	/**
	* If set to 1 the SERR# driver is enabled; otherwise, the driver is 
	* disabled.
	*/
	uint16_t SERR_Enable : 1;

	/**
	* If set to 1 indicates a device is allowed to generate fast back-to-back 
	* transactions; otherwise, fast back-to-back transactions are only allowed 
	* to the same agent.
	*/
	uint16_t FastBackToBackEnable : 1;

	/**
	* If set to 1 the assertion of the devices INTx# signal is disabled; 
	* otherwise, assertion of the signal is enabled.
	*/
	uint16_t InteruptEnable : 1;

	/**
	* Reserved
	*/
	uint16_t reserved2 : 5;

} PciCommand_t;

/**
 * Layout of the status register
 */
typedef struct _StatusField
{
	/**
	* Reserved
	*/
	uint16_t reserved : 3;

	/**
	* Represents the state of the device's interrupt signal.
	*/
	uint16_t InterruptStatus : 1;

	/**
	 * If set to one, the device implements the pointer for a new capabilities
	 * linked list at offset 0x34.
	 */
	uint16_t CapabilitiesList : 1;

	/**
	* If set to one, the device runs at 66 MHz.
	*/
	uint16_t Capable66MHz : 1;

	/**
	* Reserved
	*/
	uint16_t reserved2 : 1;

	/**
	* The device supports fast back to back transactions that are not from the
	* same agent.
	*/
	uint16_t FastBackToBackCapable : 1;

	/**
	* Master data parity error.
	*/
	uint16_t MasterDataParityError : 1;

	/**
	* Devsel Timing
	*/
	uint16_t DEVSELTiming : 2;

	/**
	* Set to one whenever a target device terminates a transaction with
	* Target-Abort.
	*/
	uint16_t SignaledTargetAbort : 1;

	/**
	* Set to one by a master device whenever it's transaction is terminated 
	* with Target-Abort.
	*/
	uint16_t RecievedTargetAbort : 1;

	/**
	* Set to one by a master device whenever it's transaction is terminated 
	* with Master-Abort.
	*/
	uint16_t RecievedMasterAbort : 1;

	/**
	* Set to one whenever the device asserts SERR#.
	*/
	uint16_t SignaledSystemError : 1;

	/**
	* Set to one whenever the device detects a parity error.
	*/
	uint16_t DetectedParityError : 1;
} PciStatus_t;

/**
 * PCI device info struct
 */
typedef struct _PciDeviceInfo
{
	/**
	* Indicates that the device is present.
	*/
	uint8_t present;

	/**
	* Identifies the manufacturer of the device. A value of 0xFFFF indicates a 
	* non-existing device.
	*/
	uint16_t vendorID;

	/**
	* Identifies the particular device.
	*/
	uint16_t deviceID;

	/**
	* Provides control over a device's ability to generate and respond to PCI
	* cycles.
	*/
	union { uint16_t command_w; PciCommand_t command; };

	/**
	* A register used to record status information for PCI bus related events.
	*/
	union { uint16_t status_w; PciStatus_t status; };

	/**
	* Specifies a revision identifier for a particular device.
	*/
	uint8_t revisionID;

	/**
	* A read-only register that specifies which register-level programming
	* interface has, if any.
	*/
	uint8_t progIntf;

	/**
	* A read-only register that specifies the specific function the device
	* performs.
	*/
	uint8_t subClass;

	/**
	* A read-only register that specifies the type of function the device
	* performs.
	*/
	uint8_t classCode;

	/**
	* Specifies the system cache line size in 32-bit units.
	*/
	uint8_t cacheLineSize;

	/**
	* Specifies the latency timer in units of PCI bus clocks.
	*/
	uint8_t latencyTimer;

	/**
	* Specifies the layout of the rest of the header beginning at offset 0x10. 
	* MSB bit specifies if the device has multiple functions.
	*/
	uint8_t headerType;

	/**
	* Represents the status and allows control of the device's built-in self-test.
	*/
	uint8_t BIST;
	
	union
	{
		/**
		 * These fields are applicable if headerType is 0x00.
		 */
		struct _PCI_HEADER_TYPE_0 {

			/**
			 * Base Address Registers (BARs)
			 */
			uint32_t BaseAddresses[6];

			/**
			 * CardBus CIS Pointer.
			 */
			uint32_t CIS;

			/**
			 * SubVendor ID
			 */
			uint16_t SubVendorID;

			/**
			 * Subsystem ID
			 */
			uint16_t SubSystemID;

			/**
			 * Expansion ROM base address
			 */
			uint32_t ROMBaseAddress;

			/**
			 * Capabilities pointer.
			 */
			uint8_t CapabilitiesPtr;

			/**
			 * Reserved
			 */
			uint8_t Reserved2[7];

			/**
			 * Specifies which input of the system interrupt controller the
			 * device's interrupt pin is connected to.
			 */
			uint8_t InterruptLine;

			/**
			 * Specifies which interrupt pin the device uses.
			 */
			uint8_t InterruptPin;

			/**
			 * A read-only register that specifies the burst period length.
			 */
			uint8_t MinimumGrant;

			/**
			 * A read-only register that specifies how often the device needs 
			 * access to the PCI bus.
			 */
			uint8_t MaximumLatency;
		} type0;

		/**
		 * These fields are applicable if headerType is 0x01.
		 */
		struct _PCI_HEADER_TYPE_1 {
			/**
			 * Base address registers.
			 */
			uint32_t BaseAddresses[2];

			/**
			 * Primary bus number
			 */
			uint8_t PrimaryBusNumber;

			/**
			 * Secondary bus number.
			 */
			uint8_t SecondaryBusNumber;

			/**
			 * Subordinate bus number.
			 */
			uint8_t SubordinateBusNumber;

			/**
			 * Secondary latency timer.
			 */
			uint8_t SecondaryLatencyTimer;
			uint8_t IOBase;
			uint8_t IOLimit;
			uint16_t SecondaryStatus;
			uint16_t MemoryBase;
			uint16_t MemoryLimit;
			uint16_t PrefetchableMemoryBase;
			uint16_t PrefetchableMemoryLimit;
			uint32_t PrefetchableMemoryBaseUpper32;
			uint32_t PrefetchableMemoryLimitUpper32;
			uint16_t IOBaseUpper;
			uint16_t IOLimitUpper;
			uint32_t Reserved2;
			uint32_t ExpansionROMBase;
			uint8_t InterruptLine;
			uint8_t InterruptPin;
			uint16_t BridgeControl;
		} type1;

		/**
		 * These fields are applicable if headerType is 0x02.
		 */	
		struct _PCI_HEADER_TYPE_2 {
			uint32_t BaseAddress;
			uint8_t CapabilitiesPtr;
			uint8_t Reserved2;
			uint16_t SecondaryStatus;
			uint8_t PrimaryBusNumber;
			uint8_t CardbusBusNumber;
			uint8_t SubordinateBusNumber;
			uint8_t CardbusLatencyTimer;
			uint32_t MemoryBase0;
			uint32_t MemoryLimit0;
			uint32_t MemoryBase1;
			uint32_t MemoryLimit1;
			uint16_t IOBase0_LO;
			uint16_t IOBase0_HI;
			uint16_t IOLimit0_LO;
			uint16_t IOLimit0_HI;
			uint16_t IOBase1_LO;
			uint16_t IOBase1_HI;
			uint16_t IOLimit1_LO;
			uint16_t IOLimit1_HI;
			uint8_t InterruptLine;
			uint8_t InterruptPin;
			uint16_t BridgeControl;
			uint16_t SubVendorID;
			uint16_t SubSystemID;
			uint32_t LegacyBaseAddress;
			uint8_t Reserved3[56];
			uint32_t SystemControl;
			uint8_t MultiMediaControl;
			uint8_t GeneralStatus;
			uint8_t Reserved4[2];
			uint8_t GPIO0Control;
			uint8_t GPIO1Control;
			uint8_t GPIO2Control;
			uint8_t GPIO3Control;
			uint32_t IRQMuxRouting;
			uint8_t RetryStatus;
			uint8_t CardControl;
			uint8_t DeviceControl;
			uint8_t Diagnostic;
		} type2;
	};

} PciDeviceInfo_t;

typedef struct _pci_device_list{

	// Device info struct
	PciDeviceInfo_t dev_info;

	// Ptr to next device
	struct _pci_device_list* next;

} pci_device_list_t;

void pci_read_device_info(uint32_t id, PciDeviceInfo_t* info);

const char* pci_device_name(uint32_t vendorID, uint32_t deviceID);
const char* pci_class_name(uint32_t classCode, uint32_t subClass, uint32_t progIntf);

// PCI Vendors
#define VENDOR_INTEL                    0x8086

// PCI Classes
#define PCI_CLASS_LEGACY                0x00
#define PCI_CLASS_STORAGE               0x01
#define PCI_CLASS_NETWORK               0x02
#define PCI_CLASS_DISPLAY               0x03
#define PCI_CLASS_MULTIMEDIA            0x04
#define PCI_CLASS_MEMORY                0x05
#define PCI_CLASS_BRIDGE_DEVICE         0x06
#define PCI_CLASS_COMMUNICATION         0x07
#define PCI_CLASS_PERIHPERALS           0x08
#define PCI_CLASS_INPUT_DEVICES         0x09
#define PCI_CLASS_DOCKING_STATION       0x0a
#define PCI_CLASS_PROCESSOR             0x0b
#define PCI_CLASS_SERIAL_BUS            0x0c
#define PCI_CLASS_WIRELESS              0x0d
#define PCI_CLASS_INTELLIGENT_IO        0x0e
#define PCI_CLASS_SATELLITE             0x0f
#define PCI_CLASS_CRYPT                 0x10
#define PCI_CLASS_SIGNAL_PROCESSING     0x11
#define PCI_CLASS_UNDEFINED             0xff

// Undefined Class
#define PCI_UNCLASSIFIED                0x0000
#define PCI_VGA_COMPATIBLE              0x0001

// Mass Storage Controller
#define PCI_STORAGE_SCSI                0x0100
#define PCI_STORAGE_IDE                 0x0101
#define PCI_STORAGE_FLOPPY              0x0102
#define PCI_STORAGE_IPI                 0x0103
#define PCI_STORAGE_RAID                0x0104
#define PCI_STORAGE_ATA                 0x0105
#define PCI_STORAGE_SATA                0x0106
#define PCI_STORAGE_OTHER               0x0180

// Network Controller
#define PCI_NETWORK_ETHERNET            0x0200
#define PCI_NETWORK_TOKEN_RING          0x0201
#define PCI_NETWORK_FDDI                0x0202
#define PCI_NETWORK_ATM                 0x0203
#define PCI_NETWORK_ISDN                0x0204
#define PCI_NETWORK_WORLDFIP            0x0205
#define PCI_NETWORK_PICGMG              0x0206
#define PCI_NETWORK_OTHER               0x0280

// Display Controller
#define PCI_DISPLAY_VGA                 0x0300
#define PCI_DISPLAY_XGA                 0x0301
#define PCI_DISPLAY_3D                  0x0302
#define PCI_DISPLAY_OTHER               0x0380

// Multimedia Controller
#define PCI_MULTIMEDIA_VIDEO            0x0400
#define PCI_MULTIMEDIA_AUDIO            0x0401
#define PCI_MULTIMEDIA_PHONE            0x0402
#define PCI_MULTIMEDIA_AUDIO_DEVICE     0x0403
#define PCI_MULTIMEDIA_OTHER            0x0480

// Memory Controller
#define PCI_MEMORY_RAM                  0x0500
#define PCI_MEMORY_FLASH                0x0501
#define PCI_MEMORY_OTHER                0x0580

// Bridge Device
#define PCI_BRIDGE_HOST                 0x0600
#define PCI_BRIDGE_ISA                  0x0601
#define PCI_BRIDGE_EISA                 0x0602
#define PCI_BRIDGE_MCA                  0x0603
#define PCI_BRIDGE_PCI                  0x0604
#define PCI_BRIDGE_PCMCIA               0x0605
#define PCI_BRIDGE_NUBUS                0x0606
#define PCI_BRIDGE_CARDBUS              0x0607
#define PCI_BRIDGE_RACEWAY              0x0608
#define PCI_BRIDGE_OTHER                0x0680

// Simple Communication Controller
#define PCI_COMM_SERIAL                 0x0700
#define PCI_COMM_PARALLEL               0x0701
#define PCI_COMM_MULTIPORT              0x0702
#define PCI_COMM_MODEM                  0x0703
#define PCI_COMM_GPIB                   0x0704
#define PCI_COMM_SMARTCARD              0x0705
#define PCI_COMM_OTHER                  0x0780

// Base System Peripherals
#define PCI_SYSTEM_PIC                  0x0800
#define PCI_SYSTEM_DMA                  0x0801
#define PCI_SYSTEM_TIMER                0x0802
#define PCI_SYSTEM_RTC                  0x0803
#define PCI_SYSTEM_PCI_HOTPLUG          0x0804
#define PCI_SYSTEM_SD                   0x0805
#define PCI_SYSTEM_OTHER                0x0880

// Input Devices
#define PCI_INPUT_KEYBOARD              0x0900
#define PCI_INPUT_PEN                   0x0901
#define PCI_INPUT_MOUSE                 0x0902
#define PCI_INPUT_SCANNER               0x0903
#define PCI_INPUT_GAMEPORT              0x0904
#define PCI_INPUT_OTHER                 0x0980

// Docking Stations
#define PCI_DOCKING_GENERIC             0x0a00
#define PCI_DOCKING_OTHER               0x0a80

// Processors
#define PCI_PROCESSOR_386               0x0b00
#define PCI_PROCESSOR_486               0x0b01
#define PCI_PROCESSOR_PENTIUM           0x0b02
#define PCI_PROCESSOR_ALPHA             0x0b10
#define PCI_PROCESSOR_POWERPC           0x0b20
#define PCI_PROCESSOR_MIPS              0x0b30
#define PCI_PROCESSOR_CO                0x0b40

// Serial Bus Controllers
#define PCI_SERIAL_FIREWIRE             0x0c00
#define PCI_SERIAL_ACCESS               0x0c01
#define PCI_SERIAL_SSA                  0x0c02
#define PCI_SERIAL_USB                  0x0c03
#define PCI_SERIAL_FIBER                0x0c04
#define PCI_SERIAL_SMBUS                0x0c05

#define PCI_SERIAL_USB_UHCI             0x00
#define PCI_SERIAL_USB_OHCI             0x10
#define PCI_SERIAL_USB_EHCI             0x20
#define PCI_SERIAL_USB_XHCI             0x30
#define PCI_SERIAL_USB_OTHER            0x80

// Wireless Controllers
#define PCI_WIRELESS_IRDA               0x0d00
#define PCI_WIRLESSS_IR                 0x0d01
#define PCI_WIRLESSS_RF                 0x0d10
#define PCI_WIRLESSS_BLUETOOTH          0x0d11
#define PCI_WIRLESSS_BROADBAND          0x0d12
#define PCI_WIRLESSS_ETHERNET_A         0x0d20
#define PCI_WIRLESSS_ETHERNET_B         0x0d21
#define PCI_WIRELESS_OTHER              0x0d80

// Intelligent I/O Controllers
#define PCI_INTELLIGENT_I2O             0x0e00

// Satellite Communication Controllers
#define PCI_SATELLITE_TV                0x0f00
#define PCI_SATELLITE_AUDIO             0x0f01
#define PCI_SATELLITE_VOICE             0x0f03
#define PCI_SATELLITE_DATA              0x0f04

// Encryption/Decryption Controllers
#define PCI_CRYPT_NETWORK               0x1000
#define PCI_CRYPT_ENTERTAINMENT         0x1001
#define PCI_CRYPT_OTHER                 0x1080

// Data Acquisition and Signal Processing Controllers
#define PCI_SP_DPIO                     0x1100
#define PCI_SP_OTHER                    0x1180

// PCI Configuration Registers
#define PCI_CONFIG_VENDOR_ID			0x00
#define PCI_CONFIG_DEVICE_ID			0x02
#define PCI_CONFIG_COMMAND				0x04
#define PCI_CONFIG_STATUS               0x06
#define PCI_CONFIG_REVISION_ID          0x08
#define PCI_CONFIG_PROG_INTF            0x09
#define PCI_CONFIG_SUBCLASS             0x0a
#define PCI_CONFIG_CLASS_CODE           0x0b
#define PCI_CONFIG_CACHELINE_SIZE       0x0c
#define PCI_CONFIG_LATENCY              0x0d
#define PCI_CONFIG_HEADER_TYPE          0x0e
#define PCI_CONFIG_BIST                 0x0f

// Type 0x00 (Generic) Configuration Registers
#define PCI_TYPE0_CONFIG_BAR0                 0x10
#define PCI_TYPE0_CONFIG_BAR1                 0x14
#define PCI_TYPE0_CONFIG_BAR2                 0x18
#define PCI_TYPE0_CONFIG_BAR3                 0x1c
#define PCI_TYPE0_CONFIG_BAR4                 0x20
#define PCI_TYPE0_CONFIG_BAR5                 0x24
#define PCI_TYPE0_CONFIG_CARDBUS_CIS          0x28
#define PCI_TYPE0_CONFIG_SUBSYSTEM_VENDOR_ID  0x2c
#define PCI_TYPE0_CONFIG_SUBSYSTEM_DEVICE_ID  0x2e
#define PCI_TYPE0_CONFIG_EXPANSION_ROM        0x30
#define PCI_TYPE0_CONFIG_CAPABILITIES         0x34
#define PCI_TYPE0_CONFIG_INTERRUPT_LINE       0x3c
#define PCI_TYPE0_CONFIG_INTERRUPT_PIN        0x3d
#define PCI_TYPE0_CONFIG_MIN_GRANT            0x3e
#define PCI_TYPE0_CONFIG_MAX_LATENCY          0x3f

#endif
