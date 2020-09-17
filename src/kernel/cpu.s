/*
**	gint:core:vbr - Assembler-level VBR management
*/

.global	_cpu_setVBR
.global _cpu_setCPUOPM
.global _cpu_getCPUOPM

/* cpu_setVBR(): Change VBR address */

.section .gint.mapped, "ax"
_cpu_setVBR_reloc:
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	stc.l	sr, @-r15

	/* Block all interrupts by setting IMASK=15 */
	mov	#0xf, r1
	shll2	r1
	shll2	r1
	stc	sr, r0
	or	r1, r0
	ldc	r0, sr

	/* Set the new VBR address */
	stc	vbr, r8
	ldc	r4, vbr

	/* Call the configuration function */
	jsr	@r5
	mov	r6, r4

	/* Enable interrupts again by restoring the status register */
	ldc.l	@r15+, sr
	lds.l	@r15+, pr
	mov	r8, r0
	rts
	mov.l	@r15+, r8

.section .gint.mappedrel, "aw"
_cpu_setVBR:
	.long	_cpu_setVBR_reloc

.text

/* cpu_setCPUOPM(): Change the CPU Operation Mode register */
_cpu_setCPUOPM:
	/* Set CPUOPM as requested */
	mov.l	1f, r0
	mov.l	r4, @r0

	/* Read CPUOPM again */
	mov.l	@r0, r5

	/* Invalidate a cache address */
	mov	#-96, r0
	shll16	r0
	shll8	r0
	icbi	@r0

	rts
	nop

/* cpu_getCPUOPM(): Get the CPU OperatioN Mode register */
_cpu_getCPUOPM:
	mov.l	1f, r0
	rts
	mov.l	@r0, r0

.align 4

1:	.long	0xff2f0000
