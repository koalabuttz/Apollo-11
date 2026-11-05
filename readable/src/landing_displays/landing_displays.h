/* SPDX-License-Identifier: MIT */
/* LANDING_ANALOG_DISPLAYS.agc (Luminary099 pp. 898-907, bank 21): R10, the
 * landing analog displays. A waitlist task four times a second, LANDISP,
 * alternates the altitude rate (ALTROUT) with the altitude (ALTOUT) on the
 * ALT/ALT RATE meter (ALTM, channel 14 bit 3), and in between brings the
 * velocity up to date (SPEEDRUN) and drives the X-pointer, the forward and
 * lateral velocity meters, through the CDU drive counters (VMONITOR,
 * CDUTCMD and CDUSCMD). DISPRSET turns the displays off. The faithful C is
 * c64-port/src/lm_landing_displays.c.
 *
 * Each entry below runs until the code leaves the file: TCF TASKOVER, the
 * TC TWIDDLE that starts INTLZE, the TC VARDELAY in SPEEDRUN, or a TC
 * LADQSAVE whose word is not one of DISINDAT's two returns. The binding,
 * src/binding/landing_displays_routines.c, runs one entry per call, as the
 * faithful port does. */
#ifndef READABLE_LANDING_DISPLAYS_H
#define READABLE_LANDING_DISPLAYS_H

#include "../agc/agc.h"

#if LUMINARY099
void landisp(void);                 /* LANDISP, R10's task */
void intlze_minus_one(void);        /* INTLZE -1: TWIDDLE's return */
void intlze(void);                  /* INTLZE, the task TWIDDLE starts */
void speedrun_after_vardelay(void); /* SPEEDRUN +0102: VARDELAY's return */
#endif

#endif
