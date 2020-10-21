//---
//	gint:clock - Clock signals, overclock, and standby modes
//---

#ifndef GINT_CLOCK
#define GINT_CLOCK

#include <gint/defs/types.h>

//---
//	Clock signals
//---

/* clock_frequency_t
   A dump of the Clock Pulse Generator's (CPG) configuration. Use the MPU
   detection functions from <gint/mpu.h> to use the correct fields. */
typedef struct
{
	union {
		int PLL1;
		int FLL;
	};
	union {
		int PLL2;
		int PLL;
	};

	int Bphi_div;
	int Iphi_div;
	int Pphi_div;

	union {
		int CKIO_f;
		int RTCCLK_f;
	};

	int Bphi_f;
	int Iphi_f;
	int Pphi_f;

} clock_frequency_t;

/* clock_freq() - get the frequency of the main clocks
   This function returns the address of a static object which is used by the
   module; this address never changes. */
const clock_frequency_t *clock_freq(void);

//---
//	Overclock
//---

/* TODO: All overclock */

//---
//	Sleep functions
//---

/* sleep() - halt the processor until an event occurs
   The function stops the processor until an interrupt is accepted; the
   duration is not known in advance. This function should be used when the
   add-in is idle, for instance while waiting for keyboard input. */
#define sleep() __asm__("sleep")

/* sleep_us(): Sleep for a fixed duration in microseconds
   Stops the processor until the specified delay in microseconds has elapsed.
   (The processor will still wake up occasionally to handle interrupts.) This
   function selects a timer with timer_setup() called with TIMER_ANY. */
void sleep_us(uint64_t delay_us);

/* sleep_us_spin(): Actively sleep for a fixed duration in microseconds
   Like sleep_us(), but uses timer_spinwait() and does not rely on interrupts
   being enabled. Useful in timer code running without interrupts. */
void sleep_us_spin(uint64_t delay_us);

/* sleep_ms(): Sleep for a fixed duration in milliseconds */
#define sleep_ms(delay_ms) sleep_us((delay_ms) * 1000ull)

#endif /* GINT_CLOCK */
