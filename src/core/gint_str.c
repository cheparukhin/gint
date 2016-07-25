#include <internals/gint.h>

const char *gint_str[] = {
	"Unknown",

	// User breaks.
	"User break (before)",
	"User break (after)",
	"User breakpoint",

	// General.
	"Inst. address error",
	"Data access error",
	"Illegal instruction",
	"Illegal slot",
	"Data address (r)",
	"Data address (w)",

	// Instruction TLB.
	"Inst. TLB miss",
	"Inst. TLB invalid",
	"Inst. TLB protect.",

	// Data TLB.
	"Data TLB miss",
	"Data TLB miss (r)",
	"Data TLB miss (w)",
	"Data TLB protection",
	"Data TLB prot. (r)",
	"Data TLB prot. (w)",
	"Data TLB invalid",

	// Others.
	"Initial page write",
	"Trap",
	"DMA address error",
};
