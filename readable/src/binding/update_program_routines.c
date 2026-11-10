/* SPDX-License-Identifier: MIT */
/* The readable update program (src/update_program/) as routines of each
 * mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_update.c, cm_update.c):
 * lm_update_routines (35 keys) and cm_update_routines (36: the CM's
 * UPACTOFF is called from outside the file too), the interpretive tables
 * lm_update_interpretive and cm_update_interpretive (UPJOB's CALL
 * INTSTALL and UPWAKE EXIT), and the entry lists the port's tests read
 * (lm_up_entries, lm_up_interp_entries and the CM's).
 *
 * Plumbing only. A basic key's function runs until control leaves the
 * file. The io record is the faithful port's: a p66_bias_io for the LM, a
 * cm_native_io for the CM. Channel 11, which UPACTOFF's WAND clears bit 3
 * of, is the program's agc_hardware word in both. An interpretive key runs
 * as one pass of the readable interpreter over UPJOB_CODE, from the
 * faithful routines' states: NEWOPS with LOC on the word, the bank 04 (CM
 * 27) in BBANK and BANKSET, no right-hand order pending (EDOP), a work
 * area at one of the VAC areas with the push-down pointer in it, and any
 * NEWJOB (neither order reads it).
 *
 * Refused, as the faithful port refuses them, with the state and the io
 * record as they were: the interrupts or SUPERBNK out of range or ZERO not
 * +0; a step the readable code does not model (an update of a register or
 * counter, a computed jump to an unintended word, a TS overflow the
 * program does not make); UPACTOFF's WAND with no agc_hardware; and
 * UPVERB -0 at the two INDEX UPVERB branches. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "../update_program/update_program.h"
#include "../agc/names.h"

#if LUMINARY099
#include "lm_update.h"
#define BANK_P27 04u
#else
#include "cm_update.h"
#define BANK_P27 027u
#endif

/* Keys: physical addresses. */
#define IN(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define B43(z)      IN(043u, z)
#define P27(z)      IN(BANK_P27, z)

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* ---- The states the faithful routines take ---- */

static bool any(void)
{
    return true;
}

/* INDEX UPVERB: the faithful port takes UPVERB 0-3 (the program stores
 * no other), and refuses -0, which the readable INDEX would run as +0. */
static bool verb_index(void)
{
    return fetch(ADDRESS_OF(UPVERB)) != MINUS_ZERO;
}

/* ---- Running a key ---- */

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, bool (*in_domain)(void), void (*routine)(void))
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    agc_hardware *hardware;

    if (!io || !bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (io->interrupt_enabled > 1u || (io->superbank & ~0160u) || state->words[7] != 0u)
        return EXEC_OUT_OF_SCOPE;
    hardware = program->hardware;
    agc.dsalmout = hardware ? &hardware->channel11 : NULL;
    agc.leave = leave;
    if (!in_domain()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

#define KEY(name, domain)                                                                          \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return run(program, state, device, domain, name);                                          \
    }
KEY(uppart2, any)
KEY(uppart2_after_phaschng, any)
KEY(uppart2_after_newmodex, verb_index)
KEY(ohwell1_terminate, any)
KEY(ohwell1_proceed, any)
KEY(ohwell1_data, any)
KEY(ohwell2_terminate, any)
KEY(ohwell2_proceed, any)
KEY(ohwell2_data, any)
KEY(upverify_terminate, any)
KEY(upverify_proceed, any)
KEY(upverify_data, any)
KEY(upstore_after_phaschng, any)
KEY(upend73_exit, any)
KEY(upfndvac_after_findvac, any)
KEY(upjob, any)
KEY(upwake, any)
KEY(upwake_after_phaschng, any)
KEY(upwake_after_upflag, verb_index)
KEY(timedidr, any)
KEY(timedidr_after_phaschng, any)
KEY(deltatok_after_tpagree, any)
KEY(deltatok_after_phaschng, any)
KEY(upout4, any)
KEY(upout_after_newmodex, any)
KEY(upend70_after_phaschng, any)
KEY(upend70_after_phaschng2, any)
KEY(uperrout_after_falton, any)
KEY(uperrout_quiet, any)
KEY(uperrout_quiet_after_falton, any)
#if COMANCHE055
KEY(upactoff, any)
#endif
KEY(v70updat, any)
KEY(v71updat, any)
KEY(v72updat, any)
KEY(v73updat, any)
KEY(v73updat_after_testxact, any)

/* By physical address: bank 04 (27) before bank 43. */
static const agc_routine_entry entries[] = {
    {P27(UPPART2), key_uppart2},
    {P27(UPPART2 + 5u), key_uppart2_after_phaschng},
    {P27(UPPART2 + 011u), key_uppart2_after_newmodex},
    {P27(OHWELL1 + 5u), key_ohwell1_terminate},
    {P27(OHWELL1 + 6u), key_ohwell1_proceed},
    {P27(OHWELL1 + 7u), key_ohwell1_data},
    {P27(OHWELL2 + 6u), key_ohwell2_terminate},
    {P27(OHWELL2 + 7u), key_ohwell2_proceed},
    {P27(OHWELL2 + 010u), key_ohwell2_data},
    {P27(UPVERIFY + 5u), key_upverify_terminate},
    {P27(UPVERIFY + 6u), key_upverify_proceed},
    {P27(UPVERIFY + 7u), key_upverify_data},
    {P27(UPSTORE + 011u), key_upstore_after_phaschng},
    {P27(UPEND73 + 5u), key_upend73_exit},
    {P27(UPFNDVAC + 4u), key_upfndvac_after_findvac},
    {P27(UPJOB), key_upjob},
    {P27(UPWAKE + 1u), key_upwake},
    {P27(UPWAKE + 3u), key_upwake_after_phaschng},
    {P27(UPWAKE + 5u), key_upwake_after_upflag},
    {P27(TIMEDIDR), key_timedidr},
    {P27(TIMEDIDR + 020u), key_timedidr_after_phaschng},
    {P27(DELTATOK + 1u), key_deltatok_after_tpagree},
    {P27(DELTATOK + 5u), key_deltatok_after_phaschng},
    {P27(UPOUT + 1u), key_upout4},
    {P27(UPOUT + 3u), key_upout_after_newmodex},
    {P27(UPEND70 + 015u), key_upend70_after_phaschng},
    {P27(UPEND70 + 034u), key_upend70_after_phaschng2},
    {P27(UPERROUT + 1u), key_uperrout_after_falton},
    {P27(UPERROUT + 2u), key_uperrout_quiet},
    {P27(UPERROUT + 3u), key_uperrout_quiet_after_falton},
#if COMANCHE055
    {P27(UPACTOFF), key_upactoff},
#endif
    {B43(V70UPDAT), key_v70updat},
    {B43(V71UPDAT), key_v71updat},
    {B43(V72UPDAT), key_v72updat},
    {B43(V73UPDAT), key_v73updat},
    {B43(V73UPDAT + 3u), key_v73updat_after_testxact},
};

/* ---- UPJOB's interpretive words ---- */

static bool vac_area(word fixloc)
{
    unsigned area;

    for (area = 0401u; area <= 0661u; area += 054u)
        if (fixloc == area) return true;
    return false;
}

static enum exec_status upjob_words(const agc_native_program *program, apollo_overlay_state *state,
                                    void *device)
{
    static const agc_native_program none;
    mission_io *io = device;
    const uint16_t *w;

    if (!state || !io) return EXEC_OUT_OF_SCOPE;
    w = state->words;
    if (io->interrupt_enabled > 1u || (io->superbank & ~0160u) || w[7] != 0u)
        return EXEC_OUT_OF_SCOPE;
    if (w[5] != NEWOPS || (w[4] & 076000u) != (BANK_P27 << 10) ||
        ((w[K_BANKSET] ^ w[4]) & 076000u) || w[K_EDOP] != 0u)
        return EXEC_OUT_OF_SCOPE;
    if (!vac_area(w[K_FIXLOC]) || w[K_PUSHLOC] < w[K_FIXLOC] || w[K_PUSHLOC] > w[K_FIXLOC] + 042u)
        return EXEC_OUT_OF_SCOPE;
    return interpret_listing_any_newjob(program ? program : &none, state, io, &UPJOB_CODE);
}

static const agc_routine_entry interpretive[] = {
    {P27(UPJOB + 1u), upjob_words}, /* CALL INTSTALL */
    {P27(UPWAKE), upjob_words},     /* EXIT */
};

#define ENTRY(bank, z) {(uint8_t)(bank), (uint16_t)(z)}
#define ENTRIES                                                                                    \
    ENTRY(BANK_P27, UPPART2), ENTRY(BANK_P27, UPPART2 + 5u), ENTRY(BANK_P27, UPPART2 + 011u),      \
        ENTRY(BANK_P27, OHWELL1 + 5u), ENTRY(BANK_P27, OHWELL1 + 6u),                              \
        ENTRY(BANK_P27, OHWELL1 + 7u), ENTRY(BANK_P27, OHWELL2 + 6u),                              \
        ENTRY(BANK_P27, OHWELL2 + 7u), ENTRY(BANK_P27, OHWELL2 + 010u),                            \
        ENTRY(BANK_P27, UPVERIFY + 5u), ENTRY(BANK_P27, UPVERIFY + 6u),                            \
        ENTRY(BANK_P27, UPVERIFY + 7u), ENTRY(BANK_P27, UPSTORE + 011u),                           \
        ENTRY(BANK_P27, UPEND73 + 5u), ENTRY(BANK_P27, UPFNDVAC + 4u), ENTRY(BANK_P27, UPJOB),     \
        ENTRY(BANK_P27, UPWAKE + 1u), ENTRY(BANK_P27, UPWAKE + 3u), ENTRY(BANK_P27, UPWAKE + 5u),  \
        ENTRY(BANK_P27, TIMEDIDR), ENTRY(BANK_P27, TIMEDIDR + 020u),                               \
        ENTRY(BANK_P27, DELTATOK + 1u), ENTRY(BANK_P27, DELTATOK + 5u),                            \
        ENTRY(BANK_P27, UPOUT + 1u), ENTRY(BANK_P27, UPOUT + 3u), ENTRY(BANK_P27, UPEND70 + 015u), \
        ENTRY(BANK_P27, UPEND70 + 034u), ENTRY(BANK_P27, UPERROUT + 1u),                           \
        ENTRY(BANK_P27, UPERROUT + 2u), ENTRY(BANK_P27, UPERROUT + 3u)

#if LUMINARY099
const lm_up_entry lm_up_entries[LM_UP_ROUTINES] = {ENTRIES,
                                                   ENTRY(043u, V70UPDAT),
                                                   ENTRY(043u, V71UPDAT),
                                                   ENTRY(043u, V72UPDAT),
                                                   ENTRY(043u, V73UPDAT),
                                                   ENTRY(043u, V73UPDAT + 3u)};
const uint16_t lm_up_interp_entries[LM_UP_INTERPRETIVE] = {(uint16_t)(UPJOB + 1u), UPWAKE};
const agc_routine_table lm_update_routines = {entries, COUNT(entries)};
const agc_routine_table lm_update_interpretive = {interpretive, COUNT(interpretive)};
#else
const cm_up_entry cm_up_entries[CM_UP_ROUTINES] = {ENTRIES,
                                                   ENTRY(BANK_P27, UPACTOFF),
                                                   ENTRY(043u, V70UPDAT),
                                                   ENTRY(043u, V71UPDAT),
                                                   ENTRY(043u, V72UPDAT),
                                                   ENTRY(043u, V73UPDAT),
                                                   ENTRY(043u, V73UPDAT + 3u)};
const uint16_t cm_up_interp_entries[CM_UP_INTERPRETIVE] = {(uint16_t)(UPJOB + 1u), UPWAKE};
const agc_routine_table cm_update_routines = {entries, COUNT(entries)};
const agc_routine_table cm_update_interpretive = {interpretive, COUNT(interpretive)};
#endif
