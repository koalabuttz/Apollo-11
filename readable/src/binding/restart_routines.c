/* SPDX-License-Identifier: MIT */
/* The readable restarts (src/restart/) as routines of the flight program,
 * under the table names and keys of the faithful port's (c64-port/src/
 * lm_restart_routines.c and lm_restart_select.c; cm_restart_routines.c),
 * with the selection function their tests call. The readable build links
 * these in their place.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * fixed memory it reads (the restart tables, 600SECS, the words after a
 * TC NEWPHASE), and take the states the faithful routines take. */
#include <stddef.h>
#include "binding.h"
#include "../restart/restart.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_restart_routines.h"
#else
#include "cm_restart_routines.h"
#include "cm_restart_table_data.inc"
#endif

#define GOLOC_ADDRESS(n) ((unsigned)(K_GOLOC + (unsigned)(n)))

static const agc_native_program *program_now;
static bool missing_word;

/* The fixed words the restart routines read: the tables, 600SECS (a
 * LONGCALL's delta time), and the program's compiled operands. */
static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    if (physical - P_SIZETAB < restart_table_words) return restart_tables[physical - P_SIZETAB];
    if (physical - P_600SECS < 2u) return SECS600[physical - P_600SECS];
    if (!program_now ||
        !agc_site_lookup(program_now->operands, program_now->operand_count, (uint16_t)physical,
                         &value) ||
        value > MINUS_ZERO)
        missing_word = true;
    return (word)(value & MINUS_ZERO);
}

/* ---- The states the faithful routines take ---- */

/* GOPROG's banks: FB 01, EB 3. */
static bool in_goprog_banks(void)
{
    return BBANK == 02003u && EBANK == 01400u && FBANK == 02000u;
}

/* A phase RESTARTS or GETPART2 can read: positive, below 02000. */
static bool phase_as_expected(void)
{
    return TEMPPHS != 0u && TEMPPHS <= 01777u;
}

/* A variable phase's delta time, read indirectly or for a LONGCALL, is at
 * an address in erasable memory away from the central registers and GOLOC:
 * RESTARTS changes those before it reads the time, the faithful routine
 * reads them as they were, and no program stores such an address. Nor
 * does one put a LONGCALL's double time across 01377 and 01400, where the
 * faithful routine reads the second word unswitched. */
static bool time_address_as_expected(void)
{
    word phase = TEMPPHS, low = (word)(phase & 7u);
    word prdt = fetch(indexed(ADDRESS_OF(PHSPRDT1), (word)(2u * MPAC(5))));
    if (!(phase & 01400u) || (phase & 01000u) || low == 2u) return true;
    if (low < 2u && (!(prdt & SIGN_BIT) || prdt == MINUS_ZERO)) return true;
    if (low > 2u && negate(prdt) == 01377u) return false;
    return negate(prdt) >= 010u &&
           (negate(prdt) + 1u < GOLOC_ADDRESS(-1) || negate(prdt) > GOLOC_ADDRESS(2));
}

static bool takes(word entry)
{
    switch (entry) {
    case RESTARTS:
        return in_goprog_banks() && MPAC(5) < 6u && phase_as_expected() &&
               time_address_as_expected();
    case GETPART2:
        return in_goprog_banks() && TEMP2G < 12u && !(TEMP2G & 1u) && phase_as_expected();
    case PHSPART2: return in_goprog_banks() && TEMP2G < 12u && !(TEMP2G & 1u);
    case GETPART2 + 7u: return true;
    default: return false;
    }
}

/* The calls RESTARTS stores at GOLOC -1, and the returns at GOLOC +2. */
static bool goloc_minus1_as_expected(void)
{
    word to = GOLOC(-1);
    return to == FINDVAC || to == NOVAC || to == WAITLIST || to == LONGCALL;
}
static bool goloc_plus2_as_expected(void)
{
    word to = GOLOC(2);
    return to == SWRETURN || ((to == PHSPART2 || to == GETPART2) && FBANK == 02000u);
}

/* ---- Running a routine ---- */

/* The faithful routines take no channels; some callers pass none. */
static mission_io no_io;

static bool bind(const agc_native_program *program, apollo_overlay_state *state, void *device)
{
    static const agc_native_program none;
    if (!state) return false;
    if (!bind_machine(program ? program : &none, state, device ? device : &no_io)) return false;
    program_now = program;
    agc.fixed_memory = fixed_memory;
    missing_word = false;
    return true;
}

/* Run a routine; one that met a fixed word it cannot read is not taken,
 * and the state is put back as it was. */
static enum exec_status run_routine(apollo_overlay_state *state, void *device,
                                    void (*routine)(void))
{
    apollo_overlay_state before = *state;
    mission_io io_before = *(mission_io *)(device ? device : &no_io);

    routine();
    if (missing_word) {
        *state = before;
        *(mission_io *)(device ? device : &no_io) = io_before;
        return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!bind(program, state, device)) return EXEC_OUT_OF_SCOPE;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case RESTARTS: return run_routine(state, device, restarts);
    case GETPART2: return run_routine(state, device, getpart2);
    case PHSPART2: return run_routine(state, device, phspart2);
    default: return run_routine(state, device, getpart2_after_findvac);
    }
}

static enum exec_status run_goloc_minus1(const agc_native_program *program,
                                         apollo_overlay_state *state, void *device)
{
    if (!bind(program, state, device) || !goloc_minus1_as_expected()) return EXEC_OUT_OF_SCOPE;
    goloc_minus1();
    return EXEC_OK;
}

static enum exec_status run_goloc_plus2(const agc_native_program *program,
                                        apollo_overlay_state *state, void *device)
{
    if (!bind(program, state, device) || !goloc_plus2_as_expected()) return EXEC_OUT_OF_SCOPE;
    goloc_plus2();
    return EXEC_OK;
}

/* ---- The selection the faithful tests call ---- */

/* What RESTARTS chose, from where it left the machine. */
enum choice { CHOSE_JOB, CHOSE_TASK, CHOSE_LONGCALL, CHOSE_DISPLAY, CHOSE_NOTHING };

static enum choice select_from(apollo_overlay_state *state, word entry)
{
    word z;

    if (!bind(NULL, state, NULL)) return CHOSE_NOTHING;
    z = Z;
    Z = entry;
    if (run(NULL, state, NULL) != EXEC_OK) {
        Z = z;
        return CHOSE_NOTHING;
    }
    if (Z == FINDVAC) return CHOSE_DISPLAY;
    if (Z != GOLOC_ADDRESS(-1)) return CHOSE_NOTHING;
    if (GOLOC(-1) == LONGCALL) return CHOSE_LONGCALL;
    if (GOLOC(-1) == WAITLIST) return CHOSE_TASK;
    return CHOSE_JOB;
}

static const word select_entries[3] = {RESTARTS, PHSPART2, GETPART2};

/* ---- The routine tables ---- */

/* Erasable keys, as the faithful port's (agc_transfer.h). */
#define E(a) (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + (a))

#if LUMINARY099
enum lm_restart_select_exit lm_restart_select(apollo_overlay_state *state,
                                              enum lm_restart_select_entry entry)
{
    static const enum lm_restart_select_exit exits[] = {
        LM_RS_SELECTED_JOB, LM_RS_SELECTED_TASK, LM_RS_SELECTED_LONGCALL, LM_RS_SELECTED_DISPLAY,
        LM_RS_SELECT_INVALID};
    if ((unsigned)entry > 2u) return LM_RS_SELECT_INVALID;
    return exits[select_from(state, select_entries[entry])];
}

static const agc_routine_entry routines[] = {{B1(K_RESTARTS), run},
                                             {B1(K_GETPART2), run},
                                             {B1(K_GETPART2 + 7u), run},
                                             {B1(K_PHSPART2), run},
                                             {E(K_GOLOC - 1u), run_goloc_minus1},
                                             {E(K_GOLOC + 2u), run_goloc_plus2}};
const agc_routine_table lm_restart_routines = {routines, COUNT(routines)};
/* The 2CADR INITDSP after GETPART2's TC FINDVAC. */
const agc_site_operand lm_restart_operands[2] = {{B1(K_GETPART2 + 5u), KW_GETPART2P5},
                                                 {B1(K_GETPART2 + 6u), KW_GETPART2P6}};
#else
enum cm_rr_exit cm_restart_routines_select(apollo_overlay_state *state, enum cm_rr_entry entry)
{
    static const enum cm_rr_exit exits[] = {CM_RR_SEL_JOB, CM_RR_SEL_TASK, CM_RR_SEL_LONGCALL,
                                            CM_RR_SEL_DISPLAY, CM_RR_INVALID};
    if ((unsigned)entry > 2u) return CM_RR_INVALID;
    return exits[select_from(state, select_entries[entry])];
}

/* NEWPHASE: the faithful routine reads the group after the TC with
 * SUPERBNK as 0, and takes groups 1-6 and an A with no overflow. */
static enum exec_status run_newphase(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    agc_word group;
    if (!device || !bind(program, state, device)) return EXEC_OUT_OF_SCOPE;
    if (*agc.interrupts > 1u || overflow_of(A) != 0 ||
        !agc_transfer_operand(program, state, 0, Q, &group) || group < 1u || group > 6u)
        return EXEC_OUT_OF_SCOPE;
    return run_routine(state, device, newphase);
}

static const agc_routine_entry routines[] = {{B1(K_RESTARTS), run},
                                             {B1(K_GETPART2), run},
                                             {B1(K_GETPART2 + 7u), run},
                                             {B1(K_PHSPART2), run},
                                             {FF(K_NEWPHASE), run_newphase},
                                             {E(K_GOLOC - 1u), run_goloc_minus1},
                                             {E(K_GOLOC + 2u), run_goloc_plus2}};
const agc_routine_table cm_restart_routines = {routines, COUNT(routines)};
const agc_site_operand cm_restart_operands[CM_RESTART_OPERAND_COUNT] = {
    {B1(K_GETPART2 + 5u), KW_GETPART2P5}, /* 2CADR INITDSP */
    {B1(K_GETPART2 + 6u), KW_GETPART2P6},
    CM_NEWPHASE_SITES_INIT};
#endif
