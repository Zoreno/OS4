.section .text

.code32

.global _start

.extern __g_main
.extern __g_exit

_start:
	# Call initialization
	call	_init

	# Call main 
	call 	__g_main

	# Push return code to stack
	push 	%eax	 

	# Call exit
	call 	__g_exit

	# Pop return code
	pop		%eax

	ret

.size _start, . - _start
