.global _start

.extern __g_main
.extern __g_exit

_start:
	call __g_main

	call __g_exit

	mov $0x12345678, %eax

	ret

wait:
	hlt
	jmp wait
