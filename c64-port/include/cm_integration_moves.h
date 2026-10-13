/* SPDX-License-Identifier: MIT */
#ifndef CM_INTEGRATION_MOVES_H
#define CM_INTEGRATION_MOVES_H
#include "overlay_runtime.h"
#include "p66_bias.h"
/* Comanche055 INTEGRATION_INITIALIZATION's basic state-vector copies, run
 * by RTB (bank 13): MOVEACSM 02720 (= MOVATHIS) and MOVEALEM 02773 copy the
 * 42 words RRECT.. into RRECTCSM/RRECTLEM, MOVEPCSM 02747 and MOVEPLEM
 * 03014 back into RRECT. Each runs from the RTB's transfer (Z at the routine,
 * FB 13) through TC SETBANK (BBANK := INTBANK, A := FORTYONE, TC Q) and the
 * INDEX DIFEQCNT loop to TC DANZIG, as the original does: 0 (RTB_OK) with Z
 * at DANZIG (06030), Q the word after the TC. The Luminary forms are
 * lm_integration_move. */
enum cm_integration_move_kind {CM_MOVE_ACSM,CM_MOVE_ALEM,CM_MOVE_PCSM,CM_MOVE_PLEM};
uint8_t cm_integration_move(apollo_overlay_state *, p66_bias_io *, enum cm_integration_move_kind);
/* TCDANZIG (ENDT(X), 27,3352), which a BOVB TCDANZIG reaches by RTB: TC
 * DANZIG. RTB_OK with Z at DANZIG, or RTB_OUT_OF_SCOPE when Z is not at
 * TCDANZIG under FB 27. */
uint8_t cm_integration_tcdanzig(apollo_overlay_state *);
#endif
