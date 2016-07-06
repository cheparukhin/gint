#include <stdlib.h>
#include <gint.h>
#include <string.h>
#include <setjmp.h>

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

	// Setting up the TLB.
	__Hmem_SetMMU(0x08102000, 0x8801e000, 108);

	// Clearing the .bss section.
	while(bss < &ebss) *bss++ = 0;
	// Copying the .data section.
	while(data < &edata) *data++ = *src++;

	__GLibAddinAplExecutionCheck(0, 1, 1);

	// Initializing everything.
	init();
	gint_init();



	// Saving the execution state there.
	x = setjmp(env);
	// If the program has just started, executing main(). Otherwise, the
	// exit code has already been set by abort() or similar.
	if(!x) exit_code = main();



	// Remember to flush and close opened streams.

	// Un-initializing everything.
	gint_quit();
	fini();

	return exit_code;
}

static void init(void)
{
	extern void
		(*bctors)(void),
		(*ectors)(void);
	void (**func)(void) = &bctors;

	// Calling the constructors.
	while(func < &ectors)
	{
		(*(*func))();
		func++;
	}
}

static void fini(void)
{
	extern void
		(*bdtors)(void),
		(*edtors)(void);
	void (**func)(void) = &bdtors;

	// Calling the destructors.
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

