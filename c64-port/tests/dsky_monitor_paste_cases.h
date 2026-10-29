/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_PASTE_CASES_H
#define DSKY_MONITOR_PASTE_CASES_H
#include "dsky_monitor_paste.h"
#define DSKY_MONITOR_PASTE_CASES UINT32_C(16384)
void dsky_monitor_paste_input(apollo_overlay_state *,p66_bias_io *,uint16_t,enum dsky_nv_mission *,enum dsky_monitor_paste_stage *);
#endif
