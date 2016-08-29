//---
//
//	gint core module: interrupt handler
//
//	Central point of the library. Controls the interrupt handler and
//	defines a few functions to configure callbacks for some interrupts.
//
//---

#include <internals/gint.h>
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



//---
//	VBR space.
//---

#include <display.h>
#define print(str, x, y) dtext(6 * (x) - 5, 8 * (y) - 7, str)
#define hexdigit(n) ((n) + '0' + 39 * ((n) > 9))

static void hex(unsigned int x, int digits, char *str)
{
	str[0] = '0';
	str[1] = 'x';
	str[digits + 2] = 0;

	while(digits)
	{
		str[digits + 1] = hexdigit(x & 0xf);
		x >>= 4;
		digits--;
	}
}

static void reverse(void)
{
	int *vram = display_getCurrentVRAM();
	int i;
	for(i = 0; i < 36; i++) vram[i] = ~vram[i];
}

/*
	gint_exc()
	Handles exceptions.
*/
void gint_exc(void)
{
	volatile unsigned int *expevt	= gint_reg(Register_EXPEVT);
	volatile unsigned int *tea	= gint_reg(Register_TEA);
	unsigned int spc;
	char str[11];

	text_configure(NULL, Color_Black);

	__asm__("\tstc spc, %0" : "=r"(spc));

	dclear();
	print("Exception raised!", 3, 1);
	reverse();
	print(gint_strerror(0), 2, 3);

	print("expevt", 2, 4);
	hex(*expevt, 3, str);
	print(str, 16, 4);

	print("pc", 2, 5);
	hex(spc, 8, str);
	print(str, 11, 5);

	print("tea", 2, 6);
	hex(*tea, 8, str);
	print(str, 11, 6);

	print("Please reset.", 2, 7);
	dupdate();
	while(1);
}

/*
	gint_tlb()
	Handles TLB misses.
*/
void gint_tlb(void)
{
	volatile unsigned int *expevt	= gint_reg(Register_EXPEVT);
	volatile unsigned int *tea	= gint_reg(Register_TEA);
	unsigned int spc;
	char str[11];

	text_configure(NULL, Color_Black);

	__asm__("\tstc spc, %0" : "=r"(spc));

	dclear();
	print("TLB error!", 6, 1);
	reverse();
	print(gint_strerror(1), 2, 3);

	print("expevt", 2, 4);
	hex(*expevt, 3, str);
	print(str, 16, 4);

	print("pc", 2, 5);
	hex(spc, 8, str);
	print(str, 11, 5);

	print("tea", 2, 6);
	hex(*tea, 8, str);
	print(str, 11, 6);

	print("Please reset.", 2, 7);
	dupdate();
	while(1);
}

/*
	gint_int()
	Handles interrupts.
*/
void gint_int(void)
{
	if(isSH3())
		gint_int_7705();
	else
		gint_int_7305();
}

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information (f.i. EXPEVT may not return the same value for a given
	exception on both 7705 and 7305).
*/
inline volatile void *gint_reg(enum Register reg)
{
	if(isSH3())
		return gint_reg_7705(reg);
	else
		return gint_reg_7305(reg);
}

/*
	gint_strerror()
	Returns a string that describe the error set in EXPEVT. This string is
	not platform-dependent.
	Some exception codes represent different errors when invoked inside the
	general exception handler and the TLB error handler. Parameter 'is_tlb'
	should be set to zero for general exception meanings, and anything non-
	zero for TLB error meanings.
*/
const char *gint_strerror(int is_tlb)
{
	if(isSH3())
		return gint_strerror_7705(is_tlb);
	else
		return gint_strerror_7305();
}
