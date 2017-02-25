//---
//
//	gint demo application
//
//	Displays some tests cases for many features of the library.
//
//---

#ifndef	_GINTDEMO_H
#define	_GINTDEMO_H

//---
//	Main routines and common functions.
//---

/*
	main()
	No need for description.
*/
int main(void);

/*
	main_menu()
	Displays the main menu and returns user's choice by setting the
	category and application. Category is 0 when the user leaves the
	application.
*/
void main_menu(int *category, int *app);

/*
	locate()
	Displays text using a system-like monospaced font on a 21*8 grid.
*/
void locate(int x, int y, const char *str);

/*
	print()
	Locates a string using formatted printing.
*/
void print(int x, int y, const char *format, ...);



//---
//	Test applications.
//---

/*
	test_keyboard_events()
	Real-time keyboard management with events.
*/
void test_keyboard_events(void);

/*
	test_gray()
	Lets the user set the gray delays and see the results.
*/
void test_gray(void);

/*
	test_bopti()
	Displays and moves many kinds of bitmaps.
*/
void test_bopti(void);

/*
	test_tales()
	Displays some text using different modes and clipping options.
*/
void test_tales(void);

/*
	test_rtc()
	Just a clock.
*/
void test_rtc(void);

/*
	test_timer()
	Clock timer and timer precision.
*/
void test_timer(void);



//---
//	Performance applications.
//---

/*
	perf_bopti()
	Compares bopti and MonochromeLib.
*/
void perf_bopti(void);

/*
	perf_tales()
	Compares tales and the system's text rendering functions.
*/
void perf_tales(void);



//---
//	Debug applications.
//---

/*
	debug_tlb()
	On SH7705, displays the TLB contents. Does nothing on SH7305.
*/
void debug_tlb(void);

#endif	// _GINTDEMO_H
