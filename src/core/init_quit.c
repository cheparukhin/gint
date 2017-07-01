#include <internals/gint.h>
#include <internals/interrupt_maps.h>
#include <internals/modules.h>
#include <gint.h>
#include <mpu.h>

gint_info_t gint;
static environment_t env;

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
	isSH3()	? gint_lock_and_setup_7705()
		: gint_lock_and_setup_7305();
}
void gint_init(void)
{
	// Detecting the MPU type. I don't like const-casting but this is still
	// better than allowing the user to change the variable by mistake.
	*((mpu_t *)&MPU_CURRENT) = getMPU();
	// Loading the register addresses of the current platform.
	mod_init();

	// Linker script symbols -- gint.
	extern uint32_t
		gint_vbr,
		gint_data,
		bgint, egint;

	uint32_t *ptr = &bgint;
	uint32_t *src = &gint_data;

	// Loading the interrupt handler into the memory.
	while(ptr < &egint) *ptr++ = *src++;

	isSH3()	? gint_save_7705(&env.env_7705)
		: gint_save_7305(&env.env_7305);

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
	isSH3()	? gint_restore_and_unlock_7705(&env.env_7705)
		: gint_restore_and_unlock_7305(&env.env_7305);
}
void gint_quit(void)
{
	// Restoring the system's VBR.
	gint_setvbr(gint.system_vbr, stop);
}



//---
//	System-switch technique
//---

#include <display.h>
#include <gray.h>

/*
	gint_switch()
	Temporarily returns to the system's main menu.
*/
static environment_t switch_env;
static void switch_(void)
{
	isSH3()	? gint_restore_and_unlock_7705(&switch_env.env_7705)
		: gint_restore_and_unlock_7305(&switch_env.env_7305);
}
void gint_switch(void)
{
	// Save the current environment in a special buffer, so that we can
	// restore it if we ever come back to gint.
	isSH3()	? gint_save_7705(&switch_env.env_7705)
		: gint_save_7305(&switch_env.env_7305);
	// Give the control back to the system.
	gint_setvbr(gint.system_vbr, stop);

	// When returning to the add-in from the menu, the system displays the
	// add-in's video ram again, but it's often blank. We thus need to copy
	// the contents of the gint video ram to the system's.
	void *vram = gray_runs()
		? gray_currentVRAM()
		: display_getCurrentVRAM();

	// Use system calls to put KEY_MENU in the key buffer and invoke
	// GetKeyWait(), triggering the main menu.
	__system_menu(vram);

	// If the user came back, restore the gint working environment.
	gint_setvbr(gint.gint_vbr, switch_);
}
