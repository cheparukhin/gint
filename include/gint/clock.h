//---
//	gint:clock - Clock signals, overclock, and standby modes
//---

#ifndef GINT_CLOCK
#define GINT_CLOCK

#include <defs/types.h>

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
void sleep(void);

/* sleep_us() - sleep for a definite duration in microseconds

   Stops the processor until [delay_us] microseconds have elapsed. Interrupts
   may occur during that time (especially timers firing), in which case the
   events will be treated as usual. The processor will resume sleeping after
   handling them.

   The user may choose the timer used to time out the sleep. Remember that only
   timers 0 to 2 have microsecond-level resolution; other timers count in units
   of about 30 us.

   @timer     Which timer to use to time out the sleep
   @us_delay  How long to sleep (in microseconds) */
void sleep_us(int timer, int us_delay);

/* sleep_ms() - sleep for a definite duration in milliseconds */
#define sleep_ms(timer, ms_delay) sleep_us(timer, (ms_delay) * 1000)

#endif /* GINT_CLOCK */
