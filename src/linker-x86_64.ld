OUTPUT_FORMAT("elf64-x86-64")
ENTRY(start)

SECTIONS
{
	/* 1 MB */
	. = 0x100000;
	
	kern_phys_start = .;

	.setup :
	{
		*(.multiboot)
		*(.setup)
	}

	.text : AT(ADDR(.text))
	{
		*(.text)
	}

	.data ALIGN(4096) : AT(ADDR(.data))
	{
		*(.data)
		*(.rodata*)
	}

	.bss ALIGN(4096) : AT(ADDR(.bss))
	{
		*(COMMON*)
		*(.bss*)
	}
	kern_phys_end = .;
}
