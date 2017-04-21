.section .text

.code32

.global _start

.extern __g_main
.extern __g_exit

_start:
	push	%ebp
	movl	%esp, %ebp 

	# Call main 
	call 	__g_main

	# Push return code to stack
	push 	%eax	 



	# Call exit
	call 	__g_exit

	# Pop return code
	pop		%eax

	# Move return code to EBX
	movl	%eax, %ebx

	# Move terminate process command to EAX
	movl	$0x1, %eax

	# Call kernel
	int		$0x80

	# This should never happen
hang:
	jmp 	hang	

.size _start, . - _start
