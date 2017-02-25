#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>

#include <internals/gint.h>
#include <internals/mmu.h>
#include <internals/clock.h>
#include <clock.h>
#include <gint.h>

int main(void);

static void init(void);
static void fini(void);

// Symbols provided by the linker script.
extern uint32_t
	etext, btext,		// Location of .text section
	bdata, edata,		// Location of .data section
	bbss, ebss,		// Location of .bss section
	bgint, egint,		// Location of interrut handler and gint data
	gint_vbr,		// VBR address
	romdata;		// ROM address of .data section contents

// This variable should be overwritten before being returned, so the default
// value doesn't matter much.
static int exit_code = EXIT_SUCCESS;
static jmp_buf env;

// Exit handlers.
void (*atexit_handlers[ATEXIT_MAX])(void);
int atexit_index = 0;



/*
	start()
	Program entry point. Loads the data section into the memory and invokes
	main(). Also prepares the execution environment by initializing all the
	modules.
*/
__attribute__((section(".pretext.entry"))) int start(void)
{
	#ifdef	GINT_DIAGNOSTICS

	// Ok, so fill as much information as possible so that in case anything
	// goes wrong we can try to analyze what's been going on.
	volatile gint_diagnostics_t *dg = gint_diagnostics();

	// All that follows is "safe" information that can be saved immediately
	// because it will never change anyway.
	// Basic environment description.
	dg->magic	= GINT_DIAGNOSTICS_MAGIC;
	dg->counter	= dg->counter + 1;
	dg->mpu		= mpu_unknown;
	dg->version	= GINT_VERSION;
	dg->stage	= stage_startup;

	// Exception records: what kind of exceptions / TLB faults / interrupts
	// occured last to get some trace in case of crash.
	dg->excepts	= 0;
	for(size_t i = 0; i < sizeof dg->except_vect; i++)
		dg->except_vect[i] = 0;
	dg->spc		= 0x00000000;
	dg->ssr		= 0x00000000;
	dg->expevt	= 0x00000000;
	dg->tea		= 0x00000000;

	// Memory map: provides information about alignement and the relative
	// size and position of each section, as well as read-only / read-write
	// types of addresses.
	dg->section_text.address	= (uint32_t)&btext;
	dg->section_text.length		= (uint32_t)&etext - (uint32_t)&btext;
	dg->section_data.address	= (uint32_t)&bdata;
	dg->section_data.length		= (uint32_t)&edata - (uint32_t)&bdata;
	dg->section_bss.address		= (uint32_t)&bbss;
	dg->section_bss.length		= (uint32_t)&ebss  - (uint32_t)&bbss;
	dg->romdata	= (uint32_t)&romdata;

	// Basic information about the running library.
	dg->vbr_address			= (uint32_t)&gint_vbr;
	dg->section_gint.address	= (uint32_t)&bgint;
	dg->section_gint.length		= (uint32_t)&egint - (uint32_t)&bgint;
	#endif

	// Clearing the .bss section.
	uint32_t *bss = &bbss;
	while(bss < &ebss) *bss++ = 0;

	// Copying the .data section.
	uint32_t *data = &bdata, *src = &romdata;
	while(data < &edata) *data++ = *src++;

	#ifdef	GINT_DIAGNOSTICS
	dg->stage	= stage_sections;
	#endif

	// Trying to get the system to fill the TLB without editing it
	// directly, so that it does not go on rampage when finding out.
	mmu_pseudoTLBInit();

	#ifdef	GINT_DIAGNOSTICS
	// At this point it would be wise to include a copy of the TLB, but
	// it's already a huge array. Maybe later...
	/* TODO Debug TLB? */
	dg->stage	= stage_mmu;
	#endif

	// Initializing gint.
	gint_init();

	#ifdef	GINT_DIAGNOSTICS
	dg->mpu		= MPU_CURRENT;
	dg->stage	= stage_gint;
	#endif

	// Measuring clock frequencies.
	clock_measure();
	clock_measure_end();

	#ifdef	GINT_DIAGNOSTICS
	clock_config_t clock = clock_config();
	dg->Bphi_f	= clock.Bphi_f / 1000000;
	dg->Iphi_f	= clock.Iphi_f / 1000000;
	dg->Pphi_f	= clock.Pphi_f / 1000000;
	dg->stage	= stage_clock;
	#endif

	// Calling global constructors.
	init();

	#ifdef	GINT_DIAGNOSTICS
	dg->stage	= stage_ctors;
	dg->stage	= stage_running;
	#endif



	// Saving the execution state there.
	int x = setjmp(env);
	// If the program has just started, executing main(). Otherwise, the
	// exit code has already been set by abort() or similar.
	if(!x) exit_code = main();


	#ifdef	GINT_DIAGNOSTICS
	dg->stage	= stage_leaving;
	#endif

	/* TODO Flush and close opened streams. */

	// Calling exit handlers.
	while(atexit_index > 0) (*atexit_handlers[--atexit_index])();

	// Calling the destructors.
	fini();

	#ifdef	GINT_DIAGNOSTICS
	dg->stage	= stage_dtors;
	#endif

	// Un-initializing gint.
	gint_quit();

	#ifdef	GINT_DIAGNOSTICS
	dg->stage	= stage_terminated;
	#endif

	return exit_code;
}

/*
	init()
	Calls the constructors.
*/
static void init(void)
{
	extern void
		(*bctors)(void),
		(*ectors)(void);
	void (**func)(void) = &bctors;

	while(func < &ectors)
	{
		(*(*func))();
		func++;
	}
}

/*
	fini()
	Calls the destructors.
*/
static void fini(void)
{
	extern void
		(*bdtors)(void),
		(*edtors)(void);
	void (**func)(void) = &bdtors;

	while(func < &edtors)
	{
		(*(*func))();
		func++;
	}
}



/*
	abort()
	Immediately ends the program without invoking the exit handlers.
*/
void abort(void)
{
	exit_code = EXIT_FAILURE;

	// Avoiding any exit handler call.
	atexit_index = 0;

	longjmp(env, 1);
}

/*
	exit()
	Ends the program and returns the given exit code status. Calls exit
	handlers before returning.
	Usually exit() would ask the operating system to stop the process but
	the fx-9860G executes only one program at a time and calls it as a
	function. Reaching the program end point is therefore an efficient way
	of achieving this goal while minimizing interaction with the operating
	system.
*/
void exit(int status)
{
	exit_code = status;
	longjmp(env, 1);
}

/*
	atexit()
	Registers a function to be called at normal program termination.
*/
int atexit(void (*function)(void))
{
	if(atexit_index >= ATEXIT_MAX) return 1;

	atexit_handlers[atexit_index++] = function;
	return 0;
}

void crt0_system_menu(void)
{
	fini();
	gint_quit();
	__system_menu();
	mmu_pseudoTLBInit();
	gint_init();
	init();
}
