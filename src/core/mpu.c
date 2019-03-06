//---
//	gint:core:mpu - Runtime MPU detection
//---

#include <gint/mpu.h>
#include <gint/defs/attributes.h>
#include <gint/defs/types.h>
#include <gint/defs/util.h>

/* Holds information about the current MPU */
GBSS const platform_t gint_platform;

/* This function is only used on fx9860g because all fxcg50 are SH4 */
#ifdef FX9860G

/* mpu_detect() - detect the underlying MPU
   Many thanks to Simon Lothar for relevant documentation.

   Processor Version Register (PVR) and Product Version Register (PRR) provide
   info for SH-4-based MPUS; SH-3 based boards are detected and distinguished
   by testing writable bits in the Port L Control Register (PLCR).

   Returns the detected MPU type, falling back on mpu_unknown */
GSECTION(".pretext")
static mpu_t mpu_detect(void)
{
	/* Processor Version Register */
	volatile uint32_t *pvr	= (void *)0xff000030;
	/* Product Version Register */
	volatile uint32_t *prr	= (void *)0xff000044;
	/* Port L Control Register */
	volatile uint16_t *plcr	= (void *)0xa4000114;

	/* Detecting SH-3-based MPUs by testing writable bits in PLCR */

	uint16_t old = *plcr;
	*plcr = 0xffff;
	uint16_t tested = *plcr;
	*plcr = old;

	if(tested == 0x00ff) return mpu_sh7337;
	if(tested == 0x0fff) return mpu_sh7355;

	/* Check that we're dealing with an SH-4-based MPU */
	if((*pvr & 0xffffff00) != 0x10300b00) return mpu_unknown;

	/* Tell SH-4 MPUs by testing the product version register */

	uint32_t ver = *prr & 0xfffffff0;
	if(ver == 0x00002c00) return mpu_sh7305;
	if(ver == 0x00002200) return mpu_sh7724;

	return mpu_unknown;
}

/* mpu_init() - detect and save information about the underlying MPU */
GSECTION(".pretext")
void mpu_init(GUNUSED uint32_t stack)
{
	const_cast(gint_platform.mpu, mpu_t) = mpu_detect();

	/* Detect additional RAM */

	volatile uint8_t *after_ram = (void *)0x88040000;
	volatile uint8_t *start_ram = (void *)0x88000000;

	uint8_t backup = *after_ram;
	*after_ram = ~backup;
	int ext = (*start_ram == backup);
	*after_ram = backup;

	const_cint(gint_platform.extended_ram) = ext;
}

#endif /* FX9860G */

#ifdef FXCG50

/* mpu_init() - detect and save information about the underlying MPU */
GSECTION(".pretext")
void mpu_init(uint32_t stack)
{
	const_cast(gint_platform.mpu, mpu_t) = mpu_sh7305;

	/* Detect Prizm models */
	int prizm = (stack < 0x8c160000);
	const_cint(gint_platform.prizm) = prizm;
}

#endif /* FXCG50 */
