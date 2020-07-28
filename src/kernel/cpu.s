/*
**	gint:core:vbr - Assembler-level VBR management
*/

.global	_cpu_setVBR
.global _cpu_setCPUOPM
.global _cpu_getCPUOPM
.global _cpu_getSR
.global _cpu_setSR

/* cpu_setVBR(): Change VBR address */

.section .gint.mapped, "ax"
_cpu_setVBR_reloc:
	mov.l	r8, @-r15
	mov.l	r9, @-r15
	sts.l	pr, @-r15

	/* Block all interrupts by setting IMASK=15 */
	mov	#0xf, r9
	shll2	r9
	shll2	r9
	stc	sr, r0
	or	r9, r0
	ldc	r0, sr

	/* Set the new VBR address */
	stc	vbr, r8
	ldc	r4, vbr

	/* Call the configuration function */
	jsr	@r5
	mov	r6, r4

	/* Enable interrupts again */
	stc	sr, r0
	not	r9, r9
	and	r9, r0
	ldc	r0, sr

	/* Return the previous VBR address */
	mov	r8, r0

	lds.l	@r15+, pr
	mov.l	@r15+, r9
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

/* cpu_getSR(): Get status register */
_cpu_getSR:
	stc	sr, r0
	rts
	nop

/* cpu_setSR(): Set status register */
_cpu_setSR:
	ldc	r4, sr
	rts
	nop
