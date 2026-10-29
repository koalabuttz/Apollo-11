/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SCALE_H
#define DSKY_SCALE_H
#include "p66_bias.h"
enum dsky_scale_entry {DSKY_DP1OUT,DSKY_DP2OUT,DSKY_DP3OUT};
/* Complete source DP1/DP2/DP3OUTSF through actual DSPDCEND (bank41), including
 * fresh READLO, TPAGREE mixed-sign normalization, verified DMPSUB and rescale.
 * Initial bank40/SUPER100, anyEB; MIXBR1 normal NOUNADD logical address or2
 * mixed IDAD1TEM[DECOUNT0..2] ECADR. Stable ordinary erasable input pair must
 * be physical1000..3776 and not cross an erasable bank. ArbitraryDP factor in
 * SFTEMP1/2. Image sole authoritative state; caller excludes producers. */
enum p66_bias_status dsky_scale_output(apollo_overlay_state *,p66_bias_io *,enum dsky_scale_entry);
#endif
