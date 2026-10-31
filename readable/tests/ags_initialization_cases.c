/* SPDX-License-Identifier: MIT */
/* The readable R47 against the faithful port's, case by case (make
 * ags-cases ROPE_LM ROPE_CM).
 *
 * Built twice: as the host runs the file, and as the flight program does
 * (-DAGC_FLIGHT_INTERPRETER: a run stops where it reaches the interpreter,
 * and the two EXIT returns are keys of their own). Each build links the
 * readable tables (src/binding/ags_initialization_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (AGSVCALC's and
 * SCALEVEC's, src/ags_initialization/ags_initialization.c), written in the
 * listing's notation, must be the original program's rope, word for word.
 * Then the keys, which must be the faithful tables'. Then the runs: each
 * case is one of the port's test states (c64-port/tests/
 * lm_ags_init_cases.c: every key, the flags, IMUCADR of each CCS class,
 * each VAC area, NEWJOB +0 or a job waiting, SCALEVEC's two QPRETs or a
 * wrong one), half of them stirred further: FB, BANKSET, EDOP, the work
 * area and its push-down pointer, QPRET, NEWJOB, the superbank, no undo
 * image. Each case runs once on each side, from its key until control
 * leaves the file. Where the faithful routine runs, the readable one must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it.
 *
 * Not counted: NEWJOB negative, which the readable interpreter refuses at
 * any pair where the faithful one refuses it only at DANZIG, as in make
 * stateup-cases. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_ags_init_cases.h"

#ifdef AGC_FLIGHT_INTERPRETER
#define MODE "flight"
#else
#define MODE "host"
#endif

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code AGSVCALC_CODE, AGSVCALC_CODE_2, SCALEVEC_CODE;
extern const agc_routine_table faithful_lm_ags_init_routines, faithful_lm_ags_init_interpretive;
extern const uint16_t faithful_ags_routine_keys[AGS_ROUTINES];

#define CASES 65536u

static uint32_t seed = 271828u;
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

static unsigned tables_differ(const agc_routine_table *readable, const agc_routine_table *faithful,
                              const char *what)
{
    unsigned i, failures = 0;

    if (readable->count != faithful->count) {
        printf("FAIL: %s has %u keys, faithful %u\n", what, readable->count, faithful->count);
        return 1;
    }
    for (i = 0; i < readable->count; ++i)
        if (readable->entries[i].physical != faithful->entries[i].physical) {
            printf("FAIL: %s key %u is %u, faithful %u\n", what, i, readable->entries[i].physical,
                   faithful->entries[i].physical);
            ++failures;
        }
    return failures;
}

/* One case: the port's test state n, and for odd n more. In the flight
 * build a quarter of the basic cases start at one of the EXIT returns. */
static void state(apollo_overlay_state *s, ags_case *c, int *undo, uint16_t n)
{
    memset(s, 0, sizeof *s);
    ags_input(s, c, n);
    *undo = 1;
#ifdef AGC_FLIGHT_INTERPRETER
    if (n % AGS_ROUTINES < AGS_BASIC_ROUTINES && n / AGS_ROUTINES % 4u == 1u)
        s->words[5] = (agc_word)(n % 2u ? 02043u : 02072u);
#endif
    if (n % 2u == 0u) return;
    if (rnd() % 16u == 0u) c->superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 16u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);                  /* FB */
    if (rnd() % 16u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u);               /* BANKSET */
    if (rnd() % 16u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);                     /* EDOP */
    if (rnd() % 16u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 16u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 8u == 0u) /* QPRET */
        s->words[(s->words[0120] + 052u) & 03777u] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 64u == 0u) *undo = 0;
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static enum exec_status run(const agc_routine_table *basic, const agc_routine_table *interp,
                            apollo_overlay_state *s, lm_ags_io *io)
{
    agc_routine_table b[1], i[1];
    agc_native_program program = {0, 0, 0, 0, 0, 0, AGC_TRANSFER_NONE, 0, 0, 0, 0, 0};
    agc_routine call;

    b[0] = *basic;
    i[0] = *interp;
    program.basic = b;
    program.basic_count = 1;
    program.interpretive = i;
    program.interpretive_count = 1;
    program.interpreter_fetch = (uint16_t)(2048u + 06070u - 04000u);
    program.loc = 0164;
    program.bankset = 0165;
    call = agc_transfer_resolve(&program, s, io->control.superbank);
    return call ? call(&program, s, io) : EXEC_OUT_OF_SCOPE;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state faithful_undo, readable_undo;
    unsigned i, n, checks = 0, failures = 0, skipped = 0, ran = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = words_differ(&AGSVCALC_CODE) + words_differ(&AGSVCALC_CODE_2) +
               words_differ(&SCALEVEC_CODE);
    failures += tables_differ(&lm_ags_init_routines, &faithful_lm_ags_init_routines, "basic");
    failures += tables_differ(&lm_ags_init_interpretive, &faithful_lm_ags_init_interpretive,
                              "interpretive");
    for (i = 0; i < AGS_ROUTINES; ++i)
        if (ags_routine_keys[i] != faithful_ags_routine_keys[i]) {
            printf("FAIL: case key %u is %u, faithful %u\n", i, ags_routine_keys[i],
                   faithful_ags_routine_keys[i]);
            ++failures;
        }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        ags_case c;
        lm_ags_io io_start, fio, rio;
        enum exec_status sf, sr;
        const char *why = 0;
        int undo;

        state(&start, &c, &undo, (uint16_t)n);
        memset(&io_start, 0, sizeof io_start);
        io_start.control.superbank = c.superbank;
        io_start.control.interrupt_enabled = c.interrupt_enabled;
        fa = ra = start;
        fio = rio = io_start;
        fio.undo = undo ? &faithful_undo : 0;
        rio.undo = undo ? &readable_undo : 0;
        sf = run(&faithful_lm_ags_init_routines, &faithful_lm_ags_init_interpretive, &fa, &fio);
        sr = run(&lm_ags_init_routines, &lm_ags_init_interpretive, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran;
        if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) ||
                              memcmp(&rio.control, &io_start.control, sizeof rio.control)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK && start.words[067] > 037777u)
            ++skipped;
        else if (sf != sr)
            why =
                sf == EXEC_OK ? "readable refused, faithful ran" : "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && memcmp(&fio.control, &rio.control, sizeof fio.control))
            why = "different io";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, Z %05o: %s\n", n, start.words[5], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    printf("ags-cases %s: %u words as the rope's, %u checks (%u run by the faithful routines, "
           "%u not counted: NEWJOB negative), %u failures\n",
           MODE, AGSVCALC_CODE.count + AGSVCALC_CODE_2.count + SCALEVEC_CODE.count, checks, ran,
           skipped, failures);
    return failures != 0;
}
