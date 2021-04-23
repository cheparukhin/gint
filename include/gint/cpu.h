//---
// gint:cpu - CPU registers and built-in functions
//---

#ifndef GINT_CPU
#define GINT_CPU

#include <gint/defs/types.h>

//---
// Atomic operations
//---

/* cpu_atomic_start(): Enter atomic mode

   This function enters "atomic mode", a mode where distractions to the CPU
   (mainly interrupts) are disabled. This is useful when doing critical
   operations on the hardware, because it ensures that no other code will see
   any intermediate state between the start and end of the atomic mode, thereby
   making the sequence atomic to other code.

   Atomic mode disables interrupts with IMASK=15, however it does not set BL=1
   because exceptions never occur on their own (and it is desirable to have
   panic reports if the atomic code is buggy), and TLB misses are almost always
   desirable. If you want to set BL=1, you can do so with cpu_setSR().

   This function uses a mutex so atomic mode can be started within atomic code;
   every cpu_atomic_start() must be paired with exactly one cpu_atomic_end().
   Entering atomic mode several times does not affect the CPU state, however
   atomic mode will be exited only after all exits have been completed.

   Once atomic mod is exited the original value of IMASK at the first call to
   cpu_atomic_start() is restored. */
void cpu_atomic_start(void);

/* cpu_atomic_end(): Exit atomic mode
   There should be exactly one cpu_atomic_end() for each cpu_atomic_start(). */
void cpu_atomic_end(void);

//---
// Access to CPU registers
//---

/* Read and write the VBR register */
uint32_t cpu_getVBR(void);
void cpu_setVBR(uint32_t VBR);

/* Read and write the CPU Operation Mode register. After a write, the register
   is re-read and an (icbi) instruction is executed to apply the change. Non-
   writable bits should be left to their initial value during a write. */
uint32_t cpu_getCPUOPM(void);
void cpu_setCPUOPM(uint32_t CPUOPM);

/* cpu_str_t: Bits of the Status Register */
typedef lword_union(cpu_sr_t,
	uint32_t	:1;
	uint32_t MD	:1;
	uint32_t RB	:1;
	uint32_t BL	:1;
	uint32_t RC	:12;
	uint32_t	:3;
	uint32_t DSP	:1;
	uint32_t DMY	:1;
	uint32_t DMX	:1;
	uint32_t M	:1;
	uint32_t Q	:1;
	uint32_t IMASK	:4;
	uint32_t RF	:2;
	uint32_t S	:1;
	uint32_t T	:1;
);

/* Read and write the SR register. When writing, only "permanent" bits are set:
   * MD, RB, BL, DSP, IMASK are set.
   * M, Q, S and T are not set to preserve the behavior of ongoing divisions
     and tests. You can change T with (sett) and (clrt).
   * RC, DMY, DMX and DF are not set: use (setrc), (setdmx), (setdmy), and
     (clrdmxy). DF is preserved for old-style (setrc) loops to work. */
cpu_sr_t cpu_getSR(void);
void cpu_setSR(cpu_sr_t sr);

//---
// Configuration
//---

/* cpu_configure_vbr(): Select the VBR address to be loaded in the driver
   This function can be used before the driver is configure()'d. It sets the
   VBR address that will be used in the next world to be initialized. After a
   configure(), this is reset to 0. */
void cpu_configure_VBR(uint32_t VBR);

#endif /* GINT_CPU */
