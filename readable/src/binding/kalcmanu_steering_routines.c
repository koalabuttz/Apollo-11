/* SPDX-License-Identifier: MIT */
/* The readable KALCMANU_STEERING (src/kalcmanu_steering/) as routines of
 * the LM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/lm_kalcmanu_steering.c):
 * lm_kalcmanu_steering_basic (the basic code) and
 * lm_kalcmanu_steering_interpretive (the order pairs), all in bank 22.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, WAITLIST, FINDVAC, SPVAC, ENDOFJOB, TASKOVER, NOGO
 * -2) or reaches NEWOPS through a TC INTPRET. An order pair is one pass of
 * the readable interpreter over the listing's words (interpret_listing());
 * a pass that ends at RTB V1STO2S runs V1STO2S (src/rtb_op_codes/) and the
 * DANZIG it ends at, as the faithful pair does. A pass that ends at an
 * EXIT or at a CALL out of the file stops there, as the faithful pair
 * does.
 *
 * The states the faithful routines take: FB bank 22; at an order pair Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO
 * +0, the bank registers agreeing, the interrupt and superbank latches in
 * range and every erasable word an ordinary one; NEWJOB +0 or a core set,
 * and FIXLOC one of the VAC areas, where the routine may go into the
 * interpreter (the order pairs, NEWDELHI +2, NEWANGL); elsewhere NEWJOB may
 * also be -0 and FIXLOC anything. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../kalcmanu_steering/kalcmanu_steering.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* Keys and physical addresses in bank 22. */
#define BANK22 044000u
#define B22(z) (uint16_t)(022u * 1024u + ((z) - 02000u))

/* V1STO2S's bank (FB), from its FCADR. */
#define V1STO2S_BANK ((word)(I_V1STO2S & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands (KEL, MXM3's and DCMTOCDU's constants)
 * are the rope's. */
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

/* The states the faithful routines take (its sane(); with any FIXLOC, and
 * NEWJOB -0 too, its sane_nv()). */
static bool sane(bool interpreting)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u && (interpreting || newjob != MINUS_ZERO)) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (interpreting && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct {
    word at;
    void (*routine)(void);
    bool interpreting; /* may go into the interpreter */
} basic_entries[] = {
    {NEWDELHI, newdelhi, false},
    {NEWDELHI + 2u, newdelhi_after_isitauto, true},
    {NEWANGL, newangl, true},
    {NEWANGL + 025u, newangl_after_exit, false},
    {TMANUCHK + 1u, tmanuchk_after_timechk, false},
    {MANUSTAL + 4u, manustal_after_waitlist, false},
    {MANUSTAT + 1u, manustat_after_exit, false},
    {MANUCALL + 4u, manucall_after_waitlist, false},
    {UPDTCALL, updtcall, false},
    {UPDTCALL + 4u, updtcall_after_findvac, false},
    {MANUSTOP, manustop, false},
    {GOODMANU, goodmanu, false},
    {GOODMANU + 5u, goodmanu_after_spvac, false},
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
    if (i == COUNT(basic_entries) || FBANK != BANK22 || !sane(basic_entries[i].interpreting))
        return EXEC_OUT_OF_SCOPE;

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

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    static const struct interpretive_code *const codes[] = {&NEWANGL_CODE, &MANUSTAT_CODE};
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (LOC >= codes[i]->first && LOC < codes[i]->first + codes[i]->count) return codes[i];
    return NULL;
}

/* One pass; after RTB V1STO2S, V1STO2S and its DANZIG. */
static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != V1STO2S || FBANK != V1STO2S_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    v1sto2s();
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
    if (Z != NEWOPS || FBANK != BANK22 || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
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

/* Each where an order pair starts that the faithful port enters. */
#define K22(z) {B22(z), interpretive}
static const agc_routine_entry pairs[] = {
    /* NEWANGL +1 */
    K22(K_NEWANGL + 1u),
    K22(K_NEWANGL + 4u),
    K22(K_NEWANGL + 6u),
    K22(K_NEWANGL + 010u),
    K22(K_NEWANGL + 012u),
    K22(K_NEWANGL + 014u),
    K22(K_NEWANGL + 017u),
    K22(K_NEWANGL + 021u),
    K22(K_NEWANGL + 022u),
    /* MANUSTAT */
    K22(K_MANUSTAT),
};

#define B(z) {B22(z), basic}
static const agc_routine_entry basic_keys[] = {
    B(K_NEWDELHI),      B(K_NEWDELHI + 2u), B(K_NEWANGL),       B(K_NEWANGL + 025u),
    B(K_TMANUCHK + 1u), B(K_MANUSTAL + 4u), B(K_MANUSTAT + 1u), B(K_MANUCALL + 4u),
    B(K_UPDTCALL),      B(K_UPDTCALL + 4u), B(K_MANUSTOP),      B(K_GOODMANU),
    B(K_GOODMANU + 5u),
};

const agc_routine_table lm_kalcmanu_steering_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_kalcmanu_steering_basic = {basic_keys, COUNT(basic_keys)};

#endif
