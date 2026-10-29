/* SPDX-License-Identifier: MIT */
#ifndef INVERSE_TRIG_H
#define INVERSE_TRIG_H
#include "overlay_runtime.h"
#include "agc_word.h"
enum inverse_trig_mission {INVERSE_TRIG_LM=0,INVERSE_TRIG_CM=1};
enum inverse_trig_status {INVERSE_TRIG_OK=0,INVERSE_TRIG_DONE=1,INVERSE_TRIG_ALARM_1301=2,INVERSE_TRIG_ALARM_1302=3,INVERSE_TRIG_OUT_OF_SCOPE=4};
/* Explicit original ARCSIN/ARCCOS stages. These basic arithmetic boundaries
 * retain canonical ownership; only completed DANZIG/EDOP0 permits a source
 * Executive check. Alarm statuses stop before the original alarm service. */
enum inverse_trig_status inverse_trig_prepare(apollo_overlay_state *,enum inverse_trig_mission,uint8_t arcsine);
enum inverse_trig_status inverse_trig_root(apollo_overlay_state *,enum inverse_trig_mission);
enum inverse_trig_status inverse_trig_polynomial(apollo_overlay_state *,enum inverse_trig_mission);
enum inverse_trig_status inverse_trig_finish(apollo_overlay_state *,enum inverse_trig_mission);
/* Original coefficient/degree and NEG12 immutable source words. */
uint8_t inverse_trig_constant_read(enum inverse_trig_mission,uint16_t,uint16_t *);
#endif
