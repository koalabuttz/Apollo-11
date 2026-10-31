/* SPDX-License-Identifier: MIT */
/* The readable R63 (src/r63/) as routines of the LM's flight program,
 * under the table names and keys of the faithful port's
 * (c64-port/src/lm_r63.c): lm_r63_basic (the basic code) and
 * lm_r63_interpretive (the order pairs), all in bank 26.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, DOWNFLAG, ENDEXT) or reaches NEWOPS through a TC
 * INTPRET. An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()); a pass that ends at RTB LOADTIME
 * runs LOADTIME (src/rtb_op_codes/) and the DANZIG it ends at, the pair's
 * DAD, as the faithful pair does. A pass that ends at RTB NORMUNIT, or at
 * a CALL out of the file, stops there, as the faithful pair does.
 *
 * The states the faithful routines take: FB bank 26; at an order pair Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO
 * +0, the bank registers agreeing, NEWJOB +0 or a core set, FIXLOC one of
 * the VAC areas, the interrupt and superbank latches in range and every
 * erasable word an ordinary one. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../r63/r63.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys and physical addresses in bank 26. */
#define BANK26 054000u
#define B26(z) (uint16_t)(026u * 1024u + ((z) - 02000u))

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands (DP1MIN, UNITX, UNITZ) are the rope's. */
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

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {V89CALL, v89call},
    {V89CALL + 2u, v89call_after_r02both},
    {V89CALL + 011u, v89call_after_terminate},
    {V89CALL + 012u, v89call_after_proceed},
    {V89CALL + 013u, v89call_after_data_in},
    {V89RECL, v89recl},
    {V89RECL + 025u, v89recl_after_exit},
    {V89CALL1 + 4u, v89call1_after_exit},
    {V89CALL1 + 6u, v89call1_after_ballangs},
    {V89CALL1 + 011u, v89call1_after_terminate},
    {V89CALL1 + 012u, v89call1_after_proceed},
    {V89CALL1 + 013u, v89call1_after_recycle},
    {V89CALL1 + 014u, v89call1_after_v89recl},
    {V89CALL1 + 016u, v89call1_after_downflag},
    {V89CALL1 + 020u, v89call1_after_r60lem},
};

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    unsigned i;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == Z) break;
    if (i == COUNT(basic_entries) || FBANK != BANK26 || !sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    basic_entries[i].routine();
    return EXEC_OK;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&V89RECL_CODE, &ALINEX_CODE, &ALINEZ_CODE};

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (LOC >= codes[i]->first && LOC < codes[i]->first + codes[i]->count) return codes[i];
    return NULL;
}

/* One pass; after RTB LOADTIME, LOADTIME and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
}

static enum exec_status interpretive(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || FBANK != BANK26 || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = pair(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

#define K26(z) {B26(z), interpretive}
static const agc_routine_entry pairs[] = {
    /* V89RECL */
    K26(K_V89RECL + 01u),
    K26(K_V89RECL + 04u),
    K26(K_V89RECL + 05u),
    K26(K_V89RECL + 07u),
    K26(K_V89RECL + 011u),
    K26(K_V89RECL + 013u),
    K26(K_V89RECL + 015u),
    K26(K_V89RECL + 020u),
    K26(K_V89RECL + 023u),
    K26(K_V89RECL + 024u),
    /* ALINEX and V89CALL1 */
    K26(K_ALINEX + 01u),
    K26(K_V89CALL1),
    K26(K_V89CALL1 + 02u),
    K26(K_V89CALL1 + 03u),
    /* ALINEZ */
    K26(K_ALINEZ + 01u),
};

#define B(z) {B26(z), basic}
static const agc_routine_entry basic_keys[] = {
    B(K_V89CALL),         B(K_V89CALL + 2u),    B(K_V89CALL + 011u),  B(K_V89CALL + 012u),
    B(K_V89CALL + 013u),  B(K_V89RECL),         B(K_V89RECL + 025u),  B(K_V89CALL1 + 4u),
    B(K_V89CALL1 + 6u),   B(K_V89CALL1 + 011u), B(K_V89CALL1 + 012u), B(K_V89CALL1 + 013u),
    B(K_V89CALL1 + 014u), B(K_V89CALL1 + 016u), B(K_V89CALL1 + 020u),
};

const agc_routine_table lm_r63_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_r63_basic = {basic_keys, COUNT(basic_keys)};

#endif
