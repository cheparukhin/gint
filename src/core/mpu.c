//---
//	gint:core:mpu - Runtime MPU detection
//---

#include <core/mpu.h>
#include <defs/attributes.h>
#include <defs/types.h>

/* This file is only useful on fx9860g machines because all fxcg50 are SH4 */
#ifdef FX9860G

/* Holds the name of the current MPU; initialized at startup by mpu_init() */
GBSS const mpu_t mpu;

/* mpu_detect() - detect the underlying MPU
   Many thanks to Simon Lothar for relevant documentation.

   Processor Version Register (PVR) and Product Version Register (PRR) provide
   info for SH-4-based MPUS; SH-3 based boards are detected and distinguished
   by testing writable bits in the Port L Control Register (PLCR).

   Returns the detected MPU type, falling back on mpu_unknown */
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
void mpu_init(void)
{
	const_cast(mpu, mpu_t) = mpu_detect();
}

#endif
