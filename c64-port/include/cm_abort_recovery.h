/* SPDX-License-Identifier: MIT */
#ifndef CM_ABORT_RECOVERY_H
#define CM_ABORT_RECOVERY_H
#include "cm_native_image.h"
/* Actual POODOO/CCSHOLE continuation after BORTENT, through the source-selected
 * WHIMPER entry. Phase0 tests V37FLBIT and either selects WHIMPER-1 or the first
 * DOWNFLAG. Phases1..3 execute the three actual named flag calls, phases4..6
 * execute BANKCALL/MR.KLEAN/SWRETURN and the actual TC WHIMPER. All selected
 * continuations are real source PCs, and every native C frame returns first.
 * The generic service body owns only SUPER/IRQ; other CM latches are retained. */
enum exec_status cm_abort_recovery_stage(apollo_overlay_state *,cm_native_io *,uint8_t phase);
#endif
