/* SPDX-License-Identifier: MIT */
/* Cases for the ENDIDLE handling and display returns: every basic block,
 * every COPINDEX and COPMPAC, CADRSTOR and OUTHERE of each sign, repeat
 * and return requests set and clear, verbs either side of V21-V23, loads
 * of the same and another noun, PRIO times either side of two seconds,
 * A with and without overflow, and the flag combinations each test reads. */
#include "lanec_suites.h"
#include "lanec_di_idle.h"
#include "lanec_symbols.h"

#define ENTRIES 40u
static const uint8_t offsets[ENTRIES] = { 0, 4, 5, 10, 14, 18, 21, 22, 28, 37, 38, 39, 40, 46, 47, 54, 64, 66,
    70, 77, 79, 80, 83, 85, 91, 92, 95, 110, 112, 118, 124, 128, 132, 136, 144, 145, 150, 158, 160, 161 };

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t signed_word(uint32_t *r, uint32_t v)
{
    uint16_t x = (uint16_t)(lanec_rand(r) & 077777u);
    switch (v % 5u) {
    case 0: return 0;
    case 1: return 077777u;
    case 2: return (uint16_t)(x | 040000u);
    case 3: return (uint16_t)(1u + x % 4u);
    default: return (uint16_t)(x & 037777u);
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, e = (uint16_t)(n % ENTRIES), mpac = LANEC_LM_MPAC, ce, li, pr, er, x;
    uint32_t r = n * 747796405u + 1u, v = n / ENTRIES / 2u;
    lanec_fill(s, io, n);
    m->mission = (n / ENTRIES) & 1u ? LANEC_CM : LANEC_LM;
    if (m->mission == LANEC_LM) { ce = LANEC_LM_CLEANEND; li = LANEC_LM_LINUSCHR; pr = LANEC_LM_PROCEED; er = LANEC_LM_ERASER; }
    else { ce = LANEC_CM_CLEANEND; li = LANEC_CM_LINUSCHR; pr = LANEC_CM_PROCEED; er = LANEC_CM_ERASER; }
    w[LANEC_LM_LOC] = (uint16_t)(v % 3u);                                        /* COPINDEX */
    w[mpac + 3u] = (uint16_t)((v / 3u) % 3u);                                    /* COPMPAC */
    w[LANEC_LM_CADRSTOR] = signed_word(&r, v / 2u);
    w[mpac + 5u] = signed_word(&r, v / 5u);                                      /* OUTHERE */
    w[mpac] = (v / 4u) % 3u == 0 ? (uint16_t)(lanec_rand(&r) & 077777u) : (uint16_t)(023u + (v / 4u) % 7u - 3u + 1u);
    x = (uint16_t)(w[LANEC_LM_NVWORD + w[mpac + 3u]] & 0177u);
    w[mpac + 1u] = (v / 6u) & 1u ? x : (uint16_t)(lanec_rand(&r) & 077777u);    /* NOUNREG */
    if ((v / 7u) & 1u) {                       /* PRIO time near 2 s: equal, exactly 2 s, or random */
        x = (v / 14u) % 3u == 0 ? 0u : (v / 14u) % 3u == 1u ? 0310u : (uint16_t)(lanec_rand(&r) % 0620u);
        w[m->mission == LANEC_LM ? LANEC_LM_PRIOTIME : LANEC_CM_PRIOTIME] =
            (uint16_t)((w[LANEC_LM_TIME1] + 077777u - x) % 077777u);
    }
    if ((v / 8u) % 3u == 0) w[LANEC_LM_NVSAVE] = 0;
    if ((v / 9u) % 3u == 0) w[m->mission == LANEC_LM ? LANEC_LM_EBANKTEM : LANEC_CM_EBANKTEM] &= (uint16_t)~050u;
    w[LANEC_LM_FLAGWRD4] &= (uint16_t)~(lanec_rand(&r) & lanec_rand(&r));
    x = (uint16_t)lanec_rand(&r);
    switch ((v / 2u) % 4u) {
    case 0: w[LANEC_A] = (uint16_t)(0040000u | (x & 037777u)); break;
    case 1: w[LANEC_A] = (uint16_t)(0100000u | (x & 037777u)); break;
    default: w[LANEC_A] = lanec_a16((uint16_t)(x & 077777u)); break;
    }
    lanec_enter(s, io, 010u, (uint16_t)(ce + offsets[e]));
    range(m, 010u, ce, (uint16_t)(li - 1u));
    range(m, 010u, pr, (uint16_t)(er + 3u));
}

const lanec_suite lanec_suite_di_idle = { "di-idle", ENTRIES * 2u * 48u, make, lanec_di_idle_dispatch };
