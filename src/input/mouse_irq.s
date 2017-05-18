[bits 32]

[global mouse_irq]
[extern mouse_irq_handler]
[extern interruptdone]

;*
;	Mouse Interrupt handler
;
mouse_irq:

	; Save state
	cli
	pushad
	
	; Call our C handler

	call	mouse_irq_handler

	; Signal that we are done with the interrupt.	
	push	12
	call 	interruptdone
	add		esp, 4	

	; Restore state
	popad
	sti

	; Return from interrupt.
	iretd
