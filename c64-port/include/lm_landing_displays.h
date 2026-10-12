/* SPDX-License-Identifier: MIT */
#ifndef LM_LANDING_DISPLAYS_H
#define LM_LANDING_DISPLAYS_H
#include "agc_transfer.h"
#include "lm_flight_io.h"
/* Luminary099 LANDING_ANALOG_DISPLAYS.agc, all of it (bank 21,
 * 02154-03076), as native routines of the control-transfer ABI; io is
 * lm_flight_io (channels 12, 14 and 30).
 *
 *   LANDISP   21,2154  the R10 waitlist task: TBASE2 and PIPCTR1, the
 *                      display flag, DISINDAT (mode switch, DIDFLAG, the
 *                      first-pass set-up to TC TWIDDLE), the altitude rate
 *                      (ARCOMP) or altitude extrapolation (ALTOUT, OLDDATA,
 *                      NEWDATA) to ALTM and channel 14, and SPEEDRUN's
 *                      velocity update to TC VARDELAY; to TASKOVER, TC
 *                      TWIDDLE or TC VARDELAY
 *   INTLZE    21,2365  TWIDDLE's task: channel 12 bit 2, IMODES33 bit 8;
 *                      TCF TASKOVER
 *   21,2364            TWIDDLE's return: TCF TASKOVER
 *   21,2476            VARDELAY's return: VHY, VHZ, LATVEL, FORVEL, the
 *                      velocity monitor (VMONITOR) to CDUTCMD/CDUSCMD and
 *                      channel 14, then TC LADQSAVE, which runs ALTROUT +1
 *                      or ALTOUT +1 when DISINDAT stored them there; to
 *                      TASKOVER, or Z = LADQSAVE for any other word
 *
 * Refused (EXEC_OUT_OF_SCOPE, image and io unchanged): an entry under an FB
 * other than bank 21 or an EB other than EBANK7 (EBANK= UNIT/R/), or a
 * missing io record. */
extern const agc_routine_table lm_landing_displays_routines;
/* The inline operand read through Q by TC TWIDDLE (ADRES INTLZE), as a
 * compiled site operand. */
#define LM_LAD_SITES 1u
extern const agc_site_operand lm_landing_displays_sites[LM_LAD_SITES];
#define LM_LAD_FB21 042000u
#define LM_LAD_EB 03400u
#define LM_LAD_LANDISP 02154u
#define LM_LAD_ALTROUT 02167u
#define LM_LAD_ALTOUT 02240u
#define LM_LAD_INTLZE 02365u
#define LM_LAD_TWIDRET 02364u
#define LM_LAD_VARRET 02476u
#define LM_LAD_ZERODATA 03050u
#define LM_LAD_BITS87 03076u
#define LM_LAD_FIRST 02154u
#define LM_LAD_LAST 03076u
#define LM_LAD_LADQSAVE 01714u
/* Where the file's calls leave it. */
#define LM_LAD_TASKOVER 05261u
#define LM_LAD_TWIDDLE 05173u
#define LM_LAD_VARDELAY 05224u
#endif
