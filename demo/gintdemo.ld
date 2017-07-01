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
	rom	: o = 0x00300200, l = 512k
	/* 0x0810000 is apparently mapped to 0x8801c0000. */
	ram	: o = 0x08100000, l = 8k
	realram	: o = 0x8800d000, l = 12k
}

SECTIONS
{
	/*
		ROM sections : binary code and read-only data.
	*/

	.text : {
		_btext = . ;

		/* Initialization code. */
		*(.pretext.entry)
		*(.pretext)

		_bctors = . ;
		*(.ctors)
		_ectors = . ;
		_bdtors = . ;
		*(.dtors)
		_edtors = . ;

		*(.text)
		*(.text.*)

		_etext = . ;
	} > rom

	.rodata : {
		*(.rodata.fxconv);
		*(.rodata.paradox);
		*(.rodata)
		. = ALIGN(4);
		*(.rodata.*)

		_romdata = ALIGN(4) ;
	} > rom



	/*
		RAM sections : bss section and read/write data.
		The BSS section is meant to be stripped from the ELF file (to
		reduce the binary size) and initialized with zeros in the
		initialization routine, therefore its location is undefined.
	*/

	.bss : {
		_bbss = . ;
		*(.bss)
		_ebss = . ;
	} > ram

	.data : AT(_romdata) ALIGN(4) {
		_bdata = . ;
		*(.data)
		*(.data.*)
		_edata = . ;
	} > ram

	.cc : AT(_romdata + SIZEOF(.data)) ALIGN(4) {
		*(.eh_frame)
		*(.jcr)

		. = ALIGN(4);
		_gint_data = _romdata + SIZEOF(.data) + SIZEOF(.cc) ;
	} > ram



	/*
		Real RAM : interrupt, exception and TLB miss handlers.
	*/

	.gint : AT(_gint_data) ALIGN(4) {
		/* The vbr needs to be 0x100-aligned because of an ld issue. */
		. = ALIGN(0x100) ;
		_gint_vbr = . ;
		_bgint = . ;

		/* Exception handler. */
		. = _gint_vbr + 0x100 ;
		*(.gint.exc)

		/* TLB miss handler. */
		. = _gint_vbr + 0x400 ;
		*(.gint.tlb)

		/* Interrupt handler. */
		. = _gint_vbr + 0x600 ;
		*(.gint.int)

		_egint = . ;
	} > realram
}
