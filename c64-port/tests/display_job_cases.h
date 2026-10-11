/* SPDX-License-Identifier: MIT */
#ifndef DISPLAY_JOB_CASES_H
#define DISPLAY_JOB_CASES_H
#include "p66_display_job.h"
#define DISPLAY_JOB_CASES 512u
#define DISPLAY_JOB_PHASES 23u
void display_job_input(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
#endif
