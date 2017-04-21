[global flush_tss]

flush_tss:
	cli
	mov		ax, 0x28
	ltr 	ax
	sti
	ret
