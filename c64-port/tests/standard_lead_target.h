/* SPDX-License-Identifier: MIT */
#ifndef STANDARD_LEAD_TARGET_H
#define STANDARD_LEAD_TARGET_H
#include "standard_lead.h"
void standard_lead_target_input(apollo_overlay_state *,p66_bias_io *,enum standard_lead_mission,uint16_t kind,uint16_t n);
bool standard_lead_target_execute(apollo_overlay_state *,p66_bias_io *,enum standard_lead_mission,uint16_t kind,uint16_t n,uint16_t phase,uint16_t *status);
#endif
