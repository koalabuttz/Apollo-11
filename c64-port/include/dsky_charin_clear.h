/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_CLEAR_H
#define DSKY_CHARIN_CLEAR_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
/* Actual CHARIN CLEAR including rejected V/N clears
 * and high-pass backup with true verb redisplay plus R1/R2/R3 blank/reset to genuine ENDOFJOB. Ordinary
 * signed display positions0..19 excluding15, one canonical image. Caller
 * excludes producers; legal clear's isolated relay entry must be nonzero. */
enum p66_bias_status dsky_charin_clear(apollo_overlay_state *,dsky_charin_io *);
enum exec_status cm_charin_clear(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
