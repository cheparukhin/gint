//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#include <gint/defs/types.h>

/* GINT_VERSION - the library version number

   gint is versioned from it's repository commits on the master branch. The
   GINT_VERSION integer contains the short commit hash.

   For instance, 0x03f7c0a0 means commit 3f7c0a0. */
extern char GINT_VERSION;
#define GINT_VERSION ((uint32_t)&GINT_VERSION)

//---
//	Library management
//---

/* gint_install() - install and start gint
   This function installs event handlers, masks interrupts and switches VBR.
   Unless you are doing experimental runtime switching and you know how this
   function is implemented, you should not call it. */
void gint_install(void);

/* gint_unload() - unload gint and give back control to the system
   This function restores the runtime environment saved by gint_install(). It
   is only called when the add-in terminates. To temporarily leave gint during
   execution, use gint_pause(). When possible, use syscalls without leaving
   gint for better performance. */
void gint_unload(void);

/* gint_pause() - return to main menu, with possibility of coming back

   This function safely invokes the calculator's main menu by unloading gint.
   If the user selects the gint application again in the menu, this function
   reloads gint and returns. Otherwise, the add-in is fully unloaded by the
   system and the application terminates.

   This function is typically called when the [MENU] key is pressed during a
   getkey() call. */
void gint_pause(void);

//---
//	Public functions
//---

/* gint_intlevel() - configure the level of interrupts

   This function changes the interrupt level of the requested interrupt. Make
   sure you are aware of interrupt assignments to avoid breaking other code.
   This function is mainly used by drivers to enable the interrupts that they
   support.

   The first parameter 'intid' identifies an interrupt by its position in the
   sequence:

     IPRA & 0xf000 ; IPRA & 0x0f00 .. IPRA & 0x000f ; IPRB & 0xf000 ..

   For instance ID 7 refers to the low nibble of IPRB. These IDs and the range
   for which there are valid is heavily platform-dependent and any call to this
   function should be wrapped inside an MPU type check. This function will
   crash if the provided interrupt ID is invalid.

   The interrupt level should be in the range 0 (disabled) .. 15 (highest
   priority).

   @intid  Interrupt ID of the targeted interrupt
   @level  Requested interrupt level
   Returns the interrupt level that was assigned before the call. */
int gint_intlevel(int intid, int level);

/* gint_inthandler() - configure interrupt handlers

   This function installs (copies) interrupt handlers in the VBR space of the
   application. Each handler is a 32-byte block aligned on a 32-byte boundary.
   When an interrupt request is accepted, the hardware jumps to a specific
   interrupt handler at an address that depends on the interrupt source.

   For safety, interrupt handlers should avoir referring to data from other
   blocks because the arrangement of blocks at runtime depends on event codes.
   The assembler program will assume that consecutive blocks in the source code
   will be consecutive in memory, which is not always true. Avoiding cross-
   references is a practical rule to avoid problems. (gint breaks this rule
   very often but does it carefully... I guess?)

   This function allows anyone to replace any interrupt handler so make sure
   you're not interfering with usual interrupt assignments.

   The first parameter 'event_code' represents the event_code associated with
   the interrupt. If it's not a multiple of 0x20 then you're doing something
   wrong. The codes are normally platform-dependent, but gint always uses
   SH7305 codes: SH3 platforms have a translation table. See the documentation
   for a list of event codes and their associated interrupts.

   The handler function must be an interrupt handler: it must not raise
   exceptions, must end with 'rte', and it will use the kernel register bank.
   For convenience I allow any block size to be loaded as an interrupt handler,
   but it should really be a multiple of 0x20 bytes and not override other
   handlers. If it's not written in assembler, then you're likely doing
   something wrong, even with __attribute__((interrupt_handler)).

   It is common for interrupt handlers to have a few bytes of data, such as the
   address of a callback function. gint often stores this data in the last
   bytes of the block. This function returns the VBR address of the block to
   allow the caller to edit the parameters.

   @event_code  Identifier of the interrupt block
   @handler     Address of handler function
   @size        How many bytes to copy
   Returns the VBR address where the handlers was installed. */
void *gint_inthandler(int event_code, const void *handler, size_t size);

#endif /* GINT_GINT */
