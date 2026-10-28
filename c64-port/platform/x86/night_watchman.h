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
 * The Executive runs natively, so a NEWJOB access is read from the steps
 * (#912 item 5):
 * - a step from one of the Executive's points that read NEWJOB (NOVAC,
 *   FINDVAC, SPVACIN, SPVAC, CHANG1, CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG,
 *   ENDOFJOB, DUMMYJOB and its restart entry, ADVAN, NUCHANG2, DANZIG), in
 *   or out of an interrupt;
 * - an interpretive step outside an interrupt, while no job is hung, that
 *   ends at NEWOPS with LOC 1 to 3 words on: it went round DANZIG, which
 *   reads NEWJOB (a CALL or GOTO also ends at NEWOPS, but at a new LOC
 *   and without DANZIG);
 * - any step that changes NEWJOB or a core set's PRIORITY.
 * Any other step is not counted, even outside an interrupt: a job starved
 * by interrupts (the boot rows on the wall-clock schedules, where every
 * boundary takes an interrupt and a module swap) can go 0.64 s on a few
 * basic steps that never reach the Executive, and the AGC would sound the
 * alarm then. While a job is hung, its steps are stuck and count for
 * nothing. The host harness checks the rule against the NEWJOB accesses
 * yaAGC makes over the same spans, at every check, on every schedule. */
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

/* The Executive's points that read NEWJOB (Luminary099 and Comanche055
 * EXECUTIVE.agc, INTERPRETER.agc; include/lm_exec_routines.h and
 * include/cm_exec_routines.h): NOVAC, FINDVAC, SPVACIN, SPVAC, CHANG1,
 * CHANG2, JOBSLEEP, JOBWAKE, PRIOCHNG, ENDOFJOB and DANZIG in fixed-fixed
 * memory, then DUMMYJOB, DUMMYJOB +2, ADVAN and NUCHANG2 in bank 01.
 * NEWOPS is where an interpretive step that went round DANZIG ends. */
static const agc_word nw_fixed_readers[2][11] = {
    {05072u, 05105u, 05112u, 05116u, 05122u, 05126u, 05133u, 05137u, 05146u, 05155u, 06061u},
    {05134u, 05147u, 05154u, 05160u, 05164u, 05170u, 05175u, 05201u, 05210u, 05217u, 06030u}};
static const agc_word nw_bank1_readers[2][4] = {{03206u, 03210u, 03214u, 03011u},
                                                {03223u, 03225u, 03231u, 03032u}};
static const agc_word nw_newops[2] = {06070u, 06037u};

/* Whether a step from before starts at one of the Executive's NEWJOB
 * readers (cm: 0 Luminary099, 1 Comanche055). */
static __attribute__((unused)) int nw_reader(const agc_word *before, unsigned cm)
{
    agc_word z = (agc_word)(before[5] & 07777u);
    unsigned k;
    for (k = 0; k < sizeof nw_fixed_readers[0] / sizeof nw_fixed_readers[0][0]; ++k)
        if (z == nw_fixed_readers[cm][k]) return 1;
    if (((before[4] >> 10) & 037u) == 1u)
        for (k = 0; k < sizeof nw_bank1_readers[0] / sizeof nw_bank1_readers[0][0]; ++k)
            if (z == nw_bank1_readers[cm][k]) return 1;
    return 0;
}

/* A step from before to after (erasable words), from an interpretive point
 * or not: whether it accessed NEWJOB by the rule above, with the job's
 * hung point updated. */
static __attribute__((unused)) int nw_step(night_watchman *n, const agc_word *before,
                                           const agc_word *after, int in_interrupt, int stuck,
                                           int interpretive, unsigned cm)
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
    if (stuck) return 0;
    if (nw_reader(before, cm)) return 1;
    if (!in_interrupt && !n->hung && interpretive && (after[5] & 07777u) == nw_newops[cm] &&
        (unsigned)((after[0164] - before[0164]) & 077777u) - 1u < 3u)
        return 1;
    if (before[NW_NEWJOB] != after[NW_NEWJOB]) return 1;
    for (k = 0; k < NW_CORE_SETS; ++k)
        if (before[NW_PRIORITY + NW_CORE_SET * k] != after[NW_PRIORITY + NW_CORE_SET * k]) return 1;
    return 0;
}

/* The rupt lock alarm (#989), as the native x86_64 flight program and the
 * host harness take it. On the AGC (yaAGC agc_engine.c) it arms every
 * 160 ms at scaler phase 0400 (of 0777) and checks 140 ms later, at 0300:
 * still armed, it sounds and the AGC restarts (GOJAM) with channel 77 bit 4
 * set. It is armed twice over: RUPT LOCK, cleared by any instruction that
 * ends outside an interrupt (so by each RESUME), and NO RUPT, cleared by any
 * instruction that ends inside one (no interrupt for 140 ms).
 *
 * Here the scaler is the clock period count again (32 scaler counts a
 * period): arm at RL_ARM periods into each RL_PERIODS, check at RL_CHECK.
 * A step ran inside an interrupt if it started or ended in one, and outside
 * one if it started or ended outside (an interrupt step ending at RESUME
 * does both). Clock periods are taken at the boundary after the steps that
 * ran in them, and a long step runs over several, so each period counts
 * what ran in it: the steps since the last period, and the last step,
 * still running, when none did. The host harness checks both flags
 * against yaAGC's over the same spans, at every check. Neither the harness
 * nor the x86 program restarts on it yet: they count where it would sound. */
enum { RL_PERIODS = 16u, RL_ARM = 8u, RL_CHECK = 6u, RL_IN = 1u, RL_OUT = 2u };

/* Which flag sounded (rl_period's result). */
enum { RL_LOCK = 1u, RL_NORUPT = 2u };

/* lock: nothing ran outside an interrupt since it armed; norupt: nothing
 * inside. ran: what ran since the last period (RL_IN, RL_OUT); last: what
 * the last step ran. */
typedef struct {
    uint8_t lock, norupt, ran, last;
} rupt_lock;

/* What a step ran: in_before, in_after whether it started and ended in an
 * interrupt. */
static __attribute__((unused)) unsigned rl_kind(int in_before, int in_after)
{
    return (in_before || in_after ? RL_IN : 0u) | (!in_before || !in_after ? RL_OUT : 0u);
}

/* A step that ran kind (rl_kind). */
static __attribute__((unused)) void rl_step(rupt_lock *r, unsigned kind)
{
    r->ran = (uint8_t)(r->ran | kind);
    r->last = (uint8_t)kind;
}

/* At the end of clock period number periods: the flags that sound the
 * alarm (RL_LOCK, RL_NORUPT), 0 when it is silent. *kind (if given): what
 * ran in the period. */
static __attribute__((unused)) int rl_period(rupt_lock *r, unsigned long periods, unsigned *kind)
{
    unsigned p = (unsigned)(periods % RL_PERIODS), k = r->ran ? r->ran : r->last;
    int sound = 0;
    r->ran = 0u;
    if (kind) *kind = k;
    if (k & RL_IN) r->norupt = 0u;
    if (k & RL_OUT) r->lock = 0u;
    if (p == RL_CHECK && (r->lock || r->norupt)) {
        sound = (r->lock ? (int)RL_LOCK : 0) | (r->norupt ? (int)RL_NORUPT : 0);
        r->lock = 0u;
        r->norupt = 0u;
    } else if (p == RL_ARM) {
        r->lock = 1u;
        r->norupt = 1u;
    }
    return sound;
}
#endif
