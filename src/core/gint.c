//---
//
//	gint core module: interrupt handler
//
//	Central point of the library. Controls the interrupt handler and
//	defines a few functions to configure callbacks for some interrupts.
//
//---

#include <gint.h>
#include <mpu.h>
#include <stddef.h>

static unsigned int
	new_vbr,
	sys_vbr;



//---
//	Local functions.
//---

/*
	gint_setup()
	Configures interrupt priorities and some parameters to allow gint to
	take control of the interrupt flow.
*/
static void gint_setup(void)
{
	if(isSH3())
		gint_setup_7705();
	else
		gint_setup_7305();
}

/*
	gint_stop()
	Un-configures the interrupt flow to give back the interrupt control to
	the system.
*/
static void gint_stop(void)
{
	if(isSH3())
		gint_stop_7705();
	else
		gint_stop_7305();
}



//---
//	Public API.
//---

/*
	gint_systemVBR()
	Returns the vbr address used by the system (saved when execution
	starts).
*/
inline unsigned int gint_systemVBR(void)
{
	return sys_vbr;
}

/*
	gint()
	Handles interrupts.
*/
void gint(void)
{
	if(isSH3())
		gint_7705();
	else
		gint_7305();
}

/*
	gint_init()
	Initializes gint. Loads the interrupt handler into the memory and sets
	the new vbr address.
*/
void gint_init(void)
{
	// Linker script symbols -- gint.
	extern unsigned int
		gint_vbr,
		gint_data,
		bgint, egint;

	unsigned int *ptr = &bgint;
	unsigned int *src = &gint_data;

	// This initialization routine is usually called before any
	// constructor. We want to ensure that the MPU type is detected, but
	// mpu_init() hasn't been called yet.
	mpu_init();

	// Loading the interrupt handler into the memory.
	while(ptr < &egint) *ptr++ = *src++;

	sys_vbr = gint_getVBR();
	new_vbr = (unsigned int)&gint_vbr;

	gint_setVBR(new_vbr, gint_setup);
}

/*
	gint_quit()
	Stops gint. Restores the system's configuration and vbr address.
*/
void gint_quit(void)
{
	gint_setVBR(sys_vbr, gint_stop);
}
