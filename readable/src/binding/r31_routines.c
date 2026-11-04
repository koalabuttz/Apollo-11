/* SPDX-License-Identifier: MIT */
/* The readable R31 (src/r31/) as routines of each mission's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_r31_file.c and cm_r31.c): lm_r31_file_basic and
 * cm_r31_basic (the basic code) and lm_r31_file_interpretive and
 * cm_r31_interpretive (the order pairs). The LM's are in bank 37, but for
 * R31CALL and DSPDELAY in bank 40; the CM's all in bank 31. The faithful
 * port also has an older module of the LM's routines (c64-port/src/
 * lm_r31_routines.c), whose basic table the flight program tries first:
 * lm_r31_basic_routines here, at its keys. Its order pairs, which the
 * flight program tries only after the file's, are left out (an empty
 * table).
 *
 * Plumbing only. Each readable routine ends where its code leaves it (at
 * a TC out of the file, a TC INTPRET or a jump to another label), and the
 * older module's table runs just that. A key of the file's tables goes on
 * as the file's routine does: through each jump to another of its labels,
 * until control leaves the file (FINDVAC, BANKCALL, ENDEXT ...) or reaches
 * a TC INTPRET, which it follows into the interpreter as far as NEWOPS.
 * An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()), which ends at the pair's DANZIG,
 * at the EXIT, at the RTB, or where a CALL or GOTO leaves its table.
 *
 * The states the faithful routines take: FB the routine's bank, banks 31
 * and 37 with the superbank bit clear and bank 40 (FB 30) with the
 * superbank latch 0100; at an order pair Z at NEWOPS, BANKSET the BBANK,
 * INTBIT15 the bank's bit 15 and FIXLOC one of the VAC areas, as for the
 * CM's basic routines that go on to the interpreter. Always ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range
 * and every erasable word an ordinary one; the CM's NEWJOB, too, +0 or
 * positive. A run the readable code refuses part way leaves the state as
 * it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../r31/r31.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* Keys: the bank, and the address in it. */
#define KEY(bank, z) (uint16_t)((bank) * 1024u + ((z) - 02000u))
#if LUMINARY099
#define ROPE     0u   /* the LM's rope */
#define R31_BANK 037u /* the file's bank */
#else
#define ROPE     1u /* the CM's */
#define R31_BANK 031u
#endif
#define SUPERBANK_BIT 0100u
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define NORMUNX1_BANK ((word)(I_NORMUNX1 & 076000u))
#define VECTOR_MODE   077776u /* MODE -1 */

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
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

/* FB the bank's: banks 40 to 43 are FB 30 to 33 with the superbank bit,
 * and 30 to 37 without it. */
static bool in_bank(word bank)
{
    if (bank >= 040u)
        return FBANK == (word)((bank - 010u) << 10) && (*agc.superbank & 0160u) == SUPERBANK_BIT;
    return FBANK == (word)(bank << 10) && !(*agc.superbank & SUPERBANK_BIT);
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
#if !LUMINARY099
    if (NEWJOB > 037777u) return false;
#endif
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* FIXLOC a VAC area. (The readable interpreter itself takes NEWJOB only
 * +0 or a core set, and FIXLOC also a NOVAC job's work area, which the
 * faithful routines do not.) */
static bool sane_vac(void)
{
    return FIXLOC >= 0401u && FIXLOC <= 0661u && (FIXLOC - 0401u) % 054u == 0u && sane_banks();
}

/* ---- The basic code ---- */

/* Where a faithful file's routine runs on: from a jump to one of these
 * labels, into the routine there. */
static const struct {
    word bank, label;
    void (*routine)(void);
} jumps[] = {
#if LUMINARY099
    {040u, DSPDELAY, dspdelay},   {R31_BANK, DISPN5X, dispn5x}, {R31_BANK, REV83, rev83},
    {R31_BANK, GETRVN, getrvn},   {R31_BANK, R31SURF, r31surf}, {R31_BANK, MUNG_Q, mung},
    {R31_BANK, ONEBASE, onebase},
#else
    {R31_BANK, DSPDELAY, dspdelay},
    {R31_BANK, DISPN5X, dispn5x},
    {R31_BANK, DISPN5X + 6u, dispn5x_r34},
    {R31_BANK, V83, v83},
#endif
};

/* The faithful files' routines (lm_r31_file.c, cm_r31.c) go on through
 * each jump to another of their labels, and at a TC INTPRET into the
 * interpreter, as far as NEWOPS. */
static void run_on(void)
{
    unsigned i;

    for (;;) {
        if (Z == INTPRET) {
            intpret();
            return;
        }
        for (i = 0; i < COUNT(jumps); ++i)
            if (Z == jumps[i].label && in_bank(jumps[i].bank)) break;
        if (i == COUNT(jumps)) return;
        jumps[i].routine();
    }
}

/* The routine, and for a faithful file's key on as far as that file's
 * routine goes. A run the readable code refuses part way leaves the state
 * as it was. */
static enum exec_status run(void (*routine)(void), bool runs_on, apollo_overlay_state *state,
                            mission_io *io)
{
    apollo_overlay_state before = *state;
    mission_io kept = *io;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    if (runs_on) run_on();
    return EXEC_OK;
}

/* A basic routine of the file's tables runs from its key, whatever Z
 * holds (the trampoline calls it there). */
static enum exec_status basic(void (*routine)(void), word bank, bool vac,
                              const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(bank) || !(vac ? sane_vac() : sane_banks())) return EXEC_OUT_OF_SCOPE;
    return run(routine, true, state, io);
}

/* Each in its bank; VAC for those that check FIXLOC. */
#define BASIC(name, bank, vac)                                                                     \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return basic(name, bank, vac, program, state, device);                                     \
    }
#if LUMINARY099
BASIC(r31call, 040u, false)
BASIC(dspdelay, 040u, false)
BASIC(dspdelay_wake, 040u, false)
BASIC(r31_taskover, 040u, false)
BASIC(dispn5x, R31_BANK, false)
BASIC(dispn5x_terminate, R31_BANK, false)
BASIC(dispn5x_proceed, R31_BANK, false)
BASIC(dispn5x_recycle, R31_BANK, false)
BASIC(v83call, R31_BANK, false)
BASIC(rev83, R31_BANK, false)
BASIC(r31_after_exit, R31_BANK, false)
BASIC(getrvn_intpret, R31_BANK, false)
BASIC(getrvn_after_exit, R31_BANK, false)
BASIC(getrvn_after_exit_intpret, R31_BANK, false)
#else
BASIC(r31call, R31_BANK, false)
BASIC(dspdelay, R31_BANK, false)
BASIC(dspdelay_wake, R31_BANK, false)
BASIC(dispn5x_r34, R31_BANK, false)
BASIC(dispn5x_terminate, R31_BANK, false)
BASIC(dispn5x_proceed, R31_BANK, false)
BASIC(dispn5x_recycle, R31_BANK, false)
BASIC(v83, R31_BANK, true)
BASIC(v83call, R31_BANK, true)
BASIC(r31_after_exit, R31_BANK, true)
BASIC(r34los_after_exit, R31_BANK, true)
#endif
#undef BASIC

#if LUMINARY099
/* The LM's older module (lm_r31_routines.c), which the flight program
 * tries first at its keys: one routine at a time, at Z its key's and with
 * Q, too, an ordinary word. */
static enum exec_status older(void (*routine)(void), word bank, word label,
                              const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(bank) || Z != label || Q > 077777u || !sane_banks()) return EXEC_OUT_OF_SCOPE;
    return run(routine, false, state, io);
}

#define OLDER(name, bank, label)                                                                   \
    static enum exec_status older_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return older(name, bank, label, program, state, device);                                   \
    }
OLDER(dispn5x, R31_BANK, DISPN5X)
OLDER(dispn5x_terminate, R31_BANK, DISPN5X + 3u)
OLDER(dispn5x_proceed, R31_BANK, DISPN5X + 4u)
OLDER(dispn5x_recycle, R31_BANK, DISPN5X + 5u)
OLDER(v83call, R31_BANK, V83CALL)
OLDER(rev83, R31_BANK, REV83)
OLDER(r31_after_exit, R31_BANK, GETRVN - 010u)
OLDER(getrvn, R31_BANK, GETRVN)
OLDER(getrvn_intpret, R31_BANK, GETRVN + 2u)
OLDER(getrvn_after_exit, R31_BANK, GETRVN + 016u)
OLDER(getrvn_after_exit_intpret, R31_BANK, GETRVN + 020u)
OLDER(r31surf, R31_BANK, R31SURF)
OLDER(mung, R31_BANK, MUNG_Q)
OLDER(onebase, R31_BANK, ONEBASE)
OLDER(r31call, 040u, R31CALL)
OLDER(dspdelay, 040u, DSPDELAY)
OLDER(dspdelay_wake, 040u, DSPDELAY + 2u)
OLDER(r31_taskover, 040u, DSPDELAY + 012u)
#undef OLDER
#endif

/* ---- The order pairs ---- */

#if LUMINARY099
static const struct interpretive_code *const codes[] = {
    &V83CALL_CODE, &REV83_CODE, &GETRVN_CODE, &GETRVN_EXIT_CODE, &R31SURF_CODE, &ONEBASE_CODE};
#else
static const struct interpretive_code *const codes[] = {&V83_CODE, &V83CALL_CODE, &COMPDISP_CODE,
                                                        &STATEXTP_CODE, &R34LOS_CODE};
#endif

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

/* One pass; after RTB LOADTIME (or the LM's RTB NORMUNX1), the routine
 * and its DANZIG, on through the rest of the pair. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == LOADTIME && FBANK == LOADTIME_BANK) routine = loadtime;
#if LUMINARY099
    else if (Z == NORMUNX1 && FBANK == NORMUNX1_BANK) {
        if (MODE != VECTOR_MODE) return EXEC_OUT_OF_SCOPE; /* (the faithful takes a vector) */
        routine = normunx1;
    }
#endif
    else
        return EXEC_OK;
    if (!sane_vac()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    routine();
    return interpret_listing(program, state, io, code);
}

static enum exec_status pair(const agc_native_program *program, apollo_overlay_state *state,
                             void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at();
    if (Z != NEWOPS || !in_bank(R31_BANK) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane_vac())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each interpretive key: the word a pass starts at, in the bank. */
#define P(z) {KEY(R31_BANK, (z)), pair}
static const agc_routine_entry pairs[] = {
#if LUMINARY099
    /* V83CALL +11 ... */
    P(03132u),
    P(03134u),
    P(03136u),
    P(03140u),
    P(03142u),
    P(03144u),
    P(03145u),
    P(03147u),
    P(03151u),
    P(03153u),
    P(03154u),
    /* REV83 +11 ... */
    P(03166u),
    P(03170u),
    P(03172u),
    P(03175u),
    P(03177u),
    P(03201u),
    P(03205u),
    P(03207u),
    P(03211u),
    P(03213u),
    P(03215u),
    P(03217u),
    P(03222u),
    P(03223u),
    P(03226u),
    P(03230u),
    P(03232u),
    P(03236u),
    P(03240u),
    P(03243u),
    P(03246u),
    P(03250u),
    P(03252u),
    P(03254u),
    P(03255u),
    P(03257u),
    P(03261u),
    P(03263u),
    P(03265u),
    P(03267u),
    P(03271u),
    P(03274u),
    P(03277u),
    P(03300u),
    P(03303u),
    P(03304u),
    P(03306u),
    P(03311u),
    P(03314u),
    P(03315u),
    /* GETRVN +3 ... */
    P(03331u),
    P(03334u),
    P(03336u),
    P(03340u),
    P(03343u),
    /* GETRVN +21 ... */
    P(03347u),
    P(03352u),
    P(03354u),
    P(03355u),
    P(03357u),
    P(03361u),
    P(03363u),
    P(03365u),
    /* R31SURF +1 ... */
    P(03371u),
    P(03373u),
    P(03375u),
    /* ONEBASE +1 */
    P(03404u),
#else
    /* V83 +1, V83CALL +1 */
    P(02030u),
    P(02033u),
    /* COMPDISP ... */
    P(02035u),
    P(02040u),
    P(02041u),
    P(02042u),
    P(02044u),
    P(02045u),
    P(02047u),
    P(02052u),
    P(02053u),
    P(02054u),
    P(02056u),
    P(02060u),
    P(02062u),
    P(02064u),
    P(02070u),
    P(02072u),
    P(02074u),
    P(02076u),
    P(02100u),
    P(02103u),
    P(02105u),
    P(02107u),
    P(02111u),
    P(02113u),
    P(02114u),
    P(02116u),
    P(02121u),
    P(02124u),
    P(02125u),
    /* STATEXTP ... */
    P(02140u),
    P(02144u),
    P(02146u),
    P(02147u),
    P(02151u),
    P(02153u),
    P(02155u),
    P(02156u),
    P(02162u),
    P(02164u),
    P(02166u),
    P(02170u),
    P(02171u),
    P(02175u),
    P(02177u),
    P(02202u),
    P(02204u),
    P(02206u),
    P(02212u),
    P(02214u),
    P(02220u),
    P(02222u),
    P(02224u),
    P(02226u),
    P(02230u),
    P(02234u),
    P(02236u),
    P(02237u),
    P(02242u),
    P(02244u),
    P(02246u),
    P(02252u),
    P(02254u),
    P(02256u),
    P(02260u),
    P(02264u),
    P(02267u),
    P(02271u),
    P(02273u),
    P(02275u),
    P(02300u),
    P(02302u),
    P(02305u),
    P(02307u),
    P(02311u),
    /* R34LOS +15 ... */
    P(02326u),
    P(02330u),
#endif
};
#undef P

#define B(bank, z, name) {KEY(bank, z), basic_##name}
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
    B(R31_BANK, DISPN5X, dispn5x),
    B(R31_BANK, DISPN5X + 3u, dispn5x_terminate),
    B(R31_BANK, DISPN5X + 4u, dispn5x_proceed),
    B(R31_BANK, DISPN5X + 5u, dispn5x_recycle),
    B(R31_BANK, V83CALL, v83call),
    B(R31_BANK, REV83, rev83),
    B(R31_BANK, GETRVN - 010u, r31_after_exit),
    B(R31_BANK, GETRVN + 2u, getrvn_intpret),
    B(R31_BANK, GETRVN + 016u, getrvn_after_exit),
    B(R31_BANK, GETRVN + 020u, getrvn_after_exit_intpret),
    B(040u, R31CALL, r31call),
    B(040u, DSPDELAY, dspdelay),
    B(040u, DSPDELAY + 2u, dspdelay_wake),
    B(040u, DSPDELAY + 012u, r31_taskover),
#else
    B(R31_BANK, R31CALL, r31call),
    B(R31_BANK, DSPDELAY, dspdelay),
    B(R31_BANK, DSPDELAY + 3u, dspdelay_wake),
    B(R31_BANK, DISPN5X + 6u, dispn5x_r34),
    B(R31_BANK, DISPN5X + 011u, dispn5x_terminate),
    B(R31_BANK, DISPN5X + 012u, dispn5x_proceed),
    B(R31_BANK, DISPN5X + 013u, dispn5x_recycle),
    B(R31_BANK, V83, v83),
    B(R31_BANK, V83CALL, v83call),
    B(R31_BANK, V83 + 077u, r31_after_exit),
    B(R31_BANK, R34LOS + 1u, r34los_after_exit),
#endif
};
#undef B

#if LUMINARY099
const agc_routine_table lm_r31_file_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_r31_file_basic = {basic_keys, COUNT(basic_keys)};

/* The older module's basic routines, in its order; its order pairs are
 * left to the file's, which the flight program tries first. */
#define O(bank, z, name) {KEY(bank, z), older_##name}
static const agc_routine_entry older_keys[] = {
    O(R31_BANK, DISPN5X, dispn5x),
    O(R31_BANK, DISPN5X + 3u, dispn5x_terminate),
    O(R31_BANK, DISPN5X + 4u, dispn5x_proceed),
    O(R31_BANK, DISPN5X + 5u, dispn5x_recycle),
    O(R31_BANK, V83CALL, v83call),
    O(R31_BANK, REV83, rev83),
    O(R31_BANK, GETRVN - 010u, r31_after_exit),
    O(R31_BANK, GETRVN, getrvn),
    O(R31_BANK, GETRVN + 2u, getrvn_intpret),
    O(R31_BANK, GETRVN + 016u, getrvn_after_exit),
    O(R31_BANK, GETRVN + 020u, getrvn_after_exit_intpret),
    O(R31_BANK, R31SURF, r31surf),
    O(R31_BANK, MUNG_Q, mung),
    O(R31_BANK, ONEBASE, onebase),
    O(040u, R31CALL, r31call),
    O(040u, DSPDELAY, dspdelay),
    O(040u, DSPDELAY + 2u, dspdelay_wake),
    O(040u, DSPDELAY + 012u, r31_taskover),
};
#undef O
const agc_routine_table lm_r31_basic_routines = {older_keys, COUNT(older_keys)};
static const agc_routine_entry none[] = {{0xFFFFu, NULL}};
const agc_routine_table lm_r31_interpretive_routines = {none, 0u};
#else
const agc_routine_table cm_r31_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_r31_basic = {basic_keys, COUNT(basic_keys)};
#endif
