//---
// gint:std:endian - Endianness conversion
//---

#ifndef GINT_STD_ENDIAN
#define GINT_STD_ENDIAN

#include <gint/defs/types.h>
#include <gint/defs/attributes.h>

/* CASIO calculators are configured as big-endian. */

#define htobe16(x) (x)
#define htole16(x) (__builtin_bswap16(x))
#define be16toh(x) (x)
#define le16toh(x) (__builtin_bswap16(x))

#define htobe32(x) (x)
#define htole32(x) (__builtin_bswap32(x))
#define be32toh(x) (x)
#define le32toh(x) (__builtin_bswap32(x))

#define htobe64(x) (x)
#define htole64(x) (__builtin_bswap64(x))
#define be64toh(x) (x)
#define le64toh(x) (__builtin_bswap64(x))

#endif /* GINT_STD_ENDIAN */
