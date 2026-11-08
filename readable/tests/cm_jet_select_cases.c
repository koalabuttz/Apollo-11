/* SPDX-License-Identifier: MIT */
/* The readable JET_SELECTION_LOGIC against the faithful port's, key by key
 * (make cm-jet-select-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_jet_select_routines.c)
 * and the faithful port's under the same names with faithful_ in front
 * (the Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and the hot table's,
 * which must be the faithful one's with the same routines as the basic
 * table. Then the runs: each readable key from 4,096 states, each one of
 * the port's frozen test states (c64-port/tests/cm_jet_select_vectors.h)
 * entered at the key, most stirred further: channel 13 (now and then
 * with its trap bits set), channel 31's translation bits
 * (often none commanded), DAPDATR1's LEM bit, XTRANS, the rotation
 * commands TAU, TAU1 and TAU2 (often +0, -0, 1 or -1, or short of 14 ms
 * or .1 s on the jets) and the quad flags RACFAIL, RBDFAIL and ACORBD
 * (often +0, -0, 1 or -1), the numbers of jets NPJETS, NYJETS and NRJETS
 * (mostly -3 to 3) and the on-times BLAST, BLAST1 and BLAST2, the
 * translation and rotation indexes
 * (mostly within their tables, now and then one that lands a read at Z or
 * just past a table), the roll word RWORD1, T5TIME and T5TEMP, A at a key
 * (mostly a table index or a word a table holds, now and then an overflow
 * or one that lands a read at Z or past a table), any erasable bank's
 * words and EBANK (now and then past E7), L and Q, the latches, or a
 * state the routines refuse (BBANK not the two banks, ZERO not +0, a word
 * of 16 bits). Each runs once on each side, as the program's trampoline
 * would call the key's routine, with a hardware record or (now and then)
 * none. Where either port runs, the other must too, with the same memory,
 * io and hardware records, word for word; where the readable one refuses,
 * it must leave everything as it found it; and the faithful routine must
 * run in at least 1 of 16 of each key's states. A timer stops a run that
 * goes on past a tenth of a second (none should); such a state must loop
 * on both sides. */
#define _POSIX_C_SOURCE 200809L /* sigsetjmp, setitimer */
#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_jet_select_vectors.h"
#include "../src/agc/names.h"

extern const agc_routine_table cm_jet_select_basic, cm_jet_select_interpretive,
    cm_jet_select_hot_routines;
extern const agc_routine_table faithful_cm_jet_select_basic, faithful_cm_jet_select_interpretive,
    faithful_cm_jet_select_hot_routines;
bool cm_jet_select_readable(uint16_t key);

/* A word's place in the state: E6's words at 03000 + (the address -
 * 01400), unswitched ones at their address. */
#define AT(name) ((unsigned)K_##name >= 01400u ? (unsigned)K_##name + 01400u : (unsigned)K_##name)

#define CASES 4096u

static uint32_t seed = 1039u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
}

static unsigned pick(unsigned n)
{
    return (unsigned)((rnd() >> 6) % n);
}

/* A run that loops for ever: what the timer stops it with. */
#define LOOPS ((enum exec_status) - 1)

static sigjmp_buf stopped;

static void stop(int signal)
{
    (void)signal;
    siglongjmp(stopped, 1);
}

/* A key's routine, run until it returns, or stopped by the timer. */
static enum exec_status run(const agc_routine_entry *entry, agc_native_program *program,
                            apollo_overlay_state *state, cm_native_io *io)
{
    static const struct itimerval tenth = {{0, 0}, {0, 100000}}, off = {{0, 0}, {0, 0}};
    enum exec_status status;

    if (sigsetjmp(stopped, 1)) return LOOPS;
    setitimer(ITIMER_REAL, &tenth, NULL);
    status = entry->call(program, state, io);
    setitimer(ITIMER_REAL, &off, NULL);
    return status;
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

/* Each hot key runs the basic table's routine for it. */
static unsigned hot_differs(void)
{
    unsigned i, j, failures = 0;

    for (i = 0; i < cm_jet_select_hot_routines.count; ++i) {
        const agc_routine_entry *hot = &cm_jet_select_hot_routines.entries[i];
        for (j = 0; j < cm_jet_select_basic.count; ++j)
            if (cm_jet_select_basic.entries[j].physical == hot->physical) break;
        if (j == cm_jet_select_basic.count || cm_jet_select_basic.entries[j].call != hot->call) {
            printf("FAIL: hot key %u does not run the basic table's routine\n", hot->physical);
            ++failures;
        }
    }
    return failures;
}

static uint16_t any_word(void)
{
    return (uint16_t)(rnd() & 077777u);
}

/* A word of a number: often an edge value. */
static uint16_t value_word(void)
{
    static const uint16_t edges[] = {0u,      077777u, 1u,      077776u, 020000u,
                                     057777u, 037777u, 040000u, 010000u, 067777u};
    return pick(4u) ? any_word() : edges[pick(sizeof edges / sizeof edges[0])];
}

/* A sign CCS branches on: often +0, -0, 1 or -1. */
static uint16_t sign_word(void)
{
    unsigned k = pick(8u);

    return k < 2u ? 0u : k == 2u ? 077777u : k == 3u ? 1u : k == 4u ? 077776u : value_word();
}

/* A small signed number as a word. */
static uint16_t number(int v)
{
    return (uint16_t)(v < 0 ? 077777u - (unsigned)-v : (unsigned)v);
}

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(uint16_t v)
{
    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

/* A rotation command for the on-times: often +0, -0, 1 or -1, or short
 * of 14 ms or .1 s by the number of jets, or any. */
static uint16_t tau_word(void)
{
    unsigned k = pick(4u);
    int v = k == 1u ? (int)pick(50u) : (int)pick(1000u);

    return k == 0u ? sign_word() : k < 3u ? number(pick(2u) ? v : -v) : value_word();
}

/* An on-time: mostly short of 14 ms or about it, or any. */
static uint16_t blast_word(void)
{
    return pick(2u) ? number((int)pick(60u) - 10) : value_word();
}

/* The frozen cases. */
static unsigned mission_cases[CM_JET_SELECT_VECTORS], mission_count;

/* A Q to return to: 3, a word of fixed memory, or any. */
static uint16_t return_word(void)
{
    unsigned k = pick(4u);

    return k == 0u    ? 3u
           : k == 1u  ? (uint16_t)(02000u + pick(06000u))
           : pick(2u) ? any_word()
                      : rnd();
}

/* An index a table read adds to its field: mostly within the table (top
 * words), now and then one past either end, one that lands at Z, or any. */
static uint16_t table_index(unsigned field, unsigned top)
{
    unsigned k = pick(16u);

    return k < 11u    ? (uint16_t)pick(top)
           : k == 11u ? number(pick(2u) ? -1 : (int)top)
           : k == 12u ? number(5 - (int)field)
           : k == 13u ? sign_word()
                      : value_word();
}

/* A number of jets, as NJET and DFTMAX are indexed by it: mostly -3 to 3,
 * now and then 4 past either end, or an index that lands a read at Z or
 * anywhere. */
static uint16_t jets_word(void)
{
    unsigned k = pick(8u);

    return k < 6u    ? number((int)pick(7u) - 3)
           : k == 6u ? number(pick(2u) ? -4 : 4)
                     : table_index(K_NJET, 4u);
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hw, uint16_t key,
                  unsigned n)
{
    static const uint16_t tables[][2] = {{K_PYTABLE, 017u}, {K_RTABLE, 017u}, {K_YZTABLE, 011u}};
    const irc_case *c = &cm_jet_select_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[3] = 03000u; /* E6, KMPAC's */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] >> 8);
    hw->channel31 = any_word();
    /* Channel 13, now and then with the HANDRUPT trap bits (12-14) set. */
    io->channel13 = (uint16_t)(pick(8u) ? any_word() & 043777u : any_word());
    if (n % 4u != 0u) {
        /* The translation hand controller, channel 31's bits 7-12 (set
         * for none), and the LEM bit. */
        k = pick(4u);
        hw->channel31 = (uint16_t)(k == 0u   ? any_word() | 07700u
                                   : k == 1u ? (any_word() | 07700u) & ~(01u << (6u + pick(6u)))
                                             : any_word());
        w[AT(DAPDATR1)] = pick(2u) ? (uint16_t)(rnd() & 020000u) : any_word();
        /* The quad flags and the rotation commands. */
        w[AT(XTRANS)] = sign_word();
        w[AT(RACFAIL)] = sign_word();
        w[AT(RBDFAIL)] = sign_word();
        w[AT(ACORBD)] = sign_word();
        w[AT(TAU)] = tau_word();
        w[AT(TAU1)] = tau_word();
        w[AT(TAU2)] = tau_word();
        /* The translation and rotation indexes. */
        w[AT(XNDX1)] = table_index(K_XLNNDX, 4u);
        w[AT(XNDX2)] = table_index(K_XLNNDX, 4u);
        w[AT(YNDX)] = table_index(K_XLNNDX, 4u);
        w[AT(ZNDX)] = table_index(K_XLNNDX, 4u);
        w[AT(PINDEX)] = table_index(K_PYTABLE, 3u);
        w[AT(YINDEX)] = table_index(K_PYTABLE, 3u);
        w[AT(RINDEX)] = table_index(K_RTABLE, 3u);
        /* The roll word, mostly a table's (its net roll bits small). */
        w[AT(RWORD1)] = pick(2u) ? (uint16_t)(rnd() & (pick(2u) ? 03760u : 034017u)) : value_word();
        w[AT(T5TIME)] = value_word();
        w[AT(T5TEMP)] = value_word();
        /* The numbers of jets, and the on-times. */
        w[AT(NPJETS)] = jets_word();
        w[AT(NYJETS)] = jets_word();
        w[AT(NRJETS)] = pick(4u) ? jets_word() : value_word();
        w[AT(BLAST)] = blast_word();
        w[AT(BLAST1)] = blast_word();
        w[AT(BLAST2)] = blast_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (pick(32u) == 0u) w[k] = value_word();
        /* A (a table index, a table's word, now and then an overflow or
         * one that lands a read at Z or past a table), L and Q. */
        k = pick(8u);
        if (k < 3u) {
            const uint16_t *t = tables[pick(3u)];
            w[0] = table_index(t[0], t[1]);
        } else if (k < 5u) {
            w[0] = (uint16_t)(rnd() & 037777u);
        } else {
            w[0] = value_word();
        }
        w[0] = register_word(w[0]);
        w[1] = register_word(value_word());
        w[2] = return_word();
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(8u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
        }
        if (pick(64u) == 0u) { /* an EBANK past E7, with BBANK agreeing */
            w[3] = (uint16_t)(pick(0100u) << 8);
            eb = (uint16_t)(w[3] >> 8);
        }
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, ZERO
     * not +0, or a word of 16 bits. */
    if (n % 4u != 0u && pick(64u) == 0u) w[6] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[7] = any_word();
    if (n % 4u != 0u && pick(64u) == 0u) w[3u + pick(03775u)] |= 0100000u;
}

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs, unsigned *written, unsigned *loops)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        if (!cm_jet_select_readable(key)) continue; /* the faithful routine, on both sides */
        ++*written;
        for (n = 0; n < CASES; ++n) {
            apollo_overlay_state start, fa, ra;
            cm_native_io io, fio, rio;
            agc_hardware hardware, fhw, rhw;
            agc_native_program p, fp, rp;
            enum exec_status sf, sr;
            const char *why = 0;

            state(&start, &io, &hardware, key, n);
            fa = ra = start;
            fio = rio = io;
            fhw = rhw = hardware;
            memset(&p, 0, sizeof p);
            fp = rp = p;
            if (n % 64u != 63u) { /* now and then none */
                fp.hardware = &fhw;
                rp.hardware = &rhw;
            }
            sf = run(&faithful->entries[i], &fp, &fa, &fio);
            sr = run(&readable->entries[i], &rp, &ra, &rio);
            ran += sf == EXEC_OK;
            *loops += sf == LOOPS;
            if ((sf == LOOPS) != (sr == LOOPS))
                why = sf == LOOPS ? "faithful loops, readable does not" : "readable loops";
            else if (sf == LOOPS)
                why = NULL;
            else if (sr != EXEC_OK &&
                     (memcmp(&ra, &start, sizeof ra) || memcmp(&rio, &io, sizeof rio) ||
                      memcmp(&rhw, &hardware, sizeof rhw)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&fa, &ra, sizeof fa))
                why = "different memory";
            else if (sf == EXEC_OK &&
                     (memcmp(&fio, &rio, sizeof fio) || memcmp(&fhw, &rhw, sizeof fhw)))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL key %u (%04o) case %u: %s\n", key, 02000u + key % 1024u, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (fa.words[w] != ra.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w, ra.words[w],
                               fa.words[w], start.words[w]);
            }
        }
        if (ran < CASES / 16u && bad == 0u) {
            printf("FAIL key %u: the faithful routine ran in %u of %u states\n", key, ran, CASES);
            ++bad;
        }
        *runs += ran;
        failures += bad;
    }
    return failures;
}

int main(int argc, char **argv)
{
    unsigned i, failures = 0, runs = 0, written = 0, loops = 0;
    struct sigaction on_alarm;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_JET_SELECT_VECTORS; ++i)
        if (cm_jet_select_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_jet_select_basic, &faithful_cm_jet_select_basic, "basic");
    failures += tables_differ(&cm_jet_select_interpretive, &faithful_cm_jet_select_interpretive,
                              "interpretive");
    failures +=
        tables_differ(&cm_jet_select_hot_routines, &faithful_cm_jet_select_hot_routines, "hot");
    failures += hot_differs();
    if (failures) {
        printf("cm-jet-select-cases: %u failures\n", failures);
        return 1;
    }
    memset(&on_alarm, 0, sizeof on_alarm);
    on_alarm.sa_handler = stop;
    sigaction(SIGALRM, &on_alarm, NULL);
    failures += keys(&cm_jet_select_basic, &faithful_cm_jet_select_basic, &runs, &written, &loops);
    printf("cm-jet-select-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u looping on both sides, %u failures\n",
           written, faithful_cm_jet_select_basic.count, CASES, runs, loops, failures);
    return failures != 0;
}
