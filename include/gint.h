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

#define	GINT_VERSION		0x01000000
#define	GINT_VERSION_STR	"01.00"

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
//	Register access.
//---

/*
	enum Register
	Represents common registers. Used as identifiers to retrieve their
	values using gint_register().
*/
enum Register
{
	Register_EXPEVT,
	Register_MMUCR,
	Register_TEA,
};

/*
	gint_register()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
volatile void *gint_reg(enum Register reg);

/*
	gint_strerror()
	Returns a string that describe the error set in EXPEVT in case of
	general exception of TLB miss exception. This string is platform-
	independent.
	Some exception codes represent different errors when invoked inside the
	general exception handler and the TLB error handler. Parameter 'is_tlb'
	should be set to zero for general exception meanings, and anything non-
	zero for TLB error meanings.
*/
const char *gint_strerror(int is_tlb);



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
	gint_reg()
	gint_strerror()
	See "Register access" section.
*/
volatile void *gint_reg_7705(enum Register reg);
volatile void *gint_reg_7305(enum Register reg);
const char *gint_strerror_7705(int is_tlb);
const char *gint_strerror_7305(void);


//---
//	Exception handling.
//---

/*
	gint_exc()
	Handles exceptions.
*/
void gint_exc(void)		__attribute__((section(".gint.exc.entry"),
					interrupt_handler));
void gint_exc_7705(void)	__attribute__((section(".gint.exc")));
void gint_exc_7305(void)	__attribute__((section(".gint.exc")));

/*
	gint_tlb()
	Handles TLB misses.
*/
void gint_tlb(void)		__attribute__((section(".gint.tlb.entry"),
					interrupt_handler));
void gint_tlb_7705(void)	__attribute__((section(".gint.tlb")));
void gint_tlb_7305(void)	__attribute__((section(".gint.tlb")));

/*
	gint_int()
	Handles interrupts.
*/
void gint_int(void)		__attribute__((section(".gint.int.entry"),
					interrupt_handler));
void gint_int_7705(void)	__attribute__((section(".gint.int")));
void gint_int_7305(void)	__attribute__((section(".gint.int")));



//---
//	Internal platform-independent definitions.
//---

#define	GINT_INTP_WDT	4
#define	GINT_INTP_RTC	12

#define GINT_INTP_GRAY	15
#define	GINT_INTP_KEY	8
#define	GINT_INTP_TIMER	10



#endif	// _GINT_H
