/* SPDX-License-Identifier: MIT */
/* The readable SERVICER against the faithful port's, case by case (make
 * servicer-cases ROPE_LM ROPE_CM).
 *
 * Built twice: as the host runs the file, and as the flight program does
 * (-DAGC_FLIGHT_INTERPRETER: a run stops where it reaches the interpreter,
 * and the words after the interpretive code's EXITs are keys of their
 * own). Each build links the readable tables
 * (src/binding/servicer_routines.c) and the faithful port's under the same
 * names with faithful_ in front (the Makefile renames them).
 *
 * First the words: the readable interpretive code, written in the
 * listing's notation, must be the original program's rope, word for word.
 * Then the keys, which must be the faithful tables'. Then the runs: each
 * case is one of the port's test states (c64-port/tests/lm_servicer_cases.c:
 * every key, the indexes one of the values their producers store, Q a
 * return into the file in half of them, the PHASE words and the values the
 * file's CCS and BZF tests single out), half of them stirred further: FB,
 * BANKSET, EDOP, the work area and its push-down pointer, NEWJOB, the
 * superbank, A, Q at the file's own return points, AVGEXIT one of the
 * 2CADRs SERVICER exits through, the PHASE words, DVCNTR, PIPAGE and the
 * PIPA reading's words, the channels, no undo image. Each case runs once
 * on each side, from its key until control leaves the file. Where the
 * faithful routine runs, the readable one must too, with the same memory
 * and io record, word for word; where the readable one refuses, it must
 * leave everything as it found it; and each key's faithful routine must
 * run in at least 1 in 16 of the states entered at it (key_floor.h).
 *
 * Not counted: a run the readable routines refuse where a computed
 * transfer (a TC Q, SETPOS's TC LRADRET, SERVOUT's DXCH Z through
 * AVGEXIT) went to a word of the file no readable function starts at,
 * which the faithful port runs on from (the case states draw Q, AVGEXIT
 * and LRADRET at random); and NEWJOB negative, which the readable
 * interpreter refuses at any pair where the faithful one refuses it only
 * at DANZIG, as in make stateup-cases. Each
 * only if the same state, with that return made one a readable function
 * starts at or with NEWJOB +0, runs on both sides alike: so the run up to
 * the transfer, or the whole run, is checked all the same. */
#include <stdio.h>
#include <string.h>
#include "rope_image.h"
#include "lm_servicer_cases.h"
#include "key_floor.h"

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
extern const struct interpretive_code GETABVAL_CODE, AVERAGEG_CODE, AVGEND_CODE, NORMLIZE_CODE,
    NORMLIZ1_CODE, MUNRETRN_CODE, NOR29NOW_CODE, CALCGRAV_CODE, CALCRVG_CODE, RVBOTH_CODE,
    RVBOTH_2_CODE, MUNRVG_CODE, MUNGRAV_CODE, POSUPDAT_CODE, POSUPDAT_2_CODE, POSUPDAT_3_CODE,
    NOREASON_CODE, NOREASON_2_CODE, VELUPDAT_CODE, VELUPDAT_2_CODE, WSTOR_CODE, SETPOS_CODE;
static const struct interpretive_code *const codes[] = {
    &GETABVAL_CODE,   &AVERAGEG_CODE, &AVGEND_CODE,     &NORMLIZE_CODE, &NORMLIZ1_CODE,
    &MUNRETRN_CODE,   &NOR29NOW_CODE, &CALCGRAV_CODE,   &CALCRVG_CODE,  &RVBOTH_CODE,
    &RVBOTH_2_CODE,   &MUNRVG_CODE,   &MUNGRAV_CODE,    &POSUPDAT_CODE, &POSUPDAT_2_CODE,
    &POSUPDAT_3_CODE, &NOREASON_CODE, &NOREASON_2_CODE, &VELUPDAT_CODE, &VELUPDAT_2_CODE,
    &WSTOR_CODE,      &SETPOS_CODE,
};
extern const agc_routine_table faithful_lm_servicer_routines, faithful_lm_servicer_interpretive;
extern uint16_t lm_servicer_unentered;

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))
#define CASES        (2u * LMSV_BASE_CASES)
#define E7(a)        (7u * 0400u + (a) - 01400u)
#define AVGEXIT      01252u
#define LRADRET      E7(01650u)

/* Whether the word the readable run stopped at (lm_servicer_unentered) is
 * where the case's own Q, AVGEXIT or LRADRET sent it. */
static int from_drawn_return(const apollo_overlay_state *start)
{
    unsigned at = lm_servicer_unentered, address;

    if (!at) return 0;
    address = at >= 04000u && at < 010000u ? at : at % 1024u + 02000u;
    return address == (start->words[2] & 07777u) || address == (start->words[AVGEXIT] & 07777u) ||
           address == (start->words[LRADRET] & 07777u);
}

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

/* The file's own return points, where its TC Qs go (after its TCs of
 * GNUFAZE5, GNUTFAZ5, PIPASR, COPYCYC, TMPTOSPT and QUIKFAZ5), and two
 * words that are none (an extracode's word, a routine's middle). */
static const uint16_t returns[] = {03412u, 03437u, 03451u, 03453u, 03524u, 03417u, 02261u, 02276u,
                                   02275u, 02353u, 02370u, 02411u, 02422u, 02433u, 03462u};
/* The 2CADRs SERVICER exits through (AVGEXIT): AVGEND's and SERVEXIT's, the
 * guidance's (P40's, P63's, P12's, the ascent's), and one into bank 33's
 * middle. */
static const uint16_t exits[][2] = {
    {03661u, 044067u}, {03770u, 064067u}, {02000u, 034067u}, {03100u, 060007u}, {02316u, 066007u}};

/* The run of the first step in the flight build: a quarter of the basic
 * cases start at a word after an EXIT instead (GETABVAL's or ASCSPOT's). */
static void state(apollo_overlay_state *s, lmsv_case *c, int *undo, unsigned n)
{
    uint16_t base = (uint16_t)(n % LMSV_BASE_CASES);
#ifdef AGC_FLIGHT_INTERPRETER
    uint16_t key = lmsv_routine_keys[base % LMSV_ROUTINES];
#endif

    memset(s, 0, sizeof *s);
    lmsv_input(s, c, base);
    *undo = 1;
    /* At CALCGRV1's BDDV* (33,2770), X1 as CALCGRAV's LXC,1 RTX2 leaves it
     * (-0 or -2), in seven eighths of the states. */
    if (lmsv_routine_keys[base % LMSV_ROUTINES] == 033u * 1024u + 0770u && rnd() % 8u != 0u)
        s->words[s->words[0120] + 046u] = (uint16_t)(rnd() % 2u ? 077777u : 077775u);
#ifdef AGC_FLIGHT_INTERPRETER
    if (base % LMSV_ROUTINES < LMSV_BASIC_ROUTINES && base / LMSV_ROUTINES % 4u == 1u) {
        if (key / 1024u == 033u) s->words[5] = 02223u;
        if (key / 1024u == 023u) s->words[5] = 02504u;
    }
#endif
    if (n % 2u == 0u && n < LMSV_BASE_CASES) goto banks;
    if (rnd() % 16u == 0u) c->superbank = (agc_word)(rnd() & 0160u);
    if (rnd() % 32u == 0u) s->words[4] = (uint16_t)(rnd() & 076000u);                  /* FB */
    if (rnd() % 16u == 0u) s->words[0165] = (uint16_t)(rnd() & 077777u);               /* BANKSET */
    if (rnd() % 16u == 0u) s->words[023] = (uint16_t)(rnd() % 4u);                     /* EDOP */
    if (rnd() % 16u == 0u) s->words[0120] = (uint16_t)(0401u + 054u * (rnd() % 6u));   /* FIXLOC */
    if (rnd() % 16u == 0u) s->words[0166] = (uint16_t)(s->words[0120] + rnd() % 050u); /* PUSHLOC */
    if (rnd() % 8u == 0u) s->words[067] = (uint16_t)(rnd() % 4u ? rnd() & 03777u : rnd() & 077777u);
    if (rnd() % 8u == 0u) /* A: overflow, a count, anything */
        s->words[0] = (uint16_t)(rnd() % 2u   ? 040000u << (rnd() % 2u) | (rnd() & 037777u)
                                 : rnd() % 2u ? rnd() % 24u
                                              : rnd() & 077777u);
    if (rnd() % 4u == 0u) s->words[2] = returns[rnd() % COUNT(returns)]; /* Q */
    if (rnd() % 4u == 0u) {                                              /* AVGEXIT */
        unsigned k = rnd() % COUNT(exits);
        s->words[AVGEXIT] = exits[k][0];
        s->words[AVGEXIT + 1u] = exits[k][1];
    }
    if (rnd() % 8u == 0u) s->words[0751u + 2u * (rnd() % 7u)] = (uint16_t)(rnd() & 077777u);
    if (rnd() % 8u == 0u) s->words[E7(01515u)] = (uint16_t)(rnd() % 3u); /* DVCNTR */
    if (rnd() % 8u == 0u) { /* PIPAGE, DELVX-Z, TEMX-Z: +0, -0 or anything */
        static const uint16_t at[] = {01257u, 0324u, 0326u, 0330u, 01254u, 01255u, 01256u};
        unsigned k = rnd() % 3u;
        s->words[at[rnd() % COUNT(at)]] = (uint16_t)(k == 0u ? 0u : k == 1u ? 077777u : rnd());
    }
    if (rnd() % 8u == 0u) c->chan32 = (agc_word)(rnd() & 077777u);
    if (rnd() % 8u == 0u) c->interrupt_enabled = (uint16_t)(rnd() & 1u);
    if (rnd() % 64u == 0u) *undo = 0;
banks:
    s->words[6] = (uint16_t)((s->words[4] & 076000u) | (s->words[3] >> 8)); /* BBANK */
}

static uint16_t resolved_point; /* the last run's entry point */
static agc_routine resolved;

static enum exec_status run(const agc_routine_table *basic, const agc_routine_table *interp,
                            apollo_overlay_state *s, lm_sv_io *io)
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
    resolved_point =
        agc_transfer_next(&program, s, io->control.superbank, AGC_TRANSFER_NONE, &call);
    resolved = call;
    return call ? call(&program, s, io) : EXEC_OUT_OF_SCOPE;
}

/* The io record but its undo pointer. */
static int io_differs(const lm_sv_io *a, const lm_sv_io *b)
{
    return memcmp(&a->control, &b->control, sizeof a->control) || a->chan11 != b->chan11 ||
           a->chan12 != b->chan12 || a->chan32 != b->chan32 || a->chan33 != b->chan33;
}

/* Both sides from one state. */
struct both {
    apollo_overlay_state fa, ra;
    lm_sv_io fio, rio;
    enum exec_status sf, sr;
};
static void both_run(struct both *b, const apollo_overlay_state *start, const lm_sv_io *io_start,
                     int undo)
{
    static apollo_overlay_state faithful_undo, readable_undo;

    b->fa = b->ra = *start;
    b->fio = b->rio = *io_start;
    b->fio.undo = undo ? &faithful_undo : 0;
    b->rio.undo = undo ? &readable_undo : 0;
    b->sf =
        run(&faithful_lm_servicer_routines, &faithful_lm_servicer_interpretive, &b->fa, &b->fio);
    b->sr = run(&lm_servicer_routines, &lm_servicer_interpretive, &b->ra, &b->rio);
}

/* What is wrong with a pair of runs from a state, or 0. */
static const char *verdict(const struct both *b, const apollo_overlay_state *start,
                           const lm_sv_io *io_start)
{
    if (b->sr != EXEC_OK && (memcmp(&b->ra, start, sizeof b->ra) || io_differs(&b->rio, io_start)))
        return "readable refusal changed the state";
    if (b->sf != b->sr)
        return b->sf == EXEC_OK ? "readable refused, faithful ran"
                                : "readable ran, faithful refused";
    if (b->sf == EXEC_OK && memcmp(&b->fa, &b->ra, sizeof b->fa)) return "different memory";
    if (b->sf == EXEC_OK && io_differs(&b->fio, &b->rio)) return "different io";
    return 0;
}

/* The state with the return that sent the readable run to a word no
 * readable function starts at made one it starts at: Q a return in the
 * bank FB selects, AVGEXIT AVGEND's 2CADR, LRADRET the return of
 * POSGOOD's TC SETPOS2. */
static void redirect(apollo_overlay_state *s)
{
    static const struct {
        unsigned bank;
        uint16_t at;
    } starts[] = {{021u, 02150u}, {022u, 03707u}, {023u, 02504u}, {027u, 03321u},
                  {032u, 03772u}, {033u, 02261u}, {037u, 03412u}};
    unsigned at = lm_servicer_unentered, i;
    unsigned address = at >= 04000u && at < 010000u ? at : at % 1024u + 02000u;

    if (address == (s->words[2] & 07777u))
        for (i = 0; i < COUNT(starts); ++i)
            if (starts[i].bank == (unsigned)(s->words[4] >> 10)) s->words[2] = starts[i].at;
    if (address == (s->words[AVGEXIT] & 07777u)) {
        s->words[AVGEXIT] = 03661u;
        s->words[AVGEXIT + 1u] = 044067u;
    }
    if (address == (s->words[LRADRET] & 07777u)) s->words[LRADRET] = 03700u;
}

int main(int argc, char **argv)
{
    static unsigned unentered_at[65536];
    static struct both b, redirected;
    unsigned i, n, checks = 0, failures = 0, unentered = 0, newjob = 0, ran = 0, words = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < COUNT(codes); ++i) {
        failures += words_differ(codes[i]);
        words += codes[i]->count;
    }
    failures += tables_differ(&lm_servicer_routines, &faithful_lm_servicer_routines, "basic");
    failures += tables_differ(&lm_servicer_interpretive, &faithful_lm_servicer_interpretive,
                              "interpretive");
    for (n = 0; n < CASES; ++n) {
        apollo_overlay_state start, start2;
        lmsv_case c;
        lm_sv_io io_start;
        const struct both *shown = &b;
        const char *why = 0;
        int undo;

        state(&start, &c, &undo, n);
        memset(&io_start, 0, sizeof io_start);
        io_start.control.superbank = c.superbank;
        io_start.control.interrupt_enabled = c.interrupt_enabled;
        io_start.chan11 = c.chan11;
        io_start.chan12 = c.chan12;
        io_start.chan32 = c.chan32;
        io_start.chan33 = c.chan33;
        both_run(&b, &start, &io_start, undo);
        ++checks;
        if (b.sf == EXEC_OK) ++ran;
#ifdef AGC_FLIGHT_INTERPRETER
        if (!key_in(&lm_servicer_interpretive, resolved_point, resolved))
#endif
            key_tally(resolved_point, resolved, b.sf == EXEC_OK);
        if (b.sr != EXEC_OK &&
            (memcmp(&b.ra, &start, sizeof b.ra) || io_differs(&b.rio, &io_start)))
            why = "readable refusal changed the state";
        else if (b.sf == EXEC_OK && b.sr != EXEC_OK && from_drawn_return(&start)) {
            /* Not counted only if the same state with that return made a
             * readable one runs alike on both sides: the run up to the
             * computed transfer is right. */
            unsigned at = lm_servicer_unentered;
            start2 = start;
            redirect(&start2);
            both_run(&redirected, &start2, &io_start, undo);
            if (verdict(&redirected, &start2, &io_start) || redirected.sf != EXEC_OK) {
                why = "on from a word no readable function starts at, and wrong with the return "
                      "redirected";
                shown = &redirected;
            } else {
                ++unentered;
                ++unentered_at[at];
            }
        } else if (b.sf == EXEC_OK && b.sr != EXEC_OK && start.words[067] > 037777u) {
            /* Not counted only if the same state with NEWJOB +0 runs
             * alike on both sides: the faithful run never reached a
             * DANZIG, so NEWJOB was all the readable one refused. */
            start2 = start;
            start2.words[067] = 0u;
            both_run(&redirected, &start2, &io_start, undo);
            if (verdict(&redirected, &start2, &io_start) || redirected.sf != EXEC_OK) {
                why = "NEWJOB negative, and wrong with NEWJOB +0";
                shown = &redirected;
            } else
                ++newjob;
        } else
            why = verdict(&b, &start, &io_start);
        if (why && ++failures <= 30) {
            unsigned w;
            printf("FAIL case %u, Z %05o FB %05o: %s\n", n, start.words[5], start.words[4], why);
            for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                if (shown->fa.words[w] != shown->ra.words[w])
                    printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, shown->ra.words[w],
                           shown->fa.words[w], start.words[w]);
        }
    }
    failures += keys_under_floor("servicer-cases " MODE);
    printf("servicer-cases %s: %u words as the rope's, %u checks (%u run by the faithful "
           "routines; not counted: %u on from a word no readable function starts at, %u NEWJOB "
           "negative), %u failures\n",
           MODE, words, checks, ran, unentered, newjob, failures);
    for (i = 0; i < 65536u; ++i)
        if (unentered_at[i]) printf("  on from physical %06o: %u\n", i, unentered_at[i]);
    return failures != 0;
}
