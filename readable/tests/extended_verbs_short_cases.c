/* SPDX-License-Identifier: MIT */
/* The readable extended verbs' short verbs against the faithful port's
 * module (c64-port/src/lanec_ev_short.c), side by side (make
 * extended-verbs-cases).
 *
 * The faithful module is built with its lanec_ev_short_dispatch renamed
 * faithful_lanec_ev_short_dispatch; the readable one of both missions is
 * lanec_ev_short_dispatch (extended_verbs_missions.c). First every case
 * of the port's suite (c64-port/tests/lanec_ev_short_cases.c: 6,368, both
 * missions, every entry). Then each of the module's entries (its own list,
 * lanec_ev_short_entries) from 4,096 states of the port's random fill,
 * entered there and stirred: A +0, -0 or anything (CKMODCAD +1 tests it);
 * Q a return into fixed memory, another of the module's entries (where
 * EXDAPOFF's TC Q runs on), a word of erasable memory, or anything;
 * MODECADR, MODREG and SKEEP6 +0, -0 or anything; the flag words, the
 * CM's E-bank 5 and 6 words and the rest as the fill leaves them; now and
 * then the bank registers or ZERO wrong, the superbank bit off, Z one word
 * off, or the other mission. Where either runs, the other must too, with
 * the same memory and channels word for word; where the readable one
 * refuses, it must leave everything as it found it; and the faithful
 * module must run from some state of every entry. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_short.h"
#include "lanec_symbols.h"

enum lanec_status faithful_lanec_ev_short_dispatch(apollo_overlay_state *, lanec_io *,
                                                   enum lanec_mission);

#define A_AT   0u
#define Q_AT   2u
#define EB_AT  3u
#define FB_AT  4u
#define Z_AT   5u
#define BB_AT  6u
#define BANK43 043u
#define STATES 4096u

static uint32_t seed = 271828u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* +0, -0 or anything. */
static uint16_t signed_word(void)
{
    switch (rnd() % 3u) {
    case 0: return 0u;
    case 1: return 077777u;
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* A 16-bit register: a sign-extended word, or now and then an overflow. */
static uint16_t reg16(void)
{
    uint16_t w = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) return (uint16_t)(rnd() | (rnd() << 1));
    return (uint16_t)(w | ((w << 1) & 0100000u));
}

/* One state at entry z of mission m, from the port's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, enum lanec_mission m,
                                uint16_t z, unsigned n)
{
    uint16_t *w = s->words, count;
    const uint16_t *entries = lanec_ev_short_entries(m, &count);
    uint16_t x;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lanec_fill(s, io, n * 2654435761u + z);
    lanec_enter(s, io, BANK43, z);
    x = (uint16_t)(rnd() % 4u);
    w[A_AT] = x == 0u ? 0u : x == 1u ? 0177777u : reg16();
    x = (uint16_t)(rnd() % 16u);
    w[Q_AT] = x < 8u    ? (uint16_t)(02000u + rnd() % 06000u)
              : x < 11u ? entries[rnd() % count]
              : x < 13u ? (uint16_t)(rnd() % 02000u)
                        : reg16();
    w[LANEC_LM_MODECADR] = signed_word();
    w[LANEC_CM_MODECADR] = signed_word();
    w[LANEC_LM_MODREG] = signed_word();
    w[LANEC_LM_SKEEP6] = signed_word();
    /* Now and then a state the module does not take. */
    switch (rnd() % 64u) {
    case 0: w[FB_AT] = (uint16_t)(rnd() & 076000u); break;
    case 1: io->superbank &= (uint16_t)~0100u; break;
    case 2: w[BB_AT] ^= 02000u; break;
    case 3: w[EB_AT] |= 1u; break;
    case 4: w[7] = 1u; break;
    case 5: m = m == LANEC_LM ? LANEC_CM : LANEC_LM; break;
    case 6: w[Z_AT] ^= 1u; break;
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
    sf = faithful_lanec_ev_short_dispatch(fa, &fio, m);
    sr = lanec_ev_short_dispatch(ra, &rio, m);
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
    static const enum lanec_mission missions[] = {LANEC_LM, LANEC_CM};
    unsigned i, k, n, keys = 0, failures = 0, suite_ran = 0;

    /* The port's suite, case by case. */
    for (n = 0; n < lanec_suite_ev_short.count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        const char *why;
        int ran;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_short.make(n, &start, &io, &meta);
        why = compare(&start, &io, meta.mission, &ran, &fa, &ra);
        suite_ran += (unsigned)ran;
        if (why && ++failures <= 3) {
            printf("FAIL suite case %u (%s, Z %04o): %s\n", n,
                   meta.mission == LANEC_LM ? "LM" : "CM", start.words[Z_AT], why);
            show(&start, &fa, &ra);
        }
    }
    printf("ev-short-cases: the port's suite, %u cases (%u run), %u failures\n",
           lanec_suite_ev_short.count, suite_ran, failures);

    /* Each entry from stirred states. */
    for (k = 0; k < 2u; ++k) {
        uint16_t count;
        const uint16_t *entries = lanec_ev_short_entries(missions[k], &count);

        for (i = 0; i < count; ++i, ++keys) {
            unsigned ran_total = 0, bad = 0;

            for (n = 0; n < STATES; ++n) {
                apollo_overlay_state start, fa, ra;
                lanec_io io;
                enum lanec_mission m = state(&start, &io, missions[k], entries[i], n);
                int ran;
                const char *why = compare(&start, &io, m, &ran, &fa, &ra);

                ran_total += (unsigned)ran;
                if (why && ++bad <= 3) {
                    printf("FAIL %s %04o state %u: %s\n", k ? "CM" : "LM", entries[i], n, why);
                    show(&start, &fa, &ra);
                }
            }
            if (ran_total == 0u && bad == 0u) {
                printf("FAIL %s %04o: the faithful routine never ran\n", k ? "CM" : "LM",
                       entries[i]);
                ++bad;
            }
            failures += bad;
        }
    }
    printf("ev-short-cases: %u entries from %u states each, %u failures\n", keys, STATES, failures);
    return failures != 0;
}
