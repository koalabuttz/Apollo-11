/* SPDX-License-Identifier: MIT */
/* The readable phase table maintenance (src/phase_table/) as routines of
 * the flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_phase_table.c and cm_phase_table.c). The
 * readable build links these in their place.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * words after each TC (the program's compiled operands), and take the
 * states the faithful routines take. */
#include <string.h>
#include "binding.h"
#include "../phase_table/phase_table.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_phase_table.h"
#include "lm_phase_sites.inc"
#else
#include "cm_phase_table.h"
#include "cm_phase_sites.inc"
#endif

#define ZERO_REGISTER 7u

/* Bank 10, where SETUPDSP, PHSCHNG2 and CON2 are. */
#define B10(z)  (uint16_t)(010u * 1024u + ((z) - 02000u))
#define FBANK10 020000u

static const agc_native_program *program_now;
static bool missing_operand;

/* The words after a TC in fixed memory are the program's compiled
 * operands; a routine that reads any other fixed word is not taken. */
static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    if (!agc_site_lookup(program_now->operands, program_now->operand_count, (uint16_t)physical,
                         &value) ||
        value > MINUS_ZERO)
        missing_operand = true;
    return (word)(value & MINUS_ZERO);
}

/* ---- The states the faithful routines take ---- */

static bool machine_as_expected(void)
{
    return agc.memory[ZERO_REGISTER] == 0u && (*agc.superbank & ~0160u) == 0u &&
           *agc.interrupts <= 1u;
}

/* The word after the TC can be read: Q is an address. */
static bool q_is_a_return(void)
{
    return Q >= 0100u && Q <= 07777u;
}

static bool takes(word entry, bool in_bank10)
{
    if (!machine_as_expected()) return false;
    if (in_bank10) return FBANK == FBANK10;
    if (entry == K_NEWMODEA) return overflow_of(A) == 0; /* TS MODREG would skip */
#if LUMINARY099
    if (entry == K_PHSCHNGA) return overflow_of(A) == 0; /* TS TEMPSW would skip */
#endif
    if (entry == K_MMDSPLAY) return true;
    return q_is_a_return();
}

/* ---- Running a routine ---- */

/* Run one entry. A routine that met a fixed word with no operand is not
 * taken, and the program's state is put back as it was. */
static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, void (*routine)(void), word entry, bool in_bank10)
{
    apollo_overlay_state before;
    mission_io io_before;

    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    if (!takes(entry, in_bank10)) return EXEC_OUT_OF_SCOPE;
    program_now = program;
    agc.fixed_memory = fixed_memory;
    missing_operand = false;
    before = *state;
    io_before = *(mission_io *)device;
    routine();
    if (missing_operand) {
        *state = before;
        *(mission_io *)device = io_before;
        return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

#define ENTRY(name, routine, in_bank10)                                                            \
    static enum exec_status run_##routine(const agc_native_program *program,                       \
                                          apollo_overlay_state *state, void *device)               \
    {                                                                                              \
        return run(program, state, device, routine, (word)(name), in_bank10);                      \
    }
ENTRY(K_NEWMODEX, newmodex, false)
ENTRY(K_NEWMODEA, newmodea, false)
ENTRY(K_MMDSPLAY, mmdsplay, false)
ENTRY(K_CHECKMM, checkmm, false)
ENTRY(K_2PHSCHNG, two_phschng, false)
ENTRY(K_PHASCHNG, phaschng, false)
#if LUMINARY099
ENTRY(K_PHSCHNGA, phschnga, false)
#endif
ENTRY(K_SETUPDSP, setupdsp, true)
ENTRY(K_SETUPDSP + 6u, setupdsp_after_novac, true)
ENTRY(K_PHSCHNG2, phschng2, true)
ENTRY(K_CON2, con2, true)

/* ---- The routine table ---- */

static const agc_routine_entry routines[] = {
    {FF(K_NEWMODEX), run_newmodex},    {FF(K_NEWMODEA), run_newmodea},
    {FF(K_MMDSPLAY), run_mmdsplay},    {FF(K_CHECKMM), run_checkmm},
    {FF(K_2PHSCHNG), run_two_phschng}, {FF(K_PHASCHNG), run_phaschng},
#if LUMINARY099
    {FF(K_PHSCHNGA), run_phschnga},
#endif
    {B10(K_SETUPDSP), run_setupdsp},   {B10(K_SETUPDSP + 6u), run_setupdsp_after_novac},
    {B10(K_PHSCHNG2), run_phschng2},   {B10(K_CON2), run_con2}};

#if LUMINARY099
const agc_routine_table lm_phase_table_routines = {routines, COUNT(routines)};
const agc_site_operand *const lm_phase_table_operands = lm_phase_sites;
#else
const agc_routine_table cm_phase_table_routines = {routines, COUNT(routines)};
const agc_site_operand *const cm_phase_table_operands = cm_phase_sites;
#endif
