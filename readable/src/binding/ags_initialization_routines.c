/* SPDX-License-Identifier: MIT */
/* The readable R47 (src/ags_initialization/) as routines of the LM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_ags_init.c): lm_ags_init_routines (the basic code) and
 * lm_ags_init_interpretive (the order pairs), with ags_routine_keys.
 *
 * Plumbing only. A routine runs the file's code from its key until control
 * leaves the file: the basic entries in bank 32, and the order pairs (Z at
 * NEWOPS, LOC on one, under BANKSET bank 32) one pass of the readable
 * interpreter each over the listing's words (interpret_listing()); a TC
 * INTPRET from the file runs INTPRET too. Control leaves at a basic
 * address of another file (ALARM, ENDEXT, BANKCALL, DOWNFLAG), an RTB's
 * routine, NEWOPS with LOC outside the file, or CHANG2.
 *
 * Built with AGC_FLIGHT_INTERPRETER, as the faithful port's flight program
 * builds its file, a run stops instead where it reaches NEWOPS: there the
 * flight program's interpreter takes the file's interpretive code, and its
 * EXITs come back to two more basic keys (AGSVCALC +5 and +034).
 *
 * The states the faithful routines take: FB bank 32 for the basic code; at
 * an order pair FB and BANKSET bank 32, EDOP +0, the work area one of the
 * five VAC areas with PUSHLOC in it, and at SCALEVEC's RVQ a return in
 * fixed memory. The io record is lm_ags_io, whose undo image the caller
 * supplies: a run that is refused after it starts leaves the state and
 * the io record as they were. One difference: the readable interpreter
 * takes no pass with NEWJOB negative (the Executive never leaves it so),
 * where the faithful one refuses only at a DANZIG. */
#include <stddef.h>
#include "binding.h"
#include "flight_undo.h"
#include "lm_ags_init.h"
#include "../ags_initialization/ags_initialization.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

#define BANK32    032u
#define RUN_LIMIT 400u
#define B32(z)    (uint16_t)(BANK32 * 1024u + ((z) - 02000u))

/* SCALEVEC's last pair, SXA,1 RVQ. */
#define SCALEVEC_RVQ (SCALEVEC + 034u)

static const struct {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {AGSINIT, agsinit},
    {AGSINIT + 6u, agsinit_after_alarm},
    {AGSDISPK + 3u, agsdispk_after_terminate},
    {AGSDISPK + 4u, agsdispk_after_proceed},
    {AGSDISPK + 5u, agsdispk_after_enter},
    {AGSVCALC + 5u, agsvcalc_after_exit},
    {AGSVCALC + 010u, agsvcalc_after_exdspret},
    {AGSVCALC + 034u, agsvcalc_after_second_exit},
    {AGSVCALC + 041u, agsvcalc_after_delayjob},
    {CKSTALL + 7u, ckstall_after_delayjob},
    {CKSTALL + 012u, ckstall_after_imuzero},
    {CKSTALL + 014u, ckstall_after_imustall},
    {AGSEND, agsend},
    {AGSEND + 2u, agsend_after_downflag},
    {AGSEND + 5u, agsend_after_terminate},
    {AGSEND + 6u, agsend_after_proceed},
    {AGSEND + 7u, agsend_after_enter},
};

static const agc_routine_entry pair_keys[24];

static const struct interpretive_code *const codes[] = {&AGSVCALC_CODE, &AGSVCALC_CODE_2,
                                                        &SCALEVEC_CODE};

static uint16_t physical(word address, word bank_register)
{
    return agc_fixed_physical(address, bank_register, *agc.superbank);
}

/* The file's basic routine at Z, under FB, or none. */
static void (*basic_at(void))(void)
{
    unsigned i;

    if (Z < 02000u || Z >= 04000u || physical(Z, FBANK) != B32(Z)) return NULL;
    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == Z) return basic_entries[i].routine;
    return NULL;
}

/* Whether LOC, under BANKSET, is in the file's interpretive code (as the
 * faithful routines take it: AGSVCALC +1 to +033, with the basic words
 * between, and SCALEVEC to its end). */
static bool in_file_code(void)
{
    return LOC >= 02000u && LOC < 04000u && physical(LOC, BANKSET) == B32(LOC) &&
           ((LOC > AGSVCALC && LOC <= AGSVCALC + 033u) ||
            (LOC >= SCALEVEC && LOC <= SCALEVEC + 035u));
}

/* The table whose order pair LOC is on, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i, k;

    for (i = 0; i < COUNT(pair_keys); ++i) {
        if (pair_keys[i].physical != physical(LOC, BANKSET)) continue;
        for (k = 0; k < COUNT(codes); ++k)
            if (LOC >= codes[k]->first && LOC < codes[k]->first + codes[k]->count) return codes[k];
    }
    return NULL;
}

static bool work_area(void)
{
    unsigned area;

    if (EDOP != 0u) return false;
    for (area = 0401u; area <= 0661u; area += 054u)
        if (FIXLOC == area) return PUSHLOC >= area && PUSHLOC <= area + 042u;
    return false;
}

enum step { GO_ON, LEFT, REFUSED };

static enum step step(const agc_native_program *program, apollo_overlay_state *state, lm_ags_io *io,
                      unsigned n)
{
    const struct interpretive_code *code;
    void (*routine)(void);

    if (Z == NEWOPS) {
        if (!in_file_code()) return LEFT;
#ifdef AGC_FLIGHT_INTERPRETER
        return LEFT; /* the flight program's interpreter goes on */
#endif
        code = code_at();
        if (!code) return REFUSED; /* an operand word, or a basic one */
        if (((FBANK ^ BANKSET) & 076000u) || !work_area()) return REFUSED;
        if (LOC == SCALEVEC_RVQ && !(agc.memory[(FIXLOC + 052u) & 03777u] & 074000u))
            return REFUSED; /* QPRET */
        if (interpret_listing(program, state, &io->control, code) != EXEC_OK) return REFUSED;
    } else if (Z == INTPRET && n > 0u) { /* the file's TC INTPRET, to NEWOPS */
        intpret();
    } else if ((routine = basic_at()) != NULL) {
        routine();
    } else {
        return LEFT;
    }
    /* (interpret_listing pointed the machine at its own hooks) */
    bind_machine(program, state, &io->control);
    return GO_ON;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    lm_ags_io *io = device, entry;
    unsigned n;

    if (!state || !io || !io->undo || FLIGHT_UNDO_ALIAS(io, state)) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    FLIGHT_UNDO_SAVE(io, state);
    entry = *io;
    bind_machine(program, state, &io->control);
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(program, state, io, n)) {
        case GO_ON: continue;
        case LEFT:
            if (n > 0u) return EXEC_OK;
            break;
        case REFUSED: break;
        }
        break;
    }
    FLIGHT_UNDO_RESTORE(&entry, state);
    *io = entry;
    return EXEC_OUT_OF_SCOPE;
}

/* Keys: bank 32, in the faithful tables' order. */
#ifdef AGC_FLIGHT_INTERPRETER
#define FLIGHT_KEY(z) {B32(z), run},
#else
#define FLIGHT_KEY(z)
#endif
static const agc_routine_entry basic_keys[] = {
    {B32(K_AGSINIT), run},
    {B32(K_AGSINIT + 6u), run},
    {B32(K_AGSDISPK + 3u), run},
    {B32(K_AGSDISPK + 4u), run},
    {B32(K_AGSDISPK + 5u), run},
    FLIGHT_KEY(K_AGSVCALC + 5u){B32(K_AGSVCALC + 010u), run},
    FLIGHT_KEY(K_AGSVCALC + 034u){B32(K_AGSVCALC + 041u), run},
    {B32(K_CKSTALL + 7u), run},
    {B32(K_CKSTALL + 012u), run},
    {B32(K_CKSTALL + 014u), run},
    {B32(K_AGSEND), run},
    {B32(K_AGSEND + 2u), run},
    {B32(K_AGSEND + 5u), run},
    {B32(K_AGSEND + 6u), run},
    {B32(K_AGSEND + 7u), run}};
/* Each pair's first word: AGSVCALC +1 and +3, the pairs from +011, and
 * SCALEVEC's. */
#define PAIR(z) {B32(z), run}
static const agc_routine_entry pair_keys[24] = {
    PAIR(K_AGSVCALC + 1u),   PAIR(K_AGSVCALC + 3u),   PAIR(K_AGSVCALC + 011u),
    PAIR(K_AGSVCALC + 013u), PAIR(K_AGSVCALC + 015u), PAIR(K_AGSVCALC + 017u),
    PAIR(K_AGSVCALC + 021u), PAIR(K_AGSVCALC + 023u), PAIR(K_AGSVCALC + 025u),
    PAIR(K_AGSVCALC + 027u), PAIR(K_AGSVCALC + 032u), PAIR(K_AGSVCALC + 033u),
    PAIR(K_SCALEVEC),        PAIR(K_SCALEVEC + 3u),   PAIR(K_SCALEVEC + 5u),
    PAIR(K_SCALEVEC + 010u), PAIR(K_SCALEVEC + 012u), PAIR(K_SCALEVEC + 014u),
    PAIR(K_SCALEVEC + 017u), PAIR(K_SCALEVEC + 021u), PAIR(K_SCALEVEC + 024u),
    PAIR(K_SCALEVEC + 026u), PAIR(K_SCALEVEC + 031u), PAIR(K_SCALEVEC + 034u)};
const agc_routine_table lm_ags_init_routines = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_ags_init_interpretive = {pair_keys, COUNT(pair_keys)};

/* The host keys, basic then interpretive (the port's cases run each). */
const uint16_t ags_routine_keys[AGS_ROUTINES] = {
    B32(K_AGSINIT),         B32(K_AGSINIT + 6u),    B32(K_AGSDISPK + 3u),   B32(K_AGSDISPK + 4u),
    B32(K_AGSDISPK + 5u),   B32(K_AGSVCALC + 010u), B32(K_AGSVCALC + 041u), B32(K_CKSTALL + 7u),
    B32(K_CKSTALL + 012u),  B32(K_CKSTALL + 014u),  B32(K_AGSEND),          B32(K_AGSEND + 2u),
    B32(K_AGSEND + 5u),     B32(K_AGSEND + 6u),     B32(K_AGSEND + 7u),     B32(K_AGSVCALC + 1u),
    B32(K_AGSVCALC + 3u),   B32(K_AGSVCALC + 011u), B32(K_AGSVCALC + 013u), B32(K_AGSVCALC + 015u),
    B32(K_AGSVCALC + 017u), B32(K_AGSVCALC + 021u), B32(K_AGSVCALC + 023u), B32(K_AGSVCALC + 025u),
    B32(K_AGSVCALC + 027u), B32(K_AGSVCALC + 032u), B32(K_AGSVCALC + 033u), B32(K_SCALEVEC),
    B32(K_SCALEVEC + 3u),   B32(K_SCALEVEC + 5u),   B32(K_SCALEVEC + 010u), B32(K_SCALEVEC + 012u),
    B32(K_SCALEVEC + 014u), B32(K_SCALEVEC + 017u), B32(K_SCALEVEC + 021u), B32(K_SCALEVEC + 024u),
    B32(K_SCALEVEC + 026u), B32(K_SCALEVEC + 031u), B32(K_SCALEVEC + 034u)};

#endif
