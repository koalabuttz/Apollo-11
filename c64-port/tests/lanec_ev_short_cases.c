/* SPDX-License-Identifier: MIT */
/* Cases for the EXTENDED_VERBS short verb bodies: every entry of both
 * missions with random flag words, MODECADR, MODREG and SKEEP6 at zero and
 * -0 as well as random, every combination of the rendezvous/track/R23 bits,
 * and the TVC and docked DAP bits. */
#include "lanec_suites.h"
#include "lanec_ev_short.h"
#include "lanec_symbols.h"

#define VARIANTS 32u
#define B 043u

/* Bank 43 code these bodies translate, per mission; each case sees every span
 * the native code may continue into, so the original stops where it does. */
static const uint16_t lm_spans[][2] = {
    { LANEC_LM_DAPATTER, LANEC_LM_TOTATTER + 2u }, { LANEC_LM_LRON, LANEC_LM_LROFF + 2u },
    { LANEC_LM_GOLOADLV, LANEC_LM_ALINTIME + 2u }, { LANEC_LM_VB64, LANEC_LM_VB64 + 6u },
    { LANEC_LM_V82PERF, LANEC_LM_ATTACHED + 4u }, { LANEC_LM_VERB96, LANEC_LM_VERB85 + 2u } };
static const uint16_t cm_spans[][2] = {
    { LANEC_CM_CKMODCAD, LANEC_CM_V63 + 4u }, { LANEC_CM_SYSTEST, LANEC_CM_CHAZFOGC + 9u },
    { LANEC_CM_VB64, LANEC_CM_CREWMANU + 6u }, { LANEC_CM_V82PERF, LANEC_CM_V86PERF + 4u },
    { LANEC_CM_LEMVEC, LANEC_CM_RESETPRF + 2u }, { LANEC_CM_VERB94, LANEC_CM_RESTSRF + 2u },
    { LANEC_CM_ATTACHED, LANEC_CM_ATTACHED + 4u } };

static void range(lanec_meta *m, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = B; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t signed_word(uint32_t *r, uint8_t k)
{
    switch (k & 3u) {
    case 0: return 0;
    case 1: return 077777u;
    default: return (uint16_t)(lanec_rand(r) & 077777u);
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    uint16_t *w = s->words, lm_n, cm_n, e, z, i;
    const uint16_t *lm = lanec_ev_short_entries(LANEC_LM, &lm_n), *cm = lanec_ev_short_entries(LANEC_CM, &cm_n);
    uint32_t r = n * 747796405u + 1u, v;
    e = (uint16_t)(n % (uint32_t)(lm_n + cm_n));
    v = n / (uint32_t)(lm_n + cm_n);
    lanec_fill(s, io, n);
    m->mission = e < lm_n ? LANEC_LM : LANEC_CM;
    z = e < lm_n ? lm[e] : cm[e - lm_n];
    lanec_enter(s, io, B, z);
    /* Callers' Q: a fixed-memory return address outside these bodies. */
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    w[LANEC_LM_MODECADR] = signed_word(&r, (uint8_t)v);           /* LM 01304 */
    w[LANEC_CM_MODECADR] = signed_word(&r, (uint8_t)(v >> 1));    /* CM 01322 */
    w[LANEC_LM_MODREG] = signed_word(&r, (uint8_t)(v >> 2));
    w[LANEC_LM_SKEEP6] = signed_word(&r, (uint8_t)(v >> 3));
    /* flag bits read by the bodies: each combination across the variants */
    w[LANEC_CM_FLAGWRD0] = (uint16_t)((w[LANEC_CM_FLAGWRD0] & ~LANEC_CM_RNDVZBIT_WORD) | (v & 1u ? LANEC_CM_RNDVZBIT_WORD : 0u));
    w[LANEC_CM_FLAGWRD1] = (uint16_t)((w[LANEC_CM_FLAGWRD1] & ~(LANEC_CM_TRACKBIT_WORD | LANEC_CM_R23BIT_WORD)) |
                                      (v & 2u ? LANEC_CM_TRACKBIT_WORD : 0u) | (v & 4u ? LANEC_CM_R23BIT_WORD : 0u));
    if (v & 8u) w[LANEC_CM_FLAGWRD6] &= (uint16_t)~LANEC_CM_OCT60000_WORD;
    if (v & 16u) w[LANEC_CM_FLAGWRD1] &= (uint16_t)~LANEC_CM_NOP01BIT_WORD;
    m->ranges = 0;
    if (m->mission == LANEC_LM) {
        for (i = 0; i < sizeof lm_spans / sizeof lm_spans[0]; ++i) range(m, lm_spans[i][0], lm_spans[i][1]);
    } else {
        if (z >= LANEC_CM_ATTACHED && z <= LANEC_CM_ATTACHED + 4u) range(m, cm_spans[6][0], cm_spans[6][1]);
        else for (i = 0; i < 6u; ++i) range(m, cm_spans[i][0], cm_spans[i][1]);
    }
    m->range[m->ranges].bank = LANEC_ANY_BANK; m->range[m->ranges].lo = LANEC_Q;   /* TC Q runs Q */
    m->range[m->ranges].hi = LANEC_Q; ++m->ranges;
}

const lanec_suite lanec_suite_ev_short = { "ev-short", (84u + 115u) * VARIANTS, make, lanec_ev_short_dispatch };
