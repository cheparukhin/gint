//---
// gint:cpu:atomic - Simulated atomic operations
//---

#include <gint/cpu.h>

/* Value of IMASK when atomic mode is entered */
static int saved_IMASK = 0;
/* Number of atomic mode levels (sort of mutex) */
static unsigned int atomic_level = 0;
/* Lock on (atomic_level) */
static char atomic_level_lock = 0;

void cpu_atomic_start(void)
{
	/* Get the lock on (atomic_level) */
	while(__atomic_test_and_set(&atomic_level_lock, __ATOMIC_RELAXED)) {}

	if(atomic_level == 0) {
		cpu_sr_t SR = cpu_getSR();
		saved_IMASK = SR.IMASK;
		SR.IMASK = 15;
		cpu_setSR(SR);
	}

	atomic_level++;

	/* Release the lock */
	__atomic_clear(&atomic_level_lock, __ATOMIC_RELAXED);
}

void cpu_atomic_end(void)
{
	while(__atomic_test_and_set(&atomic_level_lock, __ATOMIC_RELAXED)) {}

	atomic_level--;

	if(atomic_level == 0) {
		cpu_sr_t SR = cpu_getSR();
		SR.IMASK = saved_IMASK;
		saved_IMASK = 0;
		cpu_setSR(SR);
	}

	__atomic_clear(&atomic_level_lock, __ATOMIC_RELAXED);
}
