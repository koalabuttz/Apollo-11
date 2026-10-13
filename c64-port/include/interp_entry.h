/* SPDX-License-Identifier: MIT */
#ifndef INTERP_ENTRY_H
#define INTERP_ENTRY_H
/* The INTERPRETER.agc entries of Luminary099 and Comanche055 that basic code
 * reaches, for any caller (milestone interp-entry, docs/INTERP-ENTRY.md):
 *
 *   INTPRET     RELINT; EXTEND; QXCH LOC   (LOC := the word after the TC)
 *   INTPRET +2  LXCH LOC                   (interpretive branches finish here)
 *   INTRSM      LXCH BBANK; TCF INTPRET +3 (resume a suspended interpretive job)
 *
 * then INTPRET +3: BANKSET := BBANK, INTBIT15 and EDOP := BBANK & BIT15, and
 * TCF NEWOPS, the interpreter's fetch point (LM 6070, CM 6037). The
 * trampoline continues there with the interpretive routine keyed by LOC.
 * LM INTPRET is 6037 and INTRSM 6050; CM INTPRET is 6006 and INTRSM 6017.
 * Any A/L/Q and erasable contents; coherent EB/FB/BB and ZERO = 0. At
 * INTPRET +2, L (the new LOC) must be at most 3777: interpretive branches
 * stay in their bank, as the E/CALL standard lead-in's finish requires. */
#include <stdbool.h>
#include "overlay_runtime.h"
#include "agc_word.h"
enum interp_entry_point {INTERP_INTPRET,INTERP_INTPRET2,INTERP_INTRSM};
#define INTERP_ENTRY_LM_INTPRET 06037u
#define INTERP_ENTRY_CM_INTPRET 06006u
/* Runs one entry for mission cm (0 LM, 1 CM) with Z at it; interrupt_enabled
 * is the mission's interrupt-enable latch (INTPRET's RELINT sets it). False,
 * with nothing changed, outside the domain above. */
bool interp_entry(apollo_overlay_state *,unsigned cm,enum interp_entry_point,agc_word *interrupt_enabled);
#endif
