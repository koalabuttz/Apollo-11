/* SPDX-License-Identifier: MIT */
/* The readable extended verbs' tracking and state vector verbs (their
 * basic code) against the faithful port's module
 * (c64-port/src/lanec_ev_track.c), side by side (make
 * extended-verbs-cases).
 *
 * The faithful module is built with its lanec_ev_track_dispatch renamed
 * faithful_lanec_ev_track_dispatch; the readable one of both missions is
 * lanec_ev_track_dispatch (extended_verbs_missions.c). First every case of
 * the port's suite (c64-port/tests/lanec_ev_track_cases.c). Then each of
 * the module's entries (its own lists, lanec_ev_track_entries: the LM's in
 * banks 40, 42 and 43, the CM's in 43) from 4,096 states of the port's
 * random fill, entered there and stirred: A +0, -0 or anything; Q a return
 * into fixed memory, a word of erasable memory, or anything; the
 * rendezvous and track flags (FLAGWRD0, FLAGWRD1) and EXTVBACT's bit 5
 * set or clear; FIXLOC a VAC area's address, one near the end of erasable
 * memory, or anything; now and then the bank registers or ZERO wrong, the
 * superbank bit off, the mission's other bank, Z one word off, or the
 * other mission. Where either runs, the other must too, with the same
 * memory and channels word for word; where the readable one refuses, it
 * must leave everything as it found it; and the faithful module must run
 * from some state of every entry. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_track.h"
#include "lanec_symbols.h"

enum lanec_status faithful_lanec_ev_track_dispatch(apollo_overlay_state *, lanec_io *,
                                                   enum lanec_mission);

#define A_AT   0u
#define Q_AT   2u
#define EB_AT  3u
#define FB_AT  4u
#define Z_AT   5u
#define BB_AT  6u
#define STATES 4096u

static const struct {
    enum lanec_mission mission;
    uint16_t bank, other;
} banks[4] = {
    {LANEC_LM, 040u, 042u},
    {LANEC_LM, 042u, 043u},
    {LANEC_LM, 043u, 040u},
    {LANEC_CM, 043u, 042u},
};

static uint32_t seed = 173205u;
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

/* Bit set or clear in w. */
static uint16_t either(uint16_t w, uint16_t bit)
{
    return (uint16_t)(rnd() & 1u ? w | bit : w & ~bit);
}

/* One state at entry z of bank b, from the port's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, unsigned b, uint16_t z,
                                unsigned n)
{
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
    w[LANEC_LM_FLAGWRD0] = either(w[LANEC_LM_FLAGWRD0], LANEC_LM_RNDVZBIT_WORD);
    w[LANEC_LM_FLAGWRD1] = either(w[LANEC_LM_FLAGWRD1], LANEC_LM_TRACKBIT_WORD);
    w[LANEC_LM_EXTVBACT] = either(w[LANEC_LM_EXTVBACT], LANEC_LM_BIT5_WORD);
    x = (uint16_t)(rnd() % 4u);
    w[LANEC_LM_FIXLOC] = x < 2u    ? (uint16_t)(rnd() % (02000u - LANEC_LM_QPRET))
                         : x == 2u ? (uint16_t)(02000u - LANEC_LM_QPRET - 2u + rnd() % 4u)
                                   : (uint16_t)(rnd() & 077777u);
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
    sf = faithful_lanec_ev_track_dispatch(fa, &fio, m);
    sr = lanec_ev_track_dispatch(ra, &rio, m);
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
    for (n = 0; n < lanec_suite_ev_track.count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        const char *why;
        int ran;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_track.make(n, &start, &io, &meta);
        why = compare(&start, &io, meta.mission, &ran, &fa, &ra);
        suite_ran += (unsigned)ran;
        if (why && ++failures <= 3) {
            printf("FAIL suite case %u (%s, Z %04o): %s\n", n,
                   meta.mission == LANEC_LM ? "LM" : "CM", start.words[Z_AT], why);
            show(&start, &fa, &ra);
        }
    }
    printf("ev-track-cases: the port's suite, %u cases (%u run), %u failures\n",
           lanec_suite_ev_track.count, suite_ran, failures);

    /* Each entry from stirred states. */
    for (b = 0; b < 4u; ++b) {
        uint16_t count;
        const uint16_t *entries = lanec_ev_track_entries(banks[b].mission, banks[b].bank, &count);

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
    printf("ev-track-cases: %u entries from %u states each, %u failures\n", keys, STATES, failures);
    return failures != 0;
}
