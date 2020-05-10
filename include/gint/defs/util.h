//---
//	gint:defs:util - Various utility macros
//---

#ifndef GINT_DEFS_UTIL
#define GINT_DEFS_UTIL

/* synco instruction (in a form compatible with sh3eb-elf) */
#define synco() __asm__ volatile (".word 0x00ab":::"memory")

/* min(), max() (without double evaluation) */
#define	min(a, b) ({			\
	__auto_type _a = (a);		\
	__auto_type _b = (b);		\
	_a < _b ? _a : _b;		\
})
#define	max(a, b) ({			\
	__auto_type _a = (a);		\
	__auto_type _b = (b);		\
	_a > _b ? _a : _b;		\
})

/* sgn() (without double evaluation) */
#define	sgn(s) ({			\
	__auto_type _s = (s);		\
	_s < 0 ? -1 :			\
	_s > 0 ? +1 :			\
	0;				\
})

/* abs() (without double evaluation) */
#define abs(s) ({			\
	__auto_type _s = (s);		\
	_s < 0 ? -s : s;		\
})

/* swap() - exchange two variables of the same type */
#define	swap(a, b) ({			\
	__auto_type _tmp = (a);		\
	(a) = (b);			\
	(b) = _tmp;			\
})

#endif /* GINT_DEFS_UTIL */
