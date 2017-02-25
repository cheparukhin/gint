#ifndef _INTERNALS_GINT_H
#define	_INTERNALS_GINT_H	1

#include <stdint.h>
#include <gint.h>

//---
//	Interrupt handlers
//---

// General exception handler.
void gint_exc(void);
// TLB miss handler.
void gint_tlb(void);
// Interrupt handler.
void gint_int(void);



//---
//	Assembler-level VBR management
//---

/*
	gint_getvbr()
	Retrieves the current VBR address.
*/
uint32_t gint_getvbr(void);

/*
	gint_setvbr()
	Sets the VBR address and calls the configuration function while
	interrupts are disabled.
*/
void gint_setvbr(uint32_t vbr, void (*setup)(void));



//---
//	Initialization and termination routines
//---

/*
	gint_init()
	Initializes gint. Loads the interrupt handler into the memory and sets
	the new vbr address.
*/
void gint_init(void);

/*
	gint_quit()
	Stops gint. Restores the system's configuration and vbr address.
*/
void gint_quit(void);

/*
	gint_save()
	Saves many registers into a buffer to ensure that the system is not
	upset by gint's configuration when the application ends.
*/
//void gint_save_7705(gint_save_buffer_t *buffer);
//void gint_save_7305(gint_save_buffer_t *buffer);

/*
	gint_setup()
	Configures interrupt priorities and some parameters to allow gint to
	take control of the interrupt flow.
*/
void gint_setup_7705(void);
void gint_setup_7305(void);

/*
	gint_restore()
	Restores the parameters saved in a save buffer to give back the
	interrupt control to the system.
*/
//void gint_restore_7705(gint_save_buffer_t *buffer);
//void gint_restore_7305(gint_save_buffer_t *buffer);

/*
	gint_reg()
	Returns the address of a platform-shared register.
*/
volatile void *gint_reg_7705(gint_register_t reg);
volatile void *gint_reg_7305(gint_register_t reg);



//---
//	Diagnostics
//	When diagnostics are enabled, gint saves runtime information to a
//	buffer in RAM, which by chance is held if the application crashes (I
//	don't really know why). This allows deeper debugging.
//---

#ifdef	GINT_DIAGNOSTICS

#include <mpu.h>

// Determining whether the SaveDisp() buffer actually contains gint diagnostics
// is performed by checking a magic number (1/256 chance of failure) and the
// validity of all fields in the diagnostic information. Formally, a picture
// taken by SaveDisp() could fool the checks but this is *very* unlikely and
// the diagnostics should only be read just after gint crashes or stops anyway.
#define	GINT_DIAGNOSTICS_MAGIC	0xb7

typedef enum
{
	stage_startup		= 0,
	stage_sections		= 1,
	stage_mmu		= 2,
	stage_gint		= 3,
	stage_clock		= 4,
	stage_ctors		= 5,
	stage_running		= 6,
	stage_leaving		= 7,
	stage_dtors		= 8,
	stage_terminated	= 9,

} gint_stage_t;

typedef struct
{
	uint32_t address;
	uint32_t length;

} gint_memsection_t;

typedef struct
{
	// Magic number to check whether there is a diagnostic.
	uint8_t magic		:8;
	// Unique counter that is incremented at each execution, allowing to
	// distinguish diagnostics output at different times if the application
	// crashes repeatedly.
	uint8_t counter		:8;
	// How many work of initialization had been successfully done before
	// the application crashed.
	gint_stage_t stage	:8;
	// What kind of MPU the library detected (undefined if the
	// initialization stage does not reach stage_gint).
	mpu_t mpu		:8;

	// Frequency of the main clocks, in MHz.
	uint8_t Bphi_f		:8;
	uint8_t Iphi_f		:8;
	uint8_t Pphi_f		:8;

	// What kind of exceptions occurred last.
	uint8_t excepts		:8;
	uint8_t except_vect[12];
	// Last values held by registers SPC, SSR, EXPEVT / INTEVT2 / INTEVT,
	// and TEA.
	uint32_t spc;
	uint32_t ssr;
	uint32_t expevt;
	uint32_t tea;

	// Gint version number, on the form 0xMMmmbbbb, where MM is the major
	// version, mm the minor version and bbbb the build number.
	uint32_t version;
	// Location of the VBR at the time of execution.
	uint32_t vbr_address;

	// Memory map.
	uint32_t romdata;
	gint_memsection_t	section_text;
	gint_memsection_t	section_data;
	gint_memsection_t	section_bss;
	gint_memsection_t	section_gint;

} gint_diagnostics_t;

// This is somewhere inside the buffers of SaveDisp(), 3 bytes inside the first
// buffer to be exact (that's to be 4-aligned).
// This buffer is 1024-byte long so the logs must fit in 1021 bytes to be safe.
// It looks like that this RAM area is generally not cleared when the 
// calculator reboots after a crash, even though it does not seem to work with
// manual resets. Maybe it can provide useful information in some cases.
#define	gint_diagnostics() ((volatile gint_diagnostics_t *)0x88004d90)

#endif	// GINT_DIAGNOSTICS

#endif	// _INTERNALS_GINT_H
