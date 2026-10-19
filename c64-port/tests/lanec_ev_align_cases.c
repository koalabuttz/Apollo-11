/* SPDX-License-Identifier: MIT */
/* Cases for the EXTENDED_VERBS alignment and radar-designate bodies: every
 * entry of both missions with nouns 20, 72, 91 and others, RADMODES,
 * SWSAMPLE, OPTIND of each CCS class, every radar-use flag combination,
 * MODREG 22, VAC addresses for RRDESK2, random channels 12, 30 and 31, and
 * an overflowing A into AURLOKON's first store. */
#include "lanec_suites.h"
#include "lanec_ev_align.h"
#include "lanec_symbols.h"

#define VARIANTS 32u

static const uint16_t lm_spans[][3] = {
    { 043u, LANEC_LM_VBZERO, LANEC_LM_LRP2ALM + 2u }, { 043u, LANEC_LM_VBCOARK, LANEC_LM_RRDESEND + 15u },
    { 043u, LANEC_LM_IMUFINEK, LANEC_LM_FINEK2 + 9u }, { 043u, LANEC_LM_RDRUSECK, LANEC_LM_CHECKP22 + 5u },
    { 043u, LANEC_LM_IMUATTCK, LANEC_LM_IMUATTCK + 30u }, { 023u, LANEC_LM_AURLOKON, LANEC_LM_AURLKON1 + 4u } };
static const uint16_t cm_spans[][3] = {
    { 043u, LANEC_CM_VBZERO, LANEC_CM_FINEK2 + 9u }, { 043u, LANEC_CM_IMUATTCK, LANEC_CM_XACT0Q + 1u } };

static void range(lanec_meta *m, uint16_t bank, uint16_t lo, uint16_t hi)
{
    m->range[m->ranges].bank = bank; m->range[m->ranges].lo = lo; m->range[m->ranges].hi = hi; ++m->ranges;
}

static uint16_t ccs_class(uint32_t *r, uint8_t c)
{
    uint16_t v = (uint16_t)(lanec_rand(r) & 037777u);
    switch (c & 3u) {
    case 0: return v ? v : 1u;
    case 1: return 0;
    case 2: return (uint16_t)(040000u | (v == 037777u ? 0u : v));
    default: return 077777u;
    }
}

static void make(uint32_t n, apollo_overlay_state *s, lanec_io *io, lanec_meta *m)
{
    static const uint16_t nouns[4] = { 024u, 0110u, 0133u, 0u };
    uint16_t *w = s->words, lm_n, cm_n, e, key, i, bit;
    const uint16_t *lm = lanec_ev_align_entries(LANEC_LM, &lm_n), *cm = lanec_ev_align_entries(LANEC_CM, &cm_n);
    uint32_t r = n * 747796405u + 1u, v;
    e = (uint16_t)(n % (uint32_t)(lm_n + cm_n));
    v = n / (uint32_t)(lm_n + cm_n);
    lanec_fill(s, io, n);
    m->mission = e < lm_n ? LANEC_LM : LANEC_CM;
    key = e < lm_n ? lm[e] : cm[e - lm_n];
    lanec_enter(s, io, key & 010000u ? 023u : 043u, (uint16_t)(key & 07777u));
    w[LANEC_Q] = (uint16_t)(04000u + lanec_rand(&r) % 04000u);
    w[LANEC_LM_NOUNREG] = nouns[v & 3u] ? nouns[v & 3u] : (uint16_t)(lanec_rand(&r) & 0177u);
    w[LANEC_LM_RADMODES] = ccs_class(&r, (uint8_t)(v >> 1));
    w[LANEC_CM_SWSAMPLE] = ccs_class(&r, (uint8_t)(v >> 2));
    w[LANEC_CM_OPTIND] = ccs_class(&r, (uint8_t)(v >> 3));
    if ((v & 7u) == 5u) w[LANEC_LM_MODREG] = 026u;                     /* P22 */
    /* the radar-use and lift-off flag bits: each set and clear across the variants */
    for (i = 0; i < 8u; ++i) {
        static const uint16_t word[8] = { LANEC_LM_FLAGWRD3, LANEC_LM_FLAGWRD5, LANEC_LM_FLAGWRD7, LANEC_LM_FLGWRD11,
                                          LANEC_LM_FLAGWRD1, LANEC_LM_FLAGWRD0, LANEC_CM_FLAGWRD5, LANEC_LM_OPTIONX + 1u };
        static const uint16_t mask[8] = { LANEC_LM_NR29FBIT_WORD, LANEC_LM_R77FLBIT_WORD, LANEC_LM_V37FLBIT_WORD,
                                          LANEC_LM_LRBYBIT_WORD, LANEC_LM_TRACKBIT_WORD, LANEC_LM_RNDVZBIT_WORD,
                                          LANEC_CM_GRRBKBIT_WORD, LANEC_LM_BIT2_WORD };
        bit = (uint16_t)(lanec_rand(&r) & 1u);
        w[word[i]] = (uint16_t)(bit ? w[word[i]] | mask[i] : w[word[i]] & ~mask[i]);
    }
    if (v & 4u) io->channel[012] &= (uint16_t)~030u;
    if (v & 8u) io->channel[031] |= 030000u;
    if (v & 16u) io->channel[030] &= (uint16_t)~020u;
    /* RRDESK2: no VAC, or a VAC word in ordinary unswitched erasable */
    w[LANEC_LM_PRIORITY] = (uint16_t)((w[LANEC_LM_PRIORITY] & ~0777u) |
                                      ((v & 1u) ? 0u : 062u + lanec_rand(&r) % 0716u));
    /* AURLOKON +1 stores A from MAKECADR: sometimes an overflowed value */
    if (key == (010000u | (LANEC_LM_AURLOKON + 1u)) && (v & 3u) == 1u)
        w[LANEC_A] = (uint16_t)((v & 4u ? 0040000u : 0100000u) | (lanec_rand(&r) & 037777u));
    m->ranges = 0;
    if (m->mission == LANEC_LM) for (i = 0; i < 6u; ++i) range(m, lm_spans[i][0], lm_spans[i][1], lm_spans[i][2]);
    else for (i = 0; i < 2u; ++i) range(m, cm_spans[i][0], cm_spans[i][1], cm_spans[i][2]);
    m->range[m->ranges].bank = LANEC_ANY_BANK; m->range[m->ranges].lo = LANEC_Q;   /* TC Q runs Q */
    m->range[m->ranges].hi = LANEC_Q; ++m->ranges;
}

const lanec_suite lanec_suite_ev_align = { "ev-align", (95u + 63u) * VARIANTS, make, lanec_ev_align_dispatch };
