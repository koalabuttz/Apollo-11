/* SPDX-License-Identifier: MIT */
#ifndef P66_NV_ROWS_H
#define P66_NV_ROWS_H
#include "p66_nv_words.h"
/* Fixed V06N60 decimal preparation through DSPDCPUT, canonical original
 * physical component pairs retained for the subsequent fresh READLO. Entry
 * NVSUB03604, bank41/EB0/SUPER100 after noun update; ends2537 bank41/EB4.
 * No general verb or noun decoding; all source scratch and registers retained. */
enum p66_bias_status p66_nv_data_prepare(apollo_overlay_state *,p66_bias_io *);
#endif
