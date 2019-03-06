//---
//	gint:core:mpu - Runtime MPU detection
//
//	This component detects the architecture and MPU type of the underlying
//	hardware by relying on version registers and/or side-information. It
//	provides macros isSH3() and isSH4() for MPU-dependent jobs
//
//	  if(isSH3()) print("SH3 code");
//	  else print("SH4 code");
//---

#ifndef GINT_CORE_MPU
#define GINT_CORE_MPU

#include <gint/defs/types.h>

/* mpu_t - supported MPUs */
typedef enum
{
	mpu_unknown	= 0,
	mpu_sh7337	= 1,	/* fx9860g, SH-3 based */
	mpu_sh7305	= 2,	/* fx9860g II, fxcg50, SH-4A based */
	mpu_sh7355	= 3,	/* fx9860g II, SH-3 based */
	mpu_sh7724	= 4,	/* For reference */
} mpu_t;



/* On fx9860g, the 256k byte RAM might be extended to 512k if the machine is
   recent enough. This includes all SH4 and many SH3 fx-9750 GII; the only
   model known to not have it is the old fx-9860G. */
#ifdef FX9860G

typedef struct
{
	/* MPU type, one of the above values */
	mpu_t mpu;
	/* Extended RAM area (8804'0000:256k) is available */
	int extended_ram;

} platform_t;

/* mpu_id() - get the name of the underlying MPU */
#define gint_mpu() (gint_platform.mpu)

/* Quick SH-3/SH-4 tests. Unknown models are assumed to be SH-4A */
#define isSH3() (gint_mpu() & 1)
#define isSH4() (!isSH3())

#endif /* FX9860G */



/* On fxcg50, the processor is always SH4. We can still differentiate between
   modern fx-CG 50 and older fx-CG 10/20 which are called here "Prizm". (This
   is done by observing the initial stack pointer.) */
#ifdef FXCG50

typedef struct
{
	/* MPU type (always sh7305) */
	mpu_t mpu;
	/* Whether this is an fx-CG 10/20 Prizm instead of an fx-CG 50 */
	int prizm;

} platform_t;

/* All fxcg50 machines have an SH7305, which makes things simpler. */
#define gint_mpu() mpu_sh7305
#define isSH3() 0
#define isSH4() 1

#endif /* FX9860G */



/* Platform details collected by mpu_init() */
extern const platform_t gint_platform;

/* mpu_init() - detect hardware information
   This function must be executed before other functions of this header can be
   used successfully.

   @stack  Starting stack address (roughly is enough) */
void mpu_init(uint32_t stack);

#endif /* GINT_CORE_MPU */
