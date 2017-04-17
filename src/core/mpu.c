//---
//
//	gint core module: mpu
//
//	Determines which kind of MPU is running the program.
//
//---

#include <mpu.h>
#include <stdint.h>

const mpu_t MPU_CURRENT;

/*
	getMPU()

	Returns the MPU identifier of the calculator.
	Thanks to SimonLothar for this function and related information.

	Processor version register (PVR) and product control register (PRR)
	hold information about the MPU version, they but are only accessible on
	SH-4-based MPUs.
	To detect SH-3-based MPUs, this function uses port L control register
	(PLCR), whose bits 8 to 15 cannot be set with SH7337 where bits 8 to 11
	can be set with SH7355.

	Additionally, the CPU core ID register (CPIDR) at 0xff000048 returns 1
	on SH7305.
*/
mpu_t getMPU(void)
{
	// Processor version register.
	volatile uint32_t *pvr	= (void *)0xff000030;
	// Product version register.
	volatile uint32_t *prr	= (void *)0xff000044;
	// Port L control register.
	volatile uint16_t *plcr	= (void *)0xa4000114;
	// Saved value for PLCR.
	uint16_t saved_plcr;
	uint16_t tested_plcr;

	// Looking for SH-3-based MPUs by testing PLCR writing access.
	saved_plcr = *plcr;
	*plcr = 0xffff;
	tested_plcr = *plcr;
	*plcr = saved_plcr;

	// Checking whether we are working with an SH7337 or an SH7355.
	if(tested_plcr == 0x00ff) return mpu_sh7337;
	if(tested_plcr == 0x0fff) return mpu_sh7355;

	// Looking for SH-4-based MPUs by testing the version registers. This
	// needs the three upper bytes of the processor version register to
	// match 0x10300b :
	if((*pvr & 0xffffff00) != 0x10300b00) return mpu_unknown;

	// Now that we have an SH-4-based MPU, checking whether it is SH7305 or
	// SH7724, just for reference.
	switch(*prr & 0xfffffff0)
	{
	case 0x00002c00:
		return mpu_sh7305;
	case 0x00002200:
		return mpu_sh7724;
	}

	// By default, the MPU is unknown.
	return mpu_unknown;
}
