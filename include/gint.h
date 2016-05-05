#ifndef	_GINT_H
#define	_GINT_H	1

//---
//	Public API.
//---

/*
	gint_getVBR()
	Returns the current vbr address.

	@return	vbr address currently in use.
*/
unsigned int gint_getVBR(void);

/*
	gint_systemVBR()
	Returns the vbr address used by the system (saved when execution
	starts).

	@return	vbr address used by the system.
*/
unsigned int gint_systemVBR(void);



//---
//	Internal API.
//	Referenced here for documentation purposes only.
//	Do NOT call these functions, you'll most probably screw up the whole
//	interrupt handling system.
//---

/*
	gint_setVBR()
	Sets the vbr address and does some configuration while interrupts are
	disabled.

	@arg	new_vbr_address
	@arg	setup	Will be called for configuration under interrupt-safe
			environment.
*/
void gint_setVBR(unsigned int new_vbr_address, void (*setup)(void));

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
	gint_setup()
	Configures interrupt priorities and some parameters to allow gint to
	take control of the interrupt flow.
*/
void gint_setup_7705(void);
void gint_setup_7305(void);

/*
	gint_stop()
	Un-configures the interrupt flow to give back the interrupt control to
	the system.
*/
void gint_stop_7705(void);
void gint_stop_7305(void);

/*
	gint()
	Handles interrupts.
*/
void gint(void) __attribute__((
	section(".gint.int.entry"),
	interrupt_handler
));
void gint_7705(void)	__attribute__((section(".gint.int")));
void gint_7305(void)	__attribute__((section(".gint.int")));



//---
//	Internal priority definitions.
//---

#define	GINT_INTP_WDT	4
#define	GINT_INTP_RTC	9
#define	GINT_INTP_KEY	8



#endif	// _GINT_H
