//---
//	gint:core:start - Kernel initialization and C runtime
//--

#include <defs/attributes.h>
#include <defs/types.h>
#include <core/bootlog.h>
#include <core/mpu.h>
#include <core/mmu.h>
#include <gint/drivers.h>
#include <gint/gint.h>

/* Symbols provided by the linker script. For sections:
   - l* represents the load address (source address in ROM)
   - s* represents the size of the section
   - r* represents the relocation address (destination address in RAM)
   gint's BSS section is not mentioned here because it's never initialized */
extern uint32_t
	brom, srom,			/* Limits of ROM mappings */
	lgdata, sgdata, rgdata,		/* gint's data section */
	ldata,  sdata,  rdata,		/* User's data section */
	sbss, rbss,			/* User's BSS section */
	btors, mtors, etors;		/* Constructor/destructor arrays */
extern gint_driver_t
	bdrv, edrv;			/* Driver table */

/* User-provided main() function */
int main(int isappli, int optnum);

/* regcpy() - copy a memory region using symbol information
   @l  Source pointer (load address)
   @s  Size of area (should be a multiple of 16)
   @r  Destination pointer (relocation address) */
SECTION(".pretext")
static void regcpy(uint32_t * restrict l, int32_t s, uint32_t * restrict r)
{
	while(s > 0)
	{
		*r++ = *l++;
		*r++ = *l++;
		*r++ = *l++;
		*r++ = *l++;
		s -= 16;
	}
}
#define regcpy(l, s, r) regcpy(&l, (int32_t)&s, &r)

/* regclr() - clear a memory region using symbol information
   @r  Source pointer (base address)
   @s  Size of area (should be a multiple of 16) */
SECTION(".pretext")
static void regclr(uint32_t *r, int32_t s)
{
	while(s > 0)
	{
		*r++ = 0;
		*r++ = 0;
		*r++ = 0;
		*r++ = 0;
		s -= 16;
	}
}
#define regclr(r, s) regclr(&r, (int32_t)&s)

/* explore() - read data from a memory region to get it mapped by the system
   This function accesses all the 1k-blocks between b and b + s. This
   corresponds to region [b; b+s) when b and s are 1k-aligned.
   @b  Base pointer: first address checked
   @s  Size of region */
SECTION(".pretext")
static void explore(volatile void *b, int32_t s)
{
	ATTR(unused) uint8_t x;

	while(s > 0)
	{
		x = *(volatile uint8_t *)b;
		b += 1024;
		s -= 1024;
	}
}
#define explore(b, s) explore(&b, (int32_t)&s)

/* acall() - call an array of functions (constructors or destructors)
   @f  First element of array
   @l  First element outside of the array */
SECTION(".pretext")
static void acall(void (**f)(void), void (**l)(void))
{
	while(f < l) (*(*f++))();
}
#define acall(f, l) acall((void (**)(void))&f, (void (**)(void))&l)


/* start() - this is where it all starts
   Returns a status code. Invoking main menu is better than returning! */
SECTION(".pretext.entry")
int start(int isappli, int optnum)
{
	/* We are currently in a dynamic userspace mapping of an add-in run
	   from the storage memory. We are running in privileged mode with one
	   golden rule:

	       Do not disturb the operating system.

	   gint will silently run in an isolated part of the memory (fx9860g)
	   or at the start of the RAM (fxcg50). It acts as a kernel,
	   redirecting interrupts and reimplementing drivers, so we can't rely
	   too much on the system. Ladies and gentlemen, let's have fun! ;D */

	/* For now, we rely on the system to map the ROM pages. RAM is always
	   fully mapped, but we need to initialize it. We also need to do some
	   hardware detection because old fx9860g models have an older
	   processor with some incompatible features */

	/* Detect architecture - this will tell SH3 from SH4 on fx9860g */
	#ifdef FX9860G
	mpu_init();
	#endif

	/* Load data sections and wipe the bss section. This has to be done
	   first for static and global variables to be initialized */
	regcpy(lgdata, sgdata, rgdata);
	regcpy(ldata, sdata, rdata);
	regclr(rbss, sbss);

	#ifdef GINT_BOOT_LOG
	bootlog_loaded();
	#endif

	/* Traverse all ROM pages */
	explore(brom, srom);

	/* Count how much memory got mapped from this process */
	uint32_t rom, ram;
	mpuSwitch(
		tlb_mapped_memory(&rom, &ram),	/* SH3-based */
		utlb_mapped_memory(&rom, &ram)	/* SH4-based */
	);

	#ifdef GINT_BOOT_LOG
	bootlog_mapped(rom, ram);
	#endif

	//---

	/* Cancel add-in execution if not all pages are mapped
	   TODO: Resort to better graphical display, but still fxlib since
	   add-in is not mapped yet */
	if(rom < (uint32_t)&srom)
	{
		Bdisp_AllClr_VRAM();
		print(1, 1, "Missing memory!");
		print_hex(1, 2, rom, 8);
		print(1, 3, "<");
		print_hex(1, 4, (uint32_t)&srom, 8);
		Bdisp_PutDisp_DD();
		delay(20);
		return 1;
	}

	//---

	/* Install gint and switch VBR */
	gint_install();

	#ifdef GINT_BOOT_LOG
	bootlog_kernel();
	#endif

	/* We are now running on our own in kernel mode. To avoid breaking the
	   system, we want to limit our use of syscalls, so we'll have to do
	   device driving ourselves. */

	gint_driver_t *drv;

	/* Initialize all drivers by saving the system settings */
	for(drv = &bdrv; drv < &edrv; drv++)
	{
		if(isSH3() && drv->driver_sh3) drv->driver_sh3();
		drv->ctx_save(drv->sys_ctx);
		if(drv->init) drv->init();
	}

	/* With gint fully initialized, we are ready to start the hosted user
	   application. We have already loaded the RAM sections earlier; all
	   that's left is calling the constructors and rolling main(). */

	/* Call the constructors */
	acall(btors, mtors);

	/* Execute the user application */
	int ret = main(isappli, optnum);

	/* Call the destructors*/
	acall(mtors, etors);

	/* Before leaving the application, we need to clean up our mess. We
	   have changed many OS settings while accessing the peripheral
	   modules. The OS is bound to be confused (and crash) if we don't
	   restore them */

	/* Unload gint and give back control to the system. Driver settings
	   will be restored while interrupts are disabled */
	gint_unload();

	/* TODO: Invoke main menu instead of returning? */
	return ret;
}
