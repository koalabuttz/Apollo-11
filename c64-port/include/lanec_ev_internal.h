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
/* RAND, RXOR: A with a channel (3-077, not 7). */
static inline void lanec_ev_rand(apollo_overlay_state *s, const lanec_io *io, uint16_t ch)
{
    s->words[LANEC_A] = lanec_a16((uint16_t)(lanec_ev_corrected(s->words[LANEC_A]) & io->channel[ch]));
}
static inline void lanec_ev_rxor(apollo_overlay_state *s, const lanec_io *io, uint16_t ch)
{
    s->words[LANEC_A] = lanec_a16((uint16_t)(lanec_ev_corrected(s->words[LANEC_A]) ^ io->channel[ch]));
}
/* WOR, WAND: A = channel OR/AND A, written back with the channel's write rules. */
static inline void lanec_ev_wor(apollo_overlay_state *s, lanec_io *io, uint16_t ch)
{
    s->words[LANEC_A] = lanec_a16((uint16_t)(lanec_ev_corrected(s->words[LANEC_A]) | io->channel[ch]));
    lanec_write(s, io, ch);
}
static inline void lanec_ev_wand(apollo_overlay_state *s, lanec_io *io, uint16_t ch)
{
    s->words[LANEC_A] = lanec_a16((uint16_t)(lanec_ev_corrected(s->words[LANEC_A]) & io->channel[ch]));
    lanec_write(s, io, ch);
}
/* 16-bit ones'-complement add (INCR of a register, AddSP16). */
static inline uint16_t lanec_ev_add16(uint16_t a, uint16_t b)
{
    uint32_t sum = (uint32_t)a + b;
    if (sum & 0200000u) sum = (sum + 1u) & 0177777u;
    return (uint16_t)sum;
}
/* 15-bit word as the accumulator holds it after CA. */
static inline uint16_t lanec_ev_sx(uint16_t v) { return lanec_a16((uint16_t)(v & 077777u)); }
/* MP of a memory word (15-bit): A,L = corrected A times the word, as
 * Virtual AGC forms the double-precision product and its zero signs. */
static inline void lanec_ev_mp(apollo_overlay_state *s, uint16_t word)
{
    uint16_t a = lanec_ev_corrected(s->words[LANEC_A]), b = (uint16_t)(word & 077777u), ms, ls;
    if (b == 0 || b == 077777u) {
        ms = ls = 0;
    } else if (a == 0 || a == 077777u) {
        ms = ls = (uint16_t)((a == 0) == ((b & 040000u) != 0) ? 077777u : 0u);
    } else {
        uint16_t ma = (a & 040000u) ? (uint16_t)(~a & 037777u) : a;
        uint16_t mb = (b & 040000u) ? (uint16_t)(~b & 037777u) : b;
        uint32_t d = (uint32_t)ma * mb;
        uint8_t neg = (uint8_t)(((a ^ b) & 040000u) != 0);
        if (neg) d = ~d & 07777777777ul;
        ls = (uint16_t)((d & 037777u) | (neg ? 040000u : 0u));
        ms = (uint16_t)((d >> 14) & 077777u);
    }
    s->words[LANEC_A] = lanec_ev_sx(ms);
    s->words[LANEC_L] = lanec_ev_sx(ls);
}
/* DXCH k (k, k+1 memory): A,L <-> k,k+1. */
static inline void lanec_ev_dxch(apollo_overlay_state *s, uint16_t k)
{
    uint16_t lo = s->words[k + 1u], hi = s->words[k];
    s->words[k + 1u] = lanec_ev_corrected(s->words[LANEC_L]);
    s->words[LANEC_L] = lanec_ev_sx(lo);
    s->words[k] = lanec_ev_corrected(s->words[LANEC_A]);
    s->words[LANEC_A] = lanec_ev_sx(hi);
}
/* DAS k (k, k+1 memory): k,k+1 += A,L with the low word's carry; A = the
 * high word's overflow (+1, -1 or +0), L = +0. */
static inline void lanec_ev_das(apollo_overlay_state *s, uint16_t k)
{
    uint16_t lsw = lanec_ev_add16(s->words[LANEC_L], lanec_ev_sx(s->words[k + 1u]));
    uint16_t msw = lanec_ev_add16(s->words[LANEC_A], lanec_ev_sx(s->words[k]));
    if ((lsw & 0140000u) == 0040000u) msw = lanec_ev_add16(msw, 1u);
    else if ((lsw & 0140000u) == 0100000u) msw = lanec_ev_add16(msw, 0177776u);
    s->words[LANEC_A] = (msw & 0140000u) == 0100000u ? 0177776u : (msw & 0140000u) == 0040000u ? 1u : 0u;
    s->words[LANEC_L] = 0;
    s->words[k + 1u] = lanec_ev_corrected(lsw);
    s->words[k] = lanec_ev_corrected(msw);
}
#ifdef LANEC_COVER_SITES
/* Per call site outcomes for the coverage runner (see lanec.h). */
#define lanec_ev_zero(s) __extension__({ int lanec_c_ = LANEC_COVER_A(s); \
    lanec_c_ == 0 ? (lanec_ev_zero)(s) : lanec_c_ == 1 ? (lanec_ev_zero)(s) : (lanec_ev_zero)(s); })
#define lanec_ev_nonpositive(s) __extension__({ int lanec_c_ = LANEC_COVER_A(s); \
    lanec_c_ == 0 ? (lanec_ev_nonpositive)(s) : lanec_c_ == 1 ? (lanec_ev_nonpositive)(s) : \
    lanec_c_ == 2 ? (lanec_ev_nonpositive)(s) : (lanec_ev_nonpositive)(s); })
#define lanec_ev_mp(s, w) __extension__({ uint16_t lanec_w_ = (uint16_t)((w) & 077777u); \
    uint16_t lanec_m_ = lanec_ev_corrected((s)->words[LANEC_A]); \
    if (lanec_w_ == 0) (lanec_ev_mp)(s, lanec_w_); else if (lanec_w_ == 077777u) (lanec_ev_mp)(s, lanec_w_); \
    else if (lanec_m_ == 0) (lanec_ev_mp)(s, lanec_w_); else if (lanec_m_ == 077777u) (lanec_ev_mp)(s, lanec_w_); \
    else (lanec_ev_mp)(s, lanec_w_); })
#endif
#endif
