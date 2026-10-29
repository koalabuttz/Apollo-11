/* SPDX-License-Identifier: MIT */
/* Cases for the display job selection: every entry, PLAYTEM4 selecting PRIO,
 * NORM or MARK and LINUS or not, LINUS matching and non-matching flash
 * addresses with a free or locked DSKY, FLAGWRD4 with every busy and sleep
 * combination exercised, EBANKTEM with and without BIT4, sleeping and
 * non-sleeping LOCCTR, A with and without overflow, and callers' returns in
 * fixed-fixed or bank 10. */
#include "lanec_suites.h"
#include "lanec_di_play.h"
#include "lanec_symbols.h"

#define ENTRIES 23u
#define E(M, X) LANEC_##M##_##X
#define P(M, N) (LANEC_##M##_MAKEPRIO + (N))
#define ENTRY_LIST(M) { E(M,MAKEPLAY), E(M,MAKEPLAY) + 5u, E(M,MAKEPRIO), P(M,4u), P(M,8u), P(M,12u), \
    P(M,14u), P(M,18u), P(M,20u), P(M,23u), P(M,24u), P(M,26u), P(M,39u), P(M,43u), P(M,51u), P(M,53u), \
    P(M,66u), P(M,67u), P(M,68u), E(M,WITCHONE), E(M,XCHTOEND), E(M,XCHNYLOC), E(M,LINUSCHR) }
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
    uint16_t me, pp, cn, sk, wo, rq, lc;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) {
        z = entries_lm[e]; me = E(LM,MARKEND); pp = E(LM,PRIOPLAY); cn = E(LM,COPYNORM); sk = E(LM,SKIPADD);
        wo = E(LM,WITCHONE); rq = E(LM,RELINTQ); lc = E(LM,LINUSCHR);
    } else {
        z = entries_cm[e]; me = E(CM,MARKEND); pp = E(CM,PRIOPLAY); cn = E(CM,COPYNORM); sk = E(CM,SKIPADD);
        wo = E(CM,WITCHONE); rq = E(CM,RELINTQ); lc = E(CM,LINUSCHR);
    }
    w[LANEC_A] = any_a(&r, v);
    /* PLAYTEM4: PRIO, NORM or MARK (bits 15/7) and LINUS or not (bit 14). */
    w[mpac + 4u] &= (uint16_t)~(040000u | 020000u | 0100u);
    switch ((v / 4u) % 3u) {
    case 0: w[mpac + 4u] |= 0100u; break;
    case 1: break;
    default: w[mpac + 4u] |= 040000u; break;
    }
    if ((v / 12u) & 1u) w[mpac + 4u] |= 020000u;
    /* LINUS: PLAYTEM3 equal to the flash address at COPINDEX, or not. */
    w[LANEC_LM_LOC] = (uint16_t)((v / 2u) % 4u == 3u ? lanec_rand(&r) % 0301u : (v / 2u) % 3u);
    if ((v / 24u) & 1u) {                       /* MAKEPRIO uses index 0, IFLEGAL index 2 */
        uint16_t i = e <= 2u ? (uint16_t)(((v / 6u) & 1u) * 2u) : e == 12u ? 2u : w[LANEC_LM_LOC];
        w[mpac + 3u] = w[LANEC_LM_CADRFLSH + i];
    }
    w[LANEC_LM_DSPLOCK] = (v / 3u) % 3u == 0 ? 0u : (uint16_t)(lanec_rand(&r) % 040000u);
    /* FLAGWRD4: clear a random subset of the busy and sleep bits. */
    w[LANEC_LM_FLAGWRD4] &= (uint16_t)~(lanec_rand(&r) & (lanec_rand(&r) | lanec_rand(&r)));
    if ((v / 5u) % 4u == 0) w[LANEC_LM_FLAGWRD4] &= (uint16_t)~074700u;
    if ((v / 7u) & 1u) w[m->mission == LANEC_LM ? E(LM,EBANKTEM) : E(CM,EBANKTEM)] ^= 010u;
    /* LOCCTR: sleeping (positive), zero or negative. */
    switch ((v / 3u) % 4u) {
    case 0: w[LANEC_LM_LOCCTR] = 0; break;
    case 1: w[LANEC_LM_LOCCTR] = (uint16_t)(040000u | lanec_rand(&r)); w[LANEC_LM_LOCCTR] &= 077777u; break;
    default: w[LANEC_LM_LOCCTR] = (uint16_t)(1u + lanec_rand(&r) % (01376u - LANEC_LM_LOC)); break;
    }
    /* A caller's return: fixed-fixed, or bank 10 outside the translated code. */
    if ((v / 8u) & 1u) q = (uint16_t)(rq + 2u + lanec_rand(&r) % (uint32_t)(lc - rq - 2u));
    else q = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    w[LANEC_Q] = q;
    lanec_enter(s, io, 010u, z);
    range(m, 010u, me, (uint16_t)(pp + 1u));
    range(m, 010u, cn, (uint16_t)(sk + 3u));
    range(m, 010u, wo, (uint16_t)(rq + 1u));
    range(m, 010u, lc, (uint16_t)(lc + 12u));
    range(m, LANEC_ANY_BANK, 1u, 2u);
}

const lanec_suite lanec_suite_di_play = { "di-play", ENTRIES * 2u * 48u, make, lanec_di_play_dispatch };
