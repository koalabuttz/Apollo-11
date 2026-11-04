/* SPDX-License-Identifier: MIT */
/* The readable P21 (src/ground_track/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_ground_track.c): lm_ground_track_basic (the basic
 * code) and lm_ground_track_interpretive (the order pairs), all in bank
 * 24. The faithful port has an older module of the same routines
 * (c64-port/src/lm_p21_routines.c, lm_p21_basic_routines and
 * lm_p21_interpretive_routines), which the flight program tries first at
 * the keys both hold; the readable file takes the place of both, so those
 * two tables are empty here.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, GOTOPOOH) or reaches NEWOPS through a TC INTPRET.
 * An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()), which ends at the pair's DANZIG,
 * at the EXIT, at the RTB, or where a CALL or GOTO leaves its table.
 *
 * The states the faithful routines take: FB bank 24 (the superbank bit
 * either way: it moves only banks 30 to 37); at an order pair Z at
 * NEWOPS, BANKSET the BBANK, INTBIT15 the bank's bit 15, NEWJOB +0 or a
 * core set and FIXLOC one of the VAC areas. Always ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../ground_track/ground_track.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys: the bank, and the address in it. */
#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#define BANK24       050000u

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
 * the common part of its sane()). */
static bool sane_banks(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
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
    if (FBANK != BANK24 || !sane_banks()) return EXEC_OUT_OF_SCOPE;

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
BASIC(prog21)
BASIC(prog21_terminate)
BASIC(prog21_proceed)
BASIC(prog21_loaded)
BASIC(p21prog1)
BASIC(p21prog1_terminate)
BASIC(p21prog1_proceed)
BASIC(p21prog1_loaded)
BASIC(p21dsp_after_exit)
BASIC(p21dsp_terminate)
BASIC(p21dsp_proceed)
BASIC(p21dsp_recycle)
#undef BASIC

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&P21_CODE, &P21_RECYCLE_CODE};

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

static enum exec_status pair24(const agc_native_program *program, apollo_overlay_state *state,
                               void *device)
{
    return pair(BANK24, program, state, device);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each interpretive key: P21PROG1 +n, the word a pass starts at. */
#define P(n) {KEY(024u, K_P21PROG1 + (n)), pair24}
static const agc_routine_entry pairs[] = {
    P(007u),
    P(011u),
    P(013u),
    P(017u),
    P(021u),
    P(024u),
    P(027u),
    P(031u),
    P(033u),
    P(035u),
    P(037u),
    P(041u),
    P(042u),
    P(045u),
    P(050u),
    P(052u),
    P(054u),
    P(056u),
    P(060u),
    P(062u),
    P(063u),
    P(065u),
    P(067u),
    P(071u),
    P(073u),
    P(074u),
    P(077u),
    P(0101u),
    P(0103u),
    P(0106u),
    P(0110u),
    P(0113u),
    P(0116u),
    P(0120u),
    P(0122u),
    P(0125u),
    P(0127u),
    P(0130u),
    /* the recycle, after the EXIT's display */
    P(0137u),
    P(0142u),
    P(0143u),
};

#define B(z, name) {KEY(024u, z), basic_##name}
static const agc_routine_entry basic_keys[] = {
    B(K_PROG21, prog21),
    B(K_PROG21 + 5u, prog21_terminate),
    B(K_PROG21 + 6u, prog21_proceed),
    B(K_PROG21 + 7u, prog21_loaded),
    B(K_P21PROG1, p21prog1),
    B(K_P21PROG1 + 3u, p21prog1_terminate),
    B(K_P21PROG1 + 4u, p21prog1_proceed),
    B(K_P21PROG1 + 5u, p21prog1_loaded),
    B(K_P21DSP + 021u, p21dsp_after_exit),
    B(K_P21DSP + 024u, p21dsp_terminate),
    B(K_P21DSP + 025u, p21dsp_proceed),
    B(K_P21DSP + 026u, p21dsp_recycle),
};

const agc_routine_table lm_ground_track_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_ground_track_basic = {basic_keys, COUNT(basic_keys)};

/* The older module's tables: empty, the readable file runs every key. */
static const agc_routine_entry none[] = {{0xFFFFu, NULL}};
const agc_routine_table lm_p21_interpretive_routines = {none, 0u};
const agc_routine_table lm_p21_basic_routines = {none, 0u};

#endif
