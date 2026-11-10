/* SPDX-License-Identifier: MIT */
/* SPS_BACK-UP_RCS_CONTROL.agc, the LM's (Luminary099 pages 1507-1510,
 * bank 21): SPSRCS, which "CONTROLS THE FIRING OF ALL RCS JETS IN THE
 * DOCKED CONFIGURATION ACCORDING TO THE FOLLOWING PHASE" "PLANE LOGIC."
 * For one axis, P, U or V (AXISCTR -1, +0, +1), it looks at the jets'
 * present sense (TJP, TJU or TJV), the attitude error E and its rate EDOT,
 * and leaves the axis's new jet time, half a unit either way or zero, in
 * TJP, TJU or TJV; a reversal or a stop sets the axis's inhibition counter
 * (UJETCTR -1 to +1), which holds the jets off for the next passes. It
 * returns by TC Q. The faithful C is c64-port/src/lm_sps_backup.c.
 *
 * The whole file is one entry, called under INHINT by IBNKCALL (the
 * listing's calling sequence names it CADR SPSRCE), with the return in Q. */
#ifndef READABLE_SPS_BACKUP_H
#define READABLE_SPS_BACKUP_H

#include "../agc/names.h"

#if LUMINARY099
void spsrcs(void); /* SPSRCS, a subroutine */
#endif

#endif
