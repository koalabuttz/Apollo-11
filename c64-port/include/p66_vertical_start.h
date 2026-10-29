/* SPDX-License-Identifier: MIT */
#ifndef P66_VERTICAL_START_H
#define P66_VERTICAL_START_H
#include "p66_bias.h"
#include "waitlist.h"
/* VRTSTART through RODCOMP entry: phase selectors, actual STOPRATE,
 * X-override/redesignation flag clears, VERTGUID/P66VERT bank transfer,
 * phase3 termination and +100cs RODTASK scheduling. Whole image authoritative.
 * Caller excludes producers, A=+0, physical bank31/EB7. No DSKY placeholder.
 * WL_FULL stops before original BAILOUT1 recovery with source mutations intact. */
enum wl_status p66_vertical_start(apollo_overlay_state *,p66_bias_io *);
#endif
