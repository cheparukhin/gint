//---
//	gint - An alternative runtime environment for fx9860g and fxcg50
//---

#ifndef GINT_GINT
#define GINT_GINT

#include <gint/defs/types.h>
#include <gint/config.h>

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
extern int (*gint_inth_callback)(int (*function)(void *arg), void *arg);

//---
// Callback functions
//---

/* gint_callback_arg_t: All types of arguments allowed in a callback
   Other types can be used if casted, notably pointers to custom types can be
   casted to (void *). */
typedef union {
	/* Integer types */
	int i;
	unsigned int u;
	int32_t i32;
	uint32_t u32;
	/* 4-byte floating-point type */
	float f;
	/* Pointer to void */
	void *pv;
	void const *pv_c;
	void volatile *pv_v;
	void volatile const *pv_cv;
	/* Pointer to int */
	int *pi;
	int const *pi_c;
	int volatile *pi_v;
	int volatile const *pi_cv;
	/* Pointer to unsigned int */
	unsigned int *pu;
	unsigned int const *pu_c;
	unsigned int volatile *pu_v;
	unsigned int volatile const *pu_cv;
	/* Pointer to int32_t */
	int32_t *pi32;
	int32_t const *pi32_c;
	int32_t volatile *pi32_v;
	int32_t volatile const *pi32_cv;
	/* Pointer to uint32_t */
	uint32_t *pu32;
	uint32_t const *pu32_c;
	uint32_t volatile *pu32_v;
	uint32_t volatile const *pu32_cv;
	/* Pointer to float */
	float *pf;
	float const *pf_c;
	float volatile *pf_v;
	float volatile const *pf_cv;
	/* Pointer to double */
	double *pd;
	double const *pd_c;
	double volatile *pd_v;
	double volatile const *pd_cv;

} gint_callback_arg_t;

/* gint_callback_t: Callback function with up to 4 register arguments */
typedef struct {
	void *function;
	gint_callback_arg_t args[4];
} gint_callback_t;

/* GINT_CB(): Build a callback object from function and arguments

   This macro builds a callback object (of type gint_callback_t). Callback
   objects are used in various APIs (timers, RTC, DMA, USB...) to notify the
   program of events that are caused by the hardware instead of the program.

   Callbacks are often called asynchronously, which means that the function
   setting up the callback finishes first, and then the callback is called
   later while some other part of the program is running. This is tricky,
   because in order to invoke the callback:

   * The code and arguments must still exist, even though the function that
     provided them has finished long ago;
   * The call ABI is lost as soon as we store parameters instead of
     syntactically performing a call in the code.

   For the first issue, the caller has to make sure that every pointer that is
   passed to the callback will still be valid when the callback is invoked; in
   particular, pointers to variables on the stack can ony be used if the
   callback is guaranteed to be called before the function ends (eg. if there
   is a synchronous wait in the function).

   For the second issue, gint's callback mechanism guarantees ABI compatibility
   by restricting the arguments that can be passed to the callback.

   * Only arguments that fit into registers can be passed. In practice, this
     mostly excludes 64-bit integer, double floating-point values, and custom
     structures. This way, there is a somewhat solid guarantee that the
     callback function will take arguments in r4...r7.
   * Only up to 4 arguments can be passed.
   * Only values of the types listed in gint_callback_arg_t can be passed.

   If you need to work around one of these limitations, pass a pointer to a
   structure containing your arguments (if the callback is invoked after the
   current function ends, make the structure static or global).

   If you need to pass a char or a short, cast to an int and have the callback
   function take an int. If you need to pass a pointer to a type not listed in
   gint_callback_arg_t (such as a structure), cast it to (void *); the callback
   function can still take a pointer to the custom type as argument.

   If the conditions for the callback to work are not met, the compiler will
   emit on of these two errors:

   * error: static assertion failed: "GINT_CB: too many arguments (maximum 4)"
     -> This is emitted if you have more than 4 arguments.
   * error: cast to union type from type not present in union
     -> This is emitted if you pass a parameter of an invalid type.

   Both are followed with a series of compiler notes mentioning the various
   macros defined below. */
#define GINT_CB(func, ...) \
	((gint_callback_t){ .function = func, .args = { \
		__VA_OPT__(GINT_CB_ARGS1(__VA_ARGS__)) \
	}})
#define GINT_CB_ARGS1(a1, ...) \
	(gint_callback_arg_t)(a1), __VA_OPT__(GINT_CB_ARGS2(__VA_ARGS__))
#define GINT_CB_ARGS2(a2, ...) \
	(gint_callback_arg_t)(a2), __VA_OPT__(GINT_CB_ARGS3(__VA_ARGS__))
#define GINT_CB_ARGS3(a3, ...) \
	(gint_callback_arg_t)(a3), __VA_OPT__(GINT_CB_ARGS4(__VA_ARGS__))
#define GINT_CB_ARGS4(a4, ...) \
	({ __VA_OPT__(_Static_assert(0, \
		"GINT_CB: too many arguments (maximum 4)");) \
	(gint_callback_arg_t)(a4); })

/* GINT_CB_NULL: Empty callback */
#define GINT_CB_NULL ((gint_callback_t){ .function = NULL, .args = {} })

/* GINT_CB_SET(): Callback that sets an integer to 1
   This is defined as a function to make sure the pointer is to an int. */
static void GINT_CB_SET_function(int volatile *pointer)
{
	(*pointer) = 1;
}
static GINLINE gint_callback_t GINT_CB_SET(int volatile *pointer)
{
	return GINT_CB(GINT_CB_SET_function, pointer);
}

/* GINT_CB_INC(): Callback that increments an integer */
static void GINT_CB_INC_function(int volatile *pointer)
{
	(*pointer)++;
}
static GINLINE gint_callback_t GINT_CB_INC(int volatile *pointer)
{
	return GINT_CB(GINT_CB_INC_function, pointer);
}

/* gint_callback_invoke(): Invoke a callback function */
static GINLINE int gint_callback_invoke(gint_callback_t cb)
{
	int (*f)(int r4, int r5, int r6, int r7) = cb.function;
	return f(cb.args[0].i, cb.args[1].i, cb.args[2].i, cb.args[3].i);
}

#endif /* GINT_GINT */
