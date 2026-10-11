/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NV_CONSUMER_H
#define DSKY_NV_CONSUMER_H
#include "dsky_nv_blank.h"
#include "dsky_nv_update.h"
#include "dsky_nv_continuation.h"
enum dsky_nv_consumer_stage {DSKY_NV_CONSUMER_CALL,DSKY_NV_CONSUMER_MONITOR,DSKY_NV_CONSUMER_SERVICE,DSKY_NV_CONSUMER_CONTINUE};
/* Explicit compiled NVSUB admission, selected verb/noun/blank service and
 * corresponding source continuation. Each stage retains its callee's
 * declared domain and unsupported-context preservation. Paste/noun/blank
 * return to the actual saved caller; VN stops at genuine ENTPAS0. Alarm and
 * blocked admission remain visible selected boundaries. No source decoding
 * or new Executive poll; caller excludes producers; canonical image owns RAM.
 * Shared build requires both missions' immutable noun tables. */
enum p66_bias_status dsky_nv_consumer_step(apollo_overlay_state *,p66_bias_io *,agc_word *,enum dsky_nv_mission,enum dsky_nv_consumer_stage);
#endif
