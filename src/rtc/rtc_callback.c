#include <internals/rtc.h>
#include <rtc.h>
#include <mpu.h>

// Array holding callback informations.
struct rtc_cb cb_array[RTC_CB_ARRAY_SIZE] = { 0 };
// Callback identifier (unique).
static int unique_id = 1;
// Current RTC interrupt frequency.
static rtc_frequency_t rtc_freq = RTCFreq_None;
// 256-Hz tick count. This counter is stopped when no callback is registered.
static unsigned elapsed256 = 0;

/*
	rtc_cb_update()
	After successful registration or deletion of a callback, updates the
	RTC interrupt frequency stored in register RCR2. After update, the
	interrupt frequency is high enough to handle all callbacks, but nothing
	more (so that no time is wasted handling interrupts that occur too
	often).
*/
static void rtc_cb_update(void)
{
	rtc_frequency_t max = RTCFreq_None;
	int n;

	for(n = 0; n < RTC_CB_ARRAY_SIZE; n++) if(cb_array[n].id)
	{
		if(!max || (cb_array[n].freq && cb_array[n].freq < max))
			max = cb_array[n].freq;
	}

	if(rtc_freq == max) return;
	rtc_freq = max;

	volatile struct mod_rtc *RTC = isSH3() ? RTC_SH7705 : RTC_SH7305;
	RTC->RCR2.BYTE = (rtc_freq << 4) | 0x09;
}

/*
	rtc_cb_add()
	Registers a new callback for the RTC. Returns the callback id on
	success (positive integer), or one of the following error codes:
	  -1	Array is full
	  -2	Invalid parameter
	The number of repeats may be set to 0, in which case the callback is
	called indefinitely unless the user calls rtc_cb_end().
*/
int rtc_cb_add(rtc_frequency_t freq, void (*function)(void), int repeats)
{
	int n = 0;
	if(freq == RTCFreq_None || !function || repeats < 0) return -2;

	while(n < RTC_CB_ARRAY_SIZE && cb_array[n].id) n++;
	if(n >= RTC_CB_ARRAY_SIZE) return -1;

	cb_array[n].freq	= freq;
	cb_array[n].callback	= function;
	cb_array[n].repeats	= repeats;
	cb_array[n].id		= unique_id++;

	rtc_cb_update();

	return cb_array[n].id;
}

/*
	rtc_cb_end()
	Removes the callback with the given id (as returned by rtc_cb_add())
	from the callback array.
*/
void rtc_cb_end(int id)
{
	int n = 0;

	while(n < RTC_CB_ARRAY_SIZE && cb_array[n].id != id) n++;
	if(n >= RTC_CB_ARRAY_SIZE) return;

	cb_array[n].id		= 0;
	cb_array[n].freq	= RTCFreq_None;
	cb_array[n].callback	= NULL;
	cb_array[n].repeats	= 0;

	rtc_cb_update();
}

/*
	rtc_cb_edit()
	Changes information related to a callback. This function returns 0 on
	success, or one of the following error codes:
	  -1	Callback does not exist
	  -2	Invalid parameters
	This function never removes a callback. Call rtc_cb_end() for this.
*/
int rtc_cb_edit(int id, rtc_frequency_t new_freq,
	void (*new_function)(void))
{
	if(new_freq < 0 || new_freq > 7) return -2;
	int n = 0;

	while(n < RTC_CB_ARRAY_SIZE && cb_array[n].id != id) n++;
	if(n >= RTC_CB_ARRAY_SIZE) return -1;

	cb_array[n].freq	= new_freq;
	cb_array[n].callback	= new_function;
	rtc_cb_update();

	return 0;
}

/*
	rtc_cb_interrupt()
	Handles an RTC interrupt. Calls the RTC callbacks if necessary, and
	updates the repeat counts.
*/
void rtc_cb_interrupt(void)
{
	int n;

	int scales[] = {
		  1,	// 256 Hz
		  4,	//  64 Hz
		 16,	//  16 Hz
		 64,	//   4 Hz
		128,	//   2 Hz
		256,	//   1 Hz
		512,	// 0.5 Hz
	};
	// Adding to elapsed256 the number of 256-Hz ticks that correspond to
	// the current interrupt frequency, and rounding the result to a 
	// multiple of this tick number.
	elapsed256 += scales[rtc_freq - 1];
	elapsed256 &= ~(scales[rtc_freq - 1] - 1);

	for(n = 0; n < RTC_CB_ARRAY_SIZE; n++)
	{
		struct rtc_cb *cb = &cb_array[n];
		if(!cb->id || !cb->freq) continue;

		// Only execute callback when the number of elapsed 256-Hz
		// ticks reach a multiple that correspond to the callback
		// frequency.
		if(elapsed256 & (scales[cb->freq - 1] - 1)) continue;

		if(cb->callback) (*cb->callback)();
		if(cb->repeats)
		{
			if(cb->repeats == 1) rtc_cb_end(cb->id);
			else cb->repeats--;
		}
	}
}
