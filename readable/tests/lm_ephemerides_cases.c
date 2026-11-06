/* SPDX-License-Identifier: MIT */
/* The readable LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES against the
 * faithful port's, key by key, for the LM (make ephemerides-cases ROPE_LM
 * ROPE_CM).
 *
 * Built with the readable table (src/binding/ephemerides_routines.c) and
 * the faithful port's under the same name with faithful_ in front (the
 * Makefile renames it).
 *
 * First the words: the readable interpretive code (src/ephemerides/
 * ephemerides.c), written in the listing's notation, must be the original
 * program's rope, word for word. Then the keys, which must be the faithful
 * table's. Then the chains: from each of the port's sampled entry states
 * (c64-port/tests/lm_ephemeris_cases.c: LSPOS or LUNVEL, each caller's
 * return, the VAC areas, E-banks, latches, times and TEPHEM's edges), most
 * stirred further (the time in MPAC, TEPHEM, the work area's debris), the
 * key at LOC runs on each side, as the program's trampoline would call it,
 * step after step until LOC leaves the file's keys. After each step both
 * sides must have run, with the same memory and io record, word for word.
 * And from entry states the faithful entry refuses (each of its guards
 * broken in turn: Z, the banks, ZERO, EDOP, NEWJOB, INTBIT15, MODE, the
 * latches, the work area, a word of 16 bits, the return), the readable
 * one must refuse too and leave everything as it found it. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "interp_case.h"
#include "lm_ephemeris_cases.h"

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code EPHEMERIDES_CODE;
extern const agc_routine_table lm_ephemeris_routines, faithful_lm_ephemeris_routines;

#define CHAINS   65536u
#define REFUSALS 16384u

static uint32_t seed = 314159u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned words_differ(const struct interpretive_code *code)
{
    unsigned i, failures = 0;

    for (i = 0; i < code->count; ++i) {
        agc_word rope = 0;
        unsigned physical = code->bank * 1024u + code->first - 02000u + i;
        if (!apollo_rope_read(0, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++failures;
        }
    }
    return failures;
}

static unsigned tables_differ(void)
{
    unsigned i, failures = 0;

    if (lm_ephemeris_routines.count != faithful_lm_ephemeris_routines.count) {
        printf("FAIL: %u keys, faithful %u\n", lm_ephemeris_routines.count,
               faithful_lm_ephemeris_routines.count);
        return 1;
    }
    for (i = 0; i < lm_ephemeris_routines.count; ++i)
        if (lm_ephemeris_routines.entries[i].physical !=
            faithful_lm_ephemeris_routines.entries[i].physical) {
            printf("FAIL: key %u is %u, faithful %u\n", i,
                   lm_ephemeris_routines.entries[i].physical,
                   faithful_lm_ephemeris_routines.entries[i].physical);
            ++failures;
        }
    return failures;
}

/* The key at LOC (under FB, at NEWOPS), or -1. */
static int key_index(const apollo_overlay_state *s)
{
    unsigned i;
    uint16_t physical;

    if (s->words[5] != 06070u || s->words[4] != 032000u || s->words[0164] < 02000u ||
        s->words[0164] > 03777u)
        return -1;
    physical = (uint16_t)(015u * 1024u + s->words[0164] - 02000u);
    for (i = 0; i < faithful_lm_ephemeris_routines.count; ++i)
        if (faithful_lm_ephemeris_routines.entries[i].physical == physical) return (int)i;
    return -1;
}

static void show(const char *what, unsigned n, unsigned k, const apollo_overlay_state *ra,
                 const apollo_overlay_state *fa, const apollo_overlay_state *start)
{
    unsigned w, shown = 0;

    printf("FAIL case %u step %u at %04o: %s\n", n, k, start->words[0164], what);
    for (w = 0; w < APOLLO_SHARED_WORDS && shown < 12u; ++w)
        if (fa->words[w] != ra->words[w]) {
            printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra->words[w], fa->words[w],
                   start->words[w]);
            ++shown;
        }
}

/* A sampled entry state, most stirred further. */
static void entry(apollo_overlay_state *s, p66_bias_io *io, unsigned n)
{
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    lm_ephemeris_input(s, io, n);
    if (n % 4u == 0u) return;
    /* The time (TIME2, TIME1 in MPAC) and TEPHEM, as the program has them
     * (a few days in centiseconds) or any. */
    if (rnd() % 2u) {
        s->words[0154] = (uint16_t)(rnd() % 0100u);
        s->words[0155] = (uint16_t)(rnd() & 077777u);
    }
    if (rnd() % 2u) {
        s->words[01706] = (uint16_t)(rnd() % 4u);
        s->words[01707] = (uint16_t)(rnd() & 077777u);
        s->words[01710] = (uint16_t)(rnd() & 077777u);
    }
    /* The work area's other words (not X1, X2 nor QPRET), the push-down
     * pointer in it. */
    for (k = 0; k < 040u; ++k)
        if (rnd() % 4u == 0u) s->words[s->words[0120] + k] = (uint16_t)(rnd() & 077777u);
    s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 040u);
    if (rnd() % 8u == 0u) s->words[0121] = 1u; /* OVFIND */
}

static unsigned chains(unsigned *steps)
{
    unsigned n, failures = 0;

    for (n = 0; n < CHAINS && failures < 10u; ++n) {
        apollo_overlay_state fa, ra, start;
        p66_bias_io fio, rio;
        agc_native_program p;
        unsigned k;
        int key;

        entry(&fa, &fio, n);
        ra = fa;
        rio = fio;
        memset(&p, 0, sizeof p);
        for (k = 0; k < 64u && (key = key_index(&fa)) >= 0; ++k) {
            enum exec_status sf, sr;

            start = fa;
            sf = faithful_lm_ephemeris_routines.entries[key].call(&p, &fa, &fio);
            sr = lm_ephemeris_routines.entries[key].call(&p, &ra, &rio);
            if (sf != EXEC_OK && sr != EXEC_OK) break;
            ++*steps;
            if (sf != sr) {
                show(sf == EXEC_OK ? "readable refused, faithful ran"
                                   : "readable ran, faithful refused",
                     n, k, &ra, &fa, &start);
                ++failures;
                break;
            }
            if (memcmp(&fa, &ra, sizeof fa) || memcmp(&fio, &rio, sizeof fio)) {
                show("different memory or io record", n, k, &ra, &fa, &start);
                ++failures;
                break;
            }
        }
    }
    return failures;
}

/* Entry states with one of the faithful entry's guards broken. */
static unsigned refusals(void)
{
    unsigned n, failures = 0;

    for (n = 0; n < REFUSALS; ++n) {
        apollo_overlay_state s, start, ra;
        p66_bias_io io, rio;
        agc_native_program p;
        uint16_t *w = s.words, vac;
        enum exec_status sf, sr;
        int key;

        memset(&s, 0, sizeof s);
        memset(&io, 0, sizeof io);
        lm_ephemeris_input(&s, &io, n * 7u);
        vac = w[0120];
        switch (n % 14u) {
        case 0: w[5] = (uint16_t)(rnd() % 010000u); break;         /* Z */
        case 1: w[4] = (uint16_t)(rnd() & 076000u); break;         /* FB */
        case 2: w[3] = (uint16_t)(rnd() & 077777u); break;         /* EB */
        case 3: w[6] = (uint16_t)(rnd() & 077777u); break;         /* BBANK */
        case 4: w[0165] = (uint16_t)(rnd() & 077777u); break;      /* BANKSET */
        case 5: w[7] = (uint16_t)(1u + rnd() % 077776u); break;    /* ZERO */
        case 6: w[023] = (uint16_t)(1u + rnd() % 077776u); break;  /* EDOP */
        case 7: w[067] = (uint16_t)(1u + rnd() % 077776u); break;  /* NEWJOB */
        case 8: w[0115] = (uint16_t)(1u + rnd() % 077776u); break; /* INTBIT15 */
        case 9: w[0163] = rnd() % 2u ? 1u : 077776u; break;        /* MODE */
        case 10:                                                   /* latches */
            if (rnd() % 2u)
                io.interrupt_enabled = (uint16_t)(2u + rnd() % 8u);
            else
                io.superbank = (uint16_t)(1u + rnd() % 017u);
            break;
        case 11: /* FIXLOC: inside a VAC area, or a NOVAC job's (MPAC -36D) with
                  * the return where QPRET would be */
            if (rnd() % 2u) {
                w[0120] = (uint16_t)(vac + 1u + rnd() % 053u);
            } else {
                w[0120] = 0154u - 044u;
                w[0154u - 044u + 052u] = w[vac + 052u];
            }
            break;
        case 12: w[2u + rnd() % 2046u] |= 0100000u; break;           /* 16 bits */
        case 13: w[vac + 052u] = (uint16_t)(rnd() & 077777u); break; /* QPRET */
        }
        start = s;
        ra = s;
        rio = io;
        memset(&p, 0, sizeof p);
        key = w[0164] == 03775u ? 9 : 0;
        sf = faithful_lm_ephemeris_routines.entries[key].call(&p, &s, &io);
        sr = lm_ephemeris_routines.entries[key].call(&p, &ra, &rio);
        if (sr != EXEC_OK && memcmp(&ra, &start, sizeof ra)) {
            printf("FAIL refusal %u: readable refusal changed the state\n", n);
            ++failures;
        } else if (sf != sr) {
            printf("FAIL refusal %u (guard %u): %s\n", n, n % 14u,
                   sf == EXEC_OK ? "readable refused, faithful ran"
                                 : "readable ran, faithful refused");
            ++failures;
        } else if (sf == EXEC_OK && memcmp(&s, &ra, sizeof s)) {
            printf("FAIL refusal %u (guard %u): different memory\n", n, n % 14u);
            ++failures;
        }
        if (failures > 10u) break;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned failures = 0, steps = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures += words_differ(&EPHEMERIDES_CODE);
    failures += tables_differ();
    if (failures) {
        printf("ephemerides-cases LM: %u failures\n", failures);
        return 1;
    }
    failures += chains(&steps);
    failures += refusals();
    printf("ephemerides-cases LM: %u words as the rope's, %u keys, %u chains (%u steps run on "
           "both sides), %u refused entries, %u failures\n",
           EPHEMERIDES_CODE.count, lm_ephemeris_routines.count, CHAINS, steps, REFUSALS, failures);
    return failures != 0;
}
