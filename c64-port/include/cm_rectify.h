/* SPDX-License-Identifier: MIT */
#ifndef CM_RECTIFY_H
#define CM_RECTIFY_H
#include "overlay_runtime.h"
/* Explicit source groups, not interpreter opcodes. Caller owns the physical
 * image and excludes concurrent sensor, Executive and scratch producers.
 * Phases execute in order from original NEWOPS: 0..11 RECTIFY, 5..11 MINIRECT.
 * BANKSET fixed bank11, PBODY=0 (Earth) or2 (Moon), a legal CM VAC,
 * and MODE=-1 for direct MINIRECT
 * entry. No private state/pointers survive. Every group commits full source
 * numerical and control effects before returning; phases0..10 end at DANZIG,
 * phase11 at NEWOPS after RVQ. Only phases0..10 DANZIG returns admit an
 * Executive handoff; RVQ returns directly to NEWOPS without a job check. QPRET must contain an ordinary fixed-memory IVA return address.
 * Unsupported entry preconditions reject without mutation. */
enum cm_rectify_status {CM_RECTIFY_OK=0,CM_RECTIFY_OUT_OF_SCOPE=1};
enum cm_rectify_status cm_rectify_phase(apollo_overlay_state *,uint8_t phase);
#endif
