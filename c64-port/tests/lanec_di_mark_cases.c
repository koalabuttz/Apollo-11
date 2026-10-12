/* SPDX-License-Identifier: MIT */
/* Cases for the mark displays: every entry, A with and without overflow,
 * flashing and non-flashing R displays, BLANKET offsets (overlapping MPAC
 * included), every COPYPACS index, MAKEMARK with a busy, sleeping or free
 * DSKY, MARKPLAY with and without the perform bit, and callers' returns in
 * fixed-fixed or bank 10. */
#include "lanec_suites.h"
#include "lanec_di_mark.h"
#include "lanec_symbols.h"

#define ENTRIES 23u
#define E(M, X) LANEC_##M##_##X
#define ENTRY_LIST(M) { E(M,BLANKET), E(M,ENDMARK), E(M,CLEARMRK), E(M,CLEARMRK) + 2u, E(M,MARKEND), \
    E(M,GOMARK), E(M,GOMARS), E(M,KLEENEX), E(M,GOMARKF), E(M,GOMARK2), E(M,MARKFORM), E(M,GOMARK3), \
    E(M,GOMARK4), E(M,GOMARKR), E(M,GOMARKFR), E(M,GOMARK2R), E(M,GOMARK3R), E(M,MAKEMARK), E(M,MARKPLAY), \
    E(M,GOGOMARK), E(M,MARKCOP), E(M,COPYTOGO), E(M,COPYPACS) }
static const uint16_t entries_lm[ENTRIES] = ENTRY_LIST(LM);
static const uint16_t entries_cm[ENTRIES] = ENTRY_LIST(CM);

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t any_a(uint32_t *r, uint32_t v)
{
    uint16_t x = (uint16_t)lanec_rand(r);
    switch (v % 4u) {
    case 0: return (uint16_t)(0040000u | (x & 037777u));
    case 1: return (uint16_t)(0100000u | (x & 037777u));
    default: return lanec_a16((uint16_t)(x & 077777u));
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), z, mpac = LANEC_LM_MPAC, q;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    uint16_t me, pd, ex, sk, rq, bl, cl;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) {
        z = entries_lm[e]; me = E(LM,MARKEND); pd = E(LM,PRIODSP); ex = E(LM,EXDSPRET); sk = E(LM,SKIPADD);
        rq = E(LM,RELINTQ); bl = E(LM,BLANKET); cl = E(LM,CLEARMRK);
    } else {
        z = entries_cm[e]; me = E(CM,MARKEND); pd = E(CM,PRIODSP); ex = E(CM,EXDSPRET); sk = E(CM,SKIPADD);
        rq = E(CM,RELINTQ); bl = E(CM,BLANKET); cl = E(CM,CLEARMRK);
    }
    w[LANEC_A] = any_a(&r, v);
    if ((v / 4u) & 1u) w[mpac + 4u] &= (uint16_t)~010u; else w[mpac + 4u] |= 010u;
    /* A caller's return: fixed-fixed outside BLANKET..CLEARMRK, or bank 10
     * between PRIODSP and EXDSPRET (code not translated here). */
    if ((v / 8u) & 1u) q = (uint16_t)(pd + 3u + lanec_rand(&r) % (uint32_t)(ex - pd - 3u));
    else do q = (uint16_t)(04000u + lanec_rand(&r) % 04000u); while (q >= bl && q <= cl + 7u);
    w[LANEC_Q] = q;
    w[mpac + 5u] = (v / 16u) % 3u == 0 ? (uint16_t)(lanec_rand(&r) % 4u)
                 : (uint16_t)(lanec_rand(&r) % (01376u - (mpac + 4u) + 1u));
    w[LANEC_LM_LOC] = (uint16_t)(v % 3u);
    switch ((v / 2u) % 4u) {                                   /* MAKEMARK: busy, sleeping or free */
    case 0: w[LANEC_LM_FLAGWRD4] &= (uint16_t)~034300u; w[LANEC_LM_FLAGWRD4] |= 0400u; break;
    case 1: w[LANEC_LM_FLAGWRD4] &= (uint16_t)~034700u; break;
    case 2: w[LANEC_LM_FLAGWRD4] &= (uint16_t)~(034700u | 024100u); break;
    default: break;
    }
    lanec_enter(s, io, 010u, z);
    range(m, 010u, me, (uint16_t)(pd + 2u));
    range(m, 010u, ex, (uint16_t)(sk + 3u));
    range(m, 010u, rq, (uint16_t)(rq + 1u));
    range(m, LANEC_ANY_BANK, bl, (uint16_t)(cl + 7u));
    range(m, LANEC_ANY_BANK, 1u, 2u);
}

const lanec_suite lanec_suite_di_mark = { "di-mark", ENTRIES * 2u * 64u, make, lanec_di_mark_dispatch };
