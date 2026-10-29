/* SPDX-License-Identifier: MIT */
#ifndef DSKY_DISPLAY_RELEASE_H
#define DSKY_DISPLAY_RELEASE_H
#include "dsky_charin_release.h"
/* Actual internal RELDSP LM04457/CM04473 through ordinary Q caller.
 * Any aligned FB/EB and SUPER0..160; Q0100..7777. Selected nonempty
 * DSPLIST wake paths require the independently qualified Executive domain.
 * No-wake paths do not inspect unrelated Executive words. Caller exclusion;
 * canonical source image is sole authority. Unsupported input unchanged. */
enum p66_bias_status dsky_display_release(apollo_overlay_state *,dsky_charin_io *);
enum exec_status cm_display_release(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channel11);
#endif
