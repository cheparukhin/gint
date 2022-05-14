//---
// gint:image:fixed - Minimal fixed-point interface for linear transformations
//---

#ifndef GINT_IMAGE_FIXED
#define GINT_IMAGE_FIXED

/* Constants */
#define fconst(x) ((x) * 65536)

/* Multiplication */
static inline int fmul(int x, int y)
{
    return ((int64_t)x * (int64_t)y) >> 32;
}

/* Multiplication with a scalar */
static inline int fmuls(int x, int s)
{
    return ((int64_t)x * (int64_t)s) >> 16;
}

/* Division */
static inline int fdiv(int x, int y)
{
    return ((int64_t)x << 16) / y;
}

#endif /* GINT_IMAGE_FIXED */
