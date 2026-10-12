/* SPDX-License-Identifier: MIT */
#ifndef LM_INTERP_ORDERS_H
#define LM_INTERP_ORDERS_H
#include "agc_multiword.h"
#include "overlay_runtime.h"
#include "p66_bias.h"
/* General compiled Luminary099 interpretive order bodies and address
 * formation paths (companion to lm_interp_steps.h; same contract: canonical
 * image, raw 16-bit A/L, coherent banks, producers excluded by the caller).
 * `k` names the caller's fixed operand data when ADDRWD selects a fixed
 * word (NULL: the live erasable operand at C(ADDRWD)). Bodies end at DANZIG
 * (06061) unless stated. */

/* DIRADRES with the operand word given (STODL/STOVL load address). */
void lmi_diradres(apollo_overlay_state *, agc_word operand, agc_word target);
/* DIRADRES / ADDRESS when the next word is an order pair: PUSHUP. */
void lmi_diradres_pushup(apollo_overlay_state *, agc_word next, agc_word target);
void lmi_address_pushup(apollo_overlay_state *, agc_word next, agc_word target);
/* NEWOPS with any positive store code, or STADR with the complemented store
 * word: the store, then Z at its continuation (DANZIG, DODLOAD 06460,
 * DODLOAD* 06144, DOVLOAD 06463, DOVLOAD* 06466, or 15BITADR for STOTC). */
void lmi_store_code(apollo_overlay_state *, agc_word code);
void lmi_stadr(apollo_overlay_state *, agc_word word);
/* DODLOAD/DOVLOAD: load code into CYR, ending at DIRADRES 06106. */
void lmi_dodload(apollo_overlay_state *);
void lmi_dovload(apollo_overlay_state *);
void lmi_dload_op(apollo_overlay_state *, const agc_word *k);
void lmi_vload(apollo_overlay_state *, const agc_word *k);
void lmi_pdvl(apollo_overlay_state *, const agc_word *k);
void lmi_pddl_op(apollo_overlay_state *, const agc_word *k);
void lmi_push(apollo_overlay_state *);
#endif
