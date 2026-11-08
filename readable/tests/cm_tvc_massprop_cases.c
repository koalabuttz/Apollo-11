/* SPDX-License-Identifier: MIT */
/* The readable TVCMASSPROP against the faithful port's, key by key (make
 * cm-tvc-massprop-cases ROPE_LM ROPE_CM).
 *
 * Built with the readable table
 * (src/binding/cm_tvc_massprop_routines.c) and the faithful port's under
 * the same names with faithful_ in front (the Makefile renames them).
 *
 * First the keys, which must be the faithful table's, and every key must
 * run readable code. Then the runs: each key from 4,096 states, each one
 * of the port's frozen test states
 * (c64-port/tests/cm_tvc_massprop_vectors.h) entered at the key, most
 * stirred further: DAPDATR1 (LM off, LM on, the LM's ascent stage alone,
 * -0, or any word), the masses (often at an edge where DOUBLE, AD or
 * DDOUBL overflows), the breakpoints and slopes VARST0 to VARST0 +9, Q
 * (often MASSPROP or FIXCW, where the faithful port runs on after the TC
 * Q, or 3, with EBANK now and then at the edge), A and L, EBANK (often E6,
 * the file's), any erasable bank's words, the latches, or a state the
 * routines refuse (BBANK not the two banks, a word of 16 bits). Each runs
 * once on each side, as the program's trampoline would call the key's
 * routine, with a hardware record or (now and then) none. Each side runs
 * in a child process with a fiftieth of a second of its own processor time
 * to finish (a run takes microseconds): an output that overflows on every
 * pass sends FIXCW's PEGGY loop round for ever, on the AGC as in both
 * ports, and such a state must loop on both sides. Where either port
 * runs, the other must too, with the same memory and io record, word for
 * word; where the readable one refuses, it must leave everything as it
 * found it; and the faithful routine must run in at least 1 of 16 of each
 * key's states. */
#define _XOPEN_SOURCE 700 /* fork, pipe, setitimer */
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include "rope_image.h"
#include "interp_case.h"
#include "cm_native_image.h"
#include "agc_transfer.h"
#include "cm_tvc_massprop_vectors.h"

extern const agc_routine_table cm_tvc_massprop_basic, cm_tvc_massprop_interpretive;
extern const agc_routine_table faithful_cm_tvc_massprop_basic,
    faithful_cm_tvc_massprop_interpretive;
bool cm_tvc_massprop_readable(uint16_t key);

/* The addresses of the words the routines use, in E6 (03000 + the
 * address in the bank). */
#define DAPDATR1_AT 03066u /* 01466 */
#define LEMMASS_AT  03073u /* 01473 */
#define CSMMASS_AT  03074u /* 01474 */
#define VARST0_AT   03111u /* 01511 */
/* DAPDATR1's configurations: LM off, LM on, the LM's ascent stage alone. */
static const uint16_t configurations[] = {010000u, 020000u, 060000u, 077777u};
/* The words ENDMASSP's TC Q goes on at: MASSPROP, FIXCW, and 3. */
static const uint16_t returns[] = {03366u, 03422u, 3u};

#define CASES 4096u

static uint32_t seed = 951u;
static uint16_t rnd(void)
{
    seed = seed * 1103515245u + 12345u;
    return (uint16_t)(seed >> 12);
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

/* Every key runs a readable routine. */
static unsigned keys_unwritten(const agc_routine_table *table, unsigned *readable)
{
    unsigned i, failures = 0;

    for (i = 0; i < table->count; ++i) {
        uint16_t key = table->entries[i].physical;
        if (!cm_tvc_massprop_readable(key)) {
            printf("FAIL: key %u has no readable routine\n", key);
            ++failures;
        } else {
            ++*readable;
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
    return rnd() % 4u ? any_word() : edges[rnd() % (sizeof edges / sizeof edges[0])];
}

/* The frozen cases. */
static unsigned mission_cases[CM_TVC_MASSPROP_VECTORS], mission_count;

/* One state at the key: a frozen case's, and for most n more. */
static void state(apollo_overlay_state *s, cm_native_io *io, uint16_t key, unsigned n)
{
    const irc_case *c = &cm_tvc_massprop_cases[mission_cases[(n * 7u + rnd()) % mission_count]];
    uint16_t *w = s->words, eb;
    unsigned k;

    memset(s, 0, sizeof *s);
    memset(io, 0, sizeof *io);
    irc_build(s, c);
    io->superbank = c->superbank;
    io->interrupt_enabled = c->interrupt_enabled;
    if (rnd() % 4u) w[3] = rnd() % 4u ? 03000u : 03400u; /* E6, the file's, or E7 */
    w[4] = (uint16_t)((key / 1024u) << 10);
    w[5] = (uint16_t)(02000u + key % 1024u);
    eb = (uint16_t)(w[3] >> 8);
    if (n % 4u != 0u) {
        /* The configuration, masses, breakpoints and slopes. */
        w[DAPDATR1_AT] = rnd() % 4u ? configurations[rnd() % 4u] : any_word();
        w[LEMMASS_AT] = value_word();
        w[CSMMASS_AT] = value_word();
        /* the LM's ascent stage alone, its mass at an edge: a breakpoint
         * that overflows, leaving PHI333 -1 (FIXCW without DXTEST) */
        if (rnd() % 8u == 0u) {
            w[DAPDATR1_AT] = 060000u;
            w[LEMMASS_AT] = rnd() % 2u ? 040000u : (uint16_t)(040000u + rnd() % 010000u);
        }
        for (k = 0; k < 10u; ++k)
            if (rnd() % 8u == 0u) w[VARST0_AT + k] = value_word();
        /* Any of the erasable banks' words. */
        for (k = 01000u; k < 04000u; ++k)
            if (rnd() % 8u == 0u) w[k] = value_word();
        /* A, L and Q. */
        w[0] = rnd() % 2u ? any_word() : rnd();
        w[1] = rnd() % 2u ? any_word() : rnd();
        k = rnd() % 8u;
        if (k < 3u)
            w[2] = returns[rnd() % 3u];
        else if (k < 6u)
            w[2] = (uint16_t)(010u + rnd() % 07770u);
        else if (k == 6u)
            w[2] = rnd() % 2u ? any_word() : rnd();
        if (rnd() % 8u == 0u) {
            k = 010u + rnd() % 03770u;
            w[k] = any_word();
        }
        if (rnd() % 16u == 0u) io->superbank = (uint16_t)(rnd() & (rnd() % 2u ? 0160u : 0177u));
        if (rnd() % 32u == 0u) io->interrupt_enabled = (uint16_t)(rnd() % 3u);
        if (rnd() % 32u == 0u) w[4] = (uint16_t)(rnd() & 076000u);
        if (rnd() % 64u == 0u) w[3] = any_word();
        if (rnd() % 8u == 0u) w[3] = (uint16_t)((rnd() % 8u) << 8); /* any of the eight */
        /* TC Q with Q 3 runs EBANK's word as TC <EBANK>: at the edge */
        if (w[2] == 3u && rnd() % 4u == 0u) w[3] = rnd() % 2u ? 02000u : 01400u;
        if (w[3] <= 03400u && !(w[3] & 0377u)) eb = (uint16_t)(w[3] >> 8);
    }
    w[6] = (uint16_t)((w[4] & 076000u) | eb);
    /* Now and then a state no program has: BBANK not the two banks, or a
     * word of 16 bits. */
    if (n % 4u != 0u && rnd() % 64u == 0u) w[6] = any_word();
    if (n % 4u != 0u && rnd() % 64u == 0u) w[3u + rnd() % 03775u] |= 0100000u;
    if (n % 4u != 0u && rnd() % 64u == 0u) w[2] |= 0100000u; /* Q, which may hold 16 bits */
}

/* What a faithful run leaves: its status, memory, io and hardware records. */
struct faithful_run {
    enum exec_status status;
    apollo_overlay_state state;
    cm_native_io io;
    agc_hardware hardware;
};

/* A child process running the routine of entry (either side's) from
 * run's records (with the hardware record or none), with a fiftieth of a
 * second of its own processor time to finish (a run takes microseconds;
 * the timer counts no other process's time, so the machine's load does
 * not move the cut). Both sides run at once, each in its own child. */
struct child {
    pid_t pid;
    int from; /* the read end of the pipe the child's records come back on */
};

static struct child start_child(const agc_routine_entry *entry, bool with_hardware,
                                struct faithful_run *run)
{
    struct child c = {-1, -1};
    int ends[2];

    fflush(stdout);
    if (pipe(ends)) return c;
    c.pid = fork();
    if (c.pid < 0) {
        close(ends[0]);
        close(ends[1]);
        return c;
    }
    if (c.pid == 0) {
        struct itimerval limit = {{0, 0}, {0, 20000}};
        agc_native_program program;
        const char *out = (const char *)run;
        size_t put = 0;
        ssize_t n;

        close(ends[0]);
        memset(&program, 0, sizeof program);
        if (with_hardware) program.hardware = &run->hardware;
        setitimer(ITIMER_VIRTUAL, &limit, NULL);
        run->status = entry->call(&program, &run->state, &run->io);
        while (put < sizeof *run && (n = write(ends[1], out + put, sizeof *run - put)) > 0)
            put += (size_t)n;
        _exit(0);
    }
    close(ends[1]);
    c.from = ends[0];
    return c;
}

/* What the child leaves back in run: false where it does not finish. */
static bool child_finished(struct child c, struct faithful_run *run)
{
    size_t got = 0;
    ssize_t n;
    int status;

    if (c.pid < 0) return false;
    while (got < sizeof *run && (n = read(c.from, (char *)run + got, sizeof *run - got)) > 0)
        got += (size_t)n;
    close(c.from);
    waitpid(c.pid, &status, 0);
    return got == sizeof *run;
}

/* States the AGC never finishes in, on both sides. */
static unsigned unfinished;

static unsigned keys(const agc_routine_table *readable, const agc_routine_table *faithful,
                     unsigned *runs)
{
    unsigned i, n, failures = 0;

    for (i = 0; i < faithful->count; ++i) {
        uint16_t key = faithful->entries[i].physical;
        unsigned ran = 0, bad = 0;

        for (n = 0; n < CASES; ++n) {
            static struct faithful_run f, r;
            apollo_overlay_state start;
            cm_native_io io;
            enum exec_status sf, sr;
            const char *why = 0;
            bool with_hardware = n % 64u != 63u; /* now and then none */
            bool f_ends, r_ends;

            state(&start, &io, key, n);
            f.state = r.state = start;
            f.io = r.io = io;
            memset(&f.hardware, 0, sizeof f.hardware);
            r.hardware = f.hardware;
            {
                struct child fc = start_child(&faithful->entries[i], with_hardware, &f);
                struct child rc = start_child(&readable->entries[i], with_hardware, &r);
                f_ends = child_finished(fc, &f);
                r_ends = child_finished(rc, &r);
            }
            if (!f_ends && !r_ends) {
                ++unfinished;
                continue;
            }
            if (f_ends != r_ends) {
                if (++bad <= 3)
                    printf("FAIL key %u case %u: %s\n", key, n,
                           f_ends ? "readable never finishes, faithful does"
                                  : "faithful never finishes, readable does");
                continue;
            }
            sf = f.status;
            sr = r.status;
            ran += sf == EXEC_OK;
            if (sr != EXEC_OK &&
                (memcmp(&r.state, &start, sizeof start) || memcmp(&r.io, &io, sizeof io)))
                why = "readable refusal changed the state";
            else if (sf == EXEC_OK && sr != EXEC_OK)
                why = "readable refused, faithful ran";
            else if (sf != EXEC_OK && sr == EXEC_OK)
                why = "readable ran, faithful refused";
            else if (sf == EXEC_OK && memcmp(&f.state, &r.state, sizeof r.state))
                why = "different memory";
            else if (sf == EXEC_OK && (memcmp(&f.io, &r.io, sizeof r.io) ||
                                       memcmp(&f.hardware, &r.hardware, sizeof r.hardware)))
                why = "different io record";
            if (why && ++bad <= 3) {
                unsigned w;
                printf("FAIL key %u case %u: %s\n", key, n, why);
                for (w = 0; w < APOLLO_SHARED_WORDS; ++w)
                    if (f.state.words[w] != r.state.words[w])
                        printf("  word %04o: %06o, faithful %06o (start %06o)\n", w,
                               r.state.words[w], f.state.words[w], start.words[w]);
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
    unsigned readable_keys = 0;
    unsigned i, failures = 0, runs = 0;

    if (argc < 3 || !apollo_rope_load(0, argv[1]) || !apollo_rope_load(1, argv[2])) {
        printf("usage: %s ROPE_LM ROPE_CM\n", argv[0]);
        return 2;
    }
    for (i = 0; i < CM_TVC_MASSPROP_VECTORS; ++i)
        if (cm_tvc_massprop_cases[i].cm == 1u) mission_cases[mission_count++] = i;
    failures += tables_differ(&cm_tvc_massprop_basic, &faithful_cm_tvc_massprop_basic, "basic");
    failures += tables_differ(&cm_tvc_massprop_interpretive, &faithful_cm_tvc_massprop_interpretive,
                              "interpretive");
    failures += keys_unwritten(&faithful_cm_tvc_massprop_basic, &readable_keys);
    if (failures) {
        printf("cm-tvc-massprop-cases: %u failures\n", failures);
        return 1;
    }
    failures += keys(&cm_tvc_massprop_basic, &faithful_cm_tvc_massprop_basic, &runs);
    printf(
        "cm-tvc-massprop-cases: %u of %u keys readable (all basic) from %u states each, %u looping "
        "on both sides, %u run by the faithful routines, %u failures\n",
        readable_keys, faithful_cm_tvc_massprop_basic.count, CASES, unfinished, runs, failures);
    return failures != 0;
}
