/* SPDX-License-Identifier: MIT */
/* THROTTLE_CONTROL_ROUTINES.agc (Luminary099 pp. 793-797, bank 31): the
 * descent engine's throttle. THROTTLE turns the guidance's acceleration
 * (/AFC/) and the present one (ABDELV) into thrusts in pulse units (FC and
 * FP), keeps the command inside the throttleable bands, and sends the
 * difference, PIF, to the engine (THRUST, channel 14 bit 4). FLATOUT and
 * WHATOUT set the throttle outright. The faithful C is
 * c64-port/src/lm_throttle.c.
 *
 * Each entry below ends where its code leaves it: at a TC out of the file
 * (Z the routine, Q the return), or at the TC RTNHOLD or TC BUF that
 * returns to the caller (Z the caller's return). The binding,
 * src/binding/throttle_routines.c, runs one entry per call, as the
 * faithful port does. */
#ifndef READABLE_THROTTLE_H
#define READABLE_THROTTLE_H

#include "../agc/agc.h"

#if LUMINARY099
void throttle(void);                     /* THROTTLE */
void throttle_plus3(void);               /* THROTTLE +3 */
void afdump_after_massmult(void);        /* the first MASSMULT's return: FP */
void afdump_after_second_massmult(void); /* the second's: FC, to DOPIF */
void dopif_after_fastchng(void);         /* FASTCHNG's return: PIF, DOIT */
void flatout(void);                      /* FLATOUT */
void whatout(void);                      /* WHATOUT */
void massmult(void);                     /* MASSMULT */
#endif

#endif
