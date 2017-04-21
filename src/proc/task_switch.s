[global scheduler_isr]

[bits 32]

[extern _currentThread]
[extern scheduler_tick]
[extern tss_set_stack]

scheduler_isr:
	
	; Clear interrupts
	; Save current context
	cli
	pushad

	; Check if there is a current process.
	mov		eax, [_currentThread]
	cmp		eax, 0
	jz		interrupt_return


	; Save selectors
	push	ds
	push	es
	push	fs
	push	gs

	
	; Switch to kernel selectors

	mov		ax, 0x10
	mov		ds, ax
	mov		es, ax
	mov		fs, ax
	mov		gs, ax
	
	; Save ESP
	
	mov 	eax, [_currentThread]
	mov		[eax], esp	

	; Call our scheduler	
	
	call	scheduler_tick

	; Restore ESP

	mov		eax, [_currentThread]
	mov		esp, [eax]

	push	DWORD [eax + 8]
	push	DWORD [eax + 12]
	call 	tss_set_stack
	add		esp, 8

	; Send EOI

	pop		gs
	pop		fs
	pop		es
	pop		ds

interrupt_return:

	; Send EOI and return

	mov		al, 0x20
	out		0x20, al
	
	; Restore context and return from interrupt.

	popad
	iretd










	
