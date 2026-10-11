/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_BUSY_CASES_H
#define DSKY_NV_BUSY_CASES_H
#include "dsky_nv_busy.h"
#define DSKY_NV_BUSY_CASES 10240u
void dsky_nv_busy_input(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t,enum dsky_nv_mission *,enum dsky_nv_busy_entry *);
#endif
