/* SPDX-License-Identifier: MIT */
/* The port's frozen MYSUBS cases on the readable routines (make
 * cm-mysubs-cases).
 *
 * c64-port/tests/test_cm_mysubs.c checks the faithful SMALLMP and DPADD
 * against states of the original program (tests/cm_mysubs_vectors.h: 128
 * cases, each of the 2048 erasable words and the registers): half of them
 * taken at the routine's TC Q, half after it has returned. It calls the
 * faithful port's functions by name: cm_smallmp and cm_dpadd, which stop
 * at the TC Q, and cm_mysubs_return, the TC Q. Here those names run the
 * readable routines (src/binding/cm_mysubs_routines.c), which go on
 * through the TC Q: for the states taken at it, the run is stepped back
 * one instruction (Z at the TC Q, Q the caller's return); cm_mysubs_return
 * then runs the TC Q again, as tc_through() does: Z the return, Q 3, L
 * sign-corrected. A readable refusal that changed the state fails. (The
 * port's test of its own refusals is the faithful functions'; the
 * readable refusals are tests/cm_mysubs_cases.c's.) */
#include <stdio.h>
#include <string.h>
#define CM_MYSUBS_CHECK_ONLY
#include "test_cm_mysubs.c"
#include "cm_mysubs_routines.h"

static agc_native_program program;

/* The readable routine at Z, stopped at its TC Q (at). */
static enum cm_mysubs_status stop_at_tc_q(apollo_overlay_state *m, unsigned entry, agc_word at)
{
    apollo_overlay_state before = *m;
    agc_word q = m->words[2];

    if (cm_mysubs_routines.entries[entry].call(&program, m, 0) != EXEC_OK) {
        if (memcmp(m, &before, sizeof before)) return (enum cm_mysubs_status)2;
        return CM_MYSUBS_OUT_OF_SCOPE;
    }
    m->words[5] = at;
    m->words[2] = q;
    return CM_MYSUBS_OK;
}

enum cm_mysubs_status cm_smallmp(apollo_overlay_state *m)
{
    return stop_at_tc_q(m, 0u, 02053u);
}

enum cm_mysubs_status cm_dpadd(apollo_overlay_state *m)
{
    return stop_at_tc_q(m, 1u, 02073u);
}

/* TC Q at a routine's end: Z the return, Q 3, L sign-corrected. */
enum cm_mysubs_status cm_mysubs_return(apollo_overlay_state *m)
{
    agc_word l = m->words[1];

    if ((m->words[5] != 02053u && m->words[5] != 02073u) || m->words[2] < 02000u ||
        m->words[2] > 07777u || m->words[4] != 042000u || m->words[3] != 03000u ||
        m->words[6] != 042006u || m->words[7] != 0u)
        return CM_MYSUBS_OUT_OF_SCOPE;
    m->words[5] = m->words[2];
    m->words[2] = 3u;
    m->words[1] = agc_extend(agc_correct(l));
    return CM_MYSUBS_OK;
}

int main(void)
{
    unsigned failures = cm_mysubs_check();

    printf("cm-mysubs oracle: 128 frozen original-program states, %u failures\n", failures);
    return failures != 0;
}
