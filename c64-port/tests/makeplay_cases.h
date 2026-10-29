/* SPDX-License-Identifier: MIT */
#ifndef MAKEPLAY_CASES_H
#define MAKEPLAY_CASES_H
#include "p66_makeplay.h"
#define MAKEPLAY_CASES 12288u
void makeplay_input(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
enum p66_makeplay_boundary makeplay_apply(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
#endif
