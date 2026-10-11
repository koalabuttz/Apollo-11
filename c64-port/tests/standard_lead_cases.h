/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_CASES_H
#define STANDARD_LEAD_CASES_H
#include "standard_lead.h"
#define STANDARD_LEAD_STAGE_CASES 3584u
void standard_lead_input(apollo_overlay_state *,enum standard_lead_mission,
                        uint16_t,enum standard_lead_phase *,agc_word *);
#endif
