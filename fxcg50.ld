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
	rom  (rx):	o = 0x00300000, l = 220k
	/* Static RAM; stack grows down from the end of this region.
	   The first 0x2000 bytes are reserved by gint, see below */
	ram  (rw):	o = 0x08102000, l = 512k
	/* gint's VBR space, mentioned here for completeness */
	vbr  (rwx):	o = 0x8c160000, l = 5k
	/* Some RAM region from P1 area; gint's data will reside here */
	rram (rwx):	o = 0x8c161400, l = 3k
}

SECTIONS
{
	/*
	**	ROM sections
	*/

	/* First address to be mapped to ROM */
	_brom = 0x00300000;
	/* Size of ROM mappings */
	_srom = SIZEOF(.text) + SIZEOF(.rodata)
	      + SIZEOF(.gint.drivers) + SIZEOF(.gint.blocks);

	/* Machine code going to ROM:
	   - Initialization sections (.pretext.entry and .pretext)
	   - Compiler-provided constructors (.ctors) and destructors (.dtors)
	   - All text from .text and .text.* (including user code) */
	.text : {
		*(.pretext.entry)
		*(.pretext)

		_btors = . ;
		*(.ctors .ctors.*)
		_mtors = . ;
		*(.dtors .dtors.*)
		_etors = . ;

		*(.text .text.*)
	} > rom

	/* Interrupt handlers going to ROM:
	   - gint's interrupt handler blocks (.gint.blocks)

	   Although gint's blocks end up in VBR space, they are installed at
	   startup by the library/drivers, so we store them here for now */
	.gint.blocks : {
		KEEP(*(.gint.blocks));
	} > rom

	/* Driver data going to ROM:
	   - Exposed driver interfaces (.gint.drivers)

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
		*(.rodata.assets)

		*(.rodata .rodata.*)
	} > rom



	/*
	**	RAM sections
	*/

	. = ORIGIN(ram);

	/* BSS data going to RAM. The BSS section is to be stripped from the
	   ELF file later, and wiped at startup */
	.bss (NOLOAD) : {
		_rbss = . ;

		*(.bss COMMON)

		. = ALIGN(16);
	} > ram :NONE

	_sbss = SIZEOF(.bss);

	/* Read-write data sextions going to RAM (.data and .data.*) */
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



	/*
	**	gint-related sections
	**	8c160000:4k  VBR space
	**	8c161000:4k  .gint.data and .gint.bss
	*/

	/* VBR address: let's just start at the beginning of the RAM area.
	   There's an unused 0x100-byte gap at the start of the VBR space.
	   The VBR space is already a large block (> 2 kiB), so I'm cutting off
	   the gap to spare some memory */
	_gint_vbr = ORIGIN(vbr) - 0x100;

	. = ORIGIN(rram);

	/* gint's data section, going to static RAM. This section contains many
	   small objects from the library (static/global variables, etc) */
	.gint.data ALIGN(4) : ALIGN(4) {
		_lgdata = LOADADDR(.gint.data);
		_rgdata = . ;

		*(.gint.data .gint.data.*)

		. = ALIGN(16);
	} > rram AT> rom

	_sgdata = SIZEOF(.gint.data);

	/* gint's uninitialized BSS section, going to static RAM. All the large
	   data arrays will be located here */
	.gint.bss (NOLOAD) : {
		/* Since it's uninitialized, the location doesn't matter */
		*(.gint.bss .gint.bss.*)

		. = ALIGN(16);
	} > rram :NONE

	_sgbss = SIZEOF(.gint.bss);



	/*
	**	Other sections
	*/

	/* Unwanted sections going to meet Dave Null:
	   - SH3-only data sections
	   - Debug sections, often come out of libgcc
	   - Java classes registration (why are there even here?)
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
