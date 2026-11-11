/* SPDX-License-Identifier: MIT */
/* What tests/kernel_side_cases.c needs of the readable kernel beside its
 * tables.
 *
 * The overlay seams (src/binding/binding.c, built with RK_SEAMS for the
 * tests): each case is one routine from a state of its own, so there is
 * no program to tell of a job switch or a job's end.
 *
 * Why a case was refused, for the test to count by. */
#include <stddef.h>
#include "resident_kernel.h"
#include "../src/agc/agc.h"

void rk_seams_exchange(agc_word newjob)
{
    (void)newjob;
}

void rk_seams_release(void) {}

/* The state the readable routine named when it refused the last case
 * (not_modelled_state), or NULL; forgotten once read. */
const char *kernel_refused_state(void)
{
    const char *state = agc.refused_state;

    agc.refused_state = NULL;
    return state;
}
