/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_CONSUMER_CASES_H
#define DSKY_NV_CONSUMER_CASES_H
#include "dsky_nv_consumer.h"
#define DSKY_NV_CONSUMER_CASES 1792u
void dsky_nv_consumer_input(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t,enum dsky_nv_mission *);
uint8_t dsky_nv_consumer_count(uint16_t);
enum dsky_nv_consumer_stage dsky_nv_consumer_stage_at(uint16_t,uint8_t);
#endif
