/* SPDX-License-Identifier: MIT */
#ifndef CM_NBSM_H
#define CM_NBSM_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum cm_nbsm_phase {CM_NBSM_SAVE_RETURN=0,CM_NBSM_LOAD_INDEX,CM_NBSM_LOAD_ANGLES,
CM_NBSM_STORE_LOAD_VECTOR,CM_NBSM_CALL_TRANSFORM,CM_NBSM_STORE_RETURN};
enum cm_nbsm_status {CM_NBSM_OK=0,CM_NBSM_OUT_OF_SCOPE=1};
/* Source legacy NBSM23,3606. Four leading genuine/pending DANZIG states,
 * then source CALL enters TRG*NBSM at NEWOPS; caller composes the qualified
 * permanent interface before STORE_RETURN from NEWOPS/LOC03617.
 * Stable six-word S1 source angle array and VAC32D input must exclude other
 * producers during each source phase. Source EDOP0 points permit Executive
 * checks, while internal source CALL/NEWOPS returns retain ownership. */
enum cm_nbsm_status cm_nbsm(apollo_overlay_state *,enum cm_nbsm_phase);
#endif
