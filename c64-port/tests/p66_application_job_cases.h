/* SPDX-License-Identifier: MIT */
#ifndef P66_APPLICATION_JOB_CASES_H
#define P66_APPLICATION_JOB_CASES_H
#include "p66_bias.h"
#include "agc_word.h"
#define P66_APPLICATION_JOB_COUNT 13312u
void p66_application_job_input(apollo_overlay_state *,p66_bias_io *,agc_word *,agc_word *,uint16_t);
#endif
