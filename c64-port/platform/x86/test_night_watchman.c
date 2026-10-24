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

    /* NEWJOB accesses. */
    memset(&n, 0, sizeof n);
    job_at_goto();
    after[0] = 1u;
    check(nw_step(&n, before, after, 0, 0), "a job step, no job hung: an access");
    check(!nw_step(&n, before, after, 1, 0), "an interrupt step that changes no core set: none");
    after[NW_PRIORITY + NW_CORE_SET * 3u] = 020u;
    check(nw_step(&n, before, after, 1, 0),
          "an interrupt step that sets a PRIORITY (NOVAC): an access");
    job_at_goto();
    after[NW_NEWJOB] = 2u;
    check(nw_step(&n, before, after, 1, 0), "an interrupt step that sets NEWJOB: an access");
    /* A hung job: its stuck steps and the DAP's jasks are no access. */
    job_at_goto();
    check(!nw_step(&n, before, after, 0, 1) && n.hung,
          "a stuck step: none, and the job is hung there");
    before[5] = 02666u;
    before[4] = 034000u;
    memcpy(after, before, sizeof after);
    after[0] = 1u;
    check(!nw_step(&n, before, after, 0, 0) && n.hung, "a jask while the job is hung: none");
    job_at_goto();
    after[0] = 1u;
    check(nw_step(&n, before, after, 0, 0) && !n.hung,
          "a step at the hung point that is not stuck frees the job: an access");
    printf("night watchman: %s\n", failures ? "checks failed" : "all checks pass");
    return failures != 0;
}
