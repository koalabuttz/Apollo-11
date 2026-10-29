/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MONITOR_CONSUMER_CASES_H
#define DSKY_MONITOR_CONSUMER_CASES_H
#include "dsky_monitor_consumer.h"
#define DSKY_MONITOR_CONSUMER_CASES 256u
void dsky_monitor_consumer_input(apollo_overlay_state *,dsky_charin_io *,uint16_t,enum dsky_nv_mission *,agc_word *);
#endif
