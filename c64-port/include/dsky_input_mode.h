/* SPDX-License-Identifier: MIT */
#ifndef DSKY_INPUT_MODE_H
#define DSKY_INPUT_MODE_H
#include "dsky_lamp_release.h"
enum dsky_input_mode_route {DSKY_INPUT_MODE_CALLER,DSKY_INPUT_MODE_RECYCLE};
/* Actual ALLDC/OC41,3011, ALL3DEC LM42,3573/CM42,3506 and FORCEV25
 * LM42,3603/CM42,3516. Full ordinary DECBRNCH, raw A/L, ordinary
 * caller Q0100..7777, any EB/selecting SUPER/IRQ. Source edit-register,
 * signed-zero and TS overflow effects through caller or ALMCYCLE entry.
 * Canonical image under caller exclusion; unsupported input is atomic. */
enum p66_bias_status dsky_input_mode(apollo_overlay_state *,dsky_lamp_io *,enum dsky_input_mode_route *);
enum exec_status cm_input_mode(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_input_mode_route *);
#endif
