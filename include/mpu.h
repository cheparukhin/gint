#ifndef	_MPU_H
#define	_MPU_H	1

//---
//	enum MPU
//	This type holds information about the calculator's MPU.
//---

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



//---
//	MPU type tests.
//	Prefer using an 'if(isSH3()) { ... } else { ... }' alternative for best
//	results.
//---

// Global MPU variable, accessible for direct tests.
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

	@return	MPU type.
*/
enum MPU getMPU(void);

#endif // _MPU_H
