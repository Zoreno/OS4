#ifndef _VIRTMEM_H
#define _VIRTMEM_H

#include <lib/stdint.h>
#include <mm/physmem.h>

// TODO: Comment doxygen

//===================================================================
// PTE (Page Table Entry)
//===================================================================

enum PAGE_PTE_FLAGS {

	I86_PTE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PTE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PTE_USER			=	4,			//0000000000000000000000000000100
	I86_PTE_WRITETHOUGH		=	8,			//0000000000000000000000000001000
	I86_PTE_NOT_CACHEABLE	=	0x10,		//0000000000000000000000000010000
	I86_PTE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PTE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PTE_PAT				=	0x80,		//0000000000000000000000010000000
	I86_PTE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PTE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PTE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000
};

typedef uint32_t pt_entry;

// Sets a flag in the page table entry
void pt_entry_add_attrib (pt_entry* e, uint32_t attrib);

// Clears a flag in the page table entry
void pt_entry_del_attrib (pt_entry* e, uint32_t attrib);

// Sets a frame to page table entry
void pt_entry_set_frame (pt_entry* e, physical_addr addr);

// Test if page is present
int pt_entry_is_present (pt_entry e);

// Test if page is writable
int pt_entry_is_writable (pt_entry e);

// Get page table entry frame address
physical_addr pt_entry_pfn (pt_entry e);

//===================================================================
// PDE (Page Directory Entry)
//===================================================================

enum PAGE_PDE_FLAGS {

	I86_PDE_PRESENT			=	1,			//0000000000000000000000000000001
	I86_PDE_WRITABLE		=	2,			//0000000000000000000000000000010
	I86_PDE_USER			=	4,			//0000000000000000000000000000100
	I86_PDE_PWT				=	8,			//0000000000000000000000000001000
	I86_PDE_PCD				=	0x10,		//0000000000000000000000000010000
	I86_PDE_ACCESSED		=	0x20,		//0000000000000000000000000100000
	I86_PDE_DIRTY			=	0x40,		//0000000000000000000000001000000
	I86_PDE_4MB				=	0x80,		//0000000000000000000000010000000
	I86_PDE_CPU_GLOBAL		=	0x100,		//0000000000000000000000100000000
	I86_PDE_LV4_GLOBAL		=	0x200,		//0000000000000000000001000000000
   	I86_PDE_FRAME			=	0x7FFFF000 	//1111111111111111111000000000000	
};

typedef uint32_t pd_entry;

// Sets a flag in the page table entry
void pd_entry_add_attrib (pd_entry* e, uint32_t attrib);

// Clears a flag in the page table entry
void pd_entry_del_attrib (pd_entry* e, uint32_t attrib);

// Sets a frame to page table entry
void pd_entry_set_frame (pd_entry* e, physical_addr addr);

// Test if page is present
int pd_entry_is_present (pd_entry e);

// Test if directory is user mode
int pd_entry_is_user (pd_entry e);

// Test if directory contains 4mb pages
int pd_entry_is_4mb (pd_entry e);

// Test if page is writable
int pd_entry_is_writable (pd_entry e);

// Get page table entry frame address
physical_addr pd_entry_pfn (pd_entry e);

// Enable global pages
void pd_entry_enable_global (pd_entry e);

//===================================================================
// Virtual Memory Manager
//===================================================================

typedef uint32_t virtual_addr;

// Defined by architecture. Do not change.
#define PAGES_PER_TABLE	1024
#define PAGES_PER_DIR	1024

#define PAGE_DIRECTORY_INDEX(x) (((x) >> 22) & 0x3ff)
#define PAGE_TABLE_INDEX(x) (((x) >> 12) & 0x3ff)
#define PAGE_GET_PHYSICAL_ADDRESS(x) (*x & ~0xFFF)

// Page table
typedef struct {

	pt_entry m_entries[PAGES_PER_TABLE];

} ptable;

// Page directory
typedef struct {

	pd_entry m_entries[PAGES_PER_DIR];

}pdirectory;

// maps phys to virtual address
void vmmngr_map_page(void* phys, void* virt);

// initialize the memory manager
void vmmngr_initialize ();

// allocates a page in physical memory
int vmmngr_alloc_page (pt_entry*);

// frees a page in physical memory
void vmmngr_free_page (pt_entry* e);

// switch to a new page directory
int vmmngr_switch_pdirectory (pdirectory*);

// get current page directory
pdirectory* vmmngr_get_directory ();

// flushes a cached translation lookaside buffer (TLB) entry
void vmmngr_flush_tlb_entry (virtual_addr addr);

// clears a page table
void vmmngr_ptable_clear (ptable* p);

// convert virtual address to page table index
uint32_t vmmngr_ptable_virt_to_index (virtual_addr addr);

// get page entry from page table
pt_entry* vmmngr_ptable_lookup_entry (ptable* p,virtual_addr addr);

// convert virtual address to page directory index
uint32_t vmmngr_pdirectory_virt_to_index (virtual_addr addr);

// clears a page directory table
void vmmngr_pdirectory_clear (pdirectory* dir);

// get directory entry from directory table
pd_entry* vmmngr_pdirectory_lookup_entry (pdirectory* p, virtual_addr addr);

int vmmngr_createPageTable(pdirectory* dir, uint32_t virt, uint32_t flags);

void vmmngr_mapPhysicalAddress(pdirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags);

void vmmngr_unmapPageTable(pdirectory* dir, uint32_t virt);

void vmmngr_unmapPhysicalAddress(pdirectory* dir, uint32_t virt);

pdirectory* vmmngr_createAddressSpace();

void* vmmngr_getPhysicalAddress(pdirectory* dir, uint32_t virt);

#endif