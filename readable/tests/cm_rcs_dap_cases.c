/* SPDX-License-Identifier: MIT */
/* The readable RCS-CSM_DIGITAL_AUTOPILOT against the faithful port's, key
 * by key (make cm-rcs-dap-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_rcs_dap_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and the hot table's,
 * which must be the faithful one's with the same routines as the basic
 * table. Then the runs: each readable key from 4,096 states, each one of
 * the port's frozen test states (c64-port/tests/cm_rcs_dap_vectors.h)
 * entered at the key, most stirred further: T5PHASE, HOLDFLAG (+0, -0, 1,
 * -1 often), RCSFLAGS's bits, IMODES33, ATTSEC, the loop indexes SPNDX and
 * DPNDX (mostly an axis' 0-2 and 0-4, now and then one that makes a
 * read's address Z) and A (often an axis' index, now and then an
 * overflow), ATTKALMN (mostly a gain's index the faithful port's tables
 * copy, -3 to 11, now and then just past them or Z's), the rate filter's
 * words, the CDU angles and their past values (often near each other, now
 * and then more than 90 degrees apart), the desired angles and their increments, the needles'
 * inputs AK, AK1 and AK2 (often within the DACs' 384 pulses, often past
 * them), their past drives and the CDU command counters, channels 12, 14
 * and 31, L and Q (often a key of the same part, so a TC Q runs on, or Q
 * 3), any erasable bank's words, the latches, or a state the routines
 * refuse (EBANK not E6, BBANK not the two banks, a word of 16 bits). Each
 * runs once on each side, as the program's trampoline would call the key's
 * routine, with a hardware record or (now and then) none. Where either
 * port runs, the other must too, with the same memory, io and hardware
 * records, word for word; where the readable one refuses, it must leave
 * everything as it found it; and the faithful routine must run in at least
 * 1 of 16 of each key's states.
 *
 * A few states loop for ever, as the AGC would (until an interrupt): in
 * DACLOOP an index of 3 makes LXCH EDRIVEX +3 write SPNDX itself, and the
 * loop can go round again and again. A timer stops each run that goes on
 * past a tenth of a second; such a state must loop on both sides. */
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
#include "cm_rcs_dap_vectors.h"
#include "../src/agc/names.h"

extern const agc_routine_table cm_rcs_dap_basic, cm_rcs_dap_interpretive, cm_rcs_dap_hot_routines;
extern const agc_routine_table faithful_cm_rcs_dap_basic, faithful_cm_rcs_dap_interpretive,
    faithful_cm_rcs_dap_hot_routines;
bool cm_rcs_dap_readable(uint16_t key);

/* A word's place in the state: E6's words at 03000 + (the address -
 * 01400), unswitched ones at their address. */
#define AT(name) ((unsigned)K_##name >= 01400u ? (unsigned)K_##name + 01400u : (unsigned)K_##name)

#define CASES 4096u

static uint32_t seed = 1002u;
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

    for (i = 0; i < cm_rcs_dap_hot_routines.count; ++i) {
        const agc_routine_entry *hot = &cm_rcs_dap_hot_routines.entries[i];
        for (j = 0; j < cm_rcs_dap_basic.count; ++j)
            if (cm_rcs_dap_basic.entries[j].physical == hot->physical) break;
        if (j == cm_rcs_dap_basic.count || cm_rcs_dap_basic.entries[j].call != hot->call) {
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

/* A signed word of magnitude below limit. */
static uint16_t signed_below(unsigned limit)
{
    uint16_t v = (uint16_t)((rnd() >> 2) % limit);

    return pick(2u) ? v : (uint16_t)(077777u - v);
}

/* A small signed number as a word. */
static uint16_t number(int v)
{
    return (uint16_t)(v < 0 ? 077777u - (unsigned)-v : (unsigned)v);
}

/* A loop index: mostly 0 to top, now and then -0, a negative or any. */
static uint16_t index_word(unsigned top)
{
    unsigned k = pick(8u);

    return k < 6u ? (uint16_t)pick(top + 1u) : k == 6u ? sign_word() : value_word();
}

/* A 16-bit register: mostly a number sign-extended, now and then an
 * overflow. */
static uint16_t register_word(uint16_t v)
{
    v = (uint16_t)(v & 040000u ? v | 0100000u : v);
    return pick(8u) ? v : (uint16_t)(v ^ 0100000u);
}

/* The frozen cases. */
static unsigned mission_cases[CM_RCS_DAP_VECTORS], mission_count;

/* A Q to return to: often a key of the part the key is in, or 3, or a
 * word of fixed memory. */
static uint16_t return_word(uint16_t key)
{
    static const uint16_t part3[] = {02374u, 02400u, 02401u, 02407u, 02411u,
                                     02417u, 02420u, 02425u, 02443u, 02462u};
    static const uint16_t part4[] = {02465u, 02466u, 02471u, 02474u, 02476u,
                                     02477u, 02505u, 02533u, 02534u, 02544u};
    unsigned at = 02000u + key % 1024u, k = pick(8u);

    if (k < 4u) return at < 02465u ? part3[pick(10u)] : part4[pick(10u)];
    if (k == 4u) return 3u;
    if (k == 5u) return (uint16_t)(02000u + pick(06000u));
    return pick(2u) ? any_word() : rnd();
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hw, uint16_t key,
                  unsigned n)
{
    const irc_case *c = &cm_rcs_dap_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k, axis;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    memset(hw, 0, sizeof *hw);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    w[3] = 03000u; /* E6, the file's */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] >> 8);
    hw->channel31 = any_word();
    io->channel12 = any_word();
    io->channel14 = any_word();
    if (n % 4u != 0u) {
        /* The autopilot's switches and state. */
        w[AT(T5PHASE)] = sign_word();
        w[AT(HOLDFLAG)] = sign_word();
        w[AT(RCSFLAGS)] = pick(2u) ? (uint16_t)(rnd() & 040036u) : any_word();
        w[AT(IMODES33)] = any_word();
        w[AT(ATTSEC)] = pick(2u) ? (uint16_t)pick(16u) : any_word();
        hw->channel31 = (uint16_t)(pick(2u) ? rnd() & 060000u : any_word());
        io->channel12 = (uint16_t)(pick(2u) ? rnd() & 000050u : any_word());
        /* The loops' indexes and the filter's gains. */
        w[AT(SPNDX)] = index_word(2u);
        w[AT(DPNDX)] = pick(4u) ? (uint16_t)(2u * pick(3u)) : index_word(4u);
        k = pick(16u);
        w[AT(ATTKALMN)] = k < 12u    ? number((int)pick(15u) - 3)
                          : k == 12u ? number(pick(2u) ? -4 : 12) /* just past the copies */
                          : k == 13u ? number(5 - (int)(pick(2u) ? K_GAIN1 : K_GAIN2)) /* Z */
                                     : value_word();
        /* Now and then an index that makes a read's address Z itself. */
        if (pick(32u) == 0u) {
            static const unsigned single[] = {K_KMJ, K_DFT, K_AK, K_EDRIVEX};
            w[AT(SPNDX)] = number(5 - (int)single[pick(4u)]);
        }
        if (pick(32u) == 0u) {
            static const unsigned dual[] = {K_DRHO, K_ADOT + 1u, K_DELTEMPX + 1u, K_DELCDUX + 1u};
            w[AT(DPNDX)] = number(5 - (int)dual[pick(4u)]);
        }
        /* The rate filter's words, the CDU angles and their past values. */
        for (k = 0; k < 6u; ++k) {
            w[AT(DRHO) + k] = value_word();
            w[AT(ADOT) + k] = value_word();
            w[AT(DELTEMPX) + k] = value_word();
            w[AT(MERRORX) + k] = value_word();
        }
        for (axis = 0; axis < 3u; ++axis) {
            w[AT(KMJ) + axis] = value_word();
            w[AT(DFT) + axis] = value_word();
            w[AT(CDUX) + axis] = value_word();
            k = pick(4u);
            w[AT(RHO) + axis] =
                (uint16_t)(k < 2u    ? (w[AT(CDUX) + axis] + signed_below(0400u)) & 077777u
                           : k == 2u ? (w[AT(CDUX) + axis] ^ 040000u)
                                     : value_word());
        }
        for (k = 0; k < 5u; ++k) w[AT(AMGB1) + k] = value_word(); /* AMGB1, 4, 5, 7, 8 */
        /* The desired angles, their increments, and the needles. */
        for (k = 0; k < 6u; ++k) {
            w[AT(CDUXD) + k] = value_word();
            w[AT(DELCDUX) + k] = pick(2u) ? signed_below(01000u) : value_word();
        }
        for (axis = 0; axis < 3u; ++axis) {
            w[AT(THETADX) + axis] = value_word();
            k = pick(4u);
            w[AT(AK) + axis] = k < 2u    ? signed_below(03000u)
                               : k == 2u ? signed_below(010000u)
                                         : value_word();
            w[AT(EDRIVEX) + axis] = pick(2u) ? signed_below(0601u) : value_word();
            w[AT(CDUXCMD) + axis] = pick(2u) ? signed_below(01000u) : value_word();
        }
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (pick(32u) == 0u) w[k] = value_word();
        /* A (often an axis' index), L and Q. */
        k = pick(4u);
        w[0] = register_word(k < 2u    ? (uint16_t)pick(3u)
                             : k == 2u ? (uint16_t)pick(5u)
                                       : value_word());
        w[1] = register_word(value_word());
        w[2] = return_word(key);
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(32u) == 0u) {
            w[3] = pick(2u) ? any_word() : (uint16_t)(pick(8u) << 8);
            if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
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

        if (!cm_rcs_dap_readable(key)) continue;
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
    for (i = 0; i < CM_RCS_DAP_VECTORS; ++i)
        if (cm_rcs_dap_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_rcs_dap_basic, &faithful_cm_rcs_dap_basic, "basic");
    failures +=
        tables_differ(&cm_rcs_dap_interpretive, &faithful_cm_rcs_dap_interpretive, "interpretive");
    failures += tables_differ(&cm_rcs_dap_hot_routines, &faithful_cm_rcs_dap_hot_routines, "hot");
    failures += hot_differs();
    if (failures) {
        printf("cm-rcs-dap-cases: %u failures\n", failures);
        return 1;
    }
    memset(&on_alarm, 0, sizeof on_alarm);
    on_alarm.sa_handler = stop;
    sigaction(SIGALRM, &on_alarm, NULL);
    failures += keys(&cm_rcs_dap_basic, &faithful_cm_rcs_dap_basic, &runs, &written, &loops);
    printf("cm-rcs-dap-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u looping on both sides, %u failures\n",
           written, faithful_cm_rcs_dap_basic.count, CASES, runs, loops, failures);
    return failures != 0;
}
