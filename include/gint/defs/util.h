//---
//	gint:defs:util - Various utility macros
//---

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

//---
//	Const casts
//---

/* const_cast() - perform const casts
   This is intended for initialization purposes only, like "final" in Java.
   This macro is the most generic form without any type inference; using
   const_cint() or const_cptr() may be more convenient for simple types */
#define const_cast(x, T) (*((T *)(&(x))))

/* const_cptr() - perform const casts on pointers */
#define const_cptr(x) (*((void **)(&(x))))

/* const_cint() - perform const casts on integers
   This macro infers the integer type it's using, which may avoid errors when
   types change during code maintenance. It only works on primitive types and
   their aliases, this is on purpose to avoid integer coercion with tricks such
   as typeof(x + 0). */
#define const_cint(x) (*_Generic((x),					\
	char:			(char *)		(&(x)),		\
	unsigned char:		(unsigned char *)	(&(x)),		\
	short:			(short *)		(&(x)),		\
	unsigned short:		(unsigned short *)	(&(x)),		\
	int:			(int *)			(&(x)),		\
	unsigned int:		(unsigned int *)	(&(x)),		\
	long:			(long *)		(&(x)),		\
	unsigned long:		(unsigned long *)	(&(x)),		\
	long long:		(long long *)		(&(x)),		\
	unsigned long long:	(unsigned long long *)	(&(x))		\
))
