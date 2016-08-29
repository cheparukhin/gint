#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

#include <gint.h>
#include <clock.h>
#include <internals/mmu.h>

void	__Hmem_SetMMU(unsigned int, unsigned int, int);
void	__GLibAddinAplExecutionCheck(int, int, int);
int	main(void);

static void init(void);
static void fini(void);

// Symbols imported from the linker script.
extern unsigned int
	romdata,
	bbss, ebss,
	bdata, edata;

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

int start(void)
	__attribute__((
		section(".pretext.entry")
	));

int start(void)
{
	// Linker symbols.
	unsigned int *bss = &bbss;
	unsigned int *data = &bdata, *src = &romdata;
	int x;

	// Clearing the .bss section.
	while(bss < &ebss) *bss++ = 0;
	// Copying the .data section.
	while(data < &edata) *data++ = *src++;

	__GLibAddinAplExecutionCheck(0, 1, 1);

	mmu_pseudoTLBInit();

	// Initializing gint.
	gint_init();

	// Measure clock frequencies.
	clock_measure();
	clock_measure_end();

	// Calling global constructors.
	init();



	// Saving the execution state there.
	x = setjmp(env);
	// If the program has just started, executing main(). Otherwise, the
	// exit code has already been set by abort() or similar.
	if(!x) exit_code = main();



	// Remember to flush and close opened streams.

	// Calling exit handlers.
	while(atexit_index > 0) (*atexit_handlers[--atexit_index])();

	// Un-initializing everything.
	fini();
	gint_quit();

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

