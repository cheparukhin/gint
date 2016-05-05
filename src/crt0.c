// Used by the exit()-family functions to save and restore the execution state.
#include <setjmp.h>
static jmp_buf env;
// Provides EXIT_SUCCESS and EXIT_FAILURE.
#include <stdlib.h>
// Provides gint initialization functionalities.
#include <gint.h>
// Provides memset() and memcpy().
#include <string.h>

// Some syscall prototypes.
void	__Hmem_SetMMU(unsigned int, unsigned int, int);
void	__GLibAddinAplExecutionCheck(int, int, int);
int	main(void);

// Local functions.
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



/*
	start()

	Program entry point. Loads the data section into the memory and invokes
	main(). Also prepares the execution environment by initializing all the
	modules.

	@return		Execution status returned to the OS.
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

	Ends the program and returns the given exit code status.
	Calls exit handlers before returning.

	@arg	status	Exit status.
*/

void exit(int status)
{
	exit_code = status;
	longjmp(env, 1);
}

