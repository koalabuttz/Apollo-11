/* SPDX-License-Identifier: MIT */
/* The readable SPSIN and SPCOS (src/single_precision/) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/sp_routines.c): <m>_sp_routines, and the CM's copy of it
 * for the entry pack, cm_sp_hot_routines. The readable build links these
 * in their place.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * three constants it reads, and take the states the faithful routines
 * take. */
#include "binding.h"
#include "sp_routines.h"
#include "../single_precision/single_precision.h"
#include "../agc/names.h"

#define ZERO_REGISTER 7u

/* The fixed words the routines read: LIMITS -1, LIMITS and LIMITS +1
 * (POSMAX, NEG1/2 and BIT15), by INDEX. */
static word fixed_memory(unsigned physical)
{
    if (physical == FF(K_LIMITS - 1u)) return POSMAX;
    if (physical == FF(K_LIMITS)) return NEG1_2;
    if (physical == FF(K_LIMITS + 1u)) return BIT15;
    return PLUS_ZERO; /* never read */
}

/* The faithful routines take a return address in fixed memory: TC Q goes
 * back to the caller. */
static bool takes(void)
{
    return agc.memory[ZERO_REGISTER] == 0u && Q >= 02000u && Q <= 07777u;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static mission_io no_channels; /* SPSIN and SPCOS touch none: the caller may pass none */

    if (!device) device = &no_channels;
    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    agc.fixed_memory = fixed_memory;
    if (!takes()) return EXEC_OUT_OF_SCOPE;
    if (Z == SPCOS)
        spcos();
    else if (Z == SPSIN)
        spsin();
    else
        return EXEC_OUT_OF_SCOPE;
    return EXEC_OK;
}

static const agc_routine_entry routines[] = {{FF(SPCOS), run}, {FF(SPSIN), run}};
#if LUMINARY099
const agc_routine_table lm_sp_routines = {routines, COUNT(routines)};
#else
const agc_routine_table cm_sp_routines = {routines, COUNT(routines)};
const agc_routine_table cm_sp_hot_routines = {routines, COUNT(routines)};
#endif
