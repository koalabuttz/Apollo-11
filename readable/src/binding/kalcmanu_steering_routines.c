/* SPDX-License-Identifier: MIT */
/* The readable KALCMANU_STEERING (src/kalcmanu_steering/) as routines of
 * each mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_kalcmanu_steering.c and
 * cm_kalcmanu_steering.c): <m>_kalcmanu_steering_basic (the basic code)
 * and <m>_kalcmanu_steering_interpretive (the order pairs). The LM's are
 * all in bank 22; the CM's in banks 20, 21 and 22, and STOPRATE in
 * fixed-fixed memory, which runs under any bank.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, IBNKCALL, POSTJUMP, WAITLIST, FINDVAC, SPVAC,
 * ENDOFJOB, TASKOVER, NOGO -2, a TC Q back to a caller elsewhere) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()); a
 * pass that ends at RTB V1STO2S runs V1STO2S (src/rtb_op_codes/) and the
 * DANZIG it ends at, as the faithful pair does. A pass that ends at an
 * EXIT or at a CALL out of the file stops there, as the faithful pair
 * does.
 *
 * The states the faithful routines take: FB the key's bank; at an order
 * pair Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15.
 * Always ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one;
 * NEWJOB +0 or a core set, and FIXLOC one of the VAC areas, where the
 * routine may go into the interpreter (the order pairs, the LM's NEWDELHI
 * +2, NEWDELHI in the CM, NEWANGL); elsewhere NEWJOB may also be -0 and
 * FIXLOC anything. A run the readable code refuses part way leaves the
 * state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../kalcmanu_steering/kalcmanu_steering.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* A bank's FB, and the key of a word in it. */
#define BANK(n)      (word)((n) << 10)
#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
/* The key of a word in fixed-fixed memory (banks 2 and 3, 04000-07777):
 * its address. */
#define FIXED_FIXED_KEY(z) (uint16_t)(z)
#define ANY_BANK           0177777u

/* V1STO2S's bank (FB), from its FCADR. */
#define V1STO2S_BANK ((word)(I_V1STO2S & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands (KEL or DEL, MXM3's and DCMTOCDU's
 * constants) and the CM's QUADROT are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
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
    word at, bank;
    void (*routine)(void);
    bool interpreting; /* may go into the interpreter */
} basic_entries[] = {
#if LUMINARY099
    {NEWDELHI, BANK(022u), newdelhi, false},
    {NEWDELHI + 2u, BANK(022u), newdelhi_after_isitauto, true},
    {NEWANGL, BANK(022u), newangl, true},
    {NEWANGL + 025u, BANK(022u), newangl_after_exit, false},
    {TMANUCHK + 1u, BANK(022u), tmanuchk_after_timechk, false},
    {MANUSTAL + 4u, BANK(022u), manustal_after_waitlist, false},
    {MANUSTAT + 1u, BANK(022u), manustat_after_exit, false},
    {MANUCALL + 4u, BANK(022u), manucall_after_waitlist, false},
    {UPDTCALL, BANK(022u), updtcall, false},
    {UPDTCALL + 4u, BANK(022u), updtcall_after_findvac, false},
    {MANUSTOP, BANK(022u), manustop, false},
    {GOODMANU, BANK(022u), goodmanu, false},
    {GOODMANU + 5u, BANK(022u), goodmanu_after_spvac, false},
#else
    {STOPRATE, ANY_BANK, stoprate, false},
    {MANUSTAT + 1u, BANK(020u), manustat_after_exit, false},
    {CONTMANU, BANK(020u), contmanu, false},
    {MANUCALL + 3u, BANK(020u), manucall_after_waitlist, false},
    {UPDTCALL, BANK(020u), updtcall, false},
    {UPDTCALL + 4u, BANK(020u), updtcall_after_findvac, false},
    {ZEROEROR, BANK(020u), zeroeror, false},
    {MANUSTOP, BANK(021u), manustop, false},
    {MANUSTOP + 1u, BANK(021u), manustop_after_stopyz, false},
    {ENDROLL, BANK(021u), endroll, false},
    {ENDMANU, BANK(021u), endmanu, false},
    {ENDMANU + 5u, BANK(021u), endmanu_after_spvac, false},
    {NEWDELHI, BANK(022u), newdelhi, true},
    {NEWANGL, BANK(022u), newangl, true},
    {NEWANGL + 025u, BANK(022u), newangl_after_exit, false},
    {TMANUCHK + 1u, BANK(022u), tmanuchk_after_timechk, false},
    {MANUSTAL + 3u, BANK(022u), manustal_after_waitlist, false},
    {LOADCDUD, BANK(022u), loadcdud, false},
    {LOADYZ, BANK(022u), loadyz, false},
#endif
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
        if (basic_entries[i].at == Z &&
            (basic_entries[i].bank == ANY_BANK || basic_entries[i].bank == FBANK))
            break;
    if (i == COUNT(basic_entries) || !sane(basic_entries[i].interpreting)) return EXEC_OUT_OF_SCOPE;

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

/* The table LOC is in, under FB, or none. */
static const struct interpretive_code *code_at(void)
{
    static const struct interpretive_code *const codes[] = {&NEWANGL_CODE, &MANUSTAT_CODE};
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (FBANK == BANK(codes[i]->bank) && LOC >= codes[i]->first &&
            LOC < codes[i]->first + codes[i]->count)
            return codes[i];
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
    if (Z != NEWOPS || !code || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane(true))
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
#define PAIR(bank, z)  {KEY(bank, z), interpretive}
#define BASIC(bank, z) {KEY(bank, z), basic}
#if LUMINARY099
static const agc_routine_entry pairs[] = {
    /* NEWANGL +1 */
    PAIR(022u, K_NEWANGL + 1u),
    PAIR(022u, K_NEWANGL + 4u),
    PAIR(022u, K_NEWANGL + 6u),
    PAIR(022u, K_NEWANGL + 010u),
    PAIR(022u, K_NEWANGL + 012u),
    PAIR(022u, K_NEWANGL + 014u),
    PAIR(022u, K_NEWANGL + 017u),
    PAIR(022u, K_NEWANGL + 021u),
    PAIR(022u, K_NEWANGL + 022u),
    /* MANUSTAT */
    PAIR(022u, K_MANUSTAT),
};

static const agc_routine_entry basic_keys[] = {
    BASIC(022u, K_NEWDELHI),       BASIC(022u, K_NEWDELHI + 2u), BASIC(022u, K_NEWANGL),
    BASIC(022u, K_NEWANGL + 025u), BASIC(022u, K_TMANUCHK + 1u), BASIC(022u, K_MANUSTAL + 4u),
    BASIC(022u, K_MANUSTAT + 1u),  BASIC(022u, K_MANUCALL + 4u), BASIC(022u, K_UPDTCALL),
    BASIC(022u, K_UPDTCALL + 4u),  BASIC(022u, K_MANUSTOP),      BASIC(022u, K_GOODMANU),
    BASIC(022u, K_GOODMANU + 5u),
};

const agc_routine_table lm_kalcmanu_steering_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_kalcmanu_steering_basic = {basic_keys, COUNT(basic_keys)};
#else
static const agc_routine_entry pairs[] = {
    /* MANUSTAT */
    PAIR(020u, K_MANUSTAT),
    /* NEWANGL +1 */
    PAIR(022u, K_NEWANGL + 1u),
    PAIR(022u, K_NEWANGL + 4u),
    PAIR(022u, K_NEWANGL + 6u),
    PAIR(022u, K_NEWANGL + 010u),
    PAIR(022u, K_NEWANGL + 012u),
    PAIR(022u, K_NEWANGL + 014u),
    PAIR(022u, K_NEWANGL + 017u),
    PAIR(022u, K_NEWANGL + 021u),
    PAIR(022u, K_NEWANGL + 022u),
};

static const agc_routine_entry basic_keys[] = {
    {FIXED_FIXED_KEY(K_STOPRATE), basic},
    BASIC(020u, K_MANUSTAT + 1u),
    BASIC(020u, K_CONTMANU),
    BASIC(020u, K_MANUCALL + 3u),
    BASIC(020u, K_UPDTCALL),
    BASIC(020u, K_UPDTCALL + 4u),
    BASIC(020u, K_ZEROEROR),
    BASIC(021u, K_MANUSTOP),
    BASIC(021u, K_MANUSTOP + 1u),
    BASIC(021u, K_ENDROLL),
    BASIC(021u, K_ENDMANU),
    BASIC(021u, K_ENDMANU + 5u),
    BASIC(022u, K_NEWDELHI),
    BASIC(022u, K_NEWANGL),
    BASIC(022u, K_NEWANGL + 025u),
    BASIC(022u, K_TMANUCHK + 1u),
    BASIC(022u, K_MANUSTAL + 3u),
    BASIC(022u, K_LOADCDUD),
    BASIC(022u, K_LOADYZ),
};

const agc_routine_table cm_kalcmanu_steering_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_kalcmanu_steering_basic = {basic_keys, COUNT(basic_keys)};
#endif
