//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#include <gint/defs/types.h>
#include <gint/defs/call.h>
#include <gint/config.h>
#include <gint/intc.h>

/* gint_world_switch(): Switch out of gint to execute a function

   This function can be used to leave gint, restore the OS's hardware state,
   and execute code there before returning to gint. By doing this one can
   effectively interleave gint with the standard OS execution. gint drivers
   will be inactive during this time but OS features such as BFile or the
   main menu are available.

   This main uses for this switch are going back to the main menu and using
   BFile function. You can go back to the main menu easily by calling getkey()
   (or getkey_opt() with the GETKEY_MENU flag set) and pressing the MENU key,
   or by calling gint_osmenu() below which uses this switch.

   The code to execute while in OS mode is passed as a gint call; you can use
   GINT_CALL() to create one. This allows you to pass arguments to your
   function, as well as return an int.

   @function  A GINT_CALL() to execute while in OS mode
   -> Returns the return value of (function), if any, 0 if function is NULL. */
int gint_world_switch(gint_call_t function);

/* This function is an older version of gint_world_switch() which only accepts
   functions with no arguments and no return value. It will be removed in
   gint 3. */
__attribute__((deprecated("Use gint_world_switch() instead")))
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

/* This function has been moved to the INTC driver */
__attribute__((deprecated("Use intc_handler() instead")))
static GINLINE void *gint_inthandler(int code, void const *h, size_t size) {
	return intc_handler(code, h, size);
}

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
extern int (*gint_inth_callback)(int (*function)(void *arg), void *arg);

#endif /* GINT_GINT */
