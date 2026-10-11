/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_CASES_H
#define DSKY_MONITOR_CASES_H
#include "dsky_monitor.h"
#define DSKY_MONITOR_CASES 65536ul
void dsky_monitor_input(apollo_overlay_state *,dsky_charin_io *,uint16_t,enum dsky_nv_mission *,enum dsky_monitor_stage *);
#endif
