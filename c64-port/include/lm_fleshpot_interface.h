/* SPDX-License-Identifier: MIT */
#ifndef LM_FLESHPOT_INTERFACE_H
#define LM_FLESHPOT_INTERFACE_H
#include "lm_fleshpot.h"
/* Source CALCSMSC EXIT/BANKCALL/FLESHPOT-1 basic entrance; commits beforebody.
 * CALL-style caller with FIXLOC VAC and coherent bank10/INTBIT15=0 controls. */
enum lm_fleshpot_status lm_fleshpot_interface_start(apollo_overlay_state *);
enum lm_fleshpot_status lm_fleshpot_default(apollo_overlay_state *);
/* Composed FLESHPOT_FINISH output boundary: A/L must already be normalized.
 * Arbitrary raw A/L overflow entries are outside this source-return contract. */
enum lm_fleshpot_status lm_fleshpot_bank_return(apollo_overlay_state *);
enum lm_fleshpot_status lm_fleshpot_interface_reentry(apollo_overlay_state *,agc_word *irq);
#endif
