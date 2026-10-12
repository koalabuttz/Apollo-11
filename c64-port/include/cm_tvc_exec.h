/* SPDX-License-Identifier: MIT */
#ifndef CM_TVC_EXEC_H
#define CM_TVC_EXEC_H
#include "agc_image_ops.h"
/* Comanche055 TVCEXECUTIVE (bank 16, 2720-3137), compiled one original word
 * at a time over the canonical 2048-word image. Entries are the original
 * continuation points: TVCEXEC 2720 (Waitlist task), ROLLPREP 2730 and
 * NEEDLEUP 2746 (WAITLIST returns), VARGAINS 2750, 2773 and 2775 (IBNKCALL
 * returns) and the TVCRESTARTS re-entries EXECCOPY+1 3003, 1SHOTCHK 3011,
 * TEMPSET 3040, CORSETUP 3043, CORCOPY+1 3121, CNTRCOPY 3132. FB bank 16,
 * EB E6, BB coherent; any raw A/L/Q and erasable contents; channel 11
 * (DSALMOUT) is the caller's value.
 *
 * CM_TVC_EXEC_OK stops at the next transfer out of the file, as the
 * original leaves it: Z = WAITLIST 5245 (Q = 2726 or 2744, A = the delay),
 * Z = IBNKCALL 4740 (Q = 2747, 2772 or 2774) or Z = TASKOVER 5324.
 * CM_TVC_EXEC_NO_EXIT: CCS VCNTR found VCNTR negative and the source spins
 * on TCF +0 at 2763 (the "NOT USED" branch); the image is left there.
 * Out-of-scope entries leave the image unchanged. CM_TVC_EXEC_FAULT (a TS
 * overflow skip into the FCORFRAC data word) cannot occur: A there is +0. */
enum cm_tvc_exec_status { CM_TVC_EXEC_OK = 0, CM_TVC_EXEC_OUT_OF_SCOPE = 1, CM_TVC_EXEC_NO_EXIT = 2, CM_TVC_EXEC_FAULT = 3 };
enum cm_tvc_exec_status cm_tvc_exec(agc_image *, agc_word channel11);
#endif
