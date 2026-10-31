/* SPDX-License-Identifier: MIT */
/* The readable STATEUP against the faithful port's, case by case (make
 * stateup-cases ROPE_LM ROPE_CM).
 *
 * Built once per mission, with the readable table (src/binding/
 * stateup_routines.c) and the faithful port's under the same names with
 * faithful_ in front (the Makefile renames them). First the words: the
 * readable STATEUP and R00 interpretive code (src/fresh_start/stateup.c),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the order pairs: each case is one of the port's test
 * states (c64-port/tests/<m>_stateup_cases.c: every pair, the flag words
 * so each switch goes both ways, a work area of each kind, NEWJOB +0 or a
 * job waiting), half of them stirred further: interrupts, SUPERBNK, the
 * ZERO register, BBANK, BANKSET, EDOP, the work area and its push-down
 * pointer, NEWJOB, QPRET. Each pair runs once on each side. Where the
 * faithful routine runs, the readable one must too, with the same memory
 * and io record, word for word; where the readable one refuses, it must
 * leave everything as it found it.
 *
 * Not counted: NEWJOB negative, which the readable interpreter refuses
 * where the faithful pair runs (the faithful pair refuses it only at
 * DANZIG). It is a state no running job sees: the Executive stores only
 * +0, a core set's address, or -0 while no job runs (the faithful port's
 * own note, c64-port/src/<m>_stateup.c).
 *
 * The faithful routines are certified
 * against the original program (c64-port/tests/test_<m>_stateup.c), as the
 * readable ones are by make stateup-cases' first step. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#if LUMINARY099
#include "lm_stateup_cases.h"
#define MISSION       "LM"
#define ROPE          0u
#define TABLE         lm_stateup_interpretive
#define FAITHFUL      faithful_lm_stateup_interpretive
#define ENTRIES       lm_stateup_entries
#define FAITHFUL_KEYS faithful_lm_stateup_entries
#define ROUTINES      LM_STATEUP_ROUTINES
#define INPUT         lm_stateup_input
#else
#include "cm_stateup_cases.h"
#define MISSION       "CM"
#define ROPE          1u
#define TABLE         cm_stateup_interpretive
#define FAITHFUL      faithful_cm_stateup_interpretive
#define ENTRIES       cm_stateup_entries
#define FAITHFUL_KEYS faithful_cm_stateup_entries
#define ROUTINES      CM_STATEUP_ROUTINES
#define INPUT         cm_stateup_input
#endif

/* The readable words (src/fresh_start/stateup.c). */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code STATEUP_CODE, R00_CODE;
extern const agc_routine_table FAITHFUL;
extern const uint16_t FAITHFUL_KEYS[ROUTINES];

#define CASES 65536u

static uint32_t seed = 161803u;
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
        if (!apollo_rope_read(ROPE, (uint16_t)physical, &rope) || rope != code->words[i]) {
            printf("FAIL: %02o,%04o is %05o, the rope's %05o\n", code->bank, code->first + i,
                   code->words[i], rope);
            ++failures;
        }
    }
    return failures;
}

/* One case: the port's test state n, and for odd n more. */
static void state(apollo_overlay_state *s, p66_bias_io *io, uint16_t n)
{
    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    INPUT(s, io, n);
    if (n % 2u == 0u) return;
    if (rnd() % 16u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
    if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & 0377u);
    if (rnd() % 32u == 0u) s->words[7] = 1u;
    if (rnd() % 16u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);                  /* FB */
    if (rnd() % 16u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u);               /* BANKSET */
    if (rnd() % 16u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);                     /* EDOP */
    if (rnd() % 16u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 16u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 8u == 0u) /* QPRET */
        s->words[(s->words[0120] + 052u) & 03777u] = (uint16_t)(rnd() & 077777u);
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

int main(int argc, char **argv)
{
    unsigned i, n, checks = 0, failures = 0, skipped = 0, ran[64] = {0};

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = words_differ(&STATEUP_CODE) + words_differ(&R00_CODE);
    if (TABLE.count != FAITHFUL.count || TABLE.count != ROUTINES) {
        printf("FAIL: %u entries, faithful %u\n", TABLE.count, FAITHFUL.count);
        return 1;
    }
    for (i = 0; i < ROUTINES; ++i) {
        if (TABLE.entries[i].physical != FAITHFUL.entries[i].physical) {
            printf("FAIL: entry %u at key %u, faithful %u\n", i, TABLE.entries[i].physical,
                   FAITHFUL.entries[i].physical);
            ++failures;
        }
        if (ENTRIES[i] != FAITHFUL_KEYS[i]) {
            printf("FAIL: pair %u at %05o, faithful %05o\n", i, ENTRIES[i], FAITHFUL_KEYS[i]);
            ++failures;
        }
    }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        p66_bias_io io_start, fio, rio;
        agc_native_program fp, rp;
        enum exec_status sf, sr;
        const char *why = 0;
        uint16_t loc;

        state(&start, &io_start, (uint16_t)n);
        loc = ENTRIES[n % ROUTINES];
        for (i = 0; i < ROUTINES; ++i)
            if (TABLE.entries[i].physical % 1024u == loc - 02000u &&
                TABLE.entries[i].physical / 1024u == (loc >= 02100u ? 4u : 013u))
                break;
        if (i == ROUTINES) {
            printf("FAIL case %u: pair %05o not in the table\n", n, loc);
            ++failures;
            continue;
        }
        fa = ra = start;
        fio = rio = io_start;
        memset(&fp, 0, sizeof fp);
        memset(&rp, 0, sizeof rp);
        sf = FAITHFUL.entries[i].call(&fp, &fa, &fio);
        sr = TABLE.entries[i].call(&rp, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran[i];
        if (sr != EXEC_OK &&
            (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io_start, sizeof rio)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK && start.words[067] > 037777u)
            ++skipped;
        else if (sf == EXEC_OK && sr != EXEC_OK)
            why = "readable refused, faithful ran";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && memcmp(&fio, &rio, sizeof fio))
            why = "different io";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, pair %05o: %s\n", n, loc, why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    for (i = 0; i < ROUTINES; ++i)
        printf("%s key %05u: run by the faithful routine %u times\n", MISSION,
               TABLE.entries[i].physical, ran[i]);
    printf("stateup-cases %s: %u words as the rope's, %u checks (%u not counted: NEWJOB "
           "negative), %u failures\n",
           MISSION, STATEUP_CODE.count + R00_CODE.count, checks, skipped, failures);
    return failures != 0;
}
