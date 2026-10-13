/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_H
#define STANDARD_LEAD_H
#include <stdbool.h>
#include "overlay_runtime.h"
#include "agc_word.h"
#include "p66_bias.h"
enum standard_lead_mission {STANDARD_LEAD_LM,STANDARD_LEAD_CM};
enum standard_lead_phase {STANDARD_LEAD_BANK_PREPARE,STANDARD_LEAD_BANK_RETURN0,STANDARD_LEAD_BANK_RETURN1,STANDARD_LEAD_BANK_RETURN2,STANDARD_LEAD_INTERP_PREPARE,STANDARD_LEAD_INTERP_RESUME,STANDARD_LEAD_WAKE_RETURN};
/* Explicit compiled original erasable-call wrapper stages. Fixed CADR values
 * are resolved by the caller's native data reader; erasable inline values are
 * read from the sole canonical image after original aliased stores. Entry
 * and external SWCALL/INTPRET/JOBWAKE/caller boundaries remain explicit. */
bool standard_lead_stage(apollo_overlay_state *,enum standard_lead_mission,
                         enum standard_lead_phase,agc_word fixed_inline_cadr);
/* Complete original indirect CALL admission and fixed RVQ/EXIT/erasable
 * caller resumption. External fixed subroutine bodies execute as explicit
 * native continuations between CALL and RVQ; no instruction words dispatched. */
bool standard_lead_interp_call(apollo_overlay_state *,enum standard_lead_mission);
bool standard_lead_interp_rvq(apollo_overlay_state *,enum standard_lead_mission);
bool standard_lead_interp_exit(apollo_overlay_state *,enum standard_lead_mission);
bool standard_lead_interp_finish(apollo_overlay_state *,enum standard_lead_mission);
/* E/CALL's CALL EBUF2 order word (LM 04,2624, CM 13,2064) at the fetch point,
 * as a job resumed there by CHANG2 reaches it: the same CALL without
 * INTPRET's entry (Q and the interrupt latch untouched). At the fetch point
 * the interpreter has already loaded BBANK from BANKSET, so CALL and EXIT
 * there require the banks to equal BANKSET's. */
bool standard_lead_interp_call_order(apollo_overlay_state *,enum standard_lead_mission);
enum standard_lead_interp_boundary {STANDARD_LEAD_CALL,STANDARD_LEAD_RVQ,STANDARD_LEAD_EXIT,STANDARD_LEAD_FINISH,STANDARD_LEAD_CALL_ORDER};
/* External latch-aware interpreter controls: actual INTPRET enables interrupts.
 * E/CALL's TCF INTPRET+2 skips EXTEND/RELINT; FINISH, RVQ and EXIT retain the
 * caller's latch. Each control
 * starts at its explicit canonical entry; source opcode fetch is excluded. */
bool standard_lead_interp_control(apollo_overlay_state *,p66_bias_io *,enum standard_lead_mission,enum standard_lead_interp_boundary);
#endif
