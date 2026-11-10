/* SPDX-License-Identifier: MIT */
/* The readable SPS_BACK-UP_RCS_CONTROL (src/sps_backup/), the LM's, as a
 * routine of its flight program, under the table names and key of the
 * faithful port's (c64-port/src/lm_sps_backup.c): lm_sps_backup_basic, its
 * one key, SPSRCS in bank 21, with the empty lm_sps_backup_interpretive
 * beside it (the file has no interpretive code).
 *
 * Plumbing only: point the machine at the program's state, give it the
 * rope's words the file reads by address (UTIME's three, by INDEX
 * AXISCTR), and take the states the faithful routine takes: FB bank 21,
 * the bank registers agreeing, ZERO +0, NEWJOB +0, positive or -0, the
 * latches in range and every erasable word an ordinary one; beyond them,
 * AXISCTR -1, +0, -0 or +1 (the program's axes), and a return in Q from
 * 010 to 07777. A run that stops at not_modelled() leaves the state and
 * the io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "lm_sps_backup.h"
#include "../sps_backup/sps_backup.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK21         021u
#define MINUS_ONE_WORD 077776u

#define B21(z) (uint16_t)(BANK21 * 1024u + ((z) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The rope's words at UTIME -1 to +1. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical + 1u < B21(UTIME) || physical > B21(UTIME) + 1u) not_modelled();
    if (!apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool as_expected(void)
{
    word axisctr = fetch(ADDRESS_OF(AXISCTR)), newjob = fetch(ADDRESS_OF(NEWJOB));
    unsigned i;

    if (FBANK != (word)(BANK21 << 10) || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u ||
        (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (newjob > 037777u && newjob != MINUS_ZERO) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    if (axisctr > 1u && axisctr != MINUS_ONE_WORD && axisctr != MINUS_ZERO) return false;
    return Q >= 010u && Q <= 07777u;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    mission_io *io = device, kept_io;
    apollo_overlay_state before;

    if (!state || !program || !io || !bind_machine(program, state, io) || !as_expected())
        return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = rope_word;
    agc.leave = leave;

    before = *state;
    kept_io = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept_io;
        return EXEC_OUT_OF_SCOPE;
    }
    spsrcs();
    return EXEC_OK;
}

static const agc_routine_entry keys[] = {{B21(SPSRCS), run}};

const agc_routine_table lm_sps_backup_basic = {keys, COUNT(keys)};
const agc_routine_table lm_sps_backup_interpretive = {NULL, 0};

#endif
