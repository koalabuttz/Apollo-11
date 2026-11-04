/* SPDX-License-Identifier: MIT */
/* The readable THE_LUNAR_LANDING against the faithful port's, case by case
 * (make lunar-landing-cases ROPE_LM ROPE_CM).
 *
 * Built twice: as the host runs the file, and as the flight program does
 * (-DAGC_FLIGHT_INTERPRETER: a run stops where it reaches the interpreter,
 * and the words after the EXITs are keys of their own). Each build links
 * the readable tables (src/binding/lunar_landing_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (src/lunar_landing/),
 * written in the listing's notation, must be the original program's rope,
 * word for word. Then the keys, which must be the faithful tables'. Then
 * the runs: each case is one of the port's test states
 * (c64-port/tests/lm_lunar_landing_cases.c: every key, the ignition
 * algorithm's guesses converging or not, the landing radar's antenna in
 * position 1 or not), half of them stirred further: FB, BANKSET, EBANK,
 * EDOP, MODE, the work area and its push-down pointer, NEWJOB, the
 * superbank and interrupt latches, overflow in A, LOC off its pair, the
 * antenna's bit, no undo image. Each case runs once on each side, from its
 * key until control leaves the file. Where the faithful routine runs, the
 * readable one must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it.
 *
 * Not counted: NEWJOB negative, which the readable interpreter refuses at
 * any pair where the faithful one refuses it only at DANZIG, as in make
 * stateup-cases. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_lunar_landing_cases.h"

#ifdef AGC_FLIGHT_INTERPRETER
#define MODE_NAME "flight"
#else
#define MODE_NAME "host"
#endif

/* The readable words. */
struct interpretive_code {
    unsigned bank;
    uint16_t first;
    unsigned count;
    const uint16_t *words;
};
extern const struct interpretive_code IGNALG_CODE, DDUMCALC_CODE, P63SPOT2_CODE, LANDJUNK_CODE,
    GSAV_CODE;
extern const agc_routine_table faithful_lm_lunar_landing_routines,
    faithful_lm_lunar_landing_interpretive;

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
 * build a quarter of the basic cases start at a word after an EXIT. */
static void state(apollo_overlay_state *s, lld_case *c, int *undo, uint16_t n)
{
    memset(s, 0, sizeof *s);
    lld_input(s, c, n);
    *undo = 1;
#ifdef AGC_FLIGHT_INTERPRETER
    unsigned r = n % LLD_ROUTINES;

    if (r < LLD_BASIC_ROUTINES && n / LLD_ROUTINES % 4u == 1u) {
        static const uint16_t p63[] = {03202u, 03223u}, p68[] = {02200u, 02214u};
        s->words[5] = lld_routine_keys[r] / 1024u == 032u ? p63[rnd() % 2u] : p68[rnd() % 2u];
    }
#endif
    if (n % 2u == 0u) goto banks;
    if (rnd() % 16u == 0u) c->superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 16u == 0u) c->interrupt_enabled = (uint16_t)(rnd() % 2u);
    if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);    /* FB */
    if (rnd() % 16u == 0u) s->words[3] = (uint16_t)(rnd() & 03400u);     /* EBANK */
    if (rnd() % 16u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u); /* BANKSET */
    if (rnd() % 16u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);       /* EDOP */
    if (rnd() % 16u == 0u)
        s->words[0163] = (uint16_t)(rnd() % 3u ? rnd() % 2u : 077776u);                /* MODE */
    if (rnd() % 16u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 16u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 8u == 0u) /* A with overflow, either way */
        s->words[0] = (uint16_t)(rnd() % 2u ? 040000u | (rnd() & 037777u) : 0137777u & rnd());
    if (rnd() % 16u == 0u && s->words[5] == 06070u) /* LOC off its pair */
        s->words[0164] = (uint16_t)(s->words[0164] + 1u + rnd() % 3u);
    if (rnd() % 4u == 0u) c->chan33 = (agc_word)(c->chan33 ^ 040u); /* the antenna's bit */
    if (rnd() % 64u == 0u) *undo = 0;
banks:
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static enum exec_status run(const agc_routine_table *basic, const agc_routine_table *interp,
                            apollo_overlay_state *s, lm_lld_io *io)
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

/* The io record but its undo pointer. */
static int io_differs(const lm_lld_io *a, const lm_lld_io *b)
{
    return memcmp(&a->control, &b->control, sizeof a->control) || a->chan12 != b->chan12 ||
           a->chan33 != b->chan33;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state faithful_undo, readable_undo;
    unsigned n, checks = 0, failures = 0, newjob = 0, ran = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = words_differ(&IGNALG_CODE) + words_differ(&DDUMCALC_CODE) +
               words_differ(&P63SPOT2_CODE) + words_differ(&LANDJUNK_CODE) +
               words_differ(&GSAV_CODE);
    failures +=
        tables_differ(&lm_lunar_landing_routines, &faithful_lm_lunar_landing_routines, "basic");
    failures += tables_differ(&lm_lunar_landing_interpretive,
                              &faithful_lm_lunar_landing_interpretive, "interpretive");
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        lld_case c;
        lm_lld_io io_start, fio, rio;
        enum exec_status sf, sr;
        const char *why = 0;
        int undo;

        state(&start, &c, &undo, (uint16_t)n);
        memset(&io_start, 0, sizeof io_start);
        io_start.control.superbank = c.superbank;
        io_start.control.interrupt_enabled = c.interrupt_enabled;
        io_start.chan12 = c.chan12;
        io_start.chan33 = c.chan33;
        fa = ra = start;
        fio = rio = io_start;
        fio.undo = undo ? &faithful_undo : 0;
        rio.undo = undo ? &readable_undo : 0;
        sf = run(&faithful_lm_lunar_landing_routines, &faithful_lm_lunar_landing_interpretive, &fa,
                 &fio);
        sr = run(&lm_lunar_landing_routines, &lm_lunar_landing_interpretive, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran;
        if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || io_differs(&rio, &io_start)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK && start.words[067] > 037777u)
            ++newjob;
        else if (sf != sr)
            why =
                sf == EXEC_OK ? "readable refused, faithful ran" : "readable ran, faithful refused";
        else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
            why = "different memory";
        else if (sf == EXEC_OK && io_differs(&fio, &rio))
            why = "different io";
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, Z %05o FB %05o LOC %05o: %s\n", n, start.words[5], start.words[4],
                   start.words[0164], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    printf("lunar-landing-cases %s: %u words as the rope's, %u checks (%u run by the faithful "
           "routines; not counted: %u NEWJOB negative), %u failures\n",
           MODE_NAME,
           IGNALG_CODE.count + DDUMCALC_CODE.count + P63SPOT2_CODE.count + LANDJUNK_CODE.count +
               GSAV_CODE.count,
           checks, ran, newjob, failures);
    return failures != 0;
}
