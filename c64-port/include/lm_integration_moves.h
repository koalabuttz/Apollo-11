/* SPDX-License-Identifier: MIT */
#ifndef LM_INTEGRATION_MOVES_H
#define LM_INTEGRATION_MOVES_H
#include "overlay_runtime.h"
#include "p66_bias.h"
/* Luminary099 INTEGRATION_INITIALIZATION's basic state-vector copies, run
 * by RTB (bank 13): MOVEACSM 02674 and MOVEALEM 02747 (= MOVATHIS) copy the
 * 42 words RRECT.. into RRECTCSM/RRECTLEM, MOVEPCSM 02723 and MOVEPLEM
 * 02770 back into RRECT. Each runs from the RTB's transfer (Z at the routine,
 * FB 13) through TC SETBANK (BBANK := INTBANK, A := FORTYONE, TC Q) and the
 * INDEX DIFEQCNT loop to TC DANZIG, as the original does: 0 (RTB_OK) with Z
 * at DANZIG, Q the word after the TC. */
enum lm_integration_move_kind {LM_MOVE_ACSM,LM_MOVE_ALEM,LM_MOVE_PCSM,LM_MOVE_PLEM};
uint8_t lm_integration_move(apollo_overlay_state *, p66_bias_io *, enum lm_integration_move_kind);
/* TCDANZIG (ENDT(X), 27,3753), which ORBITAL_INTEGRATION's BOVB TCDANZIG
 * reaches by RTB: TC DANZIG. RTB_OK with Z at DANZIG, or RTB_OUT_OF_SCOPE
 * when Z is not at TCDANZIG under FB 27. */
uint8_t lm_integration_tcdanzig(apollo_overlay_state *);
#endif
