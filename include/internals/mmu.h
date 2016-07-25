//---
//
//	gint core module: mmu
//
//	A wise application should avoid tampering with the system's
//	configuration of the MMU and the TLB. This module implicitly calls the
//	system but does nothing by itself.
//
//---

#ifndef	_MMU_H
#define	_MMU_H	1

/*
	mmu_pseudoTLBInit()
	Tries to have the system load enough data into TLB to allow add-in to
	execute.
*/
void mmu_pseudoTLBInit(void);

#endif	// _MMU_H
