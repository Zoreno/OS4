#include <proc/task.h>

#include <hal/hal.h>

#include <mm/physmem.h>
#include <mm/virtmem.h>

#include <lib/string.h>

#include <proc/elfloader.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

#define PROC_INVALID_ID -1

//=============================================================================
// Bitmap related stuff
//=============================================================================

// Bitmap for 65 536 IDs. All are free by default
static unsigned int _ID_bitmap[2048] = {0};

unsigned int readBit(unsigned int bit);
unsigned int setBit(unsigned int bit);
unsigned int clearBit(unsigned int bit);

unsigned int getNextFreeID();
void freeID(unsigned int id);

//=============================================================================
// Process related
//=============================================================================

Process* getLastProcess();
Thread* getLastThread(Process* process);

void mapKernelSpace(pdirectory* addressSpace);

Process* _rootProcess = 0;
Process* _kernelProcess = 0;
Process* _currentProcess = 0;

Thread* _currentThread = 0;

void* create_kernel_stack();
void* create_user_stack();

extern void scheduler_isr();
void sheduler_tick();

//=============================================================================
// Implementation
//=============================================================================

unsigned int readBit(unsigned int bit)
{
	unsigned int entry = bit / 32;
	unsigned int offset = bit % 32;

	return _ID_bitmap[entry] & (1 << offset);
}

unsigned int setBit(unsigned int bit)
{
	unsigned int entry = bit / 32;
	unsigned int offset = bit % 32;

	_ID_bitmap[entry] |= (1 << offset);
}

unsigned int clearBit(unsigned int bit)
{
	unsigned int entry = bit / 32;
	unsigned int offset = bit % 32;

	_ID_bitmap[entry] &= ~(1 << offset);
}

unsigned int getNextFreeID()
{
	for(unsigned int i = 0; i < 65536; ++i)
	{
		if(!readBit(i))
		{
			setBit(i);
			return i;
		}
	}

	return PROC_INVALID_ID;
}

void freeID(unsigned int id)
{
	if(readBit(id))
		clearBit(id);
}

Process* getRootProcess()
{
	return _rootProcess;
}

Process* getCurrentProcess()
{
	return _currentProcess;
}

Thread* getCurrentThread()
{
	return _currentThread;
}

Process* getKernelProcess()
{
	return _kernelProcess;
}

Process* getLastProcess()
{
	Process* p = _rootProcess;

	if(!p)
		return 0;

	while(p->nextProcess)
		p = p->nextProcess;

	return p;
}

void mapKernelSpace(pdirectory* addressSpace)
{
	uint32_t virtAddr;
	uint32_t physAddr;

	int flags = I86_PTE_PRESENT|I86_PTE_WRITABLE;

	vmmngr_mapPhysicalAddress(addressSpace, 0x8000, 0x8000, flags);
	vmmngr_mapPhysicalAddress(addressSpace, 0x9000, 0x9000, flags);

	virtAddr = 0xC0100000;
	physAddr = 0x100000;

	// Map Kernel Image
	// TODO: Check if enough
	for(uint32_t i = 0; i < 10; ++i)
	{
		vmmngr_mapPhysicalAddress(
			addressSpace,
			virtAddr + (i*PAGE_SIZE),
			physAddr + (i*PAGE_SIZE),
			flags);
	}

	// Map Video Memory

	virtAddr = 0xC00A0000;
	physAddr = 0xC00A0000;

	for(uint32_t i = 0; i < 31; ++i)
	{
		vmmngr_mapPhysicalAddress(
			addressSpace,
			virtAddr + (i*PAGE_SIZE),
			physAddr + (i*PAGE_SIZE),
			flags);
	}

	// Map the page directory

	vmmngr_mapPhysicalAddress(
		addressSpace,
		(uint32_t) addressSpace,
		(uint32_t) addressSpace,
		flags);
	
}

#define KERNEL_STACK_ALLOC_BASE 0xE0000000

int _kernel_stack_index = 0;

void* create_kernel_stack()
{
	physical_addr	p;
	virtual_addr	location;
	void*			ret;

	p = (physical_addr) pmmngr_alloc_block();
	if(!p)
		return 0;

	location = KERNEL_STACK_ALLOC_BASE + _kernel_stack_index * PAGE_SIZE;

	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), location, p, 3);

	ret = (void*) (location + PAGE_SIZE);
	
	_kernel_stack_index++;

	return ret;
}

#define USER_STACK_ALLOC_BASE 0x70000000

int _user_stack_index = 0;

void* create_user_stack()
{
	physical_addr	p;
	virtual_addr	location;
	void*			ret;

	p = (physical_addr) pmmngr_alloc_block();
	if(!p)
		return 0;
	
	location = USER_STACK_ALLOC_BASE + _user_stack_index * PAGE_SIZE;

	vmmngr_mapPhysicalAddress(vmmngr_get_directory(), location, p, 7);

	ret = (void*)(location + PAGE_SIZE);

	_user_stack_index++;

	return ret;
}

#define KERNEL_THREAD 1
#define USER_THREAD 0

int createProcess(char* appname, int is_kernel)
{
	is_kernel = KERNEL_THREAD;  // For now

	asm volatile("cli");
	printf("Creating Process\n");

	pdirectory* addressSpace = 0;
	Process* process = 0;
	Process* lastProcess = 0;
	Thread* mainThread = 0;

	// We create a new address space and map the kernel.
	addressSpace = vmmngr_cloneAddressSpace();

	// Load the executable from file.
	ElfImage img = loadELF(appname);

	// Allocate and null a new process object.
	process = (Process*)kmalloc(sizeof(Process));
	memset(process, 0, sizeof(Process));

	// Link the it to the Process chain.
	lastProcess = getLastProcess();
	lastProcess->nextProcess = process;

	// Setup process info.
	process->id = getNextFreeID();
	process->pageDirectory = addressSpace;
	process->priority = 1;
	process->state = PROCESS_STATE_ACTIVE;
	process->is_kernel = is_kernel;

	printf("Creating main thread\n");

	// Setup the main thread.
	mainThread = createThread(process, img.entry, is_kernel);

	printf("Process done.\n");

	// Return the process ID

	asm volatile("sti");
	return process->id;
}

Thread* getLastThread(Process* process)
{
	Thread* t = 0;

	if(process->firstThread)
	{
		t = process->firstThread;
	}
	else
	{
		return 0;
	}

	while(t->nextThread)
		t = t->nextThread;
	
	return t;
}

Thread* createThread(Process* process, void(*entry)(void), int is_kernel)
{
	printf("Creating Thread\n");

	TrapFrame* frame;
	Thread* thread;

	#define USER_DATA	0x23
	#define USER_CODE	0x1B
	
	#define	KERNEL_DATA	0x10
	#define KERNEL_CODE	0x08

	uint32_t esp;

	if(is_kernel)
	{
		esp = create_kernel_stack();
	}
	else
	{
		esp = create_user_stack();
	}

	printf("%#x\n", esp);

	thread = (Thread*)kmalloc(sizeof(Thread));
	memset(thread, 0, sizeof(Thread));
	
	esp -= sizeof(TrapFrame);

	frame = ((TrapFrame*) esp);

	thread->frame = frame;

	frame->eflags = 0x202;
	frame->eip = (uint32_t)entry;
	frame->ebp = 0;
	frame->esp = esp;
	frame->esp = 0;
	frame->esi = 0;
	frame->edi = 0;
	frame->edx = 0;
	frame->ecx = 0;
	frame->ebx = 0;
	frame->eax = 0;

	if(is_kernel)
	{
		frame->cs 	= KERNEL_CODE;
		frame->ds 	= KERNEL_DATA;
		frame->es 	= KERNEL_DATA;
		frame->fs 	= KERNEL_DATA;
		frame->gs 	= KERNEL_DATA;
		thread->ss	= KERNEL_DATA;
	}
	else
	{
		frame->cs 	= USER_CODE;
		frame->ds 	= USER_DATA;
		frame->es 	= USER_DATA;
		frame->fs 	= USER_DATA;
		frame->gs 	= USER_DATA;
		thread->ss	= USER_DATA;
	}

	thread->esp = esp;

	thread->id = getNextFreeID();
	thread->parent = process;
	thread->priority = 1;
	thread->state = THREAD_RUN;
	thread->sleepTimeDelta = 0;

	thread->is_kernel = is_kernel;

	Thread* prevThread = getLastThread(process);

	if(prevThread)
		prevThread->nextThread = thread;
	else
		process->firstThread = thread;		

	process->threadCount += 1;

	return thread;
}

void initialize_scheduler()
{
	Process* kernelProcess = (Process*) kmalloc(sizeof(Process));
	memset(kernelProcess, 0, sizeof(Process));

	kernelProcess->id = getNextFreeID();
	kernelProcess->priority = 1;
	kernelProcess->state = PROCESS_STATE_ACTIVE;
	kernelProcess->pageDirectory = vmmngr_get_directory();

	_rootProcess = kernelProcess;
	_kernelProcess = kernelProcess;

	setvect(32, scheduler_isr, 0x80);
}

void thread_execute(Thread* t)
{
	_currentThread = t;
	_currentProcess = t->parent;

	asm volatile ("mov %0, %%esp"::"g" (t->esp));
	asm volatile ("pop	%gs");
	asm volatile ("pop	%fs");
	asm volatile ("pop	%es");
	asm volatile ("pop	%ds");
	asm volatile ("popal");
	asm volatile ("iret");
}

void dispatch()
{
	Thread* t = getCurrentThread();

	// If there is a next thread, switch to it.
	if(t->nextThread)
	{
		_currentThread = t->nextThread;
		return;	
	}
	
	// Else, find the next process.
	Process* p = t->parent;
	
	if(p->nextProcess)
	{
		_currentProcess = p->nextProcess;
		_currentThread = _currentProcess->firstThread;
		return;
	}

	// Else, we jump back to the the root process.

	_currentProcess = _rootProcess;
	_currentThread = _rootProcess->firstThread;
}

void scheduler_tick()
{
	dispatch();
}

void TerminateThread(Thread* thread)
{
	Process* parent = thread->parent;
	
	Thread* prevThread = parent->firstThread;

	// Relink thread list.
	if(thread == prevThread)
	{
		parent->firstThread = thread->nextThread;
	}
	else
	{
		while(prevThread->nextThread != thread)
			prevThread = prevThread->nextThread;

		prevThread->nextThread = thread->nextThread;
	}

	printf("Terminating thread %i\n", thread->id);

	// TODO: Unmap stack

	freeID(thread->id);

	parent->threadCount--;

	kfree(thread);
}

void TerminateProcess(int retCode)
{
	printf("Process exited with code %i", retCode);

	Process* current = getCurrentProcess();

	if(current->id==PROC_INVALID_ID)
		return;

	// Inefficent
	while(current->threadCount)
		TerminateThread(current->firstThread);

	// TODO: Unmap image


	// Relink process list
	Process* p = getRootProcess();

	while(p->nextProcess != current)
	{
		p = p->nextProcess;
	}

	p->nextProcess = current->nextProcess;

	printf("Terminating process %i\n", current->id);

	kfree(current);
}

void printProcessTree()
{
	Process* p = getRootProcess();

	while(p)
	{
		printf("[%i]", p->id);

		Thread* t = p->firstThread;

		while(t)
		{
			printf("->%i", t->id);
			t = t->nextThread;
		}
		printf("\n");
		p = p->nextProcess;
	}
}

