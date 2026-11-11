/* SPDX-License-Identifier: MIT */
/* What tests/interpreter_side_cases.c needs of the readable machine beside
 * the interpreter's table: why a case was refused, for the test to count
 * by. */
#include <stddef.h>
#include "../src/agc/agc.h"

/* The state the readable routine named when it refused the last case
 * (not_modelled_state), or NULL; forgotten once read. */
const char *interpreter_refused_state(void)
{
    const char *state = agc.refused_state;

    agc.refused_state = NULL;
    return state;
}
