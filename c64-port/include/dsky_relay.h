/* SPDX-License-Identifier: MIT */
#ifndef DSKY_RELAY_H
#define DSKY_RELAY_H
#include "p66_bias.h"
typedef struct {p66_bias_io control;uint16_t wrote_channel10,channel10;} dsky_relay_io;
/* Original DSPOUTSB (bank06/EB2) called after positive CCS NOUT, through
 * actual caller2070 (+0 no dirty relays) or2071 (one channel10 relay write).
 * Caller Q2070, A=NOUT-1 normal nonnegative, DSPCNT0..10 or-0, elevenDSPTAB
 * entries each nonzero/non-0. Direct native scan, real output word returned
 * for hardware adapter, no placeholder output callback or AGC decoder.
 * Canonical image authoritative, caller excludes all producers. */
enum p66_bias_status dsky_relay_drain(apollo_overlay_state *,dsky_relay_io *);
#endif
