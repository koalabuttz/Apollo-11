/* SPDX-License-Identifier: MIT */
/* Checks platform/x86/night_watchman.h, the night watchman of the x86
 * program and the host harness: when it arms and sounds, which steps are
 * stuck, and which steps count as a NEWJOB access (make x86-flight-check
 * builds and runs it). */
#include <stdio.h>
#include <string.h>
#include "night_watchman.h"

static int failures;

static void check(int ok, const char *what)
{
    if (!ok) {
        printf("FAIL %s\n", what);
        ++failures;
    }
}

static agc_word before[2048], after[2048];

/* A job active (NEWJOB +0) at GOTO, after the same as before. */
static void job_at_goto(void)
{
    memset(before, 0, sizeof before);
    before[5] = 06646u;
    before[0164] = 03770u;
    memcpy(after, before, sizeof after);
}

int main(void)
{
    night_watchman n;
    unsigned long p;
    int sounded = 0;
    memset(&n, 0, sizeof n);
    /* Armed 64 periods into each 128, sounding at the 128th. */
    for (p = 1; p < NW_ARM; ++p) sounded |= nw_period(&n, p);
    check(!sounded && !n.armed, "not armed before period 64");
    check(!nw_period(&n, NW_ARM) && n.armed, "armed at period 64");
    for (p = NW_ARM + 1u; p < NW_PERIODS; ++p) sounded |= nw_period(&n, p);
    check(!sounded, "silent until period 128");
    check(nw_period(&n, NW_PERIODS) && !n.armed, "sounds at period 128, then disarmed");
    check(!nw_period(&n, 2u * NW_PERIODS), "a disarmed watchman does not sound");

    /* Stuck: outside an interrupt, a job active, nothing changed. */
    job_at_goto();
    check(nw_stuck(before, after, 0, 0), "an unchanged job step is stuck");
    check(!nw_stuck(before, after, 1, 0), "an interrupt step is never stuck");
    check(!nw_stuck(before, after, 0, 1), "an interpretive step (it passes DANZIG) is never stuck");
    before[NW_NEWJOB] = after[NW_NEWJOB] = 077777u;
    check(!nw_stuck(before, after, 0, 0), "the idle loop (NEWJOB -0) is not stuck");
    job_at_goto();
    after[0] = 1u;
    check(!nw_stuck(before, after, 0, 0), "a step that changes a word is not stuck");

    /* NEWJOB accesses (#912 item 5). */
    memset(&n, 0, sizeof n);
    job_at_goto();
    after[0] = 1u;
    check(!nw_step(&n, before, after, 0, 0, 0, 0u),
          "a basic job step away from the Executive: none (a starved job does not reach it)");
    check(!nw_step(&n, before, after, 1, 0, 0, 0u),
          "an interrupt step that changes no core set: none");
    after[NW_PRIORITY + NW_CORE_SET * 3u] = 020u;
    check(nw_step(&n, before, after, 1, 0, 0, 0u),
          "an interrupt step that sets a PRIORITY (NOVAC): an access");
    job_at_goto();
    after[NW_NEWJOB] = 2u;
    check(nw_step(&n, before, after, 1, 0, 0, 0u), "an interrupt step that sets NEWJOB: an access");
    job_at_goto();
    before[5] = 05072u;
    memcpy(after, before, sizeof after);
    check(nw_step(&n, before, after, 1, 0, 0, 0u),
          "LM NOVAC in an interrupt, nothing changed: an access");
    check(!nw_step(&n, before, after, 1, 0, 0, 1u), "05072 on the CM is not NOVAC: none");
    before[5] = 03214u;
    before[4] = 02000u;
    memcpy(after, before, sizeof after);
    after[0] = 1u;
    check(nw_step(&n, before, after, 0, 0, 0, 0u), "LM ADVAN (bank 01): an access");
    before[4] = 04000u;
    check(!nw_step(&n, before, after, 0, 0, 0, 0u), "03214 in bank 02 is not ADVAN: none");
    /* Interpretive: round DANZIG back to NEWOPS, LOC a word on. */
    job_at_goto();
    before[5] = 06037u;
    memcpy(after, before, sizeof after);
    after[0164] = 03771u;
    check(nw_step(&n, before, after, 0, 0, 1, 1u),
          "CM interpretive step on to the next word: an access");
    after[0164] = 02100u;
    check(!nw_step(&n, before, after, 0, 0, 1, 1u), "a CALL to a new LOC (no DANZIG): none");
    after[0164] = 03771u;
    after[5] = 03140u;
    check(!nw_step(&n, before, after, 0, 0, 1, 1u),
          "an interpretive step that exits to basic code: none");
    /* A hung job: its stuck steps and the DAP's jasks are no access. */
    job_at_goto();
    check(!nw_step(&n, before, after, 0, 1, 0, 0u) && n.hung,
          "a stuck step: none, and the job is hung there");
    before[5] = 02666u;
    before[4] = 034000u;
    memcpy(after, before, sizeof after);
    after[0] = 1u;
    check(!nw_step(&n, before, after, 0, 0, 0, 0u) && n.hung, "a jask while the job is hung: none");
    job_at_goto();
    before[5] = 06070u;
    memcpy(after, before, sizeof after);
    after[0164] = 03771u;
    check(!nw_step(&n, before, after, 0, 0, 1, 0u) && n.hung,
          "an interpretive step elsewhere while the job is hung: none");
    job_at_goto();
    after[0] = 1u;
    check(!nw_step(&n, before, after, 0, 0, 0, 0u) && !n.hung,
          "a step at the hung point that is not stuck frees the job");
    {
        rupt_lock r;
        unsigned k;
        memset(&r, 0, sizeof r);
        check(rl_kind(1, 1) == RL_IN && rl_kind(0, 0) == RL_OUT &&
                  rl_kind(1, 0) == (RL_IN | RL_OUT) && rl_kind(0, 1) == (RL_IN | RL_OUT),
              "an interrupt step ending at RESUME runs both in and outside one");
        /* Armed 8 periods into each 16, checked 14 periods later. */
        for (p = 1; p < RL_ARM; ++p)
            check(!rl_period(&r, p, 0), "rupt lock silent before its first arming");
        check(!rl_period(&r, RL_ARM, 0) && r.lock && r.norupt, "rupt lock armed at period 8");
        rl_step(&r, RL_OUT);
        rl_step(&r, RL_IN);
        check(!rl_period(&r, RL_ARM + 1u, &k) && k == (RL_IN | RL_OUT) && !r.lock && !r.norupt,
              "a period counts every step since the last one");
        for (p = RL_ARM + 2u; p < RL_PERIODS + RL_CHECK; ++p)
            check(!rl_period(&r, p, 0), "rupt lock waits for its check");
        check(!rl_period(&r, RL_PERIODS + RL_CHECK, 0), "both kinds of step: no alarm");
        rl_step(&r, RL_IN);
        check(!rl_period(&r, RL_PERIODS + RL_ARM, 0) && r.lock && r.norupt,
              "rupt lock armed again");
        for (p = RL_PERIODS + RL_ARM + 1u; p < 2u * RL_PERIODS + RL_CHECK; ++p)
            (void)rl_period(&r, p, &k);
        check(k == RL_IN && r.lock && !r.norupt, "a long interrupt step runs on in each period");
        check(rl_period(&r, 2u * RL_PERIODS + RL_CHECK, 0),
              "only inside an interrupt: RUPT LOCK sounds");
        rl_step(&r, RL_OUT);
        for (p = 2u * RL_PERIODS + RL_CHECK + 1u; p < 3u * RL_PERIODS + RL_CHECK; ++p)
            (void)rl_period(&r, p, 0);
        check(!r.lock && r.norupt, "only outside: NO RUPT still armed");
        check(rl_period(&r, 3u * RL_PERIODS + RL_CHECK, 0), "no interrupt step: NO RUPT sounds");
        rl_step(&r, RL_IN | RL_OUT);
        check(!rl_period(&r, 3u * RL_PERIODS + RL_ARM, 0) &&
                  !rl_period(&r, 4u * RL_PERIODS + RL_CHECK, 0),
              "an interrupt with its RESUME in each window: silent");
    }
    printf("night watchman: %s\n", failures ? "checks failed" : "all checks pass");
    return failures != 0;
}
