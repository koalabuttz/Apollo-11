/* SPDX-License-Identifier: MIT */
/* The readable fresh start and restart (src/fresh_start/) as routines of
 * the flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_fresh_start.c, cm_fresh_start.c):
 * lm_fresh_start_routines and cm_fresh_start_routines, with
 * lm_fresh_entries and cm_fresh_entries.
 *
 * Plumbing only: point the machine at the program's state, give it the
 * channels of the fresh start io record, and take the states the faithful
 * routines take. A routine that meets a step it does not model
 * (not_modelled(), a channel it has none for, or a fixed word: it reads
 * none) is not taken: the state is put back. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "../fresh_start/fresh_start.h"
#include "../agc/names.h"
#if LUMINARY099
#include "lm_fresh_start.h"
typedef lm_fresh_io fresh_io;
#else
#include "cm_fresh_start.h"
typedef cm_fresh_io fresh_io;
#endif

#define ZERO_REGISTER 7u

/* Keys: bank 05. */
#define B5(z) (uint16_t)(5u * 1024u + ((z) - 02000u))

/* The places the routines come back to, after a TC out of the file. */
#define AFTER_MMDSPLAY (PCLOOP + 017u) /* TC MMDSPLAY at PCLOOP +16 */
#define AFTER_ALARM    (PTBAD + 2u)    /* TC ALARM; OCT 1107 */
#if COMANCHE055
#define AFTER_VAC5STOR (GOPROG + 7u)   /* TC BANKCALL; CADR VAC5STOR */
#define AFTER_SETCOARS (NOCOARSE - 4u) /* TC IBNKCALL; CADR SETCOARS */
#define AFTER_STOPRATE (NXTRST - 3u)   /* TC STOPRATE */
#define AFTER_PHASCHNG (GOTOPOOH + 2u) /* TC PHASCHNG; OCT 14 */
#endif

static fresh_io *io_now;
static const agc_native_program *program_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The routines read no fixed words (their constants are in names.h). */
static word fixed_memory(unsigned physical)
{
    (void)physical;
    leave();
    return 0;
}

/* ---- The channels of the fresh start io record ---- */

#define CONTROL (&io_now->control)
#if LUMINARY099
#define TRAPS io_now
#else
#define TRAPS (&io_now->control)
#endif

/* The channel's word: 5, 6 and 11-16, and 33. The LM's channel 11 is the
 * program's hardware, which a program may not have. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 05u: return &io_now->channel5;
    case 06u: return &io_now->channel6;
#if LUMINARY099
    case 011u: return program_now->hardware ? &program_now->hardware->channel11 : NULL;
    case 012u: return &io_now->channel12;
    case 013u: return &io_now->channel13;
    case 014u: return &io_now->channel14;
    case 015u: return &io_now->channel15;
    case 016u: return &io_now->channel16;
#else
    case 011u: return &io_now->control.channel11;
    case 012u: return &io_now->control.channel12;
    case 013u: return &io_now->control.channel13;
    case 014u: return &io_now->control.channel14;
    case 015u: return &io_now->control.main_keys;
    case 016u: return &io_now->control.navigation_keys;
#endif
    case 033u: return &io_now->channel33;
    default: return NULL;
    }
}

/* The LM reads bits 12-14 of channel 13 (the HANDRUPT traps) as 0. */
static word channel_in(unsigned number)
{
    agc_word *word_there = channel(number);

    if (!word_there) leave();
#if LUMINARY099
    if (number == 013u) return (word)(*word_there & 043777u);
#endif
    return (word)*word_there;
}

/* As yaAGC writes a channel: channel 13's bits 12-14 set the HANDRUPT
 * traps and are not kept. In the CM a write to channel 33 sets its latched
 * bits 11-15 again, but bit 14 (the AGC warning) stays 0 while the warning
 * lasts. */
static void channel_out(unsigned number, word value)
{
    agc_word *word_there = channel(number);

    if (!word_there) leave();
    if (number == 013u) {
        if (value & 004000u) TRAPS->trap31a = 1u;
        if (value & 010000u) TRAPS->trap31b = 1u;
        if (value & 020000u) TRAPS->trap32 = 1u;
        value &= 043777u;
    }
#if COMANCHE055
    if (number == 033u) {
        value = (word)(*word_there | 076000u);
        if (io_now->warning) value &= (word)~020000u;
    }
#endif
    *word_there = value;
}

/* ---- The states the faithful routines take ---- */

/* MR.KLEAN, P00KLEAN and V37KLEAN end with TC Q. Called through BANKCALL
 * or IBNKCALL they return to fixed-fixed. Not taken: the fresh start's own
 * TCs to them, which run inside the file (a readable C call returns there),
 * and a Q with an overflow, which the AGC would run as some other
 * instruction (the faithful routine goes to the corrected address). */
static bool returns_to_fixed_fixed(void)
{
    return overflow_of(Q) == 0 && Q >= 04000u && Q < 010000u;
}

static bool takes(word entry)
{
    if (agc.memory[ZERO_REGISTER] != 0u || (CONTROL->superbank & ~0160u) != 0u ||
        CONTROL->interrupt_enabled > 1u)
        return false;
    switch (entry) {
    case MR_KLEAN:
    case P00KLEAN:
    case V37KLEAN: return returns_to_fixed_fixed();
    default: return true;
    }
}

static enum exec_status run(const agc_native_program *program, apollo_overlay_state *state,
                            void *device)
{
    static const agc_native_program none;
    static apollo_overlay_state before;
    static fresh_io io_before;
    static agc_word channel11_before;
    word entry;

    program_now = program ? program : &none;
    if (!state || !device || !bind_machine(program_now, state, device)) return EXEC_OUT_OF_SCOPE;
    io_now = device;
    agc.fixed_memory = fixed_memory;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
    entry = Z;
    if (!takes(entry)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    io_before = *io_now;
    if (program_now->hardware) channel11_before = program_now->hardware->channel11;
    if (setjmp(back)) {
        *state = before;
        *io_now = io_before;
        if (program_now->hardware) program_now->hardware->channel11 = channel11_before;
        return EXEC_OUT_OF_SCOPE;
    }
    switch (entry) {
    case GOTOPOOH: gotopooh(); break;
    case SLAP1: slap1(); break;
    case MR_KLEAN: mr_klean(); break;
    case P00KLEAN: p00klean(); break;
    case V37KLEAN: v37klean(); break;
    case GOPROG: goprog(); break;
    case ENEMA: enema(); break;
#if LUMINARY099
    case GOPROG2: goprog2(); break;
#else
    case AFTER_PHASCHNG: gotopooh_after_phaschng(); break;
    case AFTER_VAC5STOR: goprog_after_vac5stor(); break;
    case AFTER_SETCOARS: elrskip_after_setcoars(); break;
    case NOCOARSE: nocoarse(); break;
    case GOPROG3: goprog3(); break;
    case AFTER_STOPRATE: goprog3_after_stoprate(); break;
    case T5IDLOC: t5idloc(); break;
#endif
    case AFTER_MMDSPLAY: goprog3_after_mmdsplay(); break;
    case PINACT: pinact(); break;
    case AFTER_ALARM: ptbad_after_alarm(); break;
    default: return EXEC_OUT_OF_SCOPE;
    }
    return EXEC_OK;
}

/* In the faithful table's order (by key). */
#if LUMINARY099
static const agc_routine_entry routines[] = {
    {FF(GOTOPOOH), run},       {B5(SLAP1), run},  {B5(MR_KLEAN), run},   {B5(P00KLEAN), run},
    {B5(V37KLEAN), run},       {B5(GOPROG), run}, {B5(ENEMA), run},      {B5(GOPROG2), run},
    {B5(AFTER_MMDSPLAY), run}, {B5(PINACT), run}, {B5(AFTER_ALARM), run}};
const uint16_t lm_fresh_entries[LM_FS_ROUTINES] = {SLAP1,  MR_KLEAN,    P00KLEAN, V37KLEAN,
                                                   GOPROG, ENEMA,       GOPROG2,  AFTER_MMDSPLAY,
                                                   PINACT, AFTER_ALARM, GOTOPOOH};
const agc_routine_table lm_fresh_start_routines = {routines, COUNT(routines)};
#else
static const agc_routine_entry routines[] = {
    {FF(GOTOPOOH), run},       {FF(AFTER_PHASCHNG), run}, {B5(SLAP1), run},
    {B5(MR_KLEAN), run},       {B5(P00KLEAN), run},       {B5(V37KLEAN), run},
    {B5(GOPROG), run},         {B5(AFTER_VAC5STOR), run}, {B5(AFTER_SETCOARS), run},
    {B5(NOCOARSE), run},       {B5(ENEMA), run},          {B5(GOPROG3), run},
    {B5(AFTER_MMDSPLAY), run}, {B5(AFTER_STOPRATE), run}, {B5(PINACT), run},
    {B5(AFTER_ALARM), run},    {B5(T5IDLOC), run}};
const uint16_t cm_fresh_entries[CM_FS_ROUTINES] = {
    GOTOPOOH,       AFTER_PHASCHNG, SLAP1,          MR_KLEAN,    P00KLEAN, V37KLEAN,
    GOPROG,         AFTER_VAC5STOR, AFTER_SETCOARS, NOCOARSE,    ENEMA,    GOPROG3,
    AFTER_MMDSPLAY, AFTER_STOPRATE, PINACT,         AFTER_ALARM, T5IDLOC};
const agc_routine_table cm_fresh_start_routines = {routines, COUNT(routines)};
#endif
