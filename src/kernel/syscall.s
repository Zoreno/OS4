[global syscall_interrupt_handler]
[extern syscall_handler]

[bits 32]
[section .text]

syscall_interrupt_handler:
	pushal			; Push state to stack
	push edx		; Push edx parameter
	push ecx		; Push ecx parameter
	push ebx		; Push ebx parameter
	push eax		; Push eax parameter
	call syscall_handler
	add esp, 16		; Restore stack
	popal			; Pop state to stack
	iretd			; Return from interrupt
	
