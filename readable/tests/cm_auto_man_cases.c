/* SPDX-License-Identifier: MIT */
/* The readable AUTOMATIC_MANEUVERS against the faithful port's, key by key
 * (make cm-auto-man-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable tables (src/binding/cm_auto_man_routines.c) and
 * the faithful port's under the same names with faithful_ in front (the
 * Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and the hot table's,
 * which must be the faithful one's with the same routines as the basic
 * table; every key must have a readable routine (the file is readable
 * whole). Then the runs: each key from 4,096 states, each one of
 * the port's frozen test states (c64-port/tests/cm_auto_man_vectors.h)
 * entered at the key, most stirred further: channel 31's free and hold
 * bits, channel 32's minimum impulse bits and CHANTEMP (often the same),
 * RCSFLAGS's bits, HOLDFLAG (+0, -0, 1, -1 often), the axes' MINTAU
 * indexes, the CDU angles and the angles held (often near each other),
 * the gimbal-to-body matrix, the biases and errors, the dead zone ADB (now
 * and then at the top of the range, where ADB+AF overflows), the loop
 * indexes SPNDX and DPNDX and A at JLOOP (mostly an axis', now and then an
 * overflow, or one that makes a read's address Z or lands it about the
 * fixed tables INDEX may read), the body and commanded rates, the rate
 * error EDOT and the phase plane's words (often +0 or -0, EDOT often near
 * +-WLH or +-WLMH), SLOPE, KMPAC (its upper word often +0 or -0), the
 * inertia to torque ratios J/M, T5LOC, RCSFLAGS' forced firing bits, any
 * erasable bank's words and EBANK, L and Q, the latches, or a state the
 * routines refuse (BBANK not the two banks, ZERO not +0, a word of 16
 * bits). Each runs once on each
 * side, as the program's trampoline would call the key's routine, with a
 * hardware record or (now and then) none. Where either port runs, the
 * other must too, with the same memory, io and hardware records, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run in at least 1 of 16 of each
 * key's states. A timer stops a run that goes on past a tenth of a second
 * (none should); such a state must loop on both sides. */
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
#include "cm_auto_man_vectors.h"
#include "../src/agc/names.h"

extern const agc_routine_table cm_auto_man_basic, cm_auto_man_interpretive,
    cm_auto_man_hot_routines;
extern const agc_routine_table faithful_cm_auto_man_basic, faithful_cm_auto_man_interpretive,
    faithful_cm_auto_man_hot_routines;
bool cm_auto_man_readable(uint16_t key);

/* A word's place in the state: E6's words at 03000 + (the address -
 * 01400), unswitched ones at their address. */
#define AT(name) ((unsigned)K_##name >= 01400u ? (unsigned)K_##name + 01400u : (unsigned)K_##name)

#define CASES 4096u

static uint32_t seed = 1025u;
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

    for (i = 0; i < cm_auto_man_hot_routines.count; ++i) {
        const agc_routine_entry *hot = &cm_auto_man_hot_routines.entries[i];
        for (j = 0; j < cm_auto_man_basic.count; ++j)
            if (cm_auto_man_basic.entries[j].physical == hot->physical) break;
        if (j == cm_auto_man_basic.count || cm_auto_man_basic.entries[j].call != hot->call) {
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
static unsigned mission_cases[CM_AUTO_MAN_VECTORS], mission_count;

/* A Q to return to: 3, a word of fixed memory, or any. */
static uint16_t return_word(void)
{
    unsigned k = pick(4u);

    return k == 0u    ? 3u
           : k == 1u  ? (uint16_t)(02000u + pick(06000u))
           : pick(2u) ? any_word()
                      : rnd();
}

/* A word near another: within limit either side. */
static uint16_t near(uint16_t v, unsigned limit)
{
    return (uint16_t)((v + signed_below(limit)) & 077777u);
}

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, agc_hardware *hw, uint16_t key,
                  unsigned n)
{
    const irc_case *c = &cm_auto_man_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k, axis;

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
    hw->channel32 = any_word();
    if (n % 4u != 0u) {
        /* The autopilot's switches: channel 31's bits 14 (free) and 13
         * (hold), channel 32's minimum impulse bits (MANROT) and CHANTEMP,
         * the last pass's. */
        k = pick(3u);
        hw->channel31 = (uint16_t)(k == 0u   ? rnd() & 030000u
                                   : k == 1u ? rnd() | 030000u
                                             : any_word());
        w[AT(CHANTEMP)] = any_word();
        hw->channel32 = pick(4u) ? (uint16_t)((any_word() & ~077u) | (rnd() & 077u)) : any_word();
        if (pick(4u) == 0u)
            hw->channel32 = (uint16_t)((hw->channel32 & ~077u) | (w[AT(CHANTEMP)] & 077u));
        k = pick(4u); /* bit 14, the damping flags (13-11), the forced firing bits (10-5) */
        w[AT(RCSFLAGS)] = k == 0u   ? (uint16_t)(rnd() & 020000u)
                          : k == 1u ? (uint16_t)(rnd() & 036000u)
                          : k == 2u ? (uint16_t)(rnd() & 017760u)
                                    : any_word();
        w[AT(HOLDFLAG)] = sign_word();
        /* The axes' minimum impulse indexes: mostly 0-3 (MINTAU's), now
         * and then any, or one that makes the read's address Z. */
        for (axis = 0; axis < 3u; ++axis) {
            k = pick(16u);
            w[AT(RMANNDX) + axis] = k < 12u    ? (uint16_t)pick(4u)
                                    : k == 12u ? number(pick(2u) ? -1 : 4) /* just past MINTAU */
                                    : k == 13u ? number(5 - (int)K_MINTAU) /* Z */
                                               : sign_word();
        }
        /* The CDU angles, the angles held (often near them) and the
         * gimbal-to-body matrix. */
        for (axis = 0; axis < 3u; ++axis) {
            w[AT(CDUX) + axis] = value_word();
            w[AT(THETADX) + axis] = pick(2u) ? near(w[AT(CDUX) + axis], 0400u) : value_word();
            w[AT(BIAS) + axis] = pick(2u) ? signed_below(01000u) : value_word();
            w[AT(ERRORX) + axis] = pick(2u) ? signed_below(01000u) : value_word();
        }
        for (k = 0; k < 5u; ++k) w[AT(AMGB1) + k] = value_word(); /* AMGB1, 4, 5, 7, 8 */
        /* The dead zone: often small, now and then at the top (AD FOUR
         * overflows). */
        k = pick(8u);
        w[AT(ADB)] = k < 5u    ? (uint16_t)pick(0200u)
                     : k == 5u ? (uint16_t)(037774u + pick(4u))
                               : value_word();
        /* The loop indexes. */
        w[AT(SPNDX)] = index_word(2u);
        w[AT(DPNDX)] = pick(4u) ? (uint16_t)(2u * pick(3u)) : index_word(4u);
        if (pick(32u) == 0u) w[AT(SPNDX)] = number(5 - (int)K_ERRORX); /* Z */
        if (pick(32u) == 0u) { /* about the fixed tables INDEX may read */
            static const unsigned table[] = {K_MINTAU, K_BIT13, K_OCT01400};
            w[AT(SPNDX)] = number((int)table[pick(3u)] - (int)K_ERRORX - 1 + (int)pick(5u));
        }
        if (pick(32u) == 0u) w[AT(DPNDX)] = number(5 - (int)K_WBODY - 1); /* Z */
        /* The body and commanded rates, the rate error and the phase
         * plane's words. */
        for (k = 0; k < 6u; ++k) {
            w[AT(ADOT) + k] = value_word();
            w[AT(WBODY) + k] = value_word();
        }
        /* EDOT often +-WLH {022, 06426} or +-WLMH, or near them (WTEMP's
         * +0 or -0). */
        if (pick(4u) == 0u) {
            bool wlmh = pick(2u); /* or WLMH, {012, 035415} */
            uint16_t upper = (uint16_t)((wlmh ? 011u : 021u) + pick(3u));
            uint16_t lower =
                pick(2u) ? (uint16_t)((wlmh ? 035414u : 06425u) + pick(3u)) : any_word();
            bool negative = pick(2u);

            w[AT(EDOT)] = negative ? (uint16_t)(077777u - upper) : upper;
            w[AT(EDOT) + 1u] = negative ? (uint16_t)(077777u - lower) : lower;
        } else {
            w[AT(EDOT)] = sign_word();
            w[AT(EDOT) + 1u] = sign_word();
        }
        w[AT(AERR)] = pick(2u) ? signed_below(0400u) : value_word();
        w[AT(EDOTVEL)] = sign_word();
        w[AT(EDOTVEL) + 1u] = value_word();
        w[AT(AERRVEL)] = pick(2u) ? signed_below(0400u) : value_word();
        w[AT(ADBVEL)] = value_word();
        w[AT(T5TEMP)] = pick(2u) ? (uint16_t)pick(0400u) : value_word();
        /* The rotation command, KMPAC (often an upper word +0 or -0), the
         * inertia to torque ratios, and T5LOC. */
        w[AT(KMPAC)] = sign_word();
        w[AT(KMPAC) + 1u] = value_word();
        for (axis = 0; axis < 3u; ++axis) w[AT(J_M) + axis] = value_word();
        w[AT(T5LOC)] = any_word();
        w[AT(T5LOC) + 1u] = any_word();
        w[AT(WTEMP)] = sign_word();
        w[AT(WTEMP) + 1u] = sign_word();
        k = pick(4u);
        w[AT(SLOPE)] = k < 2u    ? (uint16_t)(010000u + pick(010000u))
                       : k == 2u ? sign_word()
                                 : value_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (pick(32u) == 0u) w[k] = value_word();
        /* A (at JLOOP the axis, now and then a DOUBLE or TS that
         * overflows), L and Q. */
        k = pick(8u);
        w[0] = register_word(k < 4u    ? (uint16_t)pick(3u)
                             : k == 4u ? (uint16_t)(017777u + pick(3u))
                             : k == 5u ? number(5 - (int)K_ADOT - 1)
                             : k == 6u ? (uint16_t)((K_MINTAU - K_ADOT) / 2u - 1u + pick(4u))
                                       : value_word());
        w[1] = register_word(value_word());
        /* Now and then the words NJ22 +2 and NJ23 sum to -0 (or, all +0, to
         * +0), and EDOTVEL near +-WLMH, for NJ22 +2's WTEMP. */
        k = pick(16u);
        if (k == 0u) {
            w[0] = w[1] = 0u;
            w[AT(EDOTVEL)] = w[AT(EDOTVEL) + 1u] = w[AT(T5TEMP)] = w[AT(AERRVEL)] = 0u;
        } else if (k == 1u) {
            w[AT(AERRVEL)] = (uint16_t)(077777u - ((w[AT(T5TEMP)] + KW_WL_H_SLP) & 077777u));
        } else if (k == 2u) {
            bool negative = pick(2u);
            w[AT(EDOTVEL)] = (uint16_t)(011u + pick(3u));
            w[AT(EDOTVEL) + 1u] = pick(2u) ? (uint16_t)(035414u + pick(3u)) : any_word();
            if (negative) {
                w[AT(EDOTVEL)] = (uint16_t)(077777u - w[AT(EDOTVEL)]);
                w[AT(EDOTVEL) + 1u] = (uint16_t)(077777u - w[AT(EDOTVEL) + 1u]);
            }
        }
        w[2] = return_word();
        if (pick(16u) == 0u) io->superbank = (uint16_t)(rnd() & (pick(2u) ? 0160u : 0177u));
        if (pick(32u) == 0u) io->interrupt_enabled = (uint16_t)pick(3u);
        if (pick(32u) == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (pick(8u) == 0u) {
            w[3] = pick(4u) ? (uint16_t)(pick(8u) << 8) : any_word();
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

        if (!cm_auto_man_readable(key)) { /* the file is readable whole */
            printf("FAIL: key %u (%04o) has no readable routine\n", key, 02000u + key % 1024u);
            ++failures;
            continue;
        }
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
    for (i = 0; i < CM_AUTO_MAN_VECTORS; ++i)
        if (cm_auto_man_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_auto_man_basic, &faithful_cm_auto_man_basic, "basic");
    failures += tables_differ(&cm_auto_man_interpretive, &faithful_cm_auto_man_interpretive,
                              "interpretive");
    failures += tables_differ(&cm_auto_man_hot_routines, &faithful_cm_auto_man_hot_routines, "hot");
    failures += hot_differs();
    if (failures) {
        printf("cm-auto-man-cases: %u failures\n", failures);
        return 1;
    }
    memset(&on_alarm, 0, sizeof on_alarm);
    on_alarm.sa_handler = stop;
    sigaction(SIGALRM, &on_alarm, NULL);
    failures += keys(&cm_auto_man_basic, &faithful_cm_auto_man_basic, &runs, &written, &loops);
    printf("cm-auto-man-cases: %u of %u keys readable (all basic) from %u states each, %u run by "
           "the faithful routines, %u looping on both sides, %u failures\n",
           written, faithful_cm_auto_man_basic.count, CASES, runs, loops, failures);
    return failures != 0;
}
