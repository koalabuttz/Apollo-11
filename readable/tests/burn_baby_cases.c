/* SPDX-License-Identifier: MIT */
/* The readable BURN, BABY, BURN against the faithful port's, case by case
 * (make burn-baby-cases ROPE_LM ROPE_CM).
 *
 * Built twice: as the host runs the file, and as the flight program does
 * (-DAGC_FLIGHT_INTERPRETER: a run stops where it reaches the interpreter,
 * and the ABVEL display's EXIT return is a key of its own). Each build
 * links the readable tables (src/binding/burn_baby_routines.c) and the
 * faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the words: the readable interpretive code (P41SPOT's and the ABVEL
 * display's, src/burn_baby/burn_baby.c), written in the listing's notation,
 * must be the original program's rope, word for word. Then the keys, which
 * must be the faithful tables'. Then the runs: each case is one of the
 * port's test states (c64-port/tests/lm_burn_baby_cases.c: every key, WHICH
 * one of the six ignition tables, DISPDEX one of the display indexes,
 * P40/RET one of NULLCLOK's returns, the PHASE words, KILLTASK from its
 * callers with the task in the waitlist or not), half of them stirred
 * further: FB, BANKSET, EDOP, the work area and its push-down pointer,
 * NEWJOB, the superbank, WHICH, DISPDEX, P40/RET, the PHASE words, Q and
 * ITEMP1 at the file's own return points, L and the waitlist at KILLTSK2,
 * no undo image. Each case runs once on each side, from its key until
 * control leaves the file. Where the faithful routine runs, the readable
 * one must too, with the same memory and io record, word for word; where
 * the readable one refuses, it must leave everything as it found it.
 *
 * Not counted: a run the readable routines refuse where a computed
 * transfer (a TC Q, DEAD's DTCB) went to a word of the file no readable
 * function starts at, which the faithful port runs on from (the case
 * states draw Q and ITEMP1 at random); and NEWJOB negative, which the
 * readable interpreter refuses at any pair where the faithful one refuses
 * it only at DANZIG, as in make stateup-cases. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_burn_baby_cases.h"

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
extern const struct interpretive_code P41SPOT_CODE, ABVEL_CODE;
extern const agc_routine_table faithful_lm_burn_baby_routines, faithful_lm_burn_baby_interpretive;
extern uint16_t lm_burn_baby_unentered;

/* Whether the word the readable run stopped at (lm_burn_baby_unentered) is
 * where the case's own Q or ITEMP1 sent it: a TC Q, KILLTASK's return (Q
 * +1) or DEAD's DTCB (ITEMP1). */
static int from_q_or_itemp1(const apollo_overlay_state *start)
{
    unsigned at = lm_burn_baby_unentered, address;

    if (!at) return 0;
    address = at >= 04000u && at < 010000u ? at : at % 1024u + 02000u;
    return address == (start->words[2] & 07777u) || address == ((start->words[2] + 1u) & 07777u) ||
           address == (start->words[061] & 07777u);
}

#define CASES 65536u
#define E7(a) (7u * 0400u + (a) - 01400u)
#define E3(a) (3u * 0400u + (a) - 01400u)

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

/* The file's own return points, where its TC Qs and DEAD's DTCB go: after
 * NULLCLOK's TC NOULLAGE and TC KILLTASK, COMFAIL2's TC KILLTASK and
 * ULLGTASK's TC ONULLAGE (with ULLAGOFF's and the like returns), and two
 * words that are none (an INDEX's operand, a routine's middle). */
static const uint16_t returns[] = {03021u, 03023u, 02630u, 02347u, 02563u, 02436u, 02327u, 02750u};

/* One case: the port's test state n, and for odd n more. In the flight
 * build a quarter of the basic cases start at the EXIT return. */
static void state(apollo_overlay_state *s, lmbb_case *c, int *undo, uint16_t n)
{
    uint16_t key = lmbb_routine_keys[n % LMBB_ROUTINES];

    memset(s, 0, sizeof *s);
    lmbb_input(s, c, n);
    *undo = 1;
#ifdef AGC_FLIGHT_INTERPRETER
    if (n % LMBB_ROUTINES < LMBB_BASIC_ROUTINES && n / LMBB_ROUTINES % 4u == 1u &&
        key / 1024u == 036u) {
        s->words[5] = 02237u;
        s->words[4] = 074000u;
    }
#endif
    if (n % 2u == 0u) goto banks;
    if (rnd() % 16u == 0u) c->superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);                  /* FB */
    if (rnd() % 16u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u);               /* BANKSET */
    if (rnd() % 16u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);                     /* EDOP */
    if (rnd() % 16u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 16u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 8u == 0u) /* WHICH, a table's or near one */
        s->words[E7(01455u)] = (uint16_t)(02022u + rnd() % 0110u);
    if (rnd() % 8u == 0u) /* DISPDEX: a count, minus any index, or -0 */
        s->words[01163] = (uint16_t)(rnd() % 2u ? 077777u - rnd() % 040u : rnd() % 4u);
    if (rnd() % 8u == 0u) s->words[01142] = (uint16_t)(03010u + rnd() % 0100u); /* P40/RET */
    if (rnd() % 8u == 0u) s->words[0757 + 2u * (rnd() % 4u)] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 4u == 0u) s->words[2] = returns[rnd() % 8u];            /* Q */
    if (rnd() % 4u == 0u) s->words[061] = returns[rnd() % 8u];          /* ITEMP1 */
    if (rnd() % 4u == 0u) s->words[062] = (uint16_t)(036u << 10 | 07u); /* ITEMP2: bank 36 */
    if (rnd() % 8u == 0u && key / 1024u == 027u)
        s->words[1] = (uint16_t)(2u * (rnd() % 9u)); /* L */
    if (rnd() % 8u == 0u) { /* a waitlist entry: the task's GENADR and bank, or near them */
        unsigned k = 2u * (rnd() % 9u);
        s->words[E3(01410u) + k] =
            (uint16_t)(rnd() % 2u ? 02000u | (s->words[064] & 01777u) : rnd() & 077777u);
        s->words[E3(01411u) + k] = (uint16_t)(rnd() % 2u ? s->words[063] : rnd() & 077777u);
    }
    if (rnd() % 8u == 0u) c->chan30 = (agc_word)(rnd() & 077777u);
    if (rnd() % 64u == 0u) *undo = 0;
banks:
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static enum exec_status run(const agc_routine_table *basic, const agc_routine_table *interp,
                            apollo_overlay_state *s, lm_bb_io *io)
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
static int io_differs(const lm_bb_io *a, const lm_bb_io *b)
{
    return memcmp(&a->control, &b->control, sizeof a->control) || a->chan11 != b->chan11 ||
           a->chan14 != b->chan14 || a->chan30 != b->chan30;
}

int main(int argc, char **argv)
{
    static apollo_overlay_state faithful_undo, readable_undo;
    static unsigned unentered_at[65536];
    unsigned i, n, checks = 0, failures = 0, unentered = 0, newjob = 0, ran = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    failures = words_differ(&P41SPOT_CODE) + words_differ(&ABVEL_CODE);
    failures += tables_differ(&lm_burn_baby_routines, &faithful_lm_burn_baby_routines, "basic");
    failures += tables_differ(&lm_burn_baby_interpretive, &faithful_lm_burn_baby_interpretive,
                              "interpretive");
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, fa, ra;
        lmbb_case c;
        lm_bb_io io_start, fio, rio;
        enum exec_status sf, sr;
        const char *why = 0;
        int undo;

        state(&start, &c, &undo, (uint16_t)n);
        memset(&io_start, 0, sizeof io_start);
        io_start.control.superbank = c.superbank;
        io_start.control.interrupt_enabled = c.interrupt_enabled;
        io_start.chan11 = c.chan11;
        io_start.chan14 = c.chan14;
        io_start.chan30 = c.chan30;
        fa = ra = start;
        fio = rio = io_start;
        fio.undo = undo ? &faithful_undo : 0;
        rio.undo = undo ? &readable_undo : 0;
        sf = run(&faithful_lm_burn_baby_routines, &faithful_lm_burn_baby_interpretive, &fa, &fio);
        sr = run(&lm_burn_baby_routines, &lm_burn_baby_interpretive, &ra, &rio);
        ++checks;
        if (sf == EXEC_OK) ++ran;
        if (sr != EXEC_OK && (memcmp(&ra, &start, sizeof ra) || io_differs(&rio, &io_start)))
            why = "readable refusal changed the state";
        else if (sf == EXEC_OK && sr != EXEC_OK && from_q_or_itemp1(&start)) {
            ++unentered;
            ++unentered_at[lm_burn_baby_unentered];
        } else if (sf == EXEC_OK && sr != EXEC_OK && start.words[067] > 037777u)
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
            printf("FAIL case %u, Z %05o FB %05o: %s\n", n, start.words[5], start.words[4], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (fa.words[w] != ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                           fa.words[w], start.words[w]);
        }
    }
    printf("burn-baby-cases %s: %u words as the rope's, %u checks (%u run by the faithful "
           "routines; not counted: %u on from a word no readable function starts at, %u NEWJOB "
           "negative), %u failures\n",
           MODE, P41SPOT_CODE.count + ABVEL_CODE.count, checks, ran, unentered, newjob, failures);
    for (i = 0; i < 65536u; ++i)
        if (unentered_at[i]) printf("  on from physical %06o: %u\n", i, unentered_at[i]);
    return failures != 0;
}
