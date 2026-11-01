/* SPDX-License-Identifier: MIT */
/* The readable Executive's seams (src/binding/binding.c gives them in the
 * programs) for the Pinball tests. Pinball runs NOVAC (DSPMM) and JOBWAKE
 * (RECAL1) within a step, which switch no job and end none: nothing to
 * tell. */
#include "../src/executive/executive.h"

void executive_exchanging_core_sets(word newjob)
{
    (void)newjob;
}

void executive_ending_job(void) {}
