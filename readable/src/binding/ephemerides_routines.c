/* SPDX-License-Identifier: MIT */
/* The readable LUNAR_AND_SOLAR_EPHEMERIDES_SUBROUTINES (src/ephemerides/)
 * as routines of each mission's flight program, under the table names and
 * keys of the faithful port's.
 *
 * Plumbing only. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()).
 *
 * The LM's (c64-port/src/lm_ephemeris_routine_table.c, lm_ephemeris.c):
 * lm_ephemeris_routines, ten keys in bank 15, each a step of the faithful
 * port's chain: from its key, passes until the next key the faithful step
 * ends at (POSITA's loop through POSITB, POSITD's through POSITE) or out
 * of the file, to the caller. The two entries, LSPOS and LUNVEL, take the
 * states the faithful entry admits: Z at NEWOPS in bank 15, BANKSET the
 * BBANK, the bank registers agreeing, ZERO, EDOP, NEWJOB, INTBIT15 and
 * MODE +0, FIXLOC one of the VAC areas, the latches in range, Q and every
 * erasable word above it an ordinary one, and the return (QPRET) one of
 * the program's CALLs'. The steps after them take what the entry left.
 *
 * The CM's (c64-port/src/cm_ephemeris.c): cm_ephemeris_interpretive (the
 * order pairs, all in bank 26) and cm_ephemeris_basic, which is empty: the
 * file has no basic code. They take the states the faithful routines
 * take: FB bank 26, Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's
 * bit 15. ZERO +0, the bank registers agreeing, NEWJOB +0 or a core set,
 * FIXLOC one of the VAC areas, the interrupt and superbank latches in
 * range and every erasable word an ordinary one.
 *
 * In both, a run the readable code refuses part way leaves the state as it
 * was. */
#include <stddef.h>
#include "binding.h"
#include "../ephemerides/ephemerides.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys and physical addresses in bank 15. */
#define BANK15 032000u
#define B15(z) (uint16_t)(015u * 1024u + ((z) - 02000u))

#define CODE EPHEMERIDES_CODE

/* The returns of the program's CALLs of LSPOS, as QPRET holds them: in
 * ORBITAL_INTEGRATION (11,2563, 11,3273 and 11,3373), P51-P53 (14,2512)
 * and the S-band antenna's (bank 42, 32 under the superbank bit, 3627);
 * and of its one CALL of LUNVEL (ORBITAL_INTEGRATION, 11,3417). */
static const word lspos_returns[] = {022563u, 023273u, 023373u, 030512u, 065627u};
#define LUNVEL_RETURN 023417u

/* The entry states the faithful entry admits (lm_ephemeris_admitted()). */
static bool admitted(void)
{
    word qpret;
    unsigned i;

    if (Z != NEWOPS || FBANK != BANK15 || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)(BANK15 | (EBANK >> 8)) || BANKSET != BBANK)
        return false;
    if (agc.memory[ZERO_ADDRESS] != 0u || EDOP != 0u || agc.memory[ADDRESS_OF(NEWJOB)] != 0u ||
        INTBIT15 != 0u || MODE != 0u)
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 2u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    qpret = agc.memory[(FIXLOC + K_QPRET) & 03777u];
    if (LOC == K_LUNVEL) return qpret == LUNVEL_RETURN;
    for (i = 0; i < COUNT(lspos_returns); ++i)
        if (qpret == lspos_returns[i]) return true;
    return false;
}

/* Whether a step from key stops at LOC: the next key the faithful step
 * ends at. MXV KONMAT's step (POSITF +021, the sun's) goes on to the
 * caller. */
static bool stops_at(word key, word loc)
{
    switch (key) {
    case K_LSPOS: return loc == K_POSITA;
    case K_POSITA: return loc == K_POSITD;
    case K_POSITD: return loc == K_POSITA || loc == K_POSITF;
    case K_POSITF: return loc == K_POSITF + 07u;         /* COS VDEF (the moon's) */
    case K_POSITF + 07u: return loc == K_POSITF + 010u;  /* MXV UNIT */
    case K_POSITF + 010u: return loc == K_POSITF + 013u; /* DLOAD PDDL */
    case K_POSITF + 013u: return loc == K_POSITF + 020u; /* COS VDEF (the sun's) */
    case K_POSITF + 020u: return loc == K_POSITF + 021u; /* MXV UNIT */
    default: return false;
    }
}

/* Passes from the key until a stop, or out of the file: to the caller
 * after an RVQ, or out of the interpreter. */
static enum exec_status step(const agc_native_program *program, apollo_overlay_state *state,
                             void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    word key;
    unsigned passes;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    if (!bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    key = LOC;
    if ((key == K_LSPOS || key == K_LUNVEL) && !admitted()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    /* (POSITA's and POSITD's loops end in a few passes; the bound only
     * stops a run without end) */
    for (passes = 0; passes < 64u; ++passes) {
        if (interpret_listing(program, state, io, &CODE) != EXEC_OK) break;
        bind_machine(program, state, io);
        if (Z != NEWOPS || FBANK != BANK15 || LOC < CODE.first || LOC >= CODE.first + CODE.count ||
            stops_at(key, LOC))
            return EXEC_OK;
    }
    *state = before;
    *io = kept;
    return EXEC_OUT_OF_SCOPE;
}

/* ---- The keys, in the faithful table's order ---- */

#define K15(z) {B15(z), step}
static const agc_routine_entry keys[] = {
    K15(K_LSPOS),         K15(K_POSITA),        K15(K_POSITD),        K15(K_POSITF),
    K15(K_POSITF + 07u),  K15(K_POSITF + 010u), K15(K_POSITF + 013u), K15(K_POSITF + 020u),
    K15(K_POSITF + 021u), K15(K_LUNVEL),
};
const agc_routine_table lm_ephemeris_routines = {keys, COUNT(keys)};

#else

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
