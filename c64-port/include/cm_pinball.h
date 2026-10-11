/* SPDX-License-Identifier: MIT */
#ifndef CM_PINBALL_H
#define CM_PINBALL_H
#include "cm_native_image.h"
#include "dsky_pinball.h"
enum cm_pinball_status {CM_PINBALL_OK,CM_PINBALL_OUT_OF_SCOPE};
/* Actual Comanche DSPIN40,3227 / 11DSPIN40,3311 / 2BLANK40,2502.
 * The fixed relay store arithmetic is shared only after independent whole
 * Comanche source-image qualification. Physical bank40, EB0..7, SUPER100.
 * Data domains as dsky_pinball_write; blank accepts both signed zeros.
 * L raw16-bit normalization is source hardware state. Caller owns exclusion.
 * Returns to actual ordinary caller Q, no external display callback. */
enum cm_pinball_status cm_pinball_write(apollo_overlay_state *,cm_native_io *,enum dsky_pinball_entry);
#endif
