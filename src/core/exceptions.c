#include <internals/exceptions.h>
#include <display.h>
#include <string.h>

//---
//	On-screen error messages
//---

#define print(x, y, str) dtext(6 * (x) - 5, 8 * (y) - 7, str)

static void print_hex(int x, int y, uint32_t n, int digits)
{
	#define hexdigit(n) ((n) + '0' + 39 * ((n) > 9))

	char str[9];
	if(digits >= 8) digits = 8;
	str[digits] = 0;

	while(digits)
	{
		str[digits - 1] = hexdigit(n & 0xf);
		n >>= 4;
		digits--;
	}

	print(x, y, str);

	#undef	hexdigit
}

static void show_error(const char *name, uint32_t *access_mode, uint32_t *tea,
	uint32_t *opcode)
{
	uint32_t *vram = display_getCurrentVRAM();
	uint32_t spc, ssr;
	__asm__("stc spc, %0" : "=r"(spc));
	__asm__("stc ssr, %0" : "=r"(ssr));

	dclear();
	text_configure(NULL, color_black);

	print(3, 1, "EXCEPTION RAISED!");
	for(int i = 0; i < 36; i++) vram[i] = ~vram[i];
	print(1 + ((21 - strlen(name)) >> 1), 2, name);

	print(4, 4, "PC");
	print_hex(7, 4, spc, 8);
	print(4, 5, "SR");
	print_hex(7, 5, ssr, 8);

	int y = 6;
	if(access_mode)
	{
		print(2, y, "Type");
		print(7, y++, *access_mode == 1 ? "Code/Data read" :
			"Data write");
	}
	if(tea)
	{
		print(3, y, "TEA");
		print_hex(7, y++, *tea, 8);
	}
	if(opcode)
	{
		print(2, y, "Code");
		print_hex(7, y++, *opcode, 4);
	}

	dupdate();
	while(1);
}

#undef	print



//---
//	Exception handlers
//---

#pragma GCC diagnostic ignored "-Wunused-parameter"

/*
	exch_address_error()
	CPU address error, e.g. alignment issues.
*/
void exch_address_error(uint32_t pc, uint32_t tea, uint32_t access)
{
	show_error("Address error", &access, &tea, NULL);
}

/*
	exch_tlb_protection_violation()
	You don't have the right to access this address.
*/
void exch_tlb_protection_violation(uint32_t pc, uint32_t tea, uint32_t access)
{
	show_error("TLB protection", &access, &tea, NULL);
}

/*
	exch_tlb_invalid()
	The translation info for this address is marked as invalid.
*/
void exch_tlb_invalid(uint32_t pc, uint32_t tea, uint32_t access)
{
	show_error("TLB invalid", &access, &tea, NULL);
}

/*
	exch_illegal_instruction()
	What's this opcode anyway?
*/
void exch_illegal_instruction(uint32_t pc, uint32_t opcode)
{
	show_error("Illegal instruction", NULL, NULL, &opcode);
}

/*
	exch_illegal_slot()
	You can't execute this in a delay slot.
*/
void exch_illegal_slot(uint32_t pc, uint32_t opcode)
{
	show_error("Illegal slot", NULL, NULL, &opcode);
}

/*
	exch_user_break()
	One of the user break conditions you requested was fulfilled.
*/
void exch_user_break(void)
{
	show_error("User break", NULL, NULL, NULL);
}

/*
	exch_initial_page_write()
	You can't write to this memory page, it's too early.
*/
void exch_initial_page_write(void)
{
	show_error("Initial page write", NULL, NULL, NULL);
}

/*
	exch_trap()
	You asked for it.
*/
void exch_trap(uint32_t pc, uint32_t trap)
{
}

/*
	exch_dma_address()
	The DMAC is accessing badly-aligned addresses.
*/
void exch_dma_address(void)
{
	show_error("DMA address error", NULL, NULL, NULL);
}

/*
	exch_tlb_miss()
	This virtual address points nowhere.
*/
void exch_tlb_miss(uint32_t pc, uint32_t tea, uint32_t access)
{
	show_error("TLB miss", &access, &tea, NULL);
}

#pragma GCC diagnostic pop
