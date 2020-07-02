/*
	Linker script for fxcg50 add-ins. Most symbols are used in the startup
	routine in core/start.c; some others in core/setup.c.
*/

/* All fxcg50 have SH4 processors (finally rid of compatibility issues) */
OUTPUT_ARCH(sh4)
/* ELF offers a lot of symbol/section/relocation insights */
OUTPUT_FORMAT(elf32-sh)
/* Located in core/start.c */
ENTRY(_start)

MEMORY
{
	/* Userspace mapping of the add-in (without G3A header) */
	rom   (rx):   o = 0x00300000, l = 2M
	/* Static RAM; stack grows down from the end of this region.
	   The first 0x2000 bytes are reserved by gint, see below */
	ram   (rw):   o = 0x08101400, l = 507k
	/* gint's VBR space at the start of the user stack on fx-CG 50 and on
	   fx-CG 20. This address needs not be determined now because VBR code
	   is position-independent and stored in ROM so the linker doesn't even
	   need to know the value */
	vbr50 (rwx):  o = 0x8c160000, l = 5k
	vbr20 (rwx):  o = 0x88160000, l = 5k
	/* On-chip IL memory */
	ilram (rwx):  o = 0xe5200000, l = 4k
	/* On-chip X and Y memory */
	xram  (rwx):  o = 0xe5007000, l = 8k
	yram  (rwx):  o = 0xe5017000, l = 8k
}

SECTIONS
{
	/*
	**  ROM sections
	*/

	/* First address to be mapped to ROM */
	_brom = 0x00300000;
	/* Size of ROM mappings */
	_srom = SIZEOF(.text) + SIZEOF(.rodata)
	      + SIZEOF(.gint.drivers) + SIZEOF(.gint.blocks);

	/* Machine code going to ROM:
	   - Entry function (.text.entry)
	   - Compiler-provided constructors (.ctors) and destructors (.dtors)
	   - All text from .text and .text.* (including user code) */
	.text : {
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
	} > rom

	/* gint's interrupt handler blocks (.gint.blocks)
	   Although gint's blocks end up in VBR space, they are relocated at
	   startup by the library/drivers, so we store them here for now */
	.gint.blocks : {
		KEEP(*(.gint.blocks));
	} > rom

	/* Exposed driver interfaces (.gint.drivers)
	   The driver information is required to start and configure the
	   driver, even if the symbols are not referenced */
	.gint.drivers : {
		_bdrv = . ;
		KEEP(*(.gint.drivers.0));
		KEEP(*(.gint.drivers.1));
		KEEP(*(.gint.drivers.2));
		KEEP(*(.gint.drivers.3));
		KEEP(*(.gint.drivers.4));
		KEEP(*(.gint.drivers.5));
		KEEP(*(.gint.drivers.6));
		_edrv = . ;
	} > rom

	/* Read-only data going to ROM:
	   - Resources or assets from fxconv or similar converters
	   - Data marked read-only by the compiler (.rodata and .rodata.*) */
	.rodata : SUBALIGN(4) {
		/* Put these first, they need to be 4-aligned */
		*(.rodata.4)

		*(.rodata .rodata.*)
	} > rom



	/*
	**  RAM sections
	*/

	. = ORIGIN(ram);

	/* BSS data going to RAM. The BSS section is to be stripped from the
	   ELF file later, and wiped at startup */
	.bss (NOLOAD) : {
		_rbss = . ;

		*(.bss.vram)
		*(.bss COMMON)

		. = ALIGN(16);
	} > ram :NONE

	_sbss = SIZEOF(.bss);

	/* Read-write data sections going to RAM (.data and .data.*) */
	.data ALIGN(4) : ALIGN(4) {
		_ldata = LOADADDR(.data);
		_rdata = . ;

		*(.data .data.*)

		. = ALIGN(16);
	} > ram AT> rom

	/* Read-write data sub-aligned to 4 bytes (mainly from fxconv) */
	.data.4 : SUBALIGN(4) {
		*(.data.4)
		. = ALIGN(16);
	} > ram AT> rom

	_sdata = SIZEOF(.data) + SIZEOF(.data.4);

	/* On-chip memory sections: IL, X and Y memory */

	. = ORIGIN(ilram);
	.ilram ALIGN(4) : ALIGN(4) {
		_lilram = LOADADDR(.ilram);
		_rilram = . ;

		*(.ilram)
		/* Code that must remain mapped is placed here */
		*(.gint.mapped)

		. = ALIGN(16);
	} > ilram AT> rom

	. = ORIGIN(xram);
	.xram ALIGN(4) : ALIGN(4) {
		_lxram = LOADADDR(.xram);
		_rxram = . ;

		*(.xram)

		. = ALIGN(16);
	} > xram AT> rom

	. = ORIGIN(yram);
	.yram ALIGN(4) : ALIGN(4) {
		_lyram = LOADADDR(.yram);
		_ryram = . ;

		*(.yram)

		. = ALIGN(16);
	} > yram AT> rom

	_silram = SIZEOF(.ilram);
	_sxram  = SIZEOF(.xram);
	_syram  = SIZEOF(.yram);



	/*
	**  gint-related sections (VBR space, .gint.data and .gint.bss)
	*/

	/* VBR address: let's just start at the beginning of the RAM area.
	   There's an unused 0x100-byte gap at the start of the VBR space.
	   The VBR space is already a large block (> 2 kiB), so I'm cutting off
	   the gap to spare some memory */
	_gint_vbr_fxcg50 = ORIGIN(vbr50) - 0x100;
	_gint_vbr_fxcg20 = ORIGIN(vbr20) - 0x100;

	. = ORIGIN(ram) + _sbss + _sdata;

	/* gint's data section, going to static RAM. This section contains many
	   small objects from the library (static/global variables, etc) */
	.gint.data ALIGN(4) : ALIGN(4) {
		_lgdata = LOADADDR(.gint.data);
		_rgdata = . ;

		*(.gint.data .gint.data.*)

		. = ALIGN(16);
	} > ram AT> rom

	_sgdata = SIZEOF(.gint.data);

	/* gint's uninitialized BSS section, going to static RAM. All the large
	   data arrays will be located here */
	.gint.bss (NOLOAD) : {
		/* Since it's uninitialized, the location doesn't matter */
		*(.gint.bss .gint.bss.*)

		. = ALIGN(16);
	} > ram :NONE

	_sgbss = SIZEOF(.gint.bss);



	/*
	**  Other sections
	*/

	/* Unwanted sections going to meet Dave Null:
	   - SH3-only data sections
	   - Debug sections, often come out of libgcc
	   - Java classes registration (why is there any of this here?)
	   - Asynchronous unwind tables: no C++ exception handling for now ^^
	   - Comments or anything the compiler might put in its assembler */
	/DISCARD/ : {
		*(.gint.bss.sh3)
		*(.gint.data.sh3)

		*(.debug_info .debug_abbrev .debug_loc .debug_aranges
		  .debug_ranges .debug_line .debug_str)
		*(.jcr)
		*(.eh_frame_hdr)
		*(.eh_frame)
		*(.comment)
	}
}
