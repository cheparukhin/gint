/*
	This linker script links the object files when generating the ELF
	output. Note how symbols romdata, bbss, ebss, bdata and edata are used
	in the initialization routine (crt0.c) to initialize the application.

	Two ram areas are specified. The "real ram" is accessed directly while
	the other area is virtualized. It is not possible to execute code in
	virtualized ram.
*/

OUTPUT_ARCH(sh3)
ENTRY(_start)

MEMORY
{
	/* System-managed mappings map this area into the add-in data */
	rom	: o = 0x00300200, l = 512k
	/* 0x0810000 is apparently mapped to 0x8801c0000 */
	ram	: o = 0x08100000, l = 8k
	/* RAM section from P1 area, no MMU involved */
	realram	: o = 0x8800d000, l = 12k
}

SECTIONS
{
	/* ROM sections: binary code and read-only data */

	.text : {
		/* Initialization code. */
		*(.pretext.entry)
		*(.pretext)

		_bctors = ABSOLUTE(.);
		*(.ctors)
		_ectors = ABSOLUTE(.);
		_bdtors = ABSOLUTE(.);
		*(.dtors)
		_edtors = ABSOLUTE(.);

		*(.text)
		*(.text.*)
	} > rom

	.rodata : {
		*(.rodata.fxconv);
		*(.rodata.paradox);
		*(.rodata)
		. = ALIGN(4);
		*(.rodata.*)

		_romdata = ALIGN(4) ;
	} > rom

	/* RAM sections: bss section and read/write data
	   The .bss section is to be stripped from the ELF file and wiped at
	   startup, hence its location is undefined */

	.bss : {
		_bbss = ABSOLUTE(.);
		_sbss = ABSOLUTE(SIZEOF(.bss));

		*(.bss)
	} > ram

	.data : AT(_romdata) ALIGN(4) {
		_bdata = ABSOLUTE(.);
		_sdata = ABSOLUTE(SIZEOF(.data));

		*(.data)
		*(.data.*)
	} > ram

	.cc : AT(_romdata + SIZEOF(.data)) ALIGN(4) {
		*(.eh_frame)
		*(.jcr)

		. = ALIGN(4);
	} > ram

	/* Real RAM sections: interrupt handlers and some *uninitialized* gint
	   data */

	_gint_data = _romdata + SIZEOF(.data) + SIZEOF(.cc) ;

	.gint : AT(_gint_data) ALIGN(4) {
		/* The vbr needs to be 0x100-aligned because of an ld issue */
		. = ALIGN(0x100) ;

		_gint_vbr = . ;
		_bgint = ABSOLUTE(.) ;

		/* Exception handler */
		. = _gint_vbr + 0x100 ;
		*(.gint.exc)

		/* TLB miss handler */
		. = _gint_vbr + 0x400 ;
		*(.gint.tlb)

		/* Interrupt handler */
		. = _gint_vbr + 0x600 ;
		*(.gint.int)

		. = ALIGN(4);
		_egint = ABSOLUTE(.) ;
	} > realram

	.gint_bss : AT(_gint_data + SIZEOF(.gint)) ALIGN(4) {
		_bgbss = ABSOLUTE(.) ;
		*(.gint.bss)
		_egbss = ABSOLUTE(.) ;
	} > realram
}
