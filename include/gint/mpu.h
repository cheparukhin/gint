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

/* mpu_t - supported MPUs */
typedef enum
{
	mpu_unknown	= 0,
	mpu_sh7337	= 1,	/* fx9860g, SH-3 based */
	mpu_sh7305	= 2,	/* fx9860g II, fxcg50, SH-4A based */
	mpu_sh7355	= 3,	/* fx9860g II, SH-3 based */
	mpu_sh7724	= 4,	/* For reference */
} mpu_t;

#ifdef FX9860G

	/* mpu_id() - get the name of the underlying MPU */
	extern const mpu_t gint_mpu_id;
	#define gint_mpu() (gint_mpu_id)

	/* Quick SH-3/SH-4 tests. Unknown models are assumed to be SH-4A */
	#define isSH3() (gint_mpu() & 1)
	#define isSH4() (!isSH3())

	/* mpu_init() - probe the MPU type
	   This function must be executed before mpu_id() can be used. */
	void mpu_init(void);

#else /* FXCG50 */

	/* All fxcg50 machines have an SH7305, which makes things simpler. */
	#define gint_mpu() mpu_sh7305
	#define isSH3() 0
	#define isSH4() 1

#endif /* FX9860G */

#endif /* GINT_CORE_MPU */
