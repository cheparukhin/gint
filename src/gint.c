#include <gint.h>
#include <mpu.h>
#include <gray.h>
#include <stddef.h>

//---
//	Local variables.
//---

static unsigned int
	new_vbr,
	sys_vbr;

static void (*rtc_callback)(void) = NULL;



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

	@return	vbr address used by the system.
*/
unsigned int gint_systemVBR(void)
{
	return sys_vbr;
}

/*
	gint_setRTCCallback()
	Sets the callback function for the real-time clock interrupt. If
	frequency is non-NULL, the clock frequency is set to the given value.

	@arg	callback	Callback function.
	@arg	frequency	Interrupt frequency.
*/
void gint_setRTCCallback(void (*callback)(void), enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	rtc_callback = callback;

	if(isSH3())
		gint_setRTCFrequency_7705(frequency);
	else
		gint_setRTCFrequency_7305(frequency);
}

/*
	gint_getRTCCallback()
	Returns the callback function. If frequency is non-NULL, it is set to
	the current frequency value.
*/
void (*gint_getRTCCallback(enum RTCFrequency *frequency))(void)
{
	if(!frequency) return rtc_callback;

	if(isSH3())
		*frequency = gint_getRTCFrequency_7705();
	else
		*frequency = gint_getRTCFrequency_7305();

	return rtc_callback;
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
