/* SPDX-License-Identifier: MIT */
#ifndef P66_NV_WORDS_H
#define P66_NV_WORDS_H
#include "p66_nv_entry.h"
/* Source fixed V06N60 verb/noun updates, each through its NVSUB caller.
 * Verb starts UPDATVB-1 (2337/Q3602/A6/bank41/EB1); noun starts NVSUB3602
 * (same bank/EB, savednoun60). SUPER100, canonical image authoritative.
 * Selected DSPTAB is nonzero, as required by the verified Pinball service.
 * Native source noun60 table loads are literal original words/addresses;
 * no general noun or verb decoder. Each returns caller3602 or3604. */
enum p66_bias_status p66_nv_verb(apollo_overlay_state *,p66_bias_io *);
enum p66_bias_status p66_nv_noun(apollo_overlay_state *,p66_bias_io *);
#endif
