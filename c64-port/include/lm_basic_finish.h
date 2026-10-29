/* SPDX-License-Identifier: MIT */
#ifndef LM_BASIC_FINISH_H
#define LM_BASIC_FINISH_H
#include "p66_bias.h"
#include "executive.h"
/* Actual ENDOFJOB05155 through selected basic DTCB, interpreter INTRSM
 * (before NEWOPS fetch), or idle ADVAN03214 after channel11 activity output.
 * One canonical image + explicit SUPER/IRQ/channel11, caller-owned exclusion.
 * Active positive core0; low priority VAC bits0..FAKEPRET or admitted VACbase;
 * other priorities not+0; selected rawLOC must ordinarybasic or complemented
 * interpretive address with a validBBCON. Exact register/scan/scratch effects.
 * next.interpretive and positiveaddress describe real selected continuation; idle
 * address={0,0}. Unsupported input preserves image/io/channel/next entirely. */
enum exec_status lm_basic_finish(apollo_overlay_state *,p66_bias_io *,agc_word *channel11,exec_transfer *next);
#endif
