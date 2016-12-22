#include "gintdemo.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <internals/timer.h>
#include <timer.h>
#include <display.h>
#include <keyboard.h>
#include <clock.h>
#include <mpu.h>
#include <rtc.h>

static void draw(int new_tab);
static struct ClockConfig conf;

//---
//	Timer-RTC comparison.
//	The precision of the timer is measured by comparing it to the RTC.
//---

static volatile int elapsed_timer = -1;
static volatile int elapsed_rtc = -1;
static int cb_id = -1;

static void timing_rtc(void)
{
	elapsed_rtc++;
}

static void timing_timer(void)
{
	elapsed_timer++;
}

static void timing_start(void)
{
	timer_start(TIMER_USER, 64, Clock_Hz, timing_timer, 0);
	rtc_cb_edit(cb_id, RTCFreq_64Hz, timing_rtc);

	elapsed_timer = 0;
	elapsed_rtc = 0;
}



//---
//	Drawing.
//---

/*
	small_text()
	Renders small text using a minimalist bitmap-based font.
*/
static void small_text(int x, int y, const char *text, int alignment)
{
	extern Image res_clock_chars;
	Image *chars = &res_clock_chars;
	const char *table = "0123456789kMHz*/";

	if(alignment) x -= 2 * strlen(text) - 1, y -= 2;
	int c;

	while(*text)
	{
		const char *ptr = strchr(table, *text++);
		if(!ptr) continue;
		c = ptr - table;

		dimage_part(x, y, chars, c << 2, 0, 3, 5);
		x += 4;
	}
}

/*
	getFreq()
	Prints the given frequency in a string on the form:
		332kHz
	There are 1, 2 or 3 characters for the value, and 2 or 3
	characters for the unit. The string is compacted.
*/
void getFreq(char *str, int freq)
{
	if(freq < 1000) sprintf(str, "%dHz", freq);
	else if(freq < 1000000) sprintf(str, "%dkHz", (freq + 500) / 1000);
	else sprintf(str, "%dMHz", (freq + 500000) / 1000000);
}

/*
	dislay_freq()
	Displays a frequency value a unit, in an simple form.
*/
static void display_freq(int x, int y, int freq)
{
	int ratio, letter, dot, i;
	char buffer[10];

	if(freq <= 0)
	{
		dtext(x, y, "Unknown");
		return;
	}
	if(freq < 10000)
	{
		dprint(x, y, "%5d", freq);
		small_text(x + 31, y + 2, "Hz", 0);
		return;
	}

	if(freq < 10000000) ratio = 1, letter = 'k';
	else ratio = 1000, letter = 'M';

	dot = 1 + (freq >= 10000 * ratio) + (freq >= 100000 * ratio);
	freq += (ratio * (1 + 9 * (dot >= 2) + 90 * (dot >= 3))) / 2;
	snprintf(buffer, 6, "%d", freq);

	for(i = 4; i > dot; i--) buffer[i] = buffer[i - 1];
	buffer[dot] = '.';

	dprint(x, y, buffer);
	sprintf(buffer, "%cHz", letter);
	small_text(x + 31, y + 2, buffer, 0);
}

/*
	draw()
	Draws the test interface.
*/
static void draw(int tab)
{
	extern Image res_opt_timer;
	extern Image res_clock_7705;
	extern Image res_clock_7305;

	char buffer[16];

	dclear();
	dimage(0, 56, &res_opt_timer);

	if(!tab)
	{
		locate(1, 1, "Clock frequency");
		dtext(7, 20, "B\x1e");
		display_freq(24, 20, conf.Bphi_f);
		dtext(7, 28, "I\x1e");
		display_freq(24, 28, conf.Iphi_f);
		dtext(7, 36, "P\x1e");
		display_freq(24, 36, conf.Pphi_f);

		if(isSH3())
		{
			dimage(64, 0, &res_clock_7705);

			getFreq(buffer, conf.CKIO_f);
			small_text(84, 16, buffer, 1);	

			sprintf(buffer, "*%d", conf.PLL1);
			small_text(84, 34, buffer, 1);

			if(conf.Iphi_div1 == 1)
				dline(85, 43, 99, 43, Color_Black);
			else
			{
				sprintf(buffer, "/%d", conf.Iphi_div1);
				small_text(89, 41, buffer, 0);
			}
			if(conf.Pphi_div1 == 1)
				dline(85, 50, 99, 50, Color_Black);
			else
			{
				sprintf(buffer, "/%d", conf.Pphi_div1);
				small_text(89, 48, buffer, 0);
			}
		}
		else
		{
			dimage(64, 0, &res_clock_7305);

			getFreq(buffer, conf.RTCCLK_f);
			small_text(82, 13, buffer, 1);
	
			sprintf(buffer, "*%d", conf.FLL);
			small_text(82, 23, buffer, 1);

			sprintf(buffer, "*%d", conf.PLL);
			small_text(82, 33, buffer, 1);

			sprintf(buffer, "/%d", conf.Bphi_div1);
			small_text(87, 40, buffer, 0);
			sprintf(buffer, "/%d", conf.Iphi_div1);
			small_text(87, 47, buffer, 0);
			sprintf(buffer, "/%d", conf.Pphi_div1);
			small_text(87, 54, buffer, 0);
		}
	}
	else
	{
		int timer = elapsed_timer, rtc = elapsed_rtc; // just in case

		locate(1, 1, "Timer/RTC comparison");

		locate(2, 3, "Timer");
		if(timer >= 0) print(12, 3, "%04x", timer);
		else locate(12, 3, "...");

		locate(2, 4, "RTC");
		if(rtc >= 0) print(12, 4, "%04x", rtc);
		else locate(12, 4, "...");

		// We define the accuracy of the timer as the ratio between the
		// two counters.
		locate(2, 5, "Accuracy");
		if(rtc > 0 && timer > 0)
		{
			int ratio;
			if(timer <= rtc) ratio = (10000 * timer) / rtc;
			else ratio = (10000 * rtc) / timer;

			print(12, 5, "%d.%02d%%", ratio / 100, ratio % 100);
		}
		else locate(12, 5, "...");
	}

	dupdate();
}



//---
//	Timer/clock test.
//---

/*
	test_timer()
	Clock timer and timer precision.
*/
void test_timer(void)
{
	int tab = 0;

	conf = clock_config();

	elapsed_timer	= -1;
	elapsed_rtc	= -1;
	cb_id = rtc_cb_add(RTCFreq_64Hz, timing_start, 0);

	text_configure(NULL, Color_Black);

	while(1)
	{
		draw(tab);

		switch(getkey_opt(Getkey_NoOption, 1))
		{
		case KEY_EXIT:
			timer_stop(TIMER_USER);
			rtc_cb_end(cb_id);
			return;

		case KEY_F1:
			tab = 0;
			break;
		case KEY_F2:
			tab = 1;
			break;
		}
	}
}
