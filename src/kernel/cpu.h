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

   @vbr        New VBR address
   @conf_intc  Configuration function
   @arg        Additional argument for conf_intc
   Returns the previous VBR address. */
uint32_t cpu_setVBR(uint32_t vbr, void (*conf_intc)(int arg), int arg);

/* cpu_setCPUOPM(): Change the CPU Operation Mode register

   Updates the CPU Operation Mode with the specified settings, then performs a
   read and an ICBI to register the change. Only writable bits of CPUOPM should
   be changed, other bits must be left at the value given by cpu_getcpuopm().

   @CPUOPM  New operation mode */
void cpu_setCPUOPM(uint32_t CPUOPM);

/* cpu_getCPUOPM(): Get the CPU OperatioN Mode register */
uint32_t cpu_getCPUOPM(void);

#endif /* GINT_CORE_CPU */
