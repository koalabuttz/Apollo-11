/* SPDX-License-Identifier: MIT */
#ifndef P66_MODE_DISPLAY_H
#define P66_MODE_DISPLAY_H
#include "p66_bias.h"
/* Actual deferred DSPMMJB through ENDOFJOB entry, physical bank40/EB2 with
 * SUPER100. Native DSPDECVN/SHORTMP/TRACE1/DSPIN and 2BLANK bodies. MODREG
 * +0..99 or any negative word; positive>99 and clean +0 DSPTAB rejected.
 * Updates actual DSPTAB dirty relay words/NOUT, no output callback placeholder.
 * Executive job completion and T4 channel10 relay drain are separate services.
 * Canonical image, caller excludes producers for entire call. */
enum p66_bias_status p66_mode_display(apollo_overlay_state *,p66_bias_io *);
#endif
