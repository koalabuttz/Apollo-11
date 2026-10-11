/* SPDX-License-Identifier: MIT */
#ifndef LM_RADAR_LEADIN_CASES_H
#define LM_RADAR_LEADIN_CASES_H
#include "lm_radar_leadin.h"
#define LM_RADAR_CASES 8192u
uint16_t lm_radar_target_number(uint16_t);
void lm_radar_input(apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
