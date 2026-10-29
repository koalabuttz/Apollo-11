/* SPDX-License-Identifier: MIT */
#ifndef P66_LAUNCH_PHASED_H
#define P66_LAUNCH_PHASED_H
#include "p66_launch.h"
/* Fixed compiled source-call seams, never Executive scheduling points.
 * Setup starts STARTP662477 and returns STRTP66A2505 or BAILOUT1.
 * Bias starts2505 and returns VRTSTART2536. Same owned NEWJOB0/VAC domain
 * as the independently qualified uninterrupted STARTP66 composition. */
enum p66_launch_status p66_launch_setup_phase(apollo_overlay_state *,p66_bias_io *);
enum p66_launch_status p66_launch_bias_phase(apollo_overlay_state *,p66_bias_io *);
#endif
