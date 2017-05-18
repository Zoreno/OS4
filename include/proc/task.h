#ifndef _TASK_H
#define _TASK_H

#include <lib/stdint.h>

#include <mm/virtmem.h>

#define KE_USER_START	0x00400000
#define KE_KERNEL_START	0x80000000

// TEMPORARY!
#define MAX_THREAD 5

#define PROCESS_STATE_SLEEP		0
#define PROCESS_STATE_ACTIVE	1

typedef struct _TrapFrame
{
	// Pushed by ISR
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;

	// Pushed by Pushf

	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t esp;
	uint32_t ebp;

	// Pushed by CPU
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;

} TrapFrame;

typedef unsigned int ktime_t;

#define THREAD_STATE_SLEEP		1

struct _Process;

/*
	Structure:

	Root->Process1->Process2->Nullptr
		  |			|
		  V			V
		  Thread1	Thread1
		  |			|
		  V			V
		  Thread2	Nullptr
		  |
		  V
		  Nullptr

	Scheduler:
	
	Thread* t= getCurrentThread();
	if(t->nextThread)
		switch(t->nextThread);
	Process* p = t->parent;
	if(p->nextProcess)
		switch(p->nextProcess->firstThread):
	
	switch(rootProcess->firstThread);
		
*/

typedef struct _Thread
{
	uint32_t			esp;
	uint32_t			ss;
	uint32_t			kernelEsp;
	uint32_t			kernelSs;

	struct _Process*	parent;
	uint32_t			priority;
	int 				state;
	TrapFrame*			frame;

	ktime_t				sleepTimeStart;
	ktime_t 			sleepTimeDelta;
	ktime_t				sleepTimeEnd;

	uint32_t			is_kernel;
	
	struct _Thread*		nextThread;

	unsigned int 		id;

} Thread;

typedef struct _Process
{
	unsigned int		id;
	int					priority;
	pdirectory*			pageDirectory;
	uint32_t 			state;
	uint32_t			imageBase;
	uint32_t			imageSize;

	uint32_t			is_kernel;

	struct  _Process*	nextProcess;
	
	int					threadCount;

	struct _Thread*		firstThread;
} Process;

Process* getRootProcess();

Process* getKernelProcess();

Process* getCurrentProcess();

Thread* getCurrentThread();

extern int createProcess(char* appname, int is_kernel);

extern Thread* createThread(Process* process, void(*entry)(void), int is_kernel);

void TerminateThread(Thread* thread);

void TerminateProcess(int retCode);

void thread_sleep(uint32_t ticks);

void initialize_scheduler();

void thread_execute(Thread* t);

void printProcessTree();

#endif
