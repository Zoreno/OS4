.section .init
.global _init

.code32

_init:
	push	%ebp
	movl	%esp, %ebp

.section .fini
.global _fini

_fini:
	push	%ebp
	movl	%esp, %ebp
