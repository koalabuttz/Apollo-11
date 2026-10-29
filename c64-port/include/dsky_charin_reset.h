/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_RESET_H
#define DSKY_CHARIN_RESET_H
#include "dsky_charin_nv.h"
#include "cm_native_image.h"
typedef struct {
 p66_bias_io control;
 uint16_t channel11,channel13,wrote_channels,restart_light;
} dsky_reset_io;
/* Actual CHARIN RSET022 through ERROR and genuine ENDOFJOB.
 * Bank40/anyEB/SUPER100, arbitrary ordinary display and fault words.
 * Channel13 is readable latch data (write-only trap bits034000 clear).
 * Caller excludes producers; wrote_channels bit0/bit1 report channel11/13. */
enum p66_bias_status dsky_charin_reset(apollo_overlay_state *,dsky_reset_io *);
enum exec_status cm_charin_reset(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light);
#endif
