//---
//	gint:hardware - Platform information and hardware detection
//
//	This components centralizes detected information about the runtime
//	hardware, including MPU version, peripheral modules, and how drivers
//	configured them.
//
//	The most common use of this header is for the isSH3() and isSH4()
//	macros that let you run MPU-dependent jobs and are used like this:
//	  if(isSH3()) do_sh3();
//	  else do_sh4();
//---

#ifndef GINT_HARDWARE
#define GINT_HARDWARE

#include <gint/defs/types.h>

/* Most of the information here is going to be stored in (key, value) pairs for
   predetermined keys and 32-bits values that are often integers or a set of
   flags. The data will be filled by gint or its drivers. */

#define HW_KEYS 16
extern uint32_t gint[HW_KEYS];

/* MPU detection macros, with a faster version on fx-CG 50 and a generic
   dual-platform version for libraries.
   Warning: this macro is also used hardcoded in exch.s. */

#if defined(FX9860G) || (!defined(FX9860G) && !defined(FXCG50))
#define isSH3() (gint[HWMPU] & 1)
#define isSH4() (!isSH3())
#endif

#ifdef FXCG50
#define isSH3() 0
#define isSH4() 1
#endif

/* hw_detect(): Basic hardware detection
   This function probes the hardware and fills in the HWMPU, HWCPUVR and
   HWCPUPR fields. */
void hw_detect(void);

/* This bit should be set in all data longwords except HWMPU, HWCPUVR, HWCPUPR
   and HWCALC which are guaranteed to always be loaded. If not set then the
   information must be treated as invalid. */
#define HW_LOADED 0x80000000

/*
**  Key list
*/

#define HWMPU    0    /* MPU type */
#define HWCPUVR  1    /* CPU Version Register */
#define HWCPUPR  2    /* CPU Product Register */
#define HWCALC   3    /* Calculator model */
#define HWRAM    4    /* Amount of RAM */
#define HWROM    5    /* Amount of ROM */
#define HWURAM   6    /* Userspace RAM */
#define HWETMU   7    /* Extra Timer Units */
#define HWKBD    8    /* Keyboard */
#define HWKBDSF  9    /* Keyboard Scan Frequency (set iff HWKBD is loaded) */
#define HWDD     10   /* Display Driver */

/*
**  MPU type
*/

/* Unknown MPUs are all assumed to be SH-4A-based */
#define HWMPU_UNKNOWN  0
/* Used on original fx-9860G, SH-3-based */
#define HWMPU_SH7337   1
/* Used on recent fx-9860G derivatives such as the fx-9750G II, and also on the
   fx-CG 10/20/50. SH-4A-based */
#define HWMPU_SH7305   2
/* Used on the fx-9860G II, SH-3-based */
#define HWMPU_SH7355   3
/* Closest documented match to the SH7305, not used in any known calculator.
   Detected and included for reference only */
#define HWMPU_SH7724   4

/*
**  Calculator type
*/

/* SH-3-based fx-9860G-family */
#define HWCALC_FX9860G_SH3   1
/* Other SH-4A-based fx-9860G-family */
#define HWCALC_FX9860G_SH4   2
/* Graph 35+E II, an SH-4A French extension of the fx-9860G family */
#define HWCALC_G35PE2        3
/* fx-CG 10/20, also known as the "Prizm" family */
#define HWCALC_PRIZM         4
/* fx-CG 50, a late extension to the Prizm family */
#define HWCALC_FXCG50        5
/* fx-CG 50 emulator */
#define HWCALC_FXCG_MANAGER  6

/*
**  Extra Timer Units
*/

/* A single-timer ETMU unit was found. Correlated with SH3 */
#define HWETMU_1       0x01
/* A 6-timer ETMU unit was found. Correlated with SH4 */
#define HWETMU_6       0x02
/* Individual timer status. Not all timers might be operational after setting
   up the driver due to seemingly limitless behavioral differences with the
   TMU. Operational here means TCNT=TCOR=-1, interrupt disabled and cleared. */
#define HWETMU_OK0     0x04
#define HWETMU_OK1     0x08
#define HWETMU_OK2     0x10
#define HWETMU_OK3     0x20
#define HWETMU_OK4     0x40
#define HWETMU_OK5     0x80

/*
**  Keyboard
*/

/* The keyboard uses an I/O-port-based scan method. This is possible on both
   SH3 and SH4, but gint will normally do it only on SH3. */
#define HWKBD_IO       0x01
/* When using the I/O-port scanning method on SH3, whether the watchdog is used
   to delay I/O operations. */
#define HWKBD_WDD      0x02
/* The keyboard uses a KEYSC-based scan method. This is only possible on SH4 */
#define HWKBD_KSI      0x04

/*
**  Display Driver
*/

/* Display driver is known. This cannot be determined on fx9860g as the Toshiba
   T6K11 and its Graph 35+E II variant don't seem to have an identification
   command. It is set to 0 on fx9860g and used on fxcg50. */
#define HWDD_KNOWN     0x01
/* The display driver was configured to use the full screen, instead of leaving
   bands on the side. [fxcg50] */
#define HWDD_FULL      0x02
/* The contrast address for this OS version is known. [fx9860g] */
#define HWDD_CONTRAST  0x04
/* Backlight management is supported. This is used both on fx9860g models with
   back-lit screen (although that very fact cannot be detected) and fxcg50. */
#define HWDD_LIGHT     0x08

#endif /* GINT_HARDWARE */
