#include <stdlib.h>
#include <setjmp.h>

#include <internals/gint.h>
#include <internals/mmu.h>
#include <internals/clock.h>

#include <display.h>
#include <internals/init.h>
#include <internals/modules.h>

/* We need some more functionality to generate these */
#ifdef	GINT_STARTUP_LOG
  #include <gint.h>
  #include <clock.h>
  #include <events.h>
  #include <internals/keyboard.h>
  #include <modules/interrupts.h>

  #ifndef GINT_NO_SYSCALLS
    #include <internals/syscalls.h>
  #endif
#endif

int main(void);

static void init(void);
static void fini(void);

// Symbols provided by the linker script.
extern uint32_t
	bdata, sdata,		// Location of .data section
	bbss,  sbss,		// Location of .bss section
	bgint, egint,		// Location of interrupt handler and gint data
	bgbss, egbss,		// Location of interrupt handler and gint data
	gint_vbr,		// VBR address
	romdata,		// ROM address of .data section contents
	gint_data;		// ROM address of .gint section contents
extern void
	(*bctors)(void), (*ectors)(void),	// Constructors
	(*bdtors)(void), (*edtors)(void);	// Destructors

// This variable should be overwritten before being returned, so the default
// value doesn't matter much.
static int exit_code = EXIT_SUCCESS;
static jmp_buf env;

// Exit handlers.
static void (*atexit_handlers[ATEXIT_MAX])(void);
static int atexit_index = 0;

#include <internals/display.h>

/*
	start()
	Program entry point. Loads the data section into the memory and invokes
	main(). Also prepares the execution environment by initializing all the
	modules.
*/
__attribute__((section(".pretext.entry"))) int start(void)
{
	/* Configure the display output or we won't get anywhere - this is also
	   needed for the add-in even if the startup logs are disabled */
	display_useVRAM(display_getLocalVRAM());
	text_configure(NULL, color_black);

#ifdef GINT_STARTUP_LOG
	/* Start outputting information on the screen so that we can quickly
	   debug anything that would fail */
	uint32_t rom_size  = ((uint32_t)&romdata - 0x00300000) >> 10;
	uint32_t ram_size  = ((uint32_t)&gint_data - (uint32_t)&romdata)
			   + ((uint32_t)&sbss);
	uint32_t rram_size = ((uint32_t)&egbss - (uint32_t)&gint_vbr);

	dclear();
	init_stage("Startup");
	dupdate();

	print(1, 1, init_version());
	print(1, 2, "ROM  RAM  RRAM");
	print(4, 3, "k           c  d");
	print_dec(1, 3, rom_size, 3);
	print_dec(6, 3, ram_size, 4);
	print_dec(11, 3, rram_size, 4);
	print_dec(17, 3, &ectors - &bctors, 2);
	print_dec(20, 3, &edtors - &bdtors, 2);
	dupdate();
#endif

	/* Determining the processor type */
	mpu_t mpu = getMPU();

#ifdef GINT_STARTUP_LOG
	const char *mpu_names[] = { "SH7337", "SH7355", "SH7305", "SH7724" };
	if(mpu >= 1 && mpu <= 4) print(16, 2, mpu_names[mpu - 1]);
	else print_dec(16, 2, mpu, 6);
	dupdate();
#endif

	/* Make sure the MPU is of a valid type */
	if(!mpu || mpu > 4) init_halt();

#ifdef GINT_STARTUP_LOG
	init_stage("    MMU");
	dupdate();
#endif

	/* Try to get the TLB filled by the system by accessing all the pages
	   while the system still answers TLB misses */
	mmu_pseudoTLBInit();

#ifdef GINT_STARTUP_LOG
	/* Read the TLB and count how much memory has been mapped */
	/* TODO: Debug TLB at startup */
	print(1, 4, "MMU ROM:???k RAM:???k");

	init_stage("Section");
	dupdate();
#endif

	/* Copying the data section, then clearing the bss section. This
	   doesn't affect the previously used variables, which are stored in
	   another section for this purpose */
	volatile uint32_t *data = &bdata;
	volatile uint32_t *data_end = (void *)&bdata + (int)&sdata;
	volatile uint32_t *src = &romdata;
	while(data < data_end) *data++ = *src++;
	volatile uint32_t *bss = &bbss;
	volatile uint32_t *bss_end = (void *)&bbss + (int)&sbss;
	while(bss < bss_end) *bss++ = 0;

#ifdef GINT_STARTUP_LOG
	init_stage("Handler");
	dupdate();
#endif

	/* Initialize gint and debug all the interrupt priorities */
	gint_init(mpu);

#ifdef GINT_STARTUP_LOG
	if(isSH3())
	{
		print(1, 5, "ABCD");
		print_hex( 6, 5, INTC._7705.iprs.IPRA->word, 4);
		print_hex(10, 5, INTC._7705.iprs.IPRB->word, 4);
		print_hex(14, 5, INTC._7705.iprs.IPRC->word, 4);
		print_hex(18, 5, INTC._7705.iprs.IPRD->word, 4);
		print(1, 6, "EFGH");
		print_hex( 6, 6, INTC._7705.iprs.IPRE->word, 4);
		print_hex(10, 6, INTC._7705.iprs.IPRF->word, 4);
		print_hex(14, 6, INTC._7705.iprs.IPRG->word, 4);
		print_hex(18, 6, INTC._7705.iprs.IPRH->word, 4);
	}
	else
	{
		print(1, 5, "ACFG");
		print_hex( 6, 5, INTC._7305.iprs->IPRA.word, 4);
		print_hex(10, 5, INTC._7305.iprs->IPRC.word, 4);
		print_hex(14, 5, INTC._7305.iprs->IPRF.word, 4);
		print_hex(18, 5, INTC._7305.iprs->IPRG.word, 4);
		print(1, 6, "HJKL");
		print_hex( 6, 6, INTC._7305.iprs->IPRH.word, 4);
		print_hex(10, 6, INTC._7305.iprs->IPRJ.word, 4);
		print_hex(14, 6, INTC._7305.iprs->IPRK.word, 4);
		print_hex(18, 6, INTC._7305.iprs->IPRL.word, 4);
	}
#endif

	/* Checking that at least the required interrupts are enabled */
	if(isSH3()
		? (!INTC._7705.iprs.IPRA->word)
		: (!INTC._7305.iprs->IPRA.word || !INTC._7305.iprs->IPRK.word)
	) init_halt();

#ifdef GINT_STARTUP_LOG
	init_stage(" Clocks");
	dupdate();
#endif

	/* Measuring the clock speed */
	clock_measure();
	clock_measure_end();

#ifdef GINT_STARTUP_LOG
	clock_config_t clock = clock_config();
	print(1, 7, "Clock I   B   P");
	print_dec( 8, 7, clock.Iphi_f / 1000000, 2);
	print_dec(12, 7, clock.Bphi_f / 1000000, 2);
	print_dec(16, 7, clock.Pphi_f / 1000000, 2);

	init_stage("Boot OK");
	dupdate();

	/* Displaying some interrupt controller config */
	if(isSH3())
	{
		print(1, 8, "INTC:0");
		print_hex(7, 8, INTC._7705.ICR1->word, 4);
	}
	else
	{
		print(1, 8, "INTC:");
		print_hex(6, 8, INTC._7305.ICR0->word, 4);
		print_hex(10, 8, (INTC._7305.USERIMASK->lword >> 4) & 0xf, 1);
	}

	#ifndef GINT_NO_SYSCALLS
	/* Print the OS version */
	char version[11];
	__get_os_version(version);
	version[2] = version[3];
	version[3] = version[4];
	version[4] = version[6];
	version[5] = version[7];
	version[6] = version[8];
	version[7] = version[9];
	version[8] = 0;
	print(12, 8, "OS");
	print(14, 8, version);
	#endif

	dupdate();
#endif

	/* Call the global constructors */
	init();

#ifdef GINT_STARTUP_LOG
	/* Keep this visible for a second or so */
	keyboard_interrupt();
	if(pollevent().type != event_none) getkey();
#endif

	/* Otherwise, call main() and get the show on the road! */
	int x = setjmp(env);
	if(!x) exit_code = main();

	while(atexit_index > 0) (*atexit_handlers[--atexit_index])();
	fini();

	gint_quit();

	return exit_code;
}

/* init() -- call the constructors */
static void init(void)
{
	extern void
		(*bctors)(void),
		(*ectors)(void);
	void (**func)(void) = &bctors;

	while(func < &ectors) (*(*func++))();
}

/* fini() -- call the destructors */
static void fini(void)
{
	extern void
		(*bdtors)(void),
		(*edtors)(void);
	void (**func)(void) = &bdtors;

	while(func < &edtors) (*(*func++))();
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
	function. Reaching the program end point thus efficiently exits.
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
