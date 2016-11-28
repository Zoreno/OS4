
[GLOBAL gdt_flush]
[GLOBAL idt_flush]

gdt_flush:
	mov 	eax, [esp+4]
	lgdt 	[eax]
.flush:
	ret

idt_flush:
	mov 	eax, [esp+4]
	lidt	[eax]
	ret
