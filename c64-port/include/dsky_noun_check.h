/* SPDX-License-Identifier: MIT */
#ifndef DSKY_NOUN_CHECK_H
#define DSKY_NOUN_CHECK_H
#include "dsky_lamp_release.h"
enum dsky_noun_check_route {DSKY_NOUN_CHECK_CALLER,DSKY_NOUN_CHECK_ALARM,DSKY_NOUN_CHECK_RECYCLE,DSKY_NOUN_CHECK_HOLE};
/* Original COMPTEST/COMPTST1/DCOMPTST/DECTEST/DCTSTCYC/NOUNTEST/GETCOMP
 * at actual bank41 source PCs; any EB/selecting SUPER/IRQ, MIXBR1/2,
 * ordinary type words, component argument -2/-1/eitherzero only when read.
 * Ordinary Q caller (L caller for COMPTST1). Source-selected literal alarm,
 * recycle and CCSHOLE frontiers preserved; no recovery or scheduler poll.
 * One canonical image under caller exclusion; unsupported is atomic. */
enum p66_bias_status dsky_noun_check(apollo_overlay_state *,dsky_lamp_io *,enum dsky_noun_check_route *);
enum exec_status cm_noun_check(apollo_overlay_state *,cm_native_io *,uint16_t *wrote_channels,uint16_t *restart_light,enum dsky_noun_check_route *);
#endif
