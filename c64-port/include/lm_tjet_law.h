/* SPDX-License-Identifier: MIT */
#ifndef LM_TJET_LAW_H
#define LM_TJET_LAW_H
#include "agc_transfer.h"
/* Luminary099 TJET_LAW.agc, all of it (bank 17, 03252-03743), as one native
 * routine of the control-transfer ABI:
 *
 *   TJETLAW  17,3252  the jet-firing time for one axis (P, U or V by
 *                     AXISCTR = -1, 0, +1) from the attitude error E and rate
 *                     error EDOT: the phase-plane zones 1-5, the rough laws
 *                     for large errors, TJETU, NUMBERT; TC HOLDQ
 *
 * Its subroutines Z123COMP and RUFSETUP (with RUFSETUP +2) are entered only
 * by TC from inside the file and are compiled at their call sites, leaving Q
 * as the original's TC Q does. TC CCSHOLE (ALARM_AND_ABORT) is a transfer.
 * Refused (EXEC_OUT_OF_SCOPE, image unchanged): an entry outside bank 17 or
 * under an EB other than EBANK6 (EBANK= TJP), an AXISCTR other than -1, 0,
 * +1 (no caller sets another; INDEX AXISCTR would read outside AXISDIFF and
 * TJETU), and a return in Q that is not a TC into fixed memory. */
extern const agc_routine_table lm_tjet_law_routines;
#define LM_TJ_FB 036000u
#define LM_TJ_EB 03000u
#define LM_TJ_TJETLAW 03252u
#define LM_TJ_FIRST 03252u
#define LM_TJ_LAST 03743u
#define LM_TJ_AXISCTR 01505u
#define LM_TJ_HOLDQ 01745u
#endif
