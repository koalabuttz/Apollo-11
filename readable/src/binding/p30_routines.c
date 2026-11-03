/* SPDX-License-Identifier: MIT */
/* The readable P30 (src/p30/) as routines of the LM's flight program,
 * under the table names and keys of the faithful port's
 * (c64-port/src/lm_p30_file.c): lm_p30_file_basic (the basic code, bank
 * 35) and lm_p30_file_interpretive (the order pairs, banks 34 and 35).
 * The faithful port has an older module of the same routines
 * (c64-port/src/lm_p30_routines.c, lm_p30_basic_routines and
 * lm_p30_interpretive_routines), which the flight program tries first at
 * the keys both hold; the readable file takes the place of both, so those
 * two tables are empty here.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (UPFLAG, VNPOOH, DOWNFLAG) or reaches NEWOPS through a TC
 * INTPRET. An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()), which ends at the pair's DANZIG
 * or where a CALL or GOTO leaves its table.
 *
 * The states the faithful routines take: FB bank 35 (a basic routine) or
 * the pair's bank, with the superbank bit clear; at an order pair Z at
 * NEWOPS, BANKSET the BBANK, INTBIT15 the bank's bit 15, NEWJOB +0 or a
 * core set and FIXLOC one of the VAC areas. Always ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../p30/p30.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys: the bank, and the address in it. */
#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define BANK35       072000u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    bind_machine(program, state, io);
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states both kinds of routine take (the faithful basic_sane() and
 * the common part of its sane()): banks 30-37 without the superbank bit. */
static bool sane_banks(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u) || (*agc.superbank & 0100u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* An order pair's too: FIXLOC a VAC area. (The readable interpreter
 * itself takes NEWJOB only +0 or a core set, and FIXLOC also a NOVAC
 * job's work area, which the faithful routines do not.) */
static bool sane_interpretive(void)
{
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    return sane_banks();
}

/* ---- The basic code ---- */

/* A basic routine runs from its key, whatever Z holds (the trampoline
 * calls it there). */
static enum exec_status basic(void (*routine)(void), const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != BANK35 || !sane_banks()) return EXEC_OUT_OF_SCOPE;

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

#define BASIC(name)                                                                                \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return basic(name, program, state, device);                                                \
    }
BASIC(p30)
BASIC(p30_after_updatflg)
BASIC(p30n33)
BASIC(p30_after_v06n33)
BASIC(p30_after_v06n81)
BASIC(p30_after_downflag)
BASIC(param30)
BASIC(param30_after_v06n42)
#undef BASIC

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&P30_CODE, &PARAM30_CODE, &S30_1_CODE};

/* The table LOC is in, in the bank FB holds, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (FBANK == (word)(codes[i]->bank << 10) && LOC >= codes[i]->first &&
            LOC < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

static enum exec_status pair(word bank, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || FBANK != bank || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane_interpretive())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

static enum exec_status pair34(const agc_native_program *program, apollo_overlay_state *state,
                               void *device)
{
    return pair(070000u, program, state, device);
}

static enum exec_status pair35(const agc_native_program *program, apollo_overlay_state *state,
                               void *device)
{
    return pair(BANK35, program, state, device);
}

/* ---- The keys, in the faithful tables' order ---- */

#define S30(n) {KEY(034u, K_S30_1 + (n)), pair34}
static const agc_routine_entry pairs[] = {
    /* S30.1 (bank 34) */
    S30(000u),
    S30(003u),
    S30(005u),
    S30(010u),
    S30(011u),
    S30(012u),
    S30(014u),
    S30(015u),
    S30(017u),
    S30(022u),
    S30(024u),
    S30(025u),
    S30(030u),
    S30(032u),
    S30(033u),
    S30(034u),
    S30(036u),
    S30(041u),
    S30(043u),
    S30(045u),
    S30(047u),
    S30(051u),
    S30(053u),
    S30(055u),
    /* P30's (bank 35): CALL S30.1, SET EXIT, SETGO */
    {KEY(035u, K_P30N33 + 7u), pair35},
    {KEY(035u, K_P30N33 + 011u), pair35},
    {KEY(035u, K_PARAM30 + 3u), pair35},
};

#define B(z, name) {KEY(035u, z), basic_##name}
static const agc_routine_entry basic_keys[] = {
    B(K_P30, p30),
    B(K_P30 + 2u, p30_after_updatflg),
    B(K_P30N33, p30n33),
    B(K_P30N33 + 2u, p30_after_v06n33),
    B(K_P30N33 + 4u, p30_after_v06n81),
    B(K_P30N33 + 6u, p30_after_downflag),
    B(K_PARAM30, param30),
    B(K_PARAM30 + 2u, param30_after_v06n42),
};

const agc_routine_table lm_p30_file_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_p30_file_basic = {basic_keys, COUNT(basic_keys)};

/* The older module's tables: empty, the readable file runs every key. */
static const agc_routine_entry none[] = {{0xFFFFu, NULL}};
const agc_routine_table lm_p30_interpretive_routines = {none, 0u};
const agc_routine_table lm_p30_basic_routines = {none, 0u};

#endif
