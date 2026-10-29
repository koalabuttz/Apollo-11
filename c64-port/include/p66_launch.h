/* SPDX-License-Identifier: MIT */
#ifndef P66_LAUNCH_H
#define P66_LAUNCH_H
#include "p66_major_mode.h"
enum p66_launch_status {P66_LAUNCH_OK=0,P66_LAUNCH_OUT_OF_SCOPE=1,P66_LAUNCH_NO_CORE=01202,P66_LAUNCH_WAIT_FULL=01203};
/* Complete STARTP66 entry through immediate RODCOMP or original BAILOUT1.
 * Actual FASTCHNG, MODREG66/display job admission, desired-rate capture,
 * STRTP66A bias setup, STOPRATE/flag changes and +100cs task scheduling.
 * Active source bank31/EB7 core0 owns VAC with priority>=030000 and NEWJOB0;
 * this preserves original nonpreempting landing-guidance call domain.
 * DSPMMJB is genuinely queued; its deferred display body is separate work.
 * Authoritative canonical image and producers exclusively owned throughout. */
enum p66_launch_status p66_launch(apollo_overlay_state *,p66_bias_io *);
#endif
