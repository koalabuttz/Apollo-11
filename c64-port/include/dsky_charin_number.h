/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_NUMBER_H
#define DSKY_CHARIN_NUMBER_H
#include "dsky_charin_nv.h"
/* Actual LM CHARIN through NUM/DECTOBIN for octal and decimal digit entry,
 * DSPCOUNT0..19 except15, DECBRNCH lowbits0..2 (8/9 require decimal),
 * decimal inputregister0..99 V/N or0..9999 R1/R2/R3. Negative counts block
 * additional digits. Original
 * CHARIN lamp event, digit relay writes, SHORTMP debris and blocking stores.
 * Stops actual ENDOFJOB. Bank40, SUPER100, retainedEB0..7; caller excludes
 * producers. Includes R1/R2/R3 final fraction/sign stores; no Executive completion. */
enum p66_bias_status dsky_charin_number(apollo_overlay_state *,dsky_charin_io *);
#endif
