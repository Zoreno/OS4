section .text

[bits 32]

[global _start]

[extern __g_main]
[extern __g_exit]
[extern _init]
[extern _fini]

_start:

	; Setup prologue

	push	ebp
	mov		ebp, esp

	; Call initialization
	call	_init

	; Call main 
	call 	__g_main

	; Push return code to stack
	push 	eax	 

	; Call exit
	call 	__g_exit

	; Pop return code
	pop		eax

	; Move return code to EBX
	mov		ebx, eax

	; Move terminate process command to EAX
	mov		eax, 0x01

	; Call kernel
	int		0x80

	; This should never happen
hang:
	jmp 	hang	
