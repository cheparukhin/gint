//---
//
//	gint core module: mpu
//
//	Determines which kind of MPU is running the program. This module only
//	provides the following test:
//
//	if(isSH3()) { ... }
//	else { ... }
//
//---

#ifndef	_MPU_H
#define	_MPU_H	1

/*
	enum MPU
	This type holds information about the calculator's MPU.
*/
enum MPU
{
	MPU_Unknown = 0,
	// fx-9860G SH3.
	MPU_SH7337 = 1,
	// fx-9860G II SH3.
	MPU_SH7355 = 2,
	// fx-9860G II SH4.
	MPU_SH7305 = 3,
	// Just for reference.
	MPU_SH7724 = 4
};

// Global MPU variable, accessible for direct tests. Initialized at the
// beginning of execution.
extern enum MPU MPU_CURRENT;

// Quick SH3 test. It is safer to assume that an unknown model is SH4 because
// SH3-based models are not produced anymore.
#define isSH3() (MPU_CURRENT == MPU_SH7337 || MPU_CURRENT == MPU_SH7355)
#define isSH4() !isSH3()



//---
//	Public API.
//---

/*
	getMPU()
	Determines the MPU type and returns it. MPU_CURRENT is not updated.
*/
enum MPU getMPU(void);



//---
//	Internal API.
//	Referenced here for documentation purposes only. Do not call.
//---

/*
	mpu_init()
	Determines the MPU type and stores the result into MPU_CURRENT.
*/
void mpu_init(void)	__attribute__((constructor));

#endif // _MPU_H
