/* SPDX-License-Identifier: MIT */
/* The readable self-check (src/self_check/) as routines of each mission's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_self_check.c and lm_self_check_idle.c, and the CM's):
 * <m>_self_check_idle_routines, the idle loop, which the flight store
 * loads alone (and the LM's lm_self_check_idle_hot_routines, the same
 * entries for the T4RUPT pack); <m>_self_check_routines, the rest; and
 * <m>_self_check_rope_data, SUPDACAL's read of the rope while ROPECHK
 * runs.
 *
 * Plumbing only. The states the faithful routines take: FB bank 33 with
 * SUPERBNK's bit 0100 on (so bank 43) and no bits outside 0160, the
 * interrupt latch 0 or 1, ZERO +0. A run goes on through TC SFAIL, TC
 * SKEEP1 and TC Q while they lead to a word of the file the faithful
 * port goes on at (resume[] below, its jump table), and ends at an idle
 * loop word (SELFCHK, SMODECHK +3), outside the file, or, for the idle
 * table, where self-check proper starts. Any other word of the file stops
 * it, and a run the readable code refuses part way leaves the state as
 * it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "self_check.h"
#include "../self_check/self_check.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* Bank 43's keys: physical addresses. */
#define B43(address) (uint16_t)(043u * 1024u + ((address) - 02000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads and writes no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

/* FXADRS reads the rope's words. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The states the faithful routines take. */
static bool as_expected(void)
{
    return *agc.interrupts <= 1u && !(*agc.superbank & ~0160u) && (*agc.superbank & 0100u) &&
           agc.memory[ZERO_ADDRESS] == 0u && (agc.memory[FBANK_ADDRESS] & 076000u) == 033u << 10;
}

/* The words a TC through SFAIL, SKEEP1 or Q goes on at in the same run. */
static const struct resume {
    word at;
    void (*routine)(void);
} resume[] = {
    {TCALARM2 + 2u, tcalarm2_plus2},
    {ERASCHK, eraschk},
    {ERASLOOP + 18u, erasloop_plus18},
    {ERASLOOP + 31u, erasloop_plus31},
    {ELOOPFIN + 2u, eloopfin_plus2},
    {CYCLSHFT + 11u, cyclshft_plus11},
    {CYCLSHFT + 17u, cyclshft_plus17},
    {ROPECHK, ropechk},
    {STSHOSUM, stshosum},
    {COMADRS + 5u, comadrs_plus5},
    {COMADRS + 6u, comadrs_plus6},
    {FXADRS + 3u, fxadrs_plus3},
    {ADSUM, adsum},
    {CONTINU + 5u, continu_plus5},
    {ADRS_PLUS_1, adrs_plus_1},
    {NXTBNK, nxtbnk},
    {SOPTION + 3u, soption_plus3},
    {SOPT + 4u, sopt_plus4},
    {BNKCHK + 4u, bnkchk_plus4},
};

static void (*resumes_at(word at))(void)
{
    unsigned i;

    for (i = 0; i < COUNT(resume); ++i)
        if (resume[i].at == at) return resume[i].routine;
    return NULL;
}

/* Run a routine, and the file's words a TC through a register leads to,
 * until the run leaves: true if it ends where the faithful port's does. */
static bool run_on(void (*routine)(void))
{
    for (;;) {
        routine();
        if (Z == SELFCHK || Z == SMODECHK + 3u) return true; /* the idle loop */
        if (Z < SELF_CHECK_FIRST || Z > SELF_CHECK_LAST) return true;
        routine = resumes_at(Z);
        if (!routine) return false;
    }
}

/* The idle loop's run ends where self-check proper starts, at the word
 * CCS SMODE picks. */
static bool run_idle(void (*routine)(void))
{
    routine();
    return true;
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device, bool (*how)(void (*)(void)), void (*routine)(void))
{
    static const agc_native_program none;
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!bind_machine(program ? program : &none, state, io)) return EXEC_OUT_OF_SCOPE;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    if (!as_expected()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back) || !how(routine)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

#define IDLE(name)                                                                                 \
    static enum exec_status idle_##name(const agc_native_program *program,                         \
                                        apollo_overlay_state *state, void *device)                 \
    {                                                                                              \
        return run(program, state, device, run_idle, name);                                        \
    }
#define MAIN(name)                                                                                 \
    static enum exec_status main_##name(const agc_native_program *program,                         \
                                        apollo_overlay_state *state, void *device)                 \
    {                                                                                              \
        return run(program, state, device, run_on, name);                                          \
    }
IDLE(smodechk)
IDLE(smodechk_plus2)
IDLE(smodechk_plus3)
IDLE(smodechk_plus5)
IDLE(checknj)
IDLE(checknj_plus2)
IDLE(selfchk)
MAIN(tcalarm2_plus2)
MAIN(smodechk_plus4)
MAIN(smodechk_plus6)
MAIN(smodechk_plus7)
MAIN(smodechk_plus8)
MAIN(eloopfin_plus2)
MAIN(stshosum)
MAIN(comadrs_plus5)
MAIN(continu_plus5)
MAIN(adrs_plus_1)
MAIN(nxtbnk)
MAIN(soption_plus3)
MAIN(sopt_plus4)

/* The idle loop: SMODECHK, its words, CHECKNJ and SELFCHK. A restored
 * image can be caught at any of them. */
static const agc_routine_entry idle_entries[] = {
    {B43(SMODECHK), idle_smodechk},
    {B43(SMODECHK + 2u), idle_smodechk_plus2},
    {B43(SMODECHK + 3u), idle_smodechk_plus3},
    {B43(SMODECHK + 5u), idle_smodechk_plus5},
    {B43(CHECKNJ), idle_checknj},
    {B43(CHECKNJ + 2u), idle_checknj_plus2},
    {B43(SELFCHK), idle_selfchk},
};

/* The rest: where the idle loop goes on, and each return from outside. */
static const agc_routine_entry main_entries[] = {
    {B43(TCALARM2 + 2u), main_tcalarm2_plus2},
    {B43(SMODECHK + 4u), main_smodechk_plus4},
    {B43(SMODECHK + 6u), main_smodechk_plus6},
    {B43(SMODECHK + 7u), main_smodechk_plus7},
    {B43(SMODECHK + 8u), main_smodechk_plus8},
    {B43(ELOOPFIN + 2u), main_eloopfin_plus2},
    {B43(STSHOSUM), main_stshosum},
    {B43(COMADRS + 5u), main_comadrs_plus5},
    {B43(CONTINU + 5u), main_continu_plus5},
    {B43(ADRS_PLUS_1), main_adrs_plus_1},
    {B43(NXTBNK), main_nxtbnk},
    {B43(SOPTION + 3u), main_soption_plus3},
    {B43(SOPT + 4u), main_sopt_plus4},
};

#if LUMINARY099
const agc_routine_table lm_self_check_idle_routines = {idle_entries, COUNT(idle_entries)};
const agc_routine_table lm_self_check_idle_hot_routines = {idle_entries, COUNT(idle_entries)};
const agc_routine_table lm_self_check_routines = {main_entries, COUNT(main_entries)};

bool lm_self_check_rope_data(const agc_native_program *program, uint16_t physical, agc_word *word)
{
    (void)program;
    return apollo_rope_read(ROPE, physical, word);
}
#else
const agc_routine_table cm_self_check_idle_routines = {idle_entries, COUNT(idle_entries)};
const agc_routine_table cm_self_check_routines = {main_entries, COUNT(main_entries)};

bool cm_self_check_rope_data(const agc_native_program *program, uint16_t physical, agc_word *word)
{
    (void)program;
    return apollo_rope_read(ROPE, physical, word);
}
#endif
