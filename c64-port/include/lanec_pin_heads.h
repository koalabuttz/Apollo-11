/* SPDX-License-Identifier: MIT */
#ifndef LANEC_PIN_HEADS_H
#define LANEC_PIN_HEADS_H
#include "lanec.h"
/* Pinball heads (docs/LANEC-PIN-HEADS.md): the file's own words at the
 * points where an earlier module ran on through other routines in one C
 * call. Each entry runs the original from its point to the next call or
 * jump and leaves the state as the original leaves it there, so the
 * callee runs as its own routine; the flight store links each head family
 * without the callees. Both missions; out-of-scope input returns
 * LANEC_OUT_OF_SCOPE before any change.
 *   load  ABCLOAD/ABLOAD/ALOAD/BLOAD/CLOAD and each return in them
 *         (bank 41) -> COMPTEST, NOUNTEST, UPDATVB -1, REQDATX/Y/Z;
 *         LOADLV -> RELDSP, LOADLV +5 (RELDSP returns) -> POSTJUMP. */
enum lanec_status lanec_pin_head_load_run(apollo_overlay_state *, lanec_io *, enum lanec_mission);
#endif
