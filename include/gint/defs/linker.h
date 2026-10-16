/* Numeric values exported by assembly and linker scripts. */
#ifndef GINT_DEFS_LINKER
#define GINT_DEFS_LINKER

#include <stdint.h>

/* A linker symbol can denote a size (including zero), an end marker, or an
   address; it is not necessarily a C object. Materialize its value through
   an opaque register operand so GCC/LTO cannot infer nonzero-ness, alignment,
   or distinct-object bounds from the declaration. This emits no instruction
   and is not a hardware or compiler memory barrier. */
static inline uintptr_t gint_linker_value(void const *symbol)
{
    uintptr_t value = (uintptr_t)symbol;
    __asm__("" : "+r"(value));
    return value;
}

#endif /* GINT_DEFS_LINKER */
