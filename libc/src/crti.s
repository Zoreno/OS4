section .init

[global _init]

[bits 32]

_init:
	push	ebp
	mov		ebp, esp

section .fini
[global _fini]

_fini:
	push	ebp
	mov		ebp, esp
