/* SPDX-License-Identifier: MIT */
#ifndef LM_SINGLE_TRIG_H
#define LM_SINGLE_TRIG_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
enum lm_single_trig_status { LM_SINGLE_TRIG_OK=0, LM_SINGLE_TRIG_OUT_OF_SCOPE=1 };
/* Original Luminary SPSIN/SPCOS, entry PC05033/05032, raw16-bit A/L,
 * ordinary savedQ, REGZERO0. Source polynomial/quadrant effects include
 * TEMK01076 and SQ01077. Stops before TCQ05066/05071, IRQ retained.
 * No Executive check; caller retains canonical image/scratch ownership. */
enum lm_single_trig_status lm_single_trig(apollo_overlay_state *,uint8_t cosine);
/* Complete source-produced TCQ plus fixed-code return instruction in Q.
 * Saved return02000..07777, REGZERO0; retains A/banks/IRQ, corrects L,
 * sets PC=savedQ and Q=3. Unsupported inputs reject unchanged. */
enum lm_single_trig_status lm_single_trig_return(apollo_overlay_state *);
#endif
