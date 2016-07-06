#include <gint.h>
#include <mpu.h>

static void (*rtc_callback)(void) = NULL;

/*
	gint_setRTCCallback()
	Sets the callback function for the real-time clock interrupt. If
	frequency is non-NULL, the clock frequency is set to the given value.

	@arg	callback	Callback function.
	@arg	frequency	Interrupt frequency.
*/
void gint_setRTCCallback(void (*callback)(void), enum RTCFrequency frequency)
{
	if(frequency < 1 || frequency > 7) return;
	rtc_callback = callback;

	if(isSH3())
		gint_setRTCFrequency_7705(frequency);
	else
		gint_setRTCFrequency_7305(frequency);
}

/*
	gint_getRTCCallback()
	Returns the callback function. If frequency is non-NULL, it is set to
	the current frequency value.
*/
void (*gint_getRTCCallback(enum RTCFrequency *frequency))(void)
{
	if(!frequency) return rtc_callback;

	if(isSH3())
		*frequency = gint_getRTCFrequency_7705();
	else
		*frequency = gint_getRTCFrequency_7305();

	return rtc_callback;
}
