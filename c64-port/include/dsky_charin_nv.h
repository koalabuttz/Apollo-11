/* SPDX-License-Identifier: MIT */
#ifndef DSKY_CHARIN_NV_H
#define DSKY_CHARIN_NV_H
#include "dsky_nv_key.h"
typedef struct {p66_bias_io control;uint16_t channel11,wrote_channel11;} dsky_charin_io;
/* Full original CHARIN job body for actual VERB021 / NOUN037 keycodes in
 * MPAC. Original display lock, old-lock21/22REG, CADRSTOR lamp gating,
 * physical channel011 event and key dispatch compose complete VERB/NOUN.
 * Bank40 retainedEB0..7/SUPER100; producers excluded. Stops actual ENDOFJOB;
 * other key bodies and Executive completion are separate translated entries. */
enum p66_bias_status dsky_charin_nv(apollo_overlay_state *,dsky_charin_io *);
#endif
