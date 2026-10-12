/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_CASES_H
#define LM_EPHEMERIS_CASES_H
#include "lm_ephemeris.h"
/* Deterministic sampled original entry states for Luminary LSPOS/LUNPOS
 * and LUNVEL: case n selects entry, actual caller return, VAC, EB, SUPER,
 * IRQ, time and TEPHEM edges over poisoned ordinary erasable debris. */
void lm_ephemeris_input(apollo_overlay_state *,p66_bias_io *,uint32_t);
#endif
