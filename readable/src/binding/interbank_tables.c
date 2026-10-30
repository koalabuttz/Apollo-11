/* SPDX-License-Identifier: MIT */
/* The readable inter-bank routines (src/interbank/) as routines of the
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_interbank_routines.c, lm_interbank_idle.c and
 * lm_interbank_dap.c; cm_interbank_routines.c, cm_interbank_idle.c and
 * cm_interbank_postjump.c). The readable build links these in their place.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * fixed memory the routines read, and take the states the faithful
 * routines take. */
#include "binding.h"
#include "../interbank/interbank.h"
#include "../agc/names.h"

#define ZERO_REGISTER 7u

static const agc_native_program *program_now;

/* The one word of fixed memory a routine reads (the CADR after its TC, or
 * SUPDACAL's data), found before the routine runs. */
static struct {
    unsigned physical;
    word value;
} fixed_word;

static word fixed_memory(unsigned physical)
{
    agc_word value = 0;

    if (physical == fixed_word.physical) return fixed_word.value;
    if (!agc_site_lookup(program_now->operands, program_now->operand_count, (uint16_t)physical,
                         &value) &&
        program_now->fixed_data)
        program_now->fixed_data(program_now, (uint16_t)physical, &value);
    return value;
}

/* ---- The states the faithful routines take ---- */

/* The bank registers agree, ZERO is +0, the superbank is one of the
 * eight, and interrupts are on or off. */
static bool banks_as_expected(void)
{
    return agc.memory[ZERO_REGISTER] == 0u && (EBANK & ~03400u) == 0u && (FBANK & ~076000u) == 0u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && (*agc.superbank & ~0160u) == 0u &&
           *agc.interrupts <= 1u;
}

/* Q holds the return address of a TC from erasable or fixed memory. */
static bool q_is_a_return(void)
{
    return Q >= 0100u && Q <= 07775u;
}

/* The CADR after the TC can be read: an erasable word above the central
 * registers, or the caller's compiled operand. */
static bool cadr_at_q(void)
{
    agc_word cadr;

    if (Q < 02000u) return erasable_index(Q) >= 0100u;
    fixed_word.physical = fixed_physical(Q);
    if (!agc_site_lookup(program_now->operands, program_now->operand_count,
                         (uint16_t)fixed_word.physical, &cadr) ||
        cadr > MINUS_ZERO)
        return false;
    fixed_word.value = cadr;
    return true;
}

/* A return through BUF2 or RUPTREG3 runs it as a TC: it must hold one. */
static bool return_word(unsigned buffer)
{
    word to = (word)agc.memory[buffer];
    return to >= 0100u && to <= 07777u;
}

/* SUPDACAL's word of data can be read from the rope. */
static bool data_for_supdacal(void)
{
    word cadr = overflow_corrected(A);
    unsigned bank = overflow_of(A) ? FBANK >> 10 : cadr >> 10;
    agc_word data;

    if (bank >= 030u && (L & 0100u)) bank += 010u; /* the superbits in L */
    fixed_word.physical = bank * 02000u + (cadr & 01777u);
    if (!program_now->fixed_data ||
        !program_now->fixed_data(program_now, (uint16_t)fixed_word.physical, &data) ||
        data > MINUS_ZERO)
        return false;
    fixed_word.value = data;
    return true;
}

static bool takes(word entry)
{
    if (!banks_as_expected()) return false;
    switch (entry) {
    case K_BANKCALL:
    case K_IBNKCALL:
    case K_POSTJUMP:
    case K_USPRCADR: return q_is_a_return() && cadr_at_q();
    case K_SWCALL:
    case K_ISWCALL:
    case K_MAKECADR:
    case K_SUPERSW: return q_is_a_return();
    case K_SWRETURN: return return_word(K_BUF2);
    case K_ISWRETRN: return return_word(K_RUPTREG3);
    case K_BANKJUMP: return Q <= 07775u;
    case K_QP10000: return q_is_a_return() && Q <= 01777u;
    case K_SUPDACAL: return q_is_a_return() && data_for_supdacal();
    default: return false;
    }
}

/* ---- Running a routine ---- */

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    word entry;

    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    program_now = program;
    agc.fixed_memory = fixed_memory;
    fixed_word.physical = ~0u;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;
    switch (entry) {
    case K_BANKCALL: bankcall(); break;
    case K_SWCALL: swcall(); break;
    case K_SWRETURN: swreturn(); break;
    case K_POSTJUMP: postjump(); break;
    case K_BANKJUMP: bankjump(); break;
    case K_QP10000: q_plus_10000(); break;
    case K_MAKECADR: makecadr(); break;
    case K_SUPDACAL: supdacal(); break;
    case K_IBNKCALL: ibnkcall(); break;
    case K_ISWCALL: iswcall(); break;
    case K_ISWRETRN: iswretrn(); break;
    case K_USPRCADR: usprcadr(); break;
    case K_SUPERSW: supersw(); break;
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* SWCALL -1 is only BANKCALL's INCR Q: the faithful port takes it in any
 * bank state, with Z wherever the TC put it. */
static enum exec_status run_swcall_minus1(const agc_native_program *program,
                                          apollo_overlay_state *state, void *device)
{
    if (!bind_machine(program, state, device)) return EXEC_OUT_OF_SCOPE;
    if (agc.memory[ZERO_REGISTER] != 0u || Q > 07776u) return EXEC_OUT_OF_SCOPE;
    swcall_minus1();
    return EXEC_OK;
}

/* ---- The routine tables ---- */

static const agc_routine_entry routines[] = {
    {FF(K_BANKCALL), run}, {FF(K_SWCALL - 1u), run_swcall_minus1},
    {FF(K_SWCALL), run},   {FF(K_SWRETURN), run},
    {FF(K_POSTJUMP), run}, {FF(K_BANKJUMP), run},
    {FF(K_QP10000), run},  {FF(K_MAKECADR), run},
    {FF(K_SUPDACAL), run}, {FF(K_IBNKCALL), run},
    {FF(K_ISWCALL), run},  {FF(K_ISWRETRN), run},
    {FF(K_USPRCADR), run}, {FF(K_SUPERSW), run}};
/* BANKCALL through POSTJUMP, the ones the self-check idle loop uses. */
static const agc_routine_entry idle[] = {{FF(K_BANKCALL), run},
                                         {FF(K_SWCALL - 1u), run_swcall_minus1},
                                         {FF(K_SWCALL), run},
                                         {FF(K_SWRETURN), run},
                                         {FF(K_POSTJUMP), run}};
static const agc_routine_entry postjump_only[] = {{FF(K_POSTJUMP), run}};

#if LUMINARY099
static const agc_routine_entry dap[] = {{FF(K_IBNKCALL), run}, {FF(K_ISWRETRN), run}};
const agc_routine_table lm_interbank_routines = {routines, COUNT(routines)};
const agc_routine_table lm_interbank_idle_chunk = {idle, COUNT(idle)};
const agc_routine_table lm_interbank_dap_hot_routines = {dap, COUNT(dap)};
const agc_routine_table lm_interbank_postjump_hot_routines = {postjump_only, 1};
const agc_routine_table lm_interbank_postjump_t4_hot_routines = {postjump_only, 1};
#else
const agc_routine_table cm_interbank_routines = {routines, COUNT(routines)};
const agc_routine_table cm_interbank_idle_chunk = {idle, COUNT(idle)};
const agc_routine_table cm_interbank_postjump_hot_routines = {postjump_only, 1};
#endif
