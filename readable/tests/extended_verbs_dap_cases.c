/* SPDX-License-Identifier: MIT */
/* The readable extended verbs' DAP data load (verb 48) against the
 * faithful port's module (c64-port/src/lanec_ev_dap.c), side by side
 * (make extended-verbs-cases).
 *
 * The faithful module is built with its lanec_ev_dap_dispatch renamed
 * faithful_lanec_ev_dap_dispatch; the readable one of both missions is
 * lanec_ev_dap_dispatch (extended_verbs_missions.c). First every case of
 * the port's suite (c64-port/tests/lanec_ev_dap_cases.c). Then each of
 * the module's entries (its own lists, lanec_ev_dap_entries: the LM's in
 * banks 20 and 01, the CM's in 43 and 42) from 4,096 states of the port's
 * random fill, entered there and stirred: A +0, -0 or anything; Q a return
 * into fixed memory, a word of erasable memory, or anything; DAPBOOLS and
 * DAPDATR1 (the CM's in E-bank 6, and under the E-bank the fill leaves)
 * anything, with the configuration bits now and then each set or clear;
 * APSFLAG set or clear; FLAGWRD6's TVC bits 00, 01, 10 or 11; the masses
 * (LEMMASS, CSMMASS) small, at their least (where the sums are -0) or
 * anything, and MASS anything; now and then the bank registers or ZERO
 * wrong, the superbank bit off, the mission's other bank, Z one word off,
 * or the other mission. Where either runs, the other must too, with the
 * same memory and channels word for word; where the readable one refuses,
 * it must leave everything as it found it; and the faithful module must
 * run from some state of every entry. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_dap.h"
#include "lanec_symbols.h"

enum lanec_status faithful_lanec_ev_dap_dispatch(apollo_overlay_state *, lanec_io *,
                                                 enum lanec_mission);

#define A_AT   0u
#define Q_AT   2u
#define EB_AT  3u
#define FB_AT  4u
#define Z_AT   5u
#define BB_AT  6u
#define E6     06u
#define STATES 4096u

static const struct {
    enum lanec_mission mission;
    uint16_t bank, other;
} banks[4] = {
    {LANEC_LM, 020u, 01u},
    {LANEC_LM, 01u, 020u},
    {LANEC_CM, 043u, 042u},
    {LANEC_CM, 042u, 043u},
};

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A 16-bit register: a sign-extended word, or now and then an overflow. */
static uint16_t reg16(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w << 1) & 0100000u));
}

/* A mass: small, at a least (where the sums are -0), or anything. */
static uint16_t mass(uint16_t least)
{
    switch (rnd() % 4u) {
    case 0: return (uint16_t)(rnd() & 07777u);
    case 1: return least;
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* Bits 14 and 13, the configuration and stage, each set or clear. */
static uint16_t configured(uint16_t w)
{
    return (uint16_t)((w & ~030000u) | ((rnd() & 3u) << 12));
}

/* Both copies of a word of E-bank 6: its own, and under the E-bank the
 * fill leaves. */
static void e6(apollo_overlay_state *s, uint16_t address, uint16_t value)
{
    s->words[E6 * 0400u + (address & 0377u)] = value;
    s->words[lanec_erasable(s, address)] = value;
}

/* One state at entry z of bank b, from the port's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, unsigned b, uint16_t z,
                                unsigned n)
{
    static const uint16_t lem_least[2] = {02357u, 01046u}; /* both stages, the ascent's */
    uint16_t *w = s->words, x;
    enum lanec_mission m = banks[b].mission;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lanec_fill(s, io, n * 2654435761u + z + b);
    lanec_enter(s, io, banks[b].bank, z);
    x = (uint16_t)(rnd() % 4u);
    w[A_AT] = x == 0u ? 0u : x == 1u ? 0177777u : reg16();
    x = (uint16_t)(rnd() % 16u);
    w[Q_AT] = x < 10u   ? (uint16_t)(02000u + rnd() % 06000u)
              : x < 13u ? (uint16_t)(rnd() % 02000u)
                        : reg16();
    if (m == LANEC_LM) {
        w[LANEC_LM_DAPBOOLS] = rnd() & 1u ? configured(w[LANEC_LM_DAPBOOLS]) : rnd() & 077777u;
        w[LANEC_LM_DAPDATR1] = rnd() & 1u ? configured(w[LANEC_LM_DAPDATR1]) : rnd() & 077777u;
        w[LANEC_LM_FLGWRD10] =
            (uint16_t)(rnd() & 1u ? w[LANEC_LM_FLGWRD10] | LANEC_LM_APSFLBIT_WORD
                                  : w[LANEC_LM_FLGWRD10] & ~LANEC_LM_APSFLBIT_WORD);
        w[LANEC_LM_LEMMASS] = mass(lem_least[rnd() & 1u]);
        w[LANEC_LM_CSMMASS] = mass(02000u);
        w[LANEC_LM_MASS] = (uint16_t)(rnd() & 077777u);
        e6(s, LANEC_LM_STIKSENS, (uint16_t)(rnd() & 077777u));
    } else {
        e6(s, LANEC_CM_DAPDATR1,
           rnd() & 1u ? configured((uint16_t)(rnd() & 077777u)) : (uint16_t)(rnd() & 077777u));
        w[LANEC_CM_FLAGWRD6] = (uint16_t)((w[LANEC_CM_FLAGWRD6] & ~060000u) | ((rnd() & 3u) << 13));
    }
    /* Now and then a state the module does not take. */
    switch (rnd() % 64u) {
    case 0: w[FB_AT] = (uint16_t)(rnd() & 076000u); break;
    case 1: io->superbank &= (uint16_t)~0100u; break;
    case 2: w[BB_AT] ^= 02000u; break;
    case 3: w[EB_AT] |= 1u; break;
    case 4: w[7] = 1u; break;
    case 5: m = m == LANEC_LM ? LANEC_CM : LANEC_LM; break;
    case 6: w[Z_AT] ^= 1u; break;
    case 7: lanec_enter(s, io, banks[b].other, z); break;
    default: break;
    }
    return m;
}

/* Run both on one state; a description of the difference, or NULL. */
static const char *compare(const apollo_overlay_state *start, const lanec_io *io,
                           enum lanec_mission m, int *ran, apollo_overlay_state *fa,
                           apollo_overlay_state *ra)
{
    lanec_io fio = *io, rio = *io;
    enum lanec_status sf, sr;

    *fa = *ra = *start;
    sf = faithful_lanec_ev_dap_dispatch(fa, &fio, m);
    sr = lanec_ev_dap_dispatch(ra, &rio, m);
    *ran = sf == LANEC_OK;
    if (sr != LANEC_OK && (memcmp(ra, start, sizeof *ra) || memcmp(&rio, io, sizeof rio)))
        return "readable refusal changed the state";
    if (sf == LANEC_OK && sr != LANEC_OK) return "readable refused, faithful ran";
    if (sf != LANEC_OK && sr == LANEC_OK) return "readable ran, faithful refused";
    if (sf == LANEC_OK && memcmp(fa, ra, sizeof *fa)) return "different memory";
    if (sf == LANEC_OK && memcmp(&fio, &rio, sizeof fio)) return "different channels";
    return NULL;
}

static void show(const apollo_overlay_state *start, const apollo_overlay_state *fa,
                 const apollo_overlay_state *ra)
{
    unsigned w;
    for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
        if (fa->words[w] != ra->words[w])
            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra->words[w], fa->words[w],
                   start->words[w]);
}

int main(void)
{
    unsigned b, i, n, keys = 0, failures = 0, suite_ran = 0;

    /* The port's suite, case by case. */
    for (n = 0; n < lanec_suite_ev_dap.count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        const char *why;
        int ran;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_dap.make(n, &start, &io, &meta);
        why = compare(&start, &io, meta.mission, &ran, &fa, &ra);
        suite_ran += (unsigned)ran;
        if (why && ++failures <= 3) {
            printf("FAIL suite case %u (%s, Z %04o): %s\n", n,
                   meta.mission == LANEC_LM ? "LM" : "CM", start.words[Z_AT], why);
            show(&start, &fa, &ra);
        }
    }
    printf("ev-dap-cases: the port's suite, %u cases (%u run), %u failures\n",
           lanec_suite_ev_dap.count, suite_ran, failures);

    /* Each entry from stirred states. */
    for (b = 0; b < 4u; ++b) {
        uint16_t count;
        const uint16_t *entries = lanec_ev_dap_entries(banks[b].mission, banks[b].bank, &count);

        for (i = 0; i < count; ++i, ++keys) {
            unsigned ran_total = 0, bad = 0;

            for (n = 0; n < STATES; ++n) {
                apollo_overlay_state start, fa, ra;
                lanec_io io;
                enum lanec_mission m = state(&start, &io, b, entries[i], n);
                int ran;
                const char *why = compare(&start, &io, m, &ran, &fa, &ra);

                ran_total += (unsigned)ran;
                if (why && ++bad <= 3) {
                    printf("FAIL %s bank %02o %04o state %u: %s\n",
                           banks[b].mission == LANEC_LM ? "LM" : "CM", banks[b].bank, entries[i], n,
                           why);
                    show(&start, &fa, &ra);
                }
            }
            if (ran_total == 0u && bad == 0u) {
                printf("FAIL %s bank %02o %04o: the faithful routine never ran\n",
                       banks[b].mission == LANEC_LM ? "LM" : "CM", banks[b].bank, entries[i]);
                ++bad;
            }
            failures += bad;
        }
    }
    printf("ev-dap-cases: %u entries from %u states each, %u failures\n", keys, STATES, failures);
    return failures != 0;
}
