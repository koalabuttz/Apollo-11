/* SPDX-License-Identifier: MIT */
#ifndef LM_EPHEMERIS_H
#define LM_EPHEMERIS_H
#include "p66_bias.h"
#include "fixed_data.h"
#include "agc_word.h"
/* Luminary LSPOS (=LUNPOS) and LUNVEL from the original interpretive entry
 * frontier (NEWOPS, LOC 03664 or 03775, bank 15) to the actual caller NEWOPS
 * frontier selected by QPRET. 0 unsupported (image unchanged), 1 returned,
 * 2 original UNIT SQRT abort frontier. */
uint8_t lm_ephemeris(apollo_overlay_state *,p66_bias_io *);
#endif
