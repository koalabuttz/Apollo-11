/* SPDX-License-Identifier: MIT */
#ifndef DISPLAY_JOB_LIFECYCLE_CASES_H
#define DISPLAY_JOB_LIFECYCLE_CASES_H
#include "display_job_cases.h"
#define DISPLAY_JOB_LIFECYCLE_CASES 768u
void display_job_lifecycle_input(apollo_overlay_state *,p66_bias_io *,agc_word *,uint16_t);
uint16_t display_job_lifecycle_phases(uint16_t);
agc_word display_job_lifecycle_boundary(uint16_t,uint16_t);
#endif
