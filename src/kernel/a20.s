;;
;; NASM 32bit assembler
;;

[global enable_A20]

[bits 32]
[section .text]

enable_A20:
	mov ax, 0x2401
	int 0x15
	ret