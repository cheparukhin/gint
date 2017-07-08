#include <math.h>

/*
	Quickly divides by predefined integers using a 64-bit multiplication
	technique. These functions should be ~10 times faster than dividing
	using opeator "/".
*/
__attribute__((always_inline))
inline qdiv_t qdiv(uint32_t n, uint32_t divider, uint32_t magic)
{
	qdiv_t result;

	__asm__(
		"dmuls.l %1, %2		\n\t"
		"sts     mach, %0	"
		: "=r"(result.q)
		: "r"(n), "r"(magic)
		: "macl", "mach"
	);

	result.r = n - divider * result.q;
	return result;
}
