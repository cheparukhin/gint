//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#include <gint/defs/types.h>

/* GINT_VERSION - the library version number

   gint is versioned from its repository commits on the master branch. The
   GINT_VERSION integer contains the short commit hash with 7 digits.

   For instance, 0x03f7c0a0 means commit 3f7c0a0. */
extern char GINT_VERSION;
#define GINT_VERSION ((uint32_t)&GINT_VERSION)

/* gint_switch(): Switch out of gint to execute a function

   This function can be used to leave gint, restore the system's driver
   context, and execute code there before returning to gint. By doing this one
   can effectively interleave gint with the standard OS execution. The
   limitations are quite extreme though, so unless you know precisely what
   you're doing that requires getting out of gint (eg. BFile), be careful.

   This main uses for this switch are going back to the main menu and using
   BFile function. You can go back to the main menu easily by calling getkey()
   (or getkey_opt() with the GETKEY_MENU flag set) and pressing the MENU key,
   or by calling gint_osmenu() below which uses this switch.

   @function  Function to call in OS mode */
void gint_switch(void (*function)(void));

/* gint_osmenu(): Call the calculator's main menu

   This function safely invokes the calculator's main menu with gint_switch().
   If the user selects the gint application again in the menu, this function
   reloads gint and returns. Otherwise, the add-in is fully unloaded by the
   system and the application terminates.

   This function is typically called when the [MENU] key is pressed during a
   call to getkey(), but can also be called manually. */
void gint_osmenu(void);

/* gint_setrestart(): Set whether to restart the add-in after exiting

   An add-in that reaches the end of its code exits. On the calculator, except
   using OS-dependent settings, it cannot be started again unless another
   application is launched first.

   This setting allows the add-in to restart by calling gint_osmenu() instead
   of exiting. This can give a proper illusion of restarting if used correctly.

   @restart  0 to exit, 1 to restart by using gint_osmenu() */
void gint_setrestart(int restart);

/* gint_inthandler(): Install interrupt handlers

   This function installs (copies) interrupt handlers in the VBR space of the
   application. Each handler is a 32-byte block aligned on a 32-byte boundary.
   When an interrupt request is accepted, the hardware jumps to a specific
   interrupt handler at an address that depends on the interrupt source.

   For safety, interrupt handlers should avoid referring to data from other
   blocks because the arrangement of blocks at runtime depends on event codes.
   The assembler program will assume that consecutive blocks in the source code
   will be consecutive in memory, which is not always true. Avoiding cross-
   references is a practical rule to avoid problems. (gint breaks this rule
   quite often but does it safely.)

   This function allows anyone to replace any interrupt handler so make sure
   you're not interfering with interrupt assignments from gint or a library.

   The first parameter event_code represents the event code associated with the
   interrupt. If it's not a multiple of 0x20 then you're doing something wrong.
   The codes are normally platform-dependent, but gint always uses SH7305
   codes. SH3 platforms have a different, compact VBR layout. gint_inthandler()
   translates the provided SH7305 codes to the compact layout and the interrupt
   handler translates the hardware SH3 codes to the compact layout as well. See
   gint's source in <src/core/inth.S> and <src/core/kernel.c>. Please note that
   gint_inthandler() uses a table that must be modified for every new SH3
   interrupt code to extend the compact scheme.

   The handler function is run in the kernel register bank with interrupts
   disabled and must end with 'rts' (not 'rte') as the main interrupt handler
   saves some registers for you. By default, user bank registers are not saved
   except for gbr/mach/macl; if you want to call back to arbitrary code safely,
   use gint_inth_callback() in your handler.

   For convenience gint allows any block size to be loaded as an interrupt
   handler, but it should really be a multiple of 0x20 bytes and not override
   other handlers. If it's not written in assembler, then you're likely doing
   something wrong. Using __attribute__((interrupt_handler)), which uses rte,
   is especially wrong.

   It is common for interrupt handlers to have a few bytes of data, such as the
   address of a callback function. gint often stores this data in the last
   bytes of the block. This function returns the VBR address of the block which
   has just been installed, to allow the caller to edit the parameters later.

   @event_code  Identifier of the interrupt block
   @handler     Address of handler function
   @size        How many bytes to copy
   Returns the VBR address where the handler was installed. */
void *gint_inthandler(int event_code, void const *handler, size_t size);

/* gint_inth_callback(): Call back arbitrary code from an interrupt handler

   Calls the specified function with the given argument after saving the user
   context, enabling interrupts and going to user bank. This function is used
   to call user code from interrupt handlers, typically from timer or RTC
   callbacks. You can think of it as a way to escape the SR.BL=1 environment to
   safely call back virtualized and interrupt-based functions during interrupt
   handling.

   It is not safe to call from C code in any capacity and is mentioned here
   only for documentation purposes; you should really only call this from
   an interrupt handler's assembler code, typically like this:

        mov.l	.callback, r0
        mov.l	@r0, r0 # because function pointer

        mov	<function>, r4
        mov	<argument>, r5
        jsr	@r0
        nop

   .callback:
        .long	_gint_inth_callback

   This function is loaded to a platform-dependent address determined at
   runtime; call it indirectly through the function pointer.

   @callback  Callback function, may take no argument in practice
   @arg       Argument
   Returns the return value of the callback. */
int (*gint_inth_callback)(int (*function)(void *arg), void *arg);

#endif /* GINT_GINT */
