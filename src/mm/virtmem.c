#include <mm/virtmem.h>

#include <lib/stdio.h>

//===================================================================
// PTE (Page Table Entry)
//===================================================================

void pt_entry_add_attrib (pt_entry* e, uint32_t attrib){
	*e |= attrib;
}

void pt_entry_del_attrib (pt_entry* e, uint32_t attrib){
	*e &= ~attrib;
}

void pt_entry_set_frame (pt_entry* e, physical_addr addr){
	*e = (*e & ~I86_PTE_FRAME) | addr;
}

int pt_entry_is_present (pt_entry e){
	return e & I86_PTE_PRESENT;
}

int pt_entry_is_writable (pt_entry e){
	return e & I86_PTE_WRITABLE;
}

physical_addr pt_entry_pfn (pt_entry e){
	return e & I86_PTE_FRAME;
}

//===================================================================
// PDE (Page Directory Entry)
//===================================================================

void pd_entry_add_attrib (pd_entry* e, uint32_t attrib){
	*e |= attrib;
}

void pd_entry_del_attrib (pd_entry* e, uint32_t attrib){
	*e &= ~attrib;
}

void pd_entry_set_frame (pd_entry* e, physical_addr addr){
	*e = (*e & ~I86_PDE_FRAME) | addr;
}

int pd_entry_is_present (pd_entry e){
	return e & I86_PDE_PRESENT;
}

int pd_entry_is_user (pd_entry e){
	return e & I86_PDE_USER;
}

int pd_entry_is_4mb (pd_entry e){
	return e & I86_PDE_4MB;
}

int pd_entry_is_writable (pd_entry e){
	return e & I86_PDE_WRITABLE;
}

physical_addr pd_entry_pfn (pd_entry e){
	return e & I86_PDE_FRAME;
}

void pd_entry_enable_global (pd_entry e){
	// Empty for now.	
}

//===================================================================
// Virtual Memory Manager
//===================================================================

#define PTABLE_ADDR_SPACE_SIZE 0x400000
#define DTABLE_ADDR_SPACE_SIZE 0x100000000
#define PAGE_SIZE 4096

// current directory table
pdirectory* _cur_directory = 0;

// current page directory base register
physical_addr _cur_pdbr = 0;

void vmmngr_map_page(void* phys, void* virt){

	// Get page directory
	pdirectory* pageDirectory = vmmngr_get_directory();

	pd_entry* e = 
		&pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t) virt)];

	if((*e & I86_PTE_PRESENT) != I86_PTE_PRESENT){

		// Allocate page table
		ptable* table = (ptable*)pmmngr_alloc_block();

		if(!table)
			return;

		memset(table,0,sizeof(ptable));

		pd_entry* entry = 
			&pageDirectory->m_entries[PAGE_DIRECTORY_INDEX((uint32_t) virt)];

		pd_entry_add_attrib(entry, I86_PDE_PRESENT);
		pd_entry_add_attrib(entry, I86_PDE_WRITABLE);
		pd_entry_set_frame(entry, (physical_addr)table);
	}

	// Get table
	ptable* table = (ptable*) PAGE_GET_PHYSICAL_ADDRESS(e);

	// Get page
	pt_entry* page = &table->m_entries[PAGE_TABLE_INDEX((uint32_t) virt)];

	pt_entry_set_frame(page, (physical_addr)phys);
	pt_entry_add_attrib(page, I86_PTE_PRESENT);
}

void vmmngr_initialize(){

	// allocate default page table
	ptable* table = (ptable*) pmmngr_alloc_block ();
	if (!table){
		return;
	}
    	

	// allocates 3gb page table
	ptable* table2 = (ptable*) pmmngr_alloc_block ();
	if (!table2){
		return;
	}

	// clear page table
	memset (table, 0, sizeof (ptable));

	// 1st 4mb are idenitity mapped
	for (int i=0, frame=0x0, virt=0x00000000; i<1024; i++, frame+=4096, virt+=4096) {

		// create a new page
		pt_entry page=0;
		pt_entry_add_attrib (&page, I86_PTE_PRESENT);
		pt_entry_set_frame (&page, frame);

		// ...and add it to the page table
		table2->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
	}

	// map 1mb to 3gb (where we are at)
	for (int i=0, frame=0x000000, virt=0xc0000000; i<1024; i++, frame+=4096, virt+=4096) {

		// create a new page
		pt_entry page=0;
		pt_entry_add_attrib (&page, I86_PTE_PRESENT);
		pt_entry_set_frame (&page, frame);

		// ...and add it to the page table
		table->m_entries [PAGE_TABLE_INDEX (virt) ] = page;
	}

	// create default directory table
	pdirectory*   dir = (pdirectory*) pmmngr_alloc_blocks (3);
	if (!dir){
		return;
	}

	// clear directory table and set it as current
	memset (dir, 0, sizeof (pdirectory));

	// get first entry in dir table and set it up to point to our table
	pd_entry* entry = &dir->m_entries [PAGE_DIRECTORY_INDEX (0xc0000000) ];
	pd_entry_add_attrib (entry, I86_PDE_PRESENT);
	pd_entry_add_attrib (entry, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry, (physical_addr)table);

	pd_entry* entry2 = &dir->m_entries [PAGE_DIRECTORY_INDEX (0x00000000) ];
	pd_entry_add_attrib (entry2, I86_PDE_PRESENT);
	pd_entry_add_attrib (entry2, I86_PDE_WRITABLE);
	pd_entry_set_frame (entry2, (physical_addr)table2);

	// store current PDBR
	_cur_pdbr = (physical_addr) &dir->m_entries;

	// switch to our page directory
	vmmngr_switch_pdirectory (dir);

	// enable paging
	pmmngr_paging_enable (1);
}

int vmmngr_alloc_page(pt_entry* e){

	void* p = pmmngr_alloc_block();

	if (!p)
		return 0;

	pt_entry_set_frame (e, (physical_addr)p);
	pt_entry_add_attrib (e, I86_PTE_PRESENT);
	return 1;
}

void vmmngr_free_page(pt_entry* e){

	void* p = (void*)pt_entry_pfn (*e);
	if (p)
		pmmngr_free_block (p);

	pt_entry_del_attrib (e, I86_PTE_PRESENT);
}

int vmmngr_switch_pdirectory(pdirectory* dir){

	if(!dir)
		return 0;

	_cur_directory = dir;

	printf("%#0(10)p\n", _cur_pdbr);

	pmmngr_load_PBDR(_cur_pdbr);

	return 1;
}

pdirectory* vmmngr_get_directory(){
	return _cur_directory;
}

void vmmngr_flush_tlb_entry(virtual_addr addr){

	asm volatile ("cli");
	asm volatile ("invlpg (%0)" ::"b"(addr): "memory");
	asm volatile ("sti");
}

void vmmngr_ptable_clear(ptable* p) {

	if (p)
		memset(p, 0, sizeof(ptable));
}

uint32_t vmmngr_ptable_virt_to_index(virtual_addr addr) {

	//! return index only if address doesnt exceed page table address space size
	return (addr >= PTABLE_ADDR_SPACE_SIZE) ? 0 : addr / PAGE_SIZE;
}

pt_entry* vmmngr_ptable_lookup_entry(ptable* p,virtual_addr addr){

	if(p)
		return &p->m_entries[PAGE_TABLE_INDEX(addr)];
	return 0;
}

uint32_t vmmngr_pdirectory_virt_to_index(virtual_addr addr) {

	//! return index only if address doesnt exceed 4gb (page directory address space size)
	return (addr >= DTABLE_ADDR_SPACE_SIZE) ? 0 : addr / PAGE_SIZE;
}

void vmmngr_pdirectory_clear(pdirectory* dir) {

	if (dir)
		memset(dir, 0, sizeof(pdirectory));
}

pd_entry* vmmngr_pdirectory_lookup_entry(pdirectory* p, virtual_addr addr){

	if(p)
		&p->m_entries[PAGE_TABLE_INDEX(addr)];
	return 0;
}

int vmmngr_createPageTable(pdirectory* dir, uint32_t virt, uint32_t flags) {

	pd_entry* pagedir = dir->m_entries;
	if (pagedir[virt >> 22] == 0) {
		void* block = pmmngr_alloc_block();
		if (!block)
			return 0; /* Should call debugger */
		pagedir[virt >> 22] = ((uint32_t)block) | flags;
		memset((uint32_t*)pagedir[virt >> 22], 0, 4096);

		/* map page table into directory */
		vmmngr_mapPhysicalAddress(dir, (uint32_t)block, (uint32_t)block, flags);
	}
	return 1; /* success */
}

void vmmngr_mapPhysicalAddress(pdirectory* dir, uint32_t virt, uint32_t phys, uint32_t flags) {

	pd_entry* pagedir = dir->m_entries;
	if (pagedir[virt >> 22] == 0)
		vmmngr_createPageTable(dir, virt, flags);
	((uint32_t*)(pagedir[virt >> 22] & ~0xfff))[virt << 10 >> 10 >> 12] = phys | flags;
}

void vmmngr_unmapPageTable(pdirectory* dir, uint32_t virt) {
	pd_entry* pagedir = dir->m_entries;
	if (pagedir[virt >> 22] != 0) {

		/* get mapped frame */
		void* frame = (void*)(pagedir[virt >> 22] & 0x7FFFF000);

		/* unmap frame */
		pmmngr_free_block(frame);
		pagedir[virt >> 22] = 0;
	}
}

void vmmngr_unmapPhysicalAddress(pdirectory* dir, uint32_t virt) {
	/* note: we don't unallocate physical address here; callee does that */
	pd_entry* pagedir = dir->m_entries;
	if (pagedir[virt >> 22] != 0)
		vmmngr_unmapPageTable(dir, virt);
	//      ((uint32_t*) (pagedir[virt >> 22] & ~0xfff))[virt << 10 >> 10 >> 12] = 0;
}

pdirectory* vmmngr_createAddressSpace() {
	pdirectory* dir = 0;

	/* allocate page directory */
	dir = (pdirectory*)pmmngr_alloc_block();
	if (!dir)
		return 0;

	/* clear memory (marks all page tables as not present) */
	memset(dir, 0, sizeof(pdirectory));
	return dir;
}

void* vmmngr_getPhysicalAddress(pdirectory* dir, uint32_t virt) {
	pd_entry* pagedir = dir->m_entries;
	if (pagedir[virt >> 22] == 0)
		return 0;
	return (void*)((uint32_t*)(pagedir[virt >> 22] & ~0xfff))[virt << 10 >> 10 >> 12];
}