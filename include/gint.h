//---
//
//	gint core module: interrupt handler
//
//	Central point of the library. Controls the interrupt handler and
//	defines a few functions to configure callbacks for some interrupts.
//
//---

#ifndef	_GINT_H
#define	_GINT_H	1

//---
//	Interrupt handler control.
//---

/*
	gint_getVBR()
	Returns the current vbr address.
*/
unsigned int gint_getVBR(void);

/*
	gint_systemVBR()
	Returns the vbr address used by the system (saved when execution
	starts).
*/
unsigned int gint_systemVBR(void);



//---
//	Callback API.
//---

/*
	enum RTCFrequency
	Describes the possible frequencies available for the real-time clock
	interrupt.
*/
enum RTCFrequency
{
	RTCFreq_500mHz	= 7,
	RTCFreq_1Hz	= 6,
	RTCFreq_2Hz	= 5,
	RTCFreq_4Hz	= 4,
	RTCFreq_16Hz	= 3,
	RTCFreq_64Hz	= 2,
	RTCFreq_256Hz	= 1,
};

/*
	gint_setRTCCallback()
	Sets the callback function for the real-time clock interrupt. If
	frequency is non-NULL, the clock frequency is set to the given value.
*/
void gint_setRTCCallback(void (*callback)(void), enum RTCFrequency frequency);

/*
	gint_getRTCCallback()
	Returns the callback function. If frequency is non-NULL, it is set to
	the current frequency value.
*/
void (*gint_getRTCCallback(enum RTCFrequency *frequency))(void);



//---
//	Internal API.
//	Referenced here for documentation purposes only.
//	Do NOT call these functions, you'll most probably screw up the whole
//	interrupt handling system.
//---

/*
	gint_setVBR()
	Sets the vbr address and calls the configuration function while
	interrupts are disabled.
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
void gint(void)			__attribute__((section(".gint.int.entry"),
					interrupt_handler));
void gint_7705(void)		__attribute__((section(".gint.int")));
void gint_7305(void)		__attribute__((section(".gint.int")));

/*
	gint_setRTCFrequency()
	Sets the RTC interrupt frequency and enables interrupts.
*/
void gint_setRTCFrequency_7705(enum RTCFrequency frequency);
void gint_setRTCFrequency_7305(enum RTCFrequency frequency);

/*
	gint_getRTCFrequency()
	Returns the RTC interrupt frequency.
*/
enum RTCFrequency gint_getRTCFrequency_7705(void);
enum RTCFrequency gint_getRTCFrequency_7305(void);



//---
//	Internal priority definitions.
//---

#define	GINT_INTP_WDT	4
#define	GINT_INTP_RTC	12

#define GINT_INTP_GRAY	15
#define	GINT_INTP_KEY	8
#define	GINT_INTP_TIMER	10



#endif	// _GINT_H
