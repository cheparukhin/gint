#include "gintdemo.h"
#include <display.h>
#include <rtc.h>
#include <keyboard.h>

#include <stddef.h>
#include <ctype.h>

/*
	test_rtc()
	Just a clock. Of course using all this RTCTime conversion and this / 10
	is awfully un-optimized, but it's a test case so it's made to check the
	values in the structure are correct.
*/

#include <internals/rtc.h>
#include <mpu.h>

static void draw(struct RTCTime time)
{
	extern Image res_rtc_segments_start;

	const char *days[7] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	}, *months[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep",
		"Oct", "Nov", "Dec"
	};
	int x[6] = { 20, 33, 52, 65, 84, 97 };
	int digits[6];
	int i;

	digits[0] = time.hours / 10;
	digits[1] = time.hours % 10;
	digits[2] = time.minutes / 10;
	digits[3] = time.minutes % 10;
	digits[4] = time.seconds / 10;
	digits[5] = time.seconds % 10;

	// Drawing digits.
	for(i = 0; i < 6; i++) dimage_part(x[i], 8, &res_rtc_segments_start,
		12 * digits[i], 0, 11, 19);
	// Drawing ':' between pairs of digits.
	for(i = 0; i < 16; i++) dpixel(47 + 32 * (i >= 8) + (i & 1),
		14 + 5 * !!(i & 4) + !!(i & 2), Color_Black);

	// This should print time.year + 1900 but for the sake of this demo we
	// have tweaked the field so that it already contains time.year + 1900.
	print(4, 6, "%s %s %02d %4d", days[time.week_day],
		months[time.month], time.month_day, time.year);
}

static void callback(void)
{
	extern Image res_opt_rtc_start;
	struct RTCTime time = rtc_getTime();
	time.year += 1900;

	dclear();
	draw(time);
	dimage_part(0, 56, &res_opt_rtc_start, 0, 0, 19, 8);
	dupdate();
}

static void set_region(struct RTCTime *time, int region, int value)
{
	switch(region)
	{
	case 0:
		time->hours = 10 * value + (time->hours % 10);
		break;
	case 1:
		time->hours = time->hours - (time->hours % 10) + value;
		break;
	case 2:
		time->minutes = 10 * value + (time->minutes % 10);
		break;
	case 3:
		time->minutes = time->minutes - (time->minutes % 10) + value;
		break;
	case 4:
		time->seconds = 10 * value + (time->seconds % 10);
		break;
	case 5:
		time->seconds = time->seconds - (time->seconds % 10) + value;
		break;
	case 6:
		time->week_day = value;
		break;
	case 7:
		time->month = value;
		break;
	case 8:
		time->month_day = 10 * value + (time->month_day % 10);
		break;
	case 9:
		time->month_day = time->month_day - (time->month_day % 10)
			+ value;
		break;
	case 10:
		time->year = 1000 * value + (time->year % 1000);
		break;
	case 11:
		time->year = time->year - (time->year % 1000) + 100 * value
			+ (time->year % 100);
		break;
	case 12:
		time->year = time->year - (time->year % 100) + 10 * value
			+ (time->year % 10);
		break;
	case 13:
		time->year = time->year - (time->year % 10) + value;
		break;
	}
}

static void set(void)
{
	extern Image res_opt_rtc_start;
	Image *opt = &res_opt_rtc_start;

	struct {
		int x, y;
		int w, h;
	} regions[] = {
		{ 19,  7, 13, 21 }, {  32,  7, 13, 21 }, { 51,  7, 13, 21 },
		{ 64,  7, 13, 21 }, {  83,  7, 13, 21 }, { 96,  7, 13, 21 },
		{ 18, 39, 19,  9 }, {  42, 39, 19,  9 }, { 66, 39,  7,  9 },
		{ 72, 39,  7,  9 }, {  84, 39,  7,  9 }, { 90, 39,  7,  9 },
		{ 96, 39,  7,  9 }, { 102, 39,  7,  9 },
	};
	struct RTCTime time = rtc_getTime();
	int region_count = 14;
	int n = 0, slide = 0, key, leave;

	time.year += 1900;

	while(1)
	{
		dclear();
		draw(time);
		dreverse_area(regions[n].x, regions[n].y, regions[n].x
			+ regions[n].w - 1, regions[n].y + regions[n].h - 1);

		if(n == 6) dimage_part(0, 56, opt, 0, 9 * (1 + slide), 128, 8);
		if(n == 7) dimage_part(0, 56, opt, 0, 9 * (3 + slide), 128, 8);
		else dimage_part(0, 56, opt, 22 + 22 * (n == region_count - 1),
			0, 19, 8);

		dupdate();

		do
		{
			leave = 1;
			key = getkey();
			if(key == KEY_EXIT) return;

			else if(key == KEY_F1 || key == KEY_EXE)
			{
				n++;
				slide = 0;
				if(n == region_count)
				{
					time.year -= 1900;
					rtc_setTime(time);
					return;
				}
			}

			else if(key == KEY_F6)
			{
				if(n == 6) slide = (slide + 1) % 2;
				if(n == 7) slide = (slide + 1) % 3;
			}

			else if((key & 0x0f) == 9) // Other F-keys
			{
				int k = 7 - (key >> 4); // Number of F-key

				if(n == 7)
				{
					int month = k + 4 * slide - 1;
					set_region(&time, n, month);
					n++;
					slide = 0;
				}

				else if(n == 6 && (slide != 1 || k != 5))
				{
					int day = k + 4 * slide - 1;
					set_region(&time, n, day);
					n++;
					slide = 0;
				}

				else leave = 0;
			}

			else if(isdigit(keychar(key))) // Numbers
			{
				int val = keychar(key) - '0';
				int ok = 1;

				if(n == 0) ok = (val <= 2);
				if(n == 1)
				{
					int max = time.hours >= 20 ? 3 : 9;
					ok = (val <= max);
				}
				if(n == 2 || n == 4) ok = (val <= 5);
				if(n == 8) ok = (val <= 3);
				if(n == 9)
				{
					int max = time.month_day >= 30 ? 1 : 9;
					ok = (val <= max);
				}

				if(ok)
				{
					set_region(&time, n, val);
					n++;
					if(n == region_count)
					{
						time.year -= 1900;
						rtc_setTime(time);
						return;
					}
					slide = 0;
				}
				else leave = 0;
			}

			else leave = 0;
		} while(!leave);
	}

	while(getkey() != KEY_EXIT);
}

void test_rtc(void)
{
	int key;

	rtc_setCallback(callback, RTCFreq_1Hz);
	callback();

	while(1)
	{
		key = getkey();

		if(key == KEY_EXIT) break;
		if(key == KEY_F1)
		{
			rtc_setCallback(NULL, RTCFreq_1Hz);
			set();
			callback();
			rtc_setCallback(callback, RTCFreq_1Hz);
		}
	}

	rtc_setCallback(NULL, RTCFreq_1Hz);
}
