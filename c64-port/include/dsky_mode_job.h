/* SPDX-License-Identifier: MIT */
#ifndef DSKY_MODE_JOB_H
#define DSKY_MODE_JOB_H
#include "dsky_two_digit.h"
#include "cm_formatter.h"
enum dsky_mode_job_route {DSKY_MODE_JOB_END,DSKY_MODE_JOB_CCSHOLE};
/* Actual DSPMMJB LM40,3534 / CM40,3441, any EB/selecting SUPER/IRQ.
 * MODREG0..99 or any negative ordinary word, including -0 blank request;
 * DSPTAB includes clean+0 and NOUT includes every ordinary word.
 * Original complete CPU/scratch/relay stores through genuine ENDOFJOB,
 * or clean+0 DSPIN prelude through actual CCSHOLE entry. Producer exclusion,
 * one canonical image; rejected input remains unchanged. */
enum p66_bias_status dsky_mode_job(apollo_overlay_state *,p66_bias_io *,enum dsky_mode_job_route *);
enum exec_status cm_mode_job(apollo_overlay_state *,cm_native_io *,enum dsky_mode_job_route *);
#endif
