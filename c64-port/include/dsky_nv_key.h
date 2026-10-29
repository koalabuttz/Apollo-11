/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_KEY_H
#define DSKY_NV_KEY_H
#include "dsky_pinball.h"
enum dsky_nv_key {DSKY_KEY_VERB=0,DSKY_KEY_NOUN=1};
/* Complete original VERB/NOUN entry through actual ENDOFJOB. Clears the
 * selected input register, blanks its two physical relay characters and sets
 * decimal/input continuation state. Bank40 anyEB0..7/SUPER100; producers
 * excluded. CHARIN dispatch and the Executive lifecycle are separate bodies.
 * Ordinary relay words include both signed zeros. Unsupported leaves unchanged. */
enum p66_bias_status dsky_nv_key(apollo_overlay_state *,p66_bias_io *,enum dsky_nv_key);
#endif
