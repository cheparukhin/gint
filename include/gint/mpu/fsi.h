//---
// gint:mpu:fsi - FIFO-Buffered Serial Interface
//---

#ifndef GINT_MPU_FSI
#define GINT_MPU_FSI

#ifdef __cplusplus
extern "C" {
#endif

#include <gint/defs/attributes.h>
#include <gint/defs/types.h>

//---
//	SH7305 FIFO-Buffered Serial Interface.
//---

typedef volatile struct
{
	uint32_t A_DO_FMT;	/* A output serial format */
	uint32_t A_DOFF_CTL;	/* A output FIFO control */
	uint32_t A_DOFF_ST;	/* A output FIFO status */
	uint32_t A_DI_FMT;	/* A input serial format */

	uint32_t A_DIFF_CTL;	/* A input FIFO control */
	uint32_t A_DIFF_ST;	/* A input FIFO status */
	uint32_t A_CKG1;	/* A clock set 1 */
	uint32_t A_CKG2;	/* A clock set 2 */

	uint32_t A_DIDT;	/* A read data */
	uint32_t A_DODT;	/* A write data */
	uint32_t A_MUTE_ST;	/* A mute state */
	pad(0x14);

	uint32_t B_DO_FMT;	/* B output serial format */
	uint32_t B_DOFF_CTL;	/* B output FIFO control */
	uint32_t B_DOFF_ST;	/* B output FIFO status */
	uint32_t B_DI_FMT;	/* B input serial format */

	uint32_t B_DIFF_CTL;	/* B input FIFO control */
	uint32_t B_DIFF_ST;	/* B input FIFO status */
	uint32_t B_CKG1;	/* B clock set 1 */
	uint32_t B_CKG2;	/* B clock set 2 */

	uint32_t B_DIDT;	/* B read data */
	uint32_t B_DODT;	/* B write data */
	uint32_t B_MUTE_ST;	/* B mute state */
	pad(0x200-0x6c);

	uint32_t INT_ST;	/* Interrupt state */
	uint32_t IEMSK;		/* Interrupt source mask */
	uint32_t IMSK;		/* Interrupt signal mask */
	uint32_t MUTE;		/* Mute set */

	uint32_t CLK_RST;	/* Clock reset */
	uint32_t SOFT_RST;	/* Software reset */
	uint32_t FIFO_SZ;	/* FIFO size */

} GPACKED(4) sh7305_fsi_t;

#define SH7305_FSI (*((sh7305_fsi_t *)0xfe3c0000))

#ifdef __cplusplus
}
#endif

#endif /* GINT_MPU_FSI */
