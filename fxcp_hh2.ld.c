/*
	Linker script for fxCP add-ins linking with HollyHock2.
*/

OUTPUT_ARCH(sh4)
OUTPUT_FORMAT(elf32-sh)
ENTRY(_start_header)

MEMORY
{
	/* Everything mixed together loaded as one */
	ram (rwx): o = 0x8cff0000, l = 61184 /* 64k - 0x1100 */
	/* Space for the vbr */
	vbr (rwx): o = 0x8cffef00, l = 0x1100
	/* On-chip IL memory */
	ilram (rwx):  o = 0xe5200000, l = 4k
	/* On-chip X and Y memory */
	xyram (rwx):  o = 0xe500e000, l = 16k
}

SECTIONS
{
	/*
	**  ROM sections
	*/

	/* First address to be mapped to ROM */
	_brom = ORIGIN(ram);
	/* Size of ROM mappings */
	_srom = SIZEOF(.text) + SIZEOF(.rodata)
	      + SIZEOF(.gint.drivers) + SIZEOF(.gint.blocks);

	/* Machine code going to ROM:
	   - Entry function (.text.entry)
	   - Compiler-provided constructors (.ctors) and destructors (.dtors)
	   - All text from .text and .text.* (including user code) */
	.text : {
		*(.text.header)
		KEEP(*(.hh2info))
		*(.text.entry)

		_bctors = . ;
		*(.ctors .ctors.*)
		_ectors = . ;

		_bdtors = . ;
		*(.dtors .dtors.*)
		_edtors = . ;

		_gint_exch_start = . ;
		*(.gint.exch)
		_gint_exch_size = ABSOLUTE(. - _gint_exch_start);

		_gint_tlbh_start = . ;
		*(.gint.tlbh)
		_gint_tlbh_size = ABSOLUTE(. - _gint_tlbh_start);

		*(.text .text.*)
	} > ram

	/* gint's interrupt handler blocks (.gint.blocks)
	   Although gint's blocks end up in VBR space, they are relocated at
	   startup by the library/drivers, so we store them here for now */
	.gint.blocks : {
		KEEP(*(.gint.blocks));
	} > ram

	/* Exposed driver interfaces (.gint.drivers)
	   The driver information is required to start and configure the
	   driver, even if the symbols are not referenced */
	.gint.drivers : {
		_gint_drivers = . ;
		KEEP(*(SORT_BY_NAME(.gint.drivers.*)));
		_gint_drivers_end = . ;
	} > ram

	/* Read-only data going to ROM:
	   - Resources or assets from fxconv or similar converters
	   - Data marked read-only by the compiler (.rodata and .rodata.*) */
	.rodata : SUBALIGN(4) {
		/* Put these first, they need to be 4-aligned */
		*(.rodata.4)

		*(.rodata .rodata.*)
	} > ram



	/*
	**  RAM sections
	*/

	. = ALIGN(16); /* No effect? */

	/* Read-write data sections going to RAM (.data and .data.*) */
	.data ALIGN(4) : ALIGN(4) {
		_ldata = LOADADDR(.data);
		_rdata = . ;

		*(.data .data.*)
		/* Code that must remain mapped; no MMU on HH2, so fine */
		*(.gint.mapped)
		/* References to mapped code - no relocation needed */
		*(.gint.mappedrel)

		. = ALIGN(16);
	} > ram

	/* Read-write data sub-aligned to 4 bytes (mainly from fxconv) */
	.data.4 : SUBALIGN(4) {
		*(.data.4)
		. = ALIGN(16);
	} > ram

	_sdata = SIZEOF(.data) + SIZEOF(.data.4);

	/* On-chip memory sections: IL, X and Y memory */

	. = ORIGIN(ilram);
	.ilram ALIGN(4) : ALIGN(4) {
		_lilram = LOADADDR(.ilram);
		_rilram = . ;

		*(.ilram)

		. = ALIGN(16);
	} > ilram AT> ram

	. = ORIGIN(xyram);
	.xyram ALIGN(4) : ALIGN(4) {
		_lxyram = LOADADDR(.xyram);
		_rxyram = . ;

		*(.xram .yram .xyram)

		. = ALIGN(16);
	} > xyram AT> ram

	_silram = SIZEOF(.ilram);
	_sxyram = SIZEOF(.xyram);

	_lgmapped = ABSOLUTE(0);
	_sgmapped = ABSOLUTE(0);
	_lreloc = ABSOLUTE(0);
	_sreloc = ABSOLUTE(0);
	_gint_region_vbr = ORIGIN(vbr);

	/* BSS data going to RAM. The BSS section is to be stripped from the
	   ELF file later, and wiped at startup */
	.bss (NOLOAD) : {
		_rbss = . ;

		*(.bss .bss.* COMMON)

		. = ALIGN(16);
	} > ram :NONE

	_sbss = SIZEOF(.bss);

	/* gint's uninitialized BSS section, going to static RAM. All the large
	   data arrays will be located here */
	.gint.bss (NOLOAD) : {
		*(.gint.bss)
		. = ALIGN(16);

		/* End of user RAM */
		_euram = . ;
	} > ram :NONE

	_sgbss = SIZEOF(.gint.bss);



	/*
	**  Unused sections
	*/

	/DISCARD/ : {
		/* SH3-only data sections */
		*(.gint.rodata.sh3 .gint.data.sh3 .gint.bss.sh3)
		/* Java class registration (why are they even here?!) */
		*(.jcr)
		/* Asynchronous unwind tables: no C++ exception handling */
		*(.eh_frame_hdr)
		*(.eh_frame)
		/* Comments or anything the compiler might generate */
		*(.comment)
	}
}
