/*
	This linker script links the object files when generating the ELF
	output. Note how symbols romdata, bbss, ebss, bdata and edata are used
	in the initialization routine (crt0.c) to initialize the application.

	Two ram areas are specified. It happens, if I'm not wrong, that the
	"real ram" is accessed directly while the "common" ram is accessed
	through the mmu. The interrupt handler resides in "real ram" because it
	couldn't execute well in ram. While SH7335 and SH7355 had no problems,
	executing the interrupt handler in the common ram on SH7305-based new
	models caused trouble to the OS, apparently overwriting ram data.
*/

OUTPUT_ARCH(sh3)
ENTRY(_start)

MEMORY
{
	rom	: o = 0x00300200, l = 512k
	ram	: o = 0x08100000, l = 64k
	/* The "real ram" accessible length remains unknown because some parts
	are used by the system. At least 12k seem accessible. Use with care. */
	realram	: o = 0x8800d000, l = 12k
}

SECTIONS
{
	/*
		ROM sections : binary code and read-only data.
	*/

	.text : {
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
	} > rom

	.rodata : {
		*(.rodata)
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

		_gint_data = _romdata + SIZEOF(.data) + SIZEOF(.cc) ;
	} > ram



	/*
		Real RAM : interrupt handler.
	*/

	.gint_int : AT(_gint_data) ALIGN(4) {
		/* The vbr needs to be 0x100-aligned because of an ld issue. */
		. = ALIGN(0x100) ;
		_gint_vbr = . ;

		_bgint = . ;

		/* Interrupt handler. */
		. = _gint_vbr + 0x600 ;
		*(.gint.int.entry)
		*(.gint.int)

		_egint = . ;
	} > realram
}
