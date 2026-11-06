/* SPDX-License-Identifier: MIT */
/* The readable LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES (src/ephemerides/)
 * as routines of the CM's flight program, under the table names and keys
 * of the faithful port's (c64-port/src/cm_ephemeris.c):
 * cm_ephemeris_interpretive (the order pairs, all in bank 26) and
 * cm_ephemeris_basic, which is empty: the file has no basic code.
 *
 * Plumbing only. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()).
 *
 * The states the faithful routines take: FB bank 26, Z at NEWOPS, BANKSET
 * the BBANK and INTBIT15 the bank's bit 15. ZERO +0, the bank registers
 * agreeing, NEWJOB +0 or a core set, FIXLOC one of the VAC areas, the
 * interrupt and superbank latches in range and every erasable word an
 * ordinary one. A run the readable code refuses part way leaves the state
 * as it was. */
#include <stddef.h>
#include "binding.h"
#include "../ephemerides/ephemerides.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* Keys and physical addresses in bank 26. */
#define BANK26 054000u
#define B26(z) (uint16_t)(026u * 1024u + ((z) - 02000u))

#define CODE EPHEMERIDES_CODE

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    if (!bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || FBANK != BANK26 || LOC < CODE.first || LOC >= CODE.first + CODE.count ||
        BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, &CODE);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful table's order ---- */

#define K26(z) {B26(z), interpretive}
static const agc_routine_entry pairs[] = {
    K26(K_LSPOS),        K26(K_LSPOS + 02u),   K26(K_LUNPOS),       K26(K_LUNVEL),
    K26(K_SOLPOS),       K26(K_LSTIME),        K26(K_LSTIME + 03u), K26(K_LSTIME + 05u),
    K26(K_LSTIME + 07u), K26(K_LSTIME + 013u), K26(K_RES),          K26(K_RES + 02u),
    K26(K_RES + 03u),    K26(K_RES + 05u),     K26(K_RES + 07u),    K26(K_RES + 010u),
    K26(K_RES + 012u),   K26(K_RES + 013u),    K26(K_RES + 014u),   K26(K_RESA),
    K26(K_REM),          K26(K_REMA),          K26(K_REMA + 03u),   K26(K_REMA + 05u),
    K26(K_VEM),          K26(K_VEM + 03u),     K26(K_VEMA),         K26(K_VEMA + 02u),
    K26(K_VEMA + 03u),   K26(K_VEMA + 05u),    K26(K_VEMA + 010u),  K26(K_VEMA + 012u),
};
const agc_routine_table cm_ephemeris_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_ephemeris_basic = {0, 0};

#endif
