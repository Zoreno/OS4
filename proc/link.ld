ENTRY(_start)

SECTIONS
{
	__OS4_stack_size__ = 4096;

	. = 0x804800;
	.text : 
	{
		*(.text)
	}
	.data : 
	{
		*(.data)
		*(.rodata)
	}
	.bss : 
	{
		*(.bss)
	}
	.stack ALIGN(0x1000) : 
	{
		__OS4_stack_end__ = .;
		. = . + __OS4_stack_size__;
		__OS4_stack_start__ = .;
	}
}
