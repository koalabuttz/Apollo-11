/* SPDX-License-Identifier: MIT */
#ifndef MAKEPLAY_SLEEP_CASES_H
#define MAKEPLAY_SLEEP_CASES_H
#include "p66_makeplay_sleep.h"
#define MAKEPLAY_SLEEP_CASES 16384u
void makeplay_sleep_input(apollo_overlay_state *,p66_bias_io *,uint16_t);
enum p66_sleep_boundary makeplay_sleep_apply(apollo_overlay_state *,p66_bias_io *,uint16_t);
#endif
