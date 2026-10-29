/* SPDX-License-Identifier: MIT */
#ifndef INTERPRETED_SIGN_H
#define INTERPRETED_SIGN_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
enum interpreted_sign_mission {INTERPRETED_SIGN_LM=0,INTERPRETED_SIGN_CM=1};
enum interpreted_sign_kind {INTERPRETED_COMP=0,INTERPRETED_SIGN=1};
enum interpreted_sign_status {INTERPRETED_SIGN_OK=0,INTERPRETED_SIGN_OUT_OF_SCOPE=1};
/* Source COMP/SIGN basic entries, ending before DANZIG executes. Stable
 * ordinary DP sign_operand must equal the source effective operand for SIGN.
 * All ordinary MODE except invalid negative zero; MODE<0 selects vector.
 * REGZERO0, coherent EB/BB, source entry PC; other banks and IRQ retained.
 * Exclude operand/shared scratch producers until the completed source pair. */
enum interpreted_sign_status interpreted_sign(apollo_overlay_state *,
 enum interpreted_sign_mission,enum interpreted_sign_kind,const agc_dp *sign_operand);
#endif
