/* SPDX-License-Identifier: MIT */
#ifndef CM_QUICTRIG_H
#define CM_QUICTRIG_H
#include "overlay_runtime.h"
#include "agc_multiword.h"
enum cm_trig_status {CM_TRIG_OK=0,CM_TRIG_OUT_OF_SCOPE=1};
/* Native basic SPSIN/SPCOS: canonical 2048-word image, raw sixteen-bit A/L,
 * source entry PC05075/05074, ordinary saved Q. Stops before source TC Q at
 * 05130 or05133; preserves IRQ and every unowned word. cosine is0 or1.
 * Uses source polynomial and overflow branches, no host floating point.
 * Caller owns image and excludes producers for the entire basic operation. */
enum cm_trig_status cm_single_trig(apollo_overlay_state *,uint8_t cosine);
/* Native QUICTRIG, source23,3555 through23,3605 before TC A. Requires bank23
 * and a valid BB/EB pair, raw source Q <=07777, IRQ0/1. Reads actual CDUSPOT
 * two's-complement counts in X/Z/Y order. Preserves those angles and MODE;
 * retains exact old destination exchange debris, ITEMP1/2,TEMK,SQ and Q.
 * INHINT/RELINT leaves IRQ1. No Executive check occurs inside this basic
 * routine. BANKCALL/return bridge belongs to the caller. No private state. */
enum cm_trig_status cm_quictrig(apollo_overlay_state *,agc_word *interrupt_enabled);
#endif
