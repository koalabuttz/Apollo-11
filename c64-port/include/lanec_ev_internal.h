/* SPDX-License-Identifier: MIT */
#ifndef LANEC_EV_INTERNAL_H
#define LANEC_EV_INTERNAL_H
/* Small source-instruction effects shared by the EXTENDED_VERBS body
 * modules. Each block function of a module performs one basic block of the
 * source and returns the address the source transfers to next; the module's
 * dispatcher continues while that address is one of its own entries. */
#include "lanec.h"

/* Value stored by TS/XCH from the 16-bit accumulator form. */
static inline uint16_t lanec_ev_corrected(uint16_t a) { return (uint16_t)((a & 037777u) | ((a >> 1) & 040000u)); }
/* TC: Q = return address, Z = target. */
static inline uint16_t lanec_ev_call(apollo_overlay_state *s, uint16_t target, uint16_t ret)
{
    lanec_tc(s, target, ret);
    return target;
}
/* CA/CAF/CAE of a memory word (not an editing register). */
static inline void lanec_ev_ca(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = lanec_a16(word); }
/* CS of a memory word. */
static inline void lanec_ev_cs(apollo_overlay_state *s, uint16_t word) { s->words[LANEC_A] = (uint16_t)~lanec_a16(word); }
/* BZF condition: A is +0 or -0. */
static inline uint8_t lanec_ev_zero(const apollo_overlay_state *s)
{
    return (uint8_t)(s->words[LANEC_A] == 0 || s->words[LANEC_A] == 0177777u);
}
/* BZMF condition: A is +0, -0 or negative. */
static inline uint8_t lanec_ev_nonpositive(const apollo_overlay_state *s)
{
    return (uint8_t)(lanec_ev_zero(s) || (s->words[LANEC_A] & 0100000u));
}
/* ADS k: A = A + k, k = corrected A. */
static inline void lanec_ev_ads(apollo_overlay_state *s, uint16_t k)
{
    lanec_ad(s, s->words[k]);
    s->words[k] = lanec_ev_corrected(s->words[LANEC_A]);
}
/* New EB (from TS or XCH EBANK), mirrored into BB as the hardware does. */
static inline void lanec_ev_set_eb(apollo_overlay_state *s, uint16_t a)
{
    s->words[LANEC_EB] = (uint16_t)(a & 03400u);
    s->words[LANEC_BB] = (uint16_t)(s->words[LANEC_FB] | (s->words[LANEC_EB] >> 8));
}
/* L as the hardware reads it back after any instruction. */
static inline void lanec_ev_set_l(apollo_overlay_state *s, uint16_t v)
{
    s->words[LANEC_L] = lanec_a16(lanec_ev_corrected(v));
}
#endif
