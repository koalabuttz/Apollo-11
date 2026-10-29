/* SPDX-License-Identifier: MIT */
#ifndef DSKY_SCALAR_OUTPUT_H
#define DSKY_SCALAR_OUTPUT_H
#include "dsky_nv_admission.h"
enum dsky_scalar_output_entry {DSKY_SCALAR_DEGREES,DSKY_SCALAR_ARITHMETIC,DSKY_SCALAR_ARITHMETIC14,DSKY_SCALAR_MISSION_ANGLE,DSKY_SCALAR_LANDING_POSITION,DSKY_SCALAR_FRACTION};
/* Original scalar scale bodies through actual DSPDCEND41,2566. Mission angle
 * selects LM360-CDUO/CMOPDEGOUT; landing position is LM only and consumes the
 * explicit original channel33 word; fraction is CM only and performs fresh
 * READLO. Fraction domain follows ordinary physical1000..3776 input pairs,
 * not crossing an erasable bank, source MIXBR1/2 and DECOUNT0..2 for mixed.
 * Other entries accept ordinary scale/MPAC words with independent signs.
 * Canonical image, coherent bank40/anyEB, SUPER100/120/140/160, legal IRQ;
 * producer exclusion belongs to caller. Unsupported image/IO unchanged.
 * Selected decimal formatter remains a continuation; no decoder or poll. */
enum p66_bias_status dsky_scalar_output(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_mission,enum dsky_scalar_output_entry,agc_word channel33);
#endif
