/* SPDX-License-Identifier: MIT */
#ifndef NIGHT_WATCHMAN_H
#define NIGHT_WATCHMAN_H
/* The night watchman (docs/FLIGHT-RESTART.md, The night watchman), as the
 * native x86_64 flight program and the host harness (tools/flight_host_run.c)
 * take it. On the AGC (yaAGC agc_engine.c) the alarm arms every 1.28 s at
 * scaler phase 04000; any access to NEWJOB (067) disarms it; still armed
 * 0.64 s later, at phase 00000, it sounds and the AGC restarts (GOJAM). A
 * job looping without the Executive (an erasable CADR cycle in the
 * interpreter's GOTO, which loops in GOTOERS) restarts the AGC this way.
 *
 * Here the scaler is the clock period count: arm at NW_ARM periods into
 * each NW_PERIODS, sound at 0. A step from a basic point outside an
 * interrupt, with a job active (NEWJOB not -0, so not the idle loop's CCS
 * NEWJOB), that leaves every erasable word as it found it is stuck (an
 * interpretive step ends at DANZIG or the next fetch, so its loop reads
 * NEWJOB): it would run again and
 * again from the same state, as the AGC would, until an interrupt changes
 * it; the job is hung at that point until a step there is not stuck.
 *
 * The Executive runs natively, so a NEWJOB access is read from the steps:
 * - a step outside an interrupt while no job is hung: the job passes
 *   DANZIG, ADVAN or CCSNEWJOB in time;
 * - any step that changes NEWJOB or a core set's PRIORITY: NOVAC, FINDVAC
 *   and JOBWAKE read NEWJOB and change one.
 * While a job is hung, the steps outside an interrupt at other points are
 * the LM DAP's jasks (an interrupt RESUMEs into DAP code, which returns to
 * the job's point): they do not read NEWJOB. The host harness checks the
 * rule against the NEWJOB accesses yaAGC makes over the same spans. */
#include <string.h>
#include "agc_word.h"

enum {
    NW_PERIODS = 128u,
    NW_ARM = 64u,
    NW_NEWJOB = 067u,
    NW_PRIORITY = 0167u,
    NW_CORE_SETS = 7u,
    NW_CORE_SET = 12u
};

/* hung: a job is hung at Z z, FB fb, LOC loc. */
typedef struct {
    uint8_t armed, hung;
    agc_word z, fb, loc;
} night_watchman;

/* At the end of clock period number periods: 1 when the alarm sounds. */
static __attribute__((unused)) int nw_period(night_watchman *n, unsigned long periods)
{
    unsigned p = (unsigned)(periods % NW_PERIODS);
    if (p == NW_ARM)
        n->armed = 1u;
    else if (p == 0u && n->armed) {
        n->armed = 0u;
        return 1;
    }
    return 0;
}

/* Whether a step from before to after (erasable words) is stuck;
 * interpretive: it started at an interpretive point. */
static __attribute__((unused)) int nw_stuck(const agc_word *before, const agc_word *after,
                                            int in_interrupt, int interpretive)
{
    return !in_interrupt && !interpretive && (before[NW_NEWJOB] & 077777u) != 077777u &&
           !memcmp(before, after, 2048u * sizeof *before);
}

/* A step from before to after: whether it accessed NEWJOB by the rule
 * above, with the job's hung point updated. */
static __attribute__((unused)) int nw_step(night_watchman *n, const agc_word *before,
                                           const agc_word *after, int in_interrupt, int stuck)
{
    unsigned k;
    if (!in_interrupt) {
        int here = n->hung && before[5] == n->z && before[4] == n->fb && before[0164] == n->loc;
        if (stuck) {
            n->hung = 1u;
            n->z = before[5];
            n->fb = before[4];
            n->loc = before[0164];
        } else if (here)
            n->hung = 0u;
    }
    if (!in_interrupt && !stuck && !n->hung) return 1;
    if (before[NW_NEWJOB] != after[NW_NEWJOB]) return 1;
    for (k = 0; k < NW_CORE_SETS; ++k)
        if (before[NW_PRIORITY + NW_CORE_SET * k] != after[NW_PRIORITY + NW_CORE_SET * k]) return 1;
    return 0;
}
#endif
