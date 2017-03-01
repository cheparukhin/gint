#ifndef	_INTERNALS_RTC_H
#define	_INTERNALS_RTC_H

#include <rtc.h>
#include <stddef.h>

#ifndef	RTC_CB_ARRAY_SIZE
#define	RTC_CB_ARRAY_SIZE	5
#endif

/*
	struct rtc_cb
	An RTC callback.
*/
struct rtc_cb
{
	rtc_frequency_t freq;
	int id;

	void (*callback)(void);
	int repeats;
};

// The callback array.
struct rtc_cb cb_array[RTC_CB_ARRAY_SIZE];

/*
	rtc_perodic_interrupt()
	Handles periodic interrupts and calls the callbacks.
*/
void rtc_periodic_interrupt(void);

/*
	rtc_cb_interrupt()
	Calls the RTC callbacks if necessary, and updates the repeat counts.
	Should only be called when RTC periodic interrupts occur.
*/
void rtc_cb_interrupt(void);

/*
	struct mod_rtc
	This structure describes the arrangement of RTC register in the memory.
	Curious thing, on SH7705, registers RYRAR and RCR3 are at a completely
	different address than the other ones. This module does not use these
	registers, so they were not included in the structure.
*/
#pragma pack(push, 1)

struct mod_rtc
{
	unsigned char const R64CNT;
	unsigned char _1;

	union {
		unsigned char BYTE;
		struct {
			unsigned		:1;
			unsigned TENS		:3;
			unsigned ONES		:4;
		};
	} RSECCNT;
	unsigned char _2;

	union {
		unsigned char BYTE;
		struct {
			unsigned		:1;
			unsigned TENS		:3;
			unsigned ONES		:4;
		};
	} RMINCNT;
	unsigned char _3;

	union {
		unsigned char BYTE;
		struct {
			unsigned		:2;
			unsigned TENS		:2;
			unsigned ONES		:4;
		};
	} RHRCNT;
	unsigned char _4;

	// 0 = Sunday, 1 = Monday, ..., 6 = Saturday, 7 = prohibited setting.
	unsigned char RWKCNT;
	unsigned char _5;

	union {
		unsigned char BYTE;
		struct {
			unsigned		:2;
			unsigned TENS		:2;
			unsigned ONES		:4;
		};
	} RDAYCNT;
	unsigned char _6;

	union {
		unsigned char BYTE;
		struct {
			unsigned		:3;
			unsigned TENS		:1;
			unsigned ONES		:4;
		};
	} RMONCNT;
	unsigned char _7;

	union {
		unsigned short WORD;
		struct {
			unsigned THOUSANDS	:4;
			unsigned HUNDREDS	:4;
			unsigned TENS		:4;
			unsigned ONES		:4;
		};
	} RYRCNT;
	unsigned char _8[12];

	union {
		unsigned char BYTE;
		struct {
			unsigned CF		:1;
			unsigned		:2;
			unsigned CIE		:1;
			unsigned AIE		:1;
			unsigned		:2;
			unsigned AF		:1;
		};
	} RCR1;
	unsigned char _9;

	union {
		unsigned char BYTE;
		struct {
			unsigned PEF		:1;
			unsigned PES		:3;
			unsigned		:1;
			unsigned ADJ		:1;
			unsigned RESET		:1;
			unsigned START		:1;
		};
	} RCR2;
} __attribute__((packed));

#pragma pack(pop)

/*
	We don't need to access the registers in a complicated way like the
	function of the timer module. Let's make it simple.
*/
#define	RTC_SH7705	((volatile struct mod_rtc *)0xfffffec0)
#define	RTC_SH7305	((volatile struct mod_rtc *)0xa413fec0)

#endif	// _INTERNALS_RTC_H
