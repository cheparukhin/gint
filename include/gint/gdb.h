//---
// gint:gdb - GDB remote serial protocol
//---

#ifndef GINT_GDB
#define GINT_GDB

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Error codes for GDB functions */
enum {
	GDB_NO_INTERFACE = -1,
	GDB_ALREADY_STARTED = -2,
	GDB_USB_ERROR = -3,
};

/* gdb_cpu_state_t: State of the CPU when breaking
   This struct keep the same register indices as those declared by GDB to allow
   easy R/W without needing a "translation" table.
   See : https://sourceware.org/git/?p=binutils-gdb.git;a=blob;f=gdb/sh-tdep.c;
         h=c402961b80a0b4589243023ea5362d43f644a9ec;hb=4f3e26ac6ee31f7bc4b04abd
         8bdb944e7f1fc5d2#l327
 */
// TODO : Should we expose r*b*, ssr, spc ? are they double-saved when breaking
//        inside an interrupt handler ?
typedef struct {
	union {
		struct {
			uint32_t r0;
			uint32_t r1;
			uint32_t r2;
			uint32_t r3;
			uint32_t r4;
			uint32_t r5;
			uint32_t r6;
			uint32_t r7;
			uint32_t r8;
			uint32_t r9;
			uint32_t r10;
			uint32_t r11;
			uint32_t r12;
			uint32_t r13;
			uint32_t r14;
			uint32_t r15;
			uint32_t pc;
			uint32_t pr;
			uint32_t gbr;
			uint32_t vbr;
			uint32_t mach;
			uint32_t macl;
			uint32_t sr;
		} reg;
		uint32_t regs[23];
	};
} gdb_cpu_state_t;

/* gdb_start(): Start the GDB remote serial protocol server

   This function will start the GDB remote serial protocol implementation and
   block until the program is resumed from the connected debugger.
   It currently only supports USB communication and will fail if USB is already
   in use.*/
int gdb_start(void);

#ifdef __cplusplus
}
#endif

#endif /* GINT_GDB */
