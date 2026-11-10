/* SPDX-License-Identifier: MIT */
/* The readable KALMAN_FILTER (src/kalman_filter/), the LM's, as a routine
 * of its flight program, under the table names and key of the faithful
 * port's (c64-port/src/lm_kalman.c): lm_kalman_basic, its one key,
 * RATELOOP in bank 16, with the empty lm_kalman_interpretive beside it
 * (the file has no interpretive code).
 *
 * Plumbing only: point the machine at the program's state and take the
 * states the faithful routine takes: FB bank 16, EBANK 6, the bank
 * registers agreeing, ZERO +0, the latches in range and every erasable
 * word an ordinary one. The routine reads no channel and no fixed word
 * beyond its constants. A run that stops at not_modelled() leaves the
 * state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "lm_kalman.h"
#include "../kalman_filter/kalman_filter.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK16          034000u
#define EBANK6_REGISTER 03000u

#define B16(z) (uint16_t)(016u * 1024u + ((z) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static bool as_expected(void)
{
    unsigned i;

    if (FBANK != BANK16 || EBANK != EBANK6_REGISTER || agc.memory[ZERO_ADDRESS] != 0u ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    apollo_overlay_state before;

    if (!state || !device || !bind_machine(program, state, device) || !as_expected())
        return EXEC_OUT_OF_SCOPE;
    agc.leave = leave;

    before = *state;
    if (setjmp(back)) {
        *state = before;
        return EXEC_OUT_OF_SCOPE;
    }
    rateloop();
    return EXEC_OK;
}

static const agc_routine_entry routines[] = {{B16(RATELOOP), run}};
const agc_routine_table lm_kalman_basic = {routines, COUNT(routines)};
const agc_routine_table lm_kalman_interpretive = {NULL, 0};

#endif
