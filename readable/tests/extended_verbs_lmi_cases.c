/* SPDX-License-Identifier: MIT */
/* The readable LM's interpretive words of the tracking verbs against the
 * faithful port's module (c64-port/src/lanec_ev_lmi.c), side by side
 * (make extended-verbs-cases).
 *
 * First the words: the readable tables (RRLOSDSP's, TRMTRAK1's,
 * ATTACHIT's and TCPIN's, src/extended_verbs/track.c), written in the
 * listing's notation, must be the original program's rope, word for word.
 * The faithful module is built with its lanec_ev_lmi_dispatch renamed
 * faithful_lanec_ev_lmi_dispatch; the readable one is
 * lanec_ev_lmi_dispatch (extended_verbs_missions.c). Then every case of
 * the port's suite (c64-port/tests/lanec_ev_lmi_cases.c). Then each of the
 * module's entries (its own list, lanec_ev_lmi_entry: RRLOSDSP's in bank
 * 40, TRMTRAK1's in 42, ATTACHIT's and TCPIN in 43) from 4,096 states of
 * the suite's own fill, stirred: NEWJOB +0, a job change pending or
 * negative; FIXLOC a work area anywhere from 0200 to 01320, NOVAC's 0110,
 * or just outside; MOONTHIS and MOONOTH (FLAGWRD8) as the fill leaves
 * them, set or clear; the line of
 * sight (MPAC's angles, and for RRLOSDSP +6 the vector at 0D) zero, small
 * or anything; now and then EDOP, BANKSET or INTBIT15 wrong, the bank
 * registers or ZERO wrong, the superbank bit off, LOC or Z one word off,
 * or the CM. Where either runs, the other must too, with the same memory
 * and channels word for word; where the readable one refuses, it must
 * leave everything as it found it; and the faithful module must run from
 * some state of every entry. The ropes are named on the command line. */
#include <stdio.h>
#include <string.h>
#include "lanec_suites.h"
#include "lanec_ev_lmi.h"
#include "rope_image.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code RRLOSDSP_CODE, TRMTRAK1_CODE, ATTACHIT_CODE, ATTACHIT_CODE_2,
    TCPIN_CODE;

/* The table's words that are not the rope's (the LM's), each shown. */
static unsigned words_differ(const struct interpretive_code *code)
{
    unsigned i, differ = 0;

    for (i = 0; i < code->count; ++i) {
        unsigned physical = code->bank * 1024u + code->first - 02000u + i;
        agc_word rope = 0;

        if (!apollo_rope_read(0, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++differ;
        }
    }
    return differ;
}

enum lanec_status faithful_lanec_ev_lmi_dispatch(apollo_overlay_state *, lanec_io *,
                                                 enum lanec_mission);

#define A_AT     0u
#define Q_AT     2u
#define EB_AT    3u
#define FB_AT    4u
#define Z_AT     5u
#define BB_AT    6u
#define EDOP     023u
#define NEWJOB   067u
#define INTBIT15 0115u
#define FIXLOC   0120u
#define MPAC     0154u
#define LOC      0164u
#define BANKSET  0165u
#define STATES   4096u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

/* A word: zero, small, or anything. */
static uint16_t some(void)
{
    switch (rnd() % 4u) {
    case 0: return 0u;
    case 1: return (uint16_t)(rnd() & 7u);
    default: return (uint16_t)(rnd() & 077777u);
    }
}

/* One state at entry i (of count), from the suite's fill for case n. */
static enum lanec_mission state(apollo_overlay_state *s, lanec_io *io, uint16_t i, uint16_t count,
                                unsigned n)
{
    uint16_t *w = s->words, bank, z, k;
    enum lanec_mission m = LANEC_LM;
    lanec_meta meta;
    const uint16_t *at;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(&meta, 0, sizeof meta);
    lanec_suite_ev_lmi.make((uint32_t)(n * count + i), s, io, &meta);
    at = lanec_ev_lmi_entry(i, &bank, &z, &count);
    switch (rnd() % 8u) { /* NEWJOB */
    case 0: w[NEWJOB] = (uint16_t)(1u + rnd() % 037777u); break;
    case 1: w[NEWJOB] = (uint16_t)(040000u | rnd()); break;
    default: w[NEWJOB] = 0u; break;
    }
    switch (rnd() % 8u) { /* FIXLOC */
    case 0: w[FIXLOC] = 0110u; break;
    case 1: w[FIXLOC] = (uint16_t)(0200u - 1u - rnd() % 2u); break;
    case 2: w[FIXLOC] = (uint16_t)(01320u + rnd() % 2u); break;
    case 3: w[FIXLOC] = (uint16_t)(rnd() & 07777u); break;
    default: break; /* the suite's */
    }
    w[MPAC] = some(); /* the radar's angles */
    w[MPAC + 1u] = some();
    for (k = 0; k < 6u; ++k) /* the line of sight, for RRLOSDSP +6 */
        if (rnd() % 4u == 0u) w[(w[FIXLOC] + k) & 03777u] = some();
    /* Now and then a state the module does not take. */
    switch (rnd() % 64u) {
    case 0: w[EDOP] = (uint16_t)(1u + rnd() % 0177u); break;
    case 1: w[BANKSET] ^= 02000u; break;
    case 2: w[INTBIT15] ^= 040000u; break;
    case 3: w[FB_AT] = (uint16_t)(rnd() & 076000u); break;
    case 4: io->superbank &= (uint16_t)~0100u; break;
    case 5: w[BB_AT] ^= 02000u; break;
    case 6: w[EB_AT] |= 1u; break;
    case 7: w[7] = 1u; break;
    case 8: m = LANEC_CM; break;
    case 9: w[Z_AT] ^= 1u; break;
    case 10: w[LOC] = (uint16_t)(*at + 1u); break;
    case 11: w[LOC] = (uint16_t)(*at - 1u); break;
    case 12: w[BANKSET] ^= 1u; break; /* its E-bank */
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
    sf = faithful_lanec_ev_lmi_dispatch(fa, &fio, m);
    sr = lanec_ev_lmi_dispatch(ra, &rio, m);
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

int main(int argc, char **argv)
{
    unsigned i, n, failures = 0, suite_ran = 0;
    uint16_t count, bank, z;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }

    /* The words. */
    {
        const struct interpretive_code *codes[] = {&RRLOSDSP_CODE, &TRMTRAK1_CODE, &ATTACHIT_CODE,
                                                   &ATTACHIT_CODE_2, &TCPIN_CODE};
        unsigned k, words = 0;

        for (k = 0; k < sizeof codes / sizeof codes[0]; ++k) {
            failures += words_differ(codes[k]);
            words += codes[k]->count;
        }
        printf("ev-lmi-cases: %u words, %u not the rope's\n", words, failures);
    }

    /* The port's suite, case by case. */
    for (n = 0; n < lanec_suite_ev_lmi.count; ++n) {
        apollo_overlay_state start, fa, ra;
        lanec_io io;
        lanec_meta meta;
        const char *why;
        int ran;

        memset(&start, 0, sizeof start);
        memset(&io, 0, sizeof io);
        memset(&meta, 0, sizeof meta);
        lanec_suite_ev_lmi.make(n, &start, &io, &meta);
        why = compare(&start, &io, meta.mission, &ran, &fa, &ra);
        suite_ran += (unsigned)ran;
        if (why && ++failures <= 3) {
            printf("FAIL suite case %u (LOC %04o): %s\n", n, start.words[LOC], why);
            show(&start, &fa, &ra);
        }
    }
    printf("ev-lmi-cases: the port's suite, %u cases (%u run), %u failures so far\n",
           lanec_suite_ev_lmi.count, suite_ran, failures);

    /* Each entry from stirred states. */
    (void)lanec_ev_lmi_entry(0, &bank, &z, &count);
    for (i = 0; i < count; ++i) {
        unsigned ran_total = 0, bad = 0;

        for (n = 0; n < STATES; ++n) {
            apollo_overlay_state start, fa, ra;
            lanec_io io;
            enum lanec_mission m = state(&start, &io, (uint16_t)i, count, n);
            int ran;
            const char *why = compare(&start, &io, m, &ran, &fa, &ra);

            ran_total += (unsigned)ran;
            if (why && ++bad <= 3) {
                printf("FAIL entry %u (LOC %04o) state %u: %s\n", i,
                       *lanec_ev_lmi_entry((uint16_t)i, &bank, &z, &count), n, why);
                show(&start, &fa, &ra);
            }
        }
        if (ran_total == 0u && bad == 0u) {
            printf("FAIL entry %u: the faithful routine never ran\n", i);
            ++bad;
        }
        failures += bad;
    }
    printf("ev-lmi-cases: %u entries from %u states each, %u failures\n", (unsigned)count, STATES,
           failures);
    return failures != 0;
}
