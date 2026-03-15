OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

SECTIONS
{
	/* Linked virtual base address requested by KURO loader. */
	. = 0x01000000;

	.text : ALIGN(0x1000)
	{
		*(.text .text.*)
	}

	.rodata : ALIGN(0x1000)
	{
		*(.rodata .rodata.*)
	}

	.data : ALIGN(0x1000)
	{
		*(.data .data.*)
	}

	.bss : ALIGN(0x1000)
	{
		*(COMMON)
		*(.bss .bss.*)
	}
}
