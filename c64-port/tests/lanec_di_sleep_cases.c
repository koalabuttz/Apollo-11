/* SPDX-License-Identifier: MIT */
/* Cases for the display sleep and wake code: every entry, PRIO, MARK and
 * NORM indices, sleeping, absent (-1) and idle LOCCTR, mark-over-norm set
 * and clear, WAKEPLAY with and without overflow, and any EBANKTEM,
 * RESTREG and TEMPFLSH for INITDSP. */
#include "lanec_suites.h"
#include "lanec_di_sleep.h"
#include "lanec_symbols.h"

#define ENTRIES 17u
#define E(M, X) LANEC_##M##_##X
#define ENTRY_LIST(M) { E(M,GOSLEEPS), E(M,WAITMASK) + 1u, E(M,XCHSLEEP), E(M,XCHSLEEP) + 4u, \
    E(M,XCHSLEEP) + 5u, E(M,JOBXCHS), E(M,JOBXCHS) + 3u, E(M,JOBXCHS) + 8u, E(M,IDLEMASK) + 1u, \
    E(M,XCHQADD) + 1u, E(M,MARKWAKE), E(M,WAKEPLAY), E(M,MARKFMSK) + 1u, E(M,MARKFMSK) + 5u, \
    E(M,INITDSP), E(M,INITDSP) + 4u, E(M,INITDSP) + 6u }
static const uint16_t entries_lm[ENTRIES] = ENTRY_LIST(LM);
static const uint16_t entries_cm[ENTRIES] = ENTRY_LIST(CM);

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), z, x;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    uint16_t gs, id, wo, rq;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) {
        z = entries_lm[e]; gs = E(LM,GOSLEEPS); id = E(LM,INITDSP); wo = E(LM,WITCHONE); rq = E(LM,RELINTQ);
    } else {
        z = entries_cm[e]; gs = E(CM,GOSLEEPS); id = E(CM,INITDSP); wo = E(CM,WITCHONE); rq = E(CM,RELINTQ);
    }
    w[LANEC_LM_LOC] = (uint16_t)(e == 0u ? v % 3u : 1u + v % 2u);   /* COPINDEX */
    w[LANEC_LM_MPAC] = (v / 2u) % 3u == 2u ? 077777u : (uint16_t)((v / 2u) & 1u);   /* FACEREG, -0 for MARK */
    w[LANEC_LM_TEMPOR2] = (uint16_t)((v / 4u) & 1u);
    switch ((v / 4u) % 4u) {                                         /* LOCCTR after JOBWAKE */
    case 0: w[LANEC_LM_LOCCTR] = 077776u; break;
    case 1: w[LANEC_LM_LOCCTR] = 0; break;
    default: w[LANEC_LM_LOCCTR] = (uint16_t)(1u + lanec_rand(&r) % (01376u - LANEC_LM_LOC)); break;
    }
    if (e == 3u && (v / 16u) % 4u == 3u) w[LANEC_LM_LOCCTR] = (uint16_t)(040000u | (lanec_rand(&r) & 037777u));
    x = (uint16_t)lanec_rand(&r);
    switch ((v / 8u) % 4u) {                                         /* A: 0, 1 or an overflow */
    case 0: w[LANEC_A] = 0; break;
    case 1: w[LANEC_A] = 1u; break;
    case 2: w[LANEC_A] = (uint16_t)(0040000u | (x & 037777u)); break;
    default: w[LANEC_A] = (uint16_t)(0100000u | (x & 037777u)); break;
    }
    if (e == 5u) w[LANEC_A] &= 1u;
    if ((v / 3u) & 1u) w[LANEC_LM_FLAGWRD4] ^= (uint16_t)(m->mission == LANEC_LM ? LANEC_LM_MKOVBIT_WORD : LANEC_CM_BIT3_WORD);
    lanec_enter(s, io, 010u, z);
    range(m, 010u, gs, (uint16_t)(id + 8u));
    range(m, 010u, wo, (uint16_t)(rq + 1u));
    range(m, LANEC_ANY_BANK, 1u, 2u);
}

const lanec_suite lanec_suite_di_sleep = { "di-sleep", ENTRIES * 2u * 64u, make, lanec_di_sleep_dispatch };
