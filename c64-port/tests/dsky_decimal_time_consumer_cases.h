/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DECIMAL_TIME_CONSUMER_CASES_H
#define DSKY_DECIMAL_TIME_CONSUMER_CASES_H
#include "dsky_decimal_time_consumer.h"
#define DSKY_DECIMAL_TIME_CONSUMER_CASES 4096u
void dsky_decimal_time_consumer_input(apollo_overlay_state *,p66_bias_io *,uint16_t,enum dsky_nv_mission *);
uint8_t dsky_decimal_time_consumer_stage(enum dsky_nv_mission,agc_word);
#endif
