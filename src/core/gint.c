//---
//
//	gint core module: interrupt handler
//
//	Central point of the library. Controls the interrupt handler and
//	defines a few functions to configure callbacks for some interrupts.
//
//---

#include <internals/gint.h>
#include <internals/interrupt_maps.h>
#include <internals/exceptions.h>
#include <internals/interrupts.h>
#include <gint.h>
#include <mpu.h>

// Referencing the interrupt handlers to force ld to link them in (there should
// be a better way of achieving this, but I can't seem to find it).
__attribute__((used)) static void (*_exc)(void) = &gint_exc;
__attribute__((used)) static void (*_tlb)(void) = &gint_tlb;
__attribute__((used)) static void (*_int)(void) = &gint_int;



//---
//	Exception and interrupt handlers
//---

gint_interrupt_handler_t gint_handlers[] = {

	//---
	//	Resets and non-events
	//---

	{ NULL, NULL, 0, { 0x00, 0x00, 0x00 } },

	//---
	//	General exceptions
	//---

	// Address error.
	{ NULL, exch_address_error, 0,
		{  arg_pc,	arg_tea,	arg_subtype	} },
	// TLB protection violation.
	{ NULL, exch_tlb_protection_violation, 0,
		{  arg_pc,	arg_tea,	arg_subtype	} },
	// TLB Invalid (SH7705 only).
	{ NULL, exch_tlb_invalid, 0,
		{  arg_pc,	arg_tea,	arg_subtype	} },
	// Illegal instruction.
	{ NULL, exch_illegal_instruction, 0,
		{  arg_pc,	arg_opcode,	0x00		} },
	// Illegal opcode.
	{ NULL, exch_illegal_slot, 0,
		{  arg_pc,	arg_opcode,	0x00		} },
	// User break.
	{ NULL, exch_user_break, 0,
		{  0x00,	0x00,		0x00		} },
	// Initial page write.
	{ NULL, exch_initial_page_write, 0,
		{  0x00,	0x00,		0x00		} },
	// Unconditional trap.
	{ NULL, exch_trap, 0,
		{  arg_pc,	arg_trap,	0x00		} },
	// DMA address error.
	{ NULL, exch_dma_address, 0,
		{  0x00,	0x00,		0x00		} },

	//---
	//	TLB misses
	//---

	{ NULL, exch_tlb_miss,
		0, {  arg_pc,	arg_tea,	arg_subtype	} },

	//---
	//	Interrupt requests
	//---

	// Non-Maskable Interrupt.
	{ NULL, NULL, 0,
		{  0x00,	0x00,		0x00		} },
	// Timer underflow.
	{ NULL, inth_timer_underflow, 12,
		{  arg_subtype,	0x00,		0x00		} },
	// Timer channel 2 input capture (SH7705 only)
	{ NULL, NULL, 0,
		{  arg_timer_capt, 0x00,	0x00		} },
	// Real-time clock alarm interrupt.
	{ NULL, NULL, 0,
		{  0x00,	0x00,		0x00		} },
	// Real-time clock periodic interrupt.
	{ NULL, inth_rtc_periodic, 10,
		{  0x00,	 0x00,		0x00		} },
	// Real-time clock carry interrupt.
	{ NULL, NULL, 0,
		{  0x00,	0x00,		0x00		} },
};

/*
	gint_invoke()
	Invokes an interrupt or exception handler, given its type and subtype.
*/
void gint_invoke(uint8_t type, uint8_t subtype)
{
	if(type >= exc_type_max || !gint_handlers[type].function) return;

	volatile uint32_t *tea, *tra, *tcpr_2;
	uint32_t args[3];
	uint16_t *pc;

	// Getting some initial information to work with.
	__asm__("stc spc, %0" : "=rm"(pc));
	tea = gint_reg(register_tea);
	tra = gint_reg(register_tra);
	tcpr_2 = (void *)0xfffffeb8;

	// Building up the argument list.
	for(int i = 0; i < 3; i++) switch(gint_handlers[type].args[i])
	{
	case arg_none:		args[i] = 0;		break;
	case arg_subtype:	args[i] = subtype;	break;
	case arg_pc:		args[i] = (uint32_t)pc;	break;
	case arg_opcode:	args[i] = *pc;		break;
	case arg_tea:		args[i] = *tea;		break;
	case arg_trap:		args[i] = *tra >> 2;	break;
	case arg_timer_capt:	args[i] = *tcpr_2;	break;
	}

	// Calling the function with the required amount of arguments.
	if(gint_handlers[type].args[2])
	{
		void (*handler)(uint32_t, uint32_t, uint32_t);
		handler = gint_handlers[type].function;
		(*handler)(args[0], args[1], args[2]);
	}
	else if(gint_handlers[type].args[1])
	{
		void (*handler)(uint32_t, uint32_t);
		handler = gint_handlers[type].function;
		(*handler)(args[0], args[1]);
	}
	else if(gint_handlers[type].args[0])
	{
		void (*handler)(uint32_t);
		handler = gint_handlers[type].function;
		(*handler)(args[0]);
	}
	else (*(void (*)(void))gint_handlers[type].function)();
}

/*
	gint_install()
	Installs an exception or interrupt handler for one of gint's recognized
	interrupts.
*/
void gint_install(gint_interrupt_type_t type, void *function)
{
	if((unsigned)type >= exc_type_max) return;
	gint_handlers[type].function = function;
}

/*
	gint_uninstall()
	Uninstalls the exception or interrupt handler that was used for the
	given interrupt, falling back to gint's default handler.
*/
void gint_uninstall(gint_interrupt_type_t type)
{
	if((unsigned)type >= exc_type_max) return;
	gint_handlers[type].function = gint_handlers[type].default_function;
}



//---
//	Register access
//---

/*
	gint_reg()
	Returns the address of a common register. All common registers exist
	on both platforms but they may hold different values for the same
	information.
*/
volatile void *gint_reg(gint_register_t reg)
{
	return isSH3() ? gint_reg_7705(reg) : gint_reg_7305(reg);
}
