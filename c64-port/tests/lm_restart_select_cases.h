/* SPDX-License-Identifier: MIT */
#ifndef LM_RESTART_SELECT_CASES_H
#define LM_RESTART_SELECT_CASES_H
#include "lm_restart_select.h"
#include "agc_word.h"
/* Seven cohorts of 1024: variable phases, every table phase from RESTARTS,
 * the X.1 display phase, PHSPART2 second entries, GETPART2 table parts, table
 * phases again with wrapped clocks, and the signed-zero and overflow
 * branches of CHKNOVAC, FINDTIME and ITSLGCL2. */
#define LM_RSSELECT_CASES 7168u
/* Fills the image for case n; returns the entry and the original Z. */
enum lm_restart_select_entry lm_rsselect_input(apollo_overlay_state *,uint16_t n);
uint16_t lm_rsselect_pairs(void);
void lm_rsselect_pair(uint16_t k,uint8_t *group,agc_word *phase);
#endif
