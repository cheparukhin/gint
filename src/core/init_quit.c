#include <internals/gint.h>
#include <internals/interrupt_maps.h>
#include <gint.h>
#include <mpu.h>

gint_info_t gint;

//---
//	Initialization routines
//---

/*
	gint_init()
	Initializes gint. Loads the interrupt handler into the memory and sets
	the new vbr address.
*/
static void setup(void)
{
	isSH3() ? gint_setup_7705() : gint_setup_7305();
}
void gint_init(void)
{
	// Linker script symbols -- gint.
	extern uint32_t
		gint_vbr,
		gint_data,
		bgint, egint;

	uint32_t *ptr = &bgint;
	uint32_t *src = &gint_data;

	// Loading the interrupt handler into the memory.
	while(ptr < &egint) *ptr++ = *src++;

	// Installing gint's default exception/interrupt handlers.
	for(int i = 0; i < exc_type_max; i++)
	{
		gint_handlers[i].function = gint_handlers[i].default_function;
	}

	// Filling the information structure for the user.
	gint.vbr	= gint_getvbr;
	gint.system_vbr	= gint_getvbr();
	gint.gint_vbr	= (uint32_t)&gint_vbr;

	// Setting the VBR!
	gint_setvbr(gint.gint_vbr, setup);
}



//---
//	Context restoration routines
//---

/*
	gint_quit()
	Stops gint. Restores the system's configuration and vbr address.
*/
static void stop(void)
{
	isSH3() ? gint_stop_7705() : gint_stop_7305();
}
void gint_quit(void)
{
	// Restoring the system's VBR.
	gint_setvbr(gint.system_vbr, stop);
}
