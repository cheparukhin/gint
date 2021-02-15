//---
//	core:cpu - CPU registers and operation management
//---

#ifndef GINT_CORE_CPU
#define GINT_CORE_CPU

#include <gint/defs/types.h>

/* cpu_setVBR(): Change VBR address

   Blocks interrupts then changes the VBR address and calls the provided INTC
   configuration function before restoring interrupts. This function must
   configure the INTC in a way that is safe for the new VBR controller,
   including disabling all interrupts that it cannot handle.

   This function is loaded to a platform-dependent address determined at
   runtime; call it indirectly through the function pointer.

   @vbr        New VBR address
   @conf_intc  Configuration function
   @arg        Additional argument for conf_intc
   Returns the previous VBR address. */
extern uint32_t (*cpu_setVBR)(uint32_t vbr, void (*conf_intc)(int arg),
	int arg);

/* cpu_getVBR(): Query the current VBR address */
uint32_t cpu_getVBR(void);

/* cpu_setCPUOPM(): Change the CPU Operation Mode register

   Updates the CPU Operation Mode with the specified settings, then performs a
   read and an ICBI to register the change. Only writable bits of CPUOPM should
   be changed, other bits must be left at the value given by cpu_getcpuopm().

   @CPUOPM  New operation mode */
void cpu_setCPUOPM(uint32_t CPUOPM);

/* cpu_getCPUOPM(): Get the CPU OperatioN Mode register */
uint32_t cpu_getCPUOPM(void);

//---
//	Status Register
//---

/* Status Register bits */
typedef lword_union(sr_t,
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

/* Get and set sr through the sr_t type */
sr_t cpu_getSR(void);
void cpu_setSR(sr_t sr);

#endif /* GINT_CORE_CPU */
