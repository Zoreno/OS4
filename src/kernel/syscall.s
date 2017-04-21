[global syscall_interrupt_handler]
[extern syscall_handler]

[bits 32]
[section .text]

syscall_interrupt_handler:
	;pushal					; Push state to stack

	push 	eax				; Push EAX
	mov 	eax, 0x10		; Load Kernel Data Selector
	mov 	ds, ax			; Move it into Selector register
	pop 	eax				; Restore EAX

	push 	edx				; Push edx parameter
	push 	ecx				; Push ecx parameter
	push 	ebx				; Push ebx parameter
	push 	eax				; Push eax parameter
	call 	syscall_handler
	add 	esp, 16			; Restore stack

	push 	eax				; Push EAX
	mov 	eax, 0x23		; Load User Data Selector
	mov 	ds, ax			; Move it into Selector register
	pop 	eax				; Restore EAX

	;popal					; Pop state to stack
	iretd					; Return from interrupt
	
