/*
**	gint:core:vbr - Assembler-level VBR management
*/

.global	_cpu_setVBR
.global _cpu_setCPUOPM
.global _cpu_getCPUOPM

.section .gint.mapped

/* cpu_setVBR(): Change VBR address */
_cpu_setVBR:
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15

	mov	#1, r8
	mov	#28, r0
	shld	r0, r8

	/* Block all interrupts */
	stc	sr, r0
	or	r8, r0
	ldc	r0, sr

	/* Set the new VBR address */
	stc	vbr, r9
	ldc	r4, vbr

	/* Call the configuration function */
	jsr	@r5
	nop

	/* Enable interrupts again */
	stc	sr, r0
	not	r8, r8
	and	r8, r0
	ldc	r0, sr

	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov	r9, r0
	rts
	mov.l	@r15+, r9

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
