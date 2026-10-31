/* SPDX-License-Identifier: MIT */
/* The readable extended verbs' front end against the faithful port's
 * module (c64-port/src/lanec_ev_front.c), side by side (make
 * extended-verbs-cases).
 *
 * The faithful module is built with its lanec_ev_front_dispatch renamed
 * faithful_lanec_ev_front_dispatch; the readable one of both missions is
 * lanec_ev_front_dispatch (extended_verbs_missions.c). First every case
 * of the port's suite (c64-port/tests/lanec_ev_front_cases.c: 5,760, both
 * missions, every key). Then each key from 4,096 states of the port's
 * random fill, entered at the key and stirred: A, L and Q anything (A and
 * Q with an overflow now and then); EXTVBACT and FLAGWRD4 of each CCS
 * outcome, with OC24100's bits or without; MPAC a verb, or now and then
 * past the fan; MPAC +1 a return into fixed memory, or now and then
 * anything; MODREG +0, -0 or a program; now and then the bank registers
 * or ZERO wrong, the superbank bit off, or the other mission. Where either
 * runs, the other must too, with the same memory and channels word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful module must run from some state of every
 * key. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_front.h"
#include "lanec_symbols.h"

enum lanec_status faithful_lanec_ev_front_dispatch(apollo_overlay_state *, lanec_io *,
                                                   enum lanec_mission);

#define A_AT      0u
#define Q_AT      2u
#define EB_AT     3u
#define FB_AT     4u
#define Z_AT      5u
#define BB_AT     6u
#define BANK43    043u
#define STATES    4096u
#define OC24100   024100u
#define FAN_WORDS 60u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* The four CCS outcomes of a word: positive, +0, negative, -0. */
static uint16_t ccs_class(void)
{
    uint16_t v = (uint16_t)(rnd() & 037777u);
    switch (rnd() % 5u) {
    case 0: return v ? v : 1u;
    case 1: return 0u;
    case 2: return (uint16_t)(040000u | (v == 037777u ? 0u : v));
    case 3: return 077777u;
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

struct key {
    enum lanec_mission mission;
    uint16_t at;
    const char *name;
};

static const struct key keys[] = {
    {LANEC_LM, LANEC_LM_GOEXTVB, "GOEXTVB"},
    {LANEC_LM, LANEC_LM_TESTXACT, "TESTXACT"},
    {LANEC_LM, LANEC_LM_SETXTACT, "SETXTACT"},
    {LANEC_LM, LANEC_LM_SETXTACT + 5u, "SETXTACT +5"},
    {LANEC_LM, LANEC_LM_SETXTACT + 6u, "SETXTACT +6"},
    {LANEC_LM, LANEC_LM_XACTALM, "XACTALM"},
    {LANEC_LM, LANEC_LM_XACTALM + 1u, "XACTALM +1"},
    {LANEC_LM, LANEC_LM_XACT0, "XACT0"},
    {LANEC_LM, LANEC_LM_ALM_S_END, "ALM/END"},
    {LANEC_LM, LANEC_LM_GOPIN, "GOPIN"},
    {LANEC_LM, LANEC_LM_CHKPOOH, "CHKPOOH"},
    {LANEC_CM, LANEC_CM_GOEXTVB, "GOEXTVB"},
    {LANEC_CM, LANEC_CM_TESTXACT, "TESTXACT"},
    {LANEC_CM, LANEC_CM_SETXTACT, "SETXTACT"},
    {LANEC_CM, LANEC_CM_SETXTACT + 5u, "SETXTACT +5"},
    {LANEC_CM, LANEC_CM_SETXTACT + 6u, "SETXTACT +6"},
    {LANEC_CM, LANEC_CM_XACTALM, "XACTALM"},
    {LANEC_CM, LANEC_CM_XACTALM + 1u, "XACTALM +1"},
    {LANEC_CM, LANEC_CM_XACT0, "XACT0"},
    {LANEC_CM, LANEC_CM_ALM_S_END, "ALM/END"},
    {LANEC_CM, LANEC_CM_GOPIN, "GOPIN"},
};

/* One state at key k, from the port's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, const struct key *k,
                                unsigned n)
{
    uint16_t *w = s->words;
    enum lanec_mission m = k->mission;
    uint16_t x;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lanec_fill(s, io, n * 2654435761u + k->at);
    lanec_enter(s, io, BANK43, k->at);
    w[A_AT] = reg16();
    w[Q_AT] = reg16();
    w[LANEC_LM_EXTVBACT] = ccs_class();
    w[LANEC_LM_FLAGWRD4] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 2u) w[LANEC_LM_FLAGWRD4] &= (uint16_t)~OC24100;
    x = (uint16_t)(rnd() % 16u);
    w[LANEC_LM_MPAC] = x < 14u    ? (uint16_t)(rnd() % FAN_WORDS)
                       : x == 14u ? (uint16_t)(FAN_WORDS + rnd() % 8u)
                                  : (uint16_t)(rnd() & 077777u);
    x = (uint16_t)(rnd() % 16u);
    w[LANEC_LM_MPAC + 1u] = x < 13u    ? (uint16_t)(02000u + rnd() % 06000u)
                            : x == 13u ? (uint16_t)(rnd() % 02000u)
                                       : (uint16_t)(rnd() & 077777u);
    x = (uint16_t)(rnd() % 4u);
    w[LANEC_LM_MODREG] = x == 0u ? 0u : x == 1u ? 077777u : (uint16_t)(rnd() & 077777u);
    /* Now and then a state the module does not take. */
    switch (rnd() % 64u) {
    case 0: w[FB_AT] = (uint16_t)(rnd() & 076000u); break;
    case 1: io->superbank &= (uint16_t)~0100u; break;
    case 2: w[BB_AT] ^= 02000u; break;
    case 3: w[EB_AT] |= 1u; break;
    case 4: w[7] = 1u; break;
    case 5: m = m == LANEC_LM ? LANEC_CM : LANEC_LM; break;
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
    sf = faithful_lanec_ev_front_dispatch(fa, &fio, m);
    sr = lanec_ev_front_dispatch(ra, &rio, m);
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
    unsigned i, n, failures = 0, suite_ran = 0;

    /* The port's suite, case by case. */
    for (n = 0; n < lanec_suite_ev_front.count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        const char *why;
        int ran;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_front.make(n, &start, &io, &meta);
        why = compare(&start, &io, meta.mission, &ran, &fa, &ra);
        suite_ran += (unsigned)ran;
        if (why && ++failures <= 3) {
            printf("FAIL suite case %u (%s, Z %04o): %s\n", n,
                   meta.mission == LANEC_LM ? "LM" : "CM", start.words[Z_AT], why);
            show(&start, &fa, &ra);
        }
    }
    printf("ev-front-cases: the port's suite, %u cases (%u run), %u failures\n",
           lanec_suite_ev_front.count, suite_ran, failures);

    /* Each key from stirred states. */
    for (i = 0; i < sizeof keys / sizeof keys[0]; ++i) {
        unsigned ran_total = 0, bad = 0;

        for (n = 0; n < STATES; ++n) {
            apollo_overlay_state start, fa, ra;
            lanec_io io;
            enum lanec_mission m = state(&start, &io, &keys[i], n);
            int ran;
            const char *why = compare(&start, &io, m, &ran, &fa, &ra);

            ran_total += (unsigned)ran;
            if (why && ++bad <= 3) {
                printf("FAIL %s %s state %u: %s\n", keys[i].mission == LANEC_LM ? "LM" : "CM",
                       keys[i].name, n, why);
                show(&start, &fa, &ra);
            }
        }
        if (ran_total == 0u && bad == 0u) {
            printf("FAIL %s %s: the faithful routine never ran\n",
                   keys[i].mission == LANEC_LM ? "LM" : "CM", keys[i].name);
            ++bad;
        }
        failures += bad;
    }
    printf("ev-front-cases: %u keys from %u states each, %u failures\n",
           (unsigned)(sizeof keys / sizeof keys[0]), STATES, failures);
    return failures != 0;
}
