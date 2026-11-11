/* SPDX-License-Identifier: MIT */
/* The seams of the Pinball tests.
 *
 * The readable Executive's (src/binding/binding.c gives them in the
 * programs): Pinball runs NOVAC (DSPMM) and JOBWAKE (RECAL1, RELDSP) within
 * a step, which switch no job and end none: nothing to tell.
 *
 * The rope, which the binding reads as data where the program gives none
 * (src/binding/pinball_routines.c, rope_word): the test runner's reader
 * (lanec_fixed_read, which c64-port/tools/lanec/native.c and
 * tests/pinball_steps.c set).
 *
 * Why a step was refused, for the step test to count by. */
#include <stddef.h>
#include "lanec.h"
#include "rope_image.h"
#include "../src/executive/executive.h"

void executive_exchanging_core_sets(word newjob)
{
    (void)newjob;
}

void executive_ending_job(void) {}

/* The state the readable routines named when they refused the last step
 * (not_modelled_state), or NULL; forgotten once read. For
 * tests/pinball_steps.c, which links both missions' routines. */
#if LUMINARY099
const char *pinball_refused_state_lm(void)
#else
const char *pinball_refused_state_cm(void)
#endif
{
    const char *state = agc.refused_state;

    agc.refused_state = NULL;
    return state;
}

bool apollo_rope_read(unsigned mission, uint16_t physical, agc_word *value)
{
    return lanec_fixed_read && lanec_fixed_read(mission ? LANEC_CM : LANEC_LM, physical, value);
}
