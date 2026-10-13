/* SPDX-License-Identifier: MIT */
#ifndef KEYRUPT_UPRUPT_H
#define KEYRUPT_UPRUPT_H
#include "agc_transfer.h"
/* KEYRUPT_UPRUPT.agc of both missions as native routines of the
 * control-transfer ABI (docs/KEYRUPT-UPRUPT.md). The file is the same code
 * at LM 04,3274 and CM 07,3673; offsets below are from KEYRUPT1. Every entry
 * runs inside the interrupt (program->hardware) with FB in the file's bank,
 * as the KEYRUPT1/UPRUPT lead-ins and the callers' returns leave it.
 *
 *   KEYRUPT1 +0, UPRUPT +023  save BB and Q (BANKRUPT, QRUPT); TC LODSAMPT
 *                             (Z at LODSAMPT, Q at the entry +4)
 *   KEYRUPT1 +4               CAF LOW5; RAND MNKEYIN (channel 15); KEYCOM;
 *                             ACCEPTUP: CAF CHRPRIO; TC NOVAC (Z at NOVAC,
 *                             Q at +015, the 2CADR of CHARIN)
 *   KEYCOM +7                 the same from KEYCOM (Comanche SXTMARK's key
 *                             path)
 *   ACCEPTUP +017             NOVAC's return: the key code to the new job's
 *                             MPAC (INDEX LOCCTR); TC RESUME
 *   UPRUPT +027               INLINK to KEYTEMP1 (INLINK cleared), UPACT on
 *                             (WOR channel 11 bit 3), the triple-redundancy
 *                             test (UPTEST twice), then ERROR RESET clears
 *                             UPLOCKFL, an accepted code goes to ACCEPTUP
 *                             (NOVAC), a locked or failed code ends at RESUME
 *                             (TMFAIL2 sets UPLOCKFL)
 * LOCCTR from NOVAC is a core set offset (a multiple of 12 within the core
 * sets); anything no producer supplies is refused unchanged. */
extern const agc_routine_table lm_keyrupt_routines;
extern const agc_routine_table cm_keyrupt_routines;
#endif
