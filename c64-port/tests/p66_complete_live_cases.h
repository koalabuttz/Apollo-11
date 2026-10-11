/* SPDX-License-Identifier: MIT */
#ifndef P66_COMPLETE_LIVE_CASES_H
#define P66_COMPLETE_LIVE_CASES_H
#include "p66_descent_live_cases.h"
#include "p66_display_job.h"
#define PCL_WORDS 2052u
void pcl_prepare(apollo_overlay_state *,executive *,p66_bias_io *,agc_word *,agc_word *,uint16_t);
void pcl_words(const apollo_overlay_state *,p66_bias_io,agc_word,agc_word,uint16_t[PCL_WORDS],unsigned);
#endif
