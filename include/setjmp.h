//---
//
//	gint standard module: setjmp
//
//	Long jumps. The register contents are saved in a buffer when setjmp()
//	is called and restored at any time when longjmp() performs the jump.
//
//---

#ifndef	_SETJMP_H
#define	_SETJMP_H

// There are 16 CPU registers that *must* be saved to ensure a basically
// safe jump.
typedef	unsigned int jmp_buf[16];



//---
//	Long jump functions.
//---

/*
	setjmp()	O(1)
	Configures a jump by saving data to the given jump buffer.
*/
int setjmp(jmp_buf env);

/*
	longjmp()	O(1)
	Performs a long jump.
*/
void longjmp(jmp_buf env, int value);

#endif	// _SETJMP_H
