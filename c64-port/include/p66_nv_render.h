/* SPDX-License-Identifier: MIT */
#ifndef P66_NV_RENDER_H
#define P66_NV_RENDER_H
#include "p66_nv_rows.h"
/* Original fixed noun60 scale selection from DSPDCPUT2537 through its
 * actual DP1OUTSF/DP3OUTSF entry. DECOUNT0..2, canonical mixed addresses,
 * original bank41, any ordinary EB, SUPER100. Source display data remains
 * authoritative and the scale service performs a later fresh READLO. */
enum p66_bias_status p66_nv_scale_select(apollo_overlay_state *,p66_bias_io *);
/* Actual DSPDCEND2566 BANKCALL/DSPDECWD/SWRETURN to2570. */
enum p66_bias_status p66_nv_decimal_row(apollo_overlay_state *,p66_bias_io *);
/* Source CCS/row loop or actual NVSUBEND/SUPDXCHZ caller return03122. */
enum p66_bias_status p66_nv_next_row(apollo_overlay_state *,p66_bias_io *);
#endif
