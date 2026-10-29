/* SPDX-License-Identifier: MIT */
#ifndef CM_RELAY_H
#define CM_RELAY_H
#include "cm_native_image.h"
typedef struct {uint16_t wrote_channel10,channel10;} cm_relay_output;
/* Original Comanche DSPOUTSB06,2024 through actual callerQ or Q+1.
 * Bank06/EB2, SUPER0..160 steps020, A normal NOUT-1, DSPCNT0..10/-0;
 * nonzero/non--0 relay words. Any ordinary callerQ0100..07776.
 * One actual channel10 event or no-event preserving previous latch; real
 * device consumption follows return. Caller owns exclusion. */
enum exec_status cm_relay_drain(apollo_overlay_state *,cm_native_io *,cm_relay_output *);
#endif
