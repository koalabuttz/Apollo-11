/* SPDX-License-Identifier: MIT */
/* The readable IMU performance test (LM P07) against the faithful port's,
 * case by case (make imu-performance-cases ROPE_LM ROPE_CM).
 *
 * Built twice: as the host runs the files, and as the flight program does
 * (-DAGC_FLIGHT_INTERPRETER: a run stops where it reaches the interpreter,
 * and the seven EXIT returns are keys of their own). Each build links the
 * readable tables (src/binding/imu_performance_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (the ten tables of
 * src/imu_performance/imu_performance.c), written in the listing's
 * notation, must be the original program's rope, word for word. Then the
 * keys, which must be the faithful tables'. Then the runs: each case is
 * one of the port's test states (c64-port/tests/
 * lm_imu_performance_cases.c: every key in 384 states, with the scaler
 * words, the CCS classes of NDXCTR, LENGTHOT, TORQNDX, ALTIM and the rest,
 * the erasable returns, MODE, the index registers and steps, NEWJOB +0 or
 * a job waiting), the odd ones stirred further: FB, BANKSET, EDOP, the
 * work area and its push-down pointer, X1, X2, MODE, MPAC, QPRET, QPLACE,
 * QPLACES, ZERONDX, NDXCTR, PIPINDEX, NEWJOB, the superbank, no undo
 * image. Each case runs once on each side, from its key until control
 * leaves the files. Where the faithful routine runs, the readable one must
 * too, with the same memory and io record, word for word; where the
 * readable one refuses, it must leave everything as it found it.
 *
 * Not counted: NEWJOB negative, which the readable interpreter refuses at
 * any pair where the faithful one refuses it only at DANZIG, as in make
 * stateup-cases. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_imu_performance_cases.h"

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
extern const struct interpretive_code *const IMU_PERFORMANCE_CODES[];
extern const unsigned IMU_PERFORMANCE_CODE_COUNT;
extern const agc_routine_table faithful_lm_imu_performance_routines,
    faithful_lm_imu_performance_interpretive;
extern const uint16_t faithful_ipt_routine_keys[IPT_ROUTINES];

#define ROUNDS 2u
#define CASES  (ROUNDS * IPT_CASES)

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

/* A word of the work area. */
#define WORK(s, k) ((s)->words[((s)->words[0120] + (k)) & 03777u])

/* X1 or X2: mostly within +-037, as the port's states draw them (an index
 * far beyond the program's can carry an address across an erasable bank,
 * which neither port's tests draw), sometimes a wild one (which both
 * refuse where it indexes). */
static uint16_t index_register(void)
{
    uint16_t x = (uint16_t)(rnd() % 040u);

    if (rnd() % 8u == 0u) return (uint16_t)(rnd() & 077777u);
    return rnd() % 2u ? x : (uint16_t)(~x & 077777u);
}

/* A return word (QPLACE, QPLACES, ZERONDX): one of the file's call sites,
 * a caller elsewhere, or a word no caller leaves (in FINETIME, SWCALL or the
 * interpreter, erasable, or not a TC's), which both refuse. A wild word
 * that lands inside another routine's code is not drawn: the faithful port
 * runs any instruction word it lands on, the readable one refuses all but
 * the words its functions start at. */
static uint16_t return_word(void)
{
    static const uint16_t words[] = {02005u, 02016u, 02101u, 02104u, 02126u, 02131u,  02144u,
                                     02171u, 02200u, 02230u, 02231u, 02252u, 02270u,  02456u,
                                     03025u, 03037u, 04000u, 05072u, 03717u, 04102u,  04622u,
                                     06037u, 07000u, 07707u, 01400u, 0100u,  010000u, 077777u};

    return words[rnd() % (sizeof words / sizeof words[0])];
}

/* One case: the port's test state n % IPT_CASES, and in the second round
 * (and for odd n) more. In the flight build a quarter of the basic cases
 * start at one of the EXIT returns. */
static void state(apollo_overlay_state *s, ipt_case *c, int *undo, unsigned n)
{
    static const uint16_t exits[] = {02067u, 02226u, 02455u, 02535u, 02773u, 03021u, 03033u};
    unsigned k = n % IPT_CASES;

    memset(s, 0, sizeof *s);
    ipt_input(s, c, (uint16_t)k);
    *undo = 1;
#ifdef AGC_FLIGHT_INTERPRETER
    if (k % IPT_ROUTINES < IPT_BASIC_ROUTINES && k / IPT_ROUTINES % 4u == 1u)
        s->words[5] = exits[k % 7u];
#else
    (void)exits;
#endif
    if (n < IPT_CASES && n % 2u == 0u) return;
    if (rnd() % 16u == 0u) c->superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);                  /* FB */
    if (rnd() % 32u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u);               /* BANKSET */
    if (rnd() % 32u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);                     /* EDOP */
    if (rnd() % 32u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 32u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) WORK(s, 046u) = index_register();
    if (rnd() % 8u == 0u) WORK(s, 047u) = index_register();
    if (rnd() % 8u == 0u)
        s->words[0163] = (uint16_t)(rnd() % 3u == 0u ? 0u : rnd() % 2u ? 1u : 077776u);
    if (rnd() % 8u == 0u) { /* MPAC */
        s->words[0154] = (uint16_t)(rnd() % 4u ? rnd() % 4u : rnd() & 077777u);
        s->words[0155] = (uint16_t)(rnd() & 077777u);
    }
    if (rnd() % 8u == 0u) WORK(s, 052u) = (uint16_t)(rnd() & 077777u); /* QPRET */
    if (rnd() % 16u == 0u) s->words[02417] = return_word();            /* QPLACE */
    if (rnd() % 16u == 0u) s->words[02420] = return_word();            /* QPLACES */
    if (rnd() % 16u == 0u) s->words[02571] = return_word();            /* ZERONDX */
    if (rnd() % 16u == 0u) s->words[02414] = (uint16_t)(rnd() % 2u ? rnd() % 4u : rnd() | 040000u);
    if (rnd() % 16u == 0u) s->words[02415] = (uint16_t)(rnd() % 2u ? rnd() % 8u : rnd() & 077777u);
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 64u == 0u) *undo = 0;
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static enum exec_status run(const agc_routine_table *basic, const agc_routine_table *interp,
                            apollo_overlay_state *s, lm_ipt_io *io)
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
    unsigned i, n, words = 0, checks = 0, failures = 0, skipped = 0, ran = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < IMU_PERFORMANCE_CODE_COUNT; ++i) {
        failures += words_differ(IMU_PERFORMANCE_CODES[i]);
        words += IMU_PERFORMANCE_CODES[i]->count;
    }
    failures +=
        tables_differ(&lm_imu_performance_routines, &faithful_lm_imu_performance_routines, "basic");
    failures += tables_differ(&lm_imu_performance_interpretive,
                              &faithful_lm_imu_performance_interpretive, "interpretive");
    for (i = 0; i < IPT_ROUTINES; ++i)
        if (ipt_routine_keys[i] != faithful_ipt_routine_keys[i]) {
            printf("FAIL: case key %u is %u, faithful %u\n", i, ipt_routine_keys[i],
                   faithful_ipt_routine_keys[i]);
            ++failures;
        }
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        ipt_case c;
        lm_ipt_io io_start, fio, rio;
        enum exec_status sf, sr;
        const char *why = 0;
        int undo;

        state(&start, &c, &undo, n);
        memset(&io_start, 0, sizeof io_start);
        io_start.control.superbank = c.superbank;
        io_start.control.interrupt_enabled = c.interrupt_enabled;
        io_start.hiscalar = c.hiscalar;
        io_start.loscalar = c.loscalar;
        fa = ra = start;
        fio = rio = io_start;
        fio.undo = undo ? &faithful_undo : 0;
        rio.undo = undo ? &readable_undo : 0;
        sf = run(&faithful_lm_imu_performance_routines, &faithful_lm_imu_performance_interpretive,
                 &fa, &fio);
        sr = run(&lm_imu_performance_routines, &lm_imu_performance_interpretive, &ra, &rio);
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
            printf("FAIL case %u (key %u), Z %05o LOC %05o: %s\n", n, n % IPT_ROUTINES,
                   start.words[5], start.words[0164], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    printf("imu-performance-cases %s: %u words as the rope's, %u checks (%u run by the faithful "
           "routines, %u not counted: NEWJOB negative), %u failures\n",
           MODE, words, checks, ran, skipped, failures);
    return failures != 0;
}
