/* SPDX-License-Identifier: MIT */
/* The overlay seams of the readable Executive (src/binding/binding.c, built
 * with RK_SEAMS for the tests) for a test that runs only JOBWAKE, which
 * switches no job and ends none: nothing to tell. */
#include "resident_kernel.h"

void rk_seams_exchange(agc_word newjob)
{
    (void)newjob;
}

void rk_seams_release(void) {}
