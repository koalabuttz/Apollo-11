/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_BANK_H
#define STANDARD_LEAD_BANK_H
#include "standard_lead.h"
#include "p66_bias.h"
/* Original SWCALL/SWRETURN controls used by E/BKCALL. These returned C frames
 * surround the caller-owned native fixed routine; they never dispatch a rope
 * word. Call starts after BANK_PREPARE, return starts at source SWRETURN.
 * The fixed routine selects return0/1/2 by preserving or advancing BUF2.
 * Producer exclusion includes EBUF2 and BUF2 until BANK_RETURN finishes.
 * External SUPERBANK/interrupt latches are retained. Unsupported unchanged. */
bool standard_lead_bank_call(apollo_overlay_state *,p66_bias_io *,enum standard_lead_mission);
bool standard_lead_bank_return(apollo_overlay_state *,p66_bias_io *,enum standard_lead_mission);
#endif
