/* SPDX-License-Identifier: MIT */
/* The readable P38-P39 (src/stable_orbit/) as routines of each mission's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_stable_orbit.c and cm_stable_orbit.c):
 * lm_stable_orbit_basic and cm_stable_orbit_basic, the basic code, and
 * lm_stable_orbit_interpretive and cm_stable_orbit_interpretive, the order
 * pairs. The LM's in bank 34, the CM's in bank 4.
 *
 * Plumbing only. Each readable routine ends where its code leaves it (at a
 * TC out of the file, a TCF out of it, a TC INTPRET or the CM's TC RTRN); a
 * TC INTPRET goes on into the interpreter as far as NEWOPS, as the faithful
 * routine does. An order pair is one pass of the readable interpreter over
 * the listing's words (interpret_listing()), which ends at the pair's
 * DANZIG, at the EXIT, or where a CALL or GOTO leaves its table. The LM's
 * RTB LOADTIME runs the routine and goes on through the pair; the CM's
 * stops there, and its DANZIG (with DAD waiting in EDOP) is a key of its
 * own.
 *
 * The states the faithful routines take: FB the file's bank (the LM's with
 * the superbank bit clear), ZERO +0, the bank registers agreeing, the
 * interrupt and superbank latches in range and every erasable word an
 * ordinary one; for all the LM's routines and the CM's order pairs, NEWJOB
 * +0 or positive and FIXLOC one of the VAC areas too. At an order pair, too,
 * Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. The CM's
 * RTRN runs under any FB but banks 40-43, and only the four words the
 * file's TC VNDSPLYs leave there. Both missions' routines take only the
 * latches from the io record. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <string.h>
#include "agc_transfer.h"
#include "binding.h"
#include "rope_image.h"
#include "../stable_orbit/stable_orbit.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* Keys: the file's bank, and the address in it. */
#if LUMINARY099
#define STABLE_ORBIT_BANK 034u
#define ROPE              0u   /* the LM's rope */
#define BASIC_IN_A_JOB    true /* the LM's basic code takes a job's state */
#else
#define STABLE_ORBIT_BANK 04u
#define ROPE              1u /* the CM's */
#define BASIC_IN_A_JOB    false
#endif
#define KEY(z)        (uint16_t)(STABLE_ORBIT_BANK * 1024u + ((z) - 02000u))
#define FILE_FB       ((word)(STABLE_ORBIT_BANK << 10))
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define SUPERBANK_BIT 0100u /* channel 7 bit 7 */
/* A label's address in its bank (02000-03777). */
#define IN_BANK(label) ((word)(I_##label % 02000u + 02000u))

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

/* The work area: a VAC area's. */
static bool work_area(void)
{
    return FIXLOC >= VACNUSE_ADDRESS(1u) + 1u && FIXLOC <= VACNUSE_ADDRESS(5u) + 1u &&
           (FIXLOC - VACNUSE_ADDRESS(1u) - 1u) % 054u == 0u;
}

/* The states every routine takes (the faithful sane() and basic_sane()):
 * with a job, NEWJOB +0 or positive and a VAC area; without (the CM's
 * basic code), any. */
static bool sane(bool job)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
#if !LUMINARY099
    /* Nor, for the CM, an FB of banks 30-37 with the superbank bit set
     * (banks 40-43): the faithful port takes none there. */
    if (FBANK >= 060000u && (*agc.superbank & SUPERBANK_BIT)) return false;
#endif
    if (job && (NEWJOB > 037777u || !work_area())) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* FB the file's bank: the LM's bank 34 is FB 34 with the superbank bit
 * clear. */
static bool in_file_bank(void)
{
#if LUMINARY099
    return FBANK == FILE_FB && !(*agc.superbank & SUPERBANK_BIT);
#else
    return FBANK == FILE_FB;
#endif
}

/* ---- The basic code ---- */

/* The routine, then for a TC INTPRET on into the interpreter. The LM's
 * basic code takes a job's state, the CM's any. */
static enum exec_status run_basic(void (*routine)(void), bool any_fb,
                                  const agc_native_program *program, apollo_overlay_state *state,
                                  void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if ((!any_fb && !in_file_bank()) || !sane(BASIC_IN_A_JOB)) return EXEC_OUT_OF_SCOPE;
    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    if (Z == INTPRET) intpret();
    return EXEC_OK;
}

#define B(name)                                                                                    \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return run_basic(name, false, program, state, device);                                     \
    }
B(p38)
B(p38_active)
B(p78)
B(p78_active)
B(p38_tig)
B(p38_centang)
B(p38_centang_terminate)
B(p38_centang_proceed)
B(p38_centang_recycle)
B(p38_centang_blank)
B(p38_centang_end)
B(p38_option_terminate)
B(p38_option_proceed)
B(p38_option_recycle)
B(p38_option_blank)
B(p38_option_end)
B(p38_deltar)
B(p38_deltar_terminate)
B(p38_deltar_proceed)
B(p38_deltar_recycle)
B(p38_deltar_blank)
B(p38_deltar_end)
B(p38_intercept_done)
B(p39p79_exit)
B(dsply58_exit)
B(dsply81)
B(dsply81_done)
B(p39)
B(p39_active)
B(p79)
B(p79_active)
B(p39_p79a_done)
B(vndsply_terminate)
B(vndsply_proceed)
B(vndsply_recycle)
#undef B

#if !LUMINARY099
/* The CM's RTRN (E4,1767), where TC RTRN leaves Z: the faithful port runs
 * only the words the file's four TC VNDSPLYs leave there, each a TC to the
 * word after its TC VNDSPLY (P78 +3, DELTA R's display +10, DSPLY58 +2 and
 * DSPLY81 +1). */
#define RTRN_KEY (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + 02367u)
static bool a_vndsply_return(word instruction)
{
    return instruction == P78 + 4u || instruction == DELTAR_DISPLAY + 013u ||
           instruction == DSPLY58 + 3u || instruction == DSPLY81 + 2u;
}

static enum exec_status basic_rtrn(const agc_native_program *program, apollo_overlay_state *state,
                                   void *device)
{
    if (!program || !state || !device) return EXEC_OUT_OF_SCOPE;
    bind(program, state, device);
    if (Z != ADDRESS_OF(RTRN) || EBANK != 02000u || !sane(false) ||
        !a_vndsply_return(agc.memory[02367u]))
        return EXEC_OUT_OF_SCOPE;
    return run_basic(rtrn, true, program, state, device);
}
#endif

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    &P38_CODE, &JUNCTN1_CODE, &MAINRTN1_CODE, &DSPLY81_CODE, &P39_CODE,
};

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (LOC >= codes[i]->first && LOC < codes[i]->first + codes[i]->count) return codes[i];
    return NULL;
}

/* One pass; for the LM, after RTB LOADTIME, the routine and its DANZIG, on
 * through the rest of the pair. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
#if LUMINARY099
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
#else
    return EXEC_OK;
#endif
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
    if (Z != NEWOPS || !code || !in_file_bank() || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
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

#if !LUMINARY099
/* The CM's P39/P79B: its RTB LOADTIME's DANZIG, with DAD waiting in EDOP
 * (its code plus one) and LOC at the RTB's operand, resumes the pass there.
 * The state the faithful port takes: Z at DANZIG, BANKSET the file's bank
 * with no E-bank bits, INTBIT15 its bit 15 (FB any); its pair takes LOC to
 * be there, which the binding asks. */
static enum exec_status pending_dad(const agc_native_program *program, apollo_overlay_state *state,
                                    void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != DANZIG || EDOP != ORDER_DAD + 1u || LOC != IN_BANK(P39_P79B) + 1u ||
        (BANKSET & 076000u) != FILE_FB || (BANKSET & 01770u) || INTBIT15 != (FILE_FB & 040000u) ||
        !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, &P39_CODE);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}
#endif

/* ---- The keys, in the faithful tables' order ---- */

/* Each interpretive key: the word a pass starts at. */
#define P(z) {KEY(z), pair}
static const agc_routine_entry pairs[] = {
#if LUMINARY099
    P(03333u), P(03335u), P(03340u), P(03341u), P(03344u), P(03347u), P(03351u), P(03354u),
    P(03355u), /* P38_CODE */
    P(03375u), P(03400u), P(03402u), P(03406u), P(03410u), P(03412u), P(03414u), P(03416u),
    P(03420u), P(03422u), P(03424u), P(03427u), P(03430u), P(03433u), P(03434u), P(03436u),
    P(03440u), P(03441u), P(03445u), P(03447u), P(03454u), P(03456u), /* JUNCTN1 ... P39P79 */
    P(03460u), P(03462u), P(03464u), P(03466u), P(03470u), P(03473u), P(03475u), P(03476u),
    P(03502u),                       /* MAINRTN1 ... DSPLY58 */
    P(03510u), P(03513u), P(03515u), /* DSPLY81 +3 */
    P(03536u), P(03541u), P(03544u), P(03545u), P(03547u), P(03551u), P(03554u), P(03557u),
    P(03561u), P(03564u), P(03566u), P(03567u), P(03570u), P(03572u), P(03574u), P(03576u),
    P(03600u), P(03602u), P(03605u), P(03611u), P(03613u), P(03615u), P(03617u), /* P39_CODE */
#else
    P(02777u), P(03002u), P(03004u),
    P(03007u), P(03010u), P(03013u),
    P(03016u), P(03020u), P(03023u),
    P(03024u), P(03025u), /* P38_CODE */
    P(03042u), P(03045u), P(03047u),
    P(03053u), P(03055u), P(03057u),
    P(03061u), P(03063u), P(03065u),
    P(03067u), P(03071u), P(03074u),
    P(03075u), P(03100u), P(03101u),
    P(03103u), P(03105u), P(03106u),
    P(03112u), P(03114u), P(03121u),
    P(03123u), /* JUNCTN1 ... P39P79 */
    P(03125u), P(03127u), P(03131u),
    P(03133u), P(03135u), P(03140u),
    P(03142u), P(03143u), P(03147u), /* MAINRTN1 ... DSPLY58 */
    P(03155u), P(03160u), P(03162u), /* DSPLY81 +3 */
    P(03200u), P(03203u), {KEY(03204u), pending_dad},
    P(03206u), P(03207u), P(03211u),
    P(03213u), P(03216u), P(03221u),
    P(03223u), P(03226u), P(03230u),
    P(03231u), P(03232u), P(03234u),
    P(03236u), P(03240u), P(03242u),
    P(03244u), P(03247u), P(03253u),
    P(03255u), P(03257u), P(03261u), /* P39_CODE */
#endif
};
#undef P

#define K(z, name) {KEY(z), basic_##name}
static const agc_routine_entry basic_keys[] = {
    K(P38, p38),
#if LUMINARY099
    K(P38 + 2u, p38_active),
    K(P78, p78),
    K(P78 + 2u, p78_active),
    K(P78 + 4u, p38_tig),
#else
    K(P38 + 1u, p38_active),
    K(P78, p78),
    K(P78 + 1u, p78_active),
    K(P78 + 2u, p38_tig),
#endif
    K(CENTANG_DISPLAY, p38_centang),
    K(CENTANG_DISPLAY + 3u, p38_centang_terminate),
    K(CENTANG_DISPLAY + 4u, p38_centang_proceed),
    K(CENTANG_DISPLAY + 5u, p38_centang_recycle),
    K(CENTANG_DISPLAY + 6u, p38_centang_blank),
    K(CENTANG_DISPLAY + 010u, p38_centang_end),
    K(OPTION_DISPLAY + 3u, p38_option_terminate),
    K(OPTION_DISPLAY + 4u, p38_option_proceed),
    K(OPTION_DISPLAY + 5u, p38_option_recycle),
    K(OPTION_DISPLAY + 6u, p38_option_blank),
    K(OPTION_DISPLAY + 010u, p38_option_end),
    K(DELTAR_DISPLAY, p38_deltar),
    K(DELTAR_DISPLAY + 3u, p38_deltar_terminate),
    K(DELTAR_DISPLAY + 4u, p38_deltar_proceed),
    K(DELTAR_DISPLAY + 5u, p38_deltar_recycle),
    K(DELTAR_DISPLAY + 6u, p38_deltar_blank),
    K(DELTAR_DISPLAY + 010u, p38_deltar_end),
    K(JUNCTN1 - 1u, p38_intercept_done),
    K(P39P79 + 1u, p39p79_exit),
    K(DSPLY58 + 1u, dsply58_exit),
    K(DSPLY81, dsply81),
    K(DSPLY81 + 2u, dsply81_done),
    K(P39, p39),
#if LUMINARY099
    K(P39 + 2u, p39_active),
    K(P79, p79),
    K(P79 + 2u, p79_active),
    K(P39_P79A + 3u, p39_p79a_done),
#else
    K(P39 + 1u, p39_active),
    K(P79, p79),
    K(P79 + 1u, p79_active),
    K(P39_P79A + 2u, p39_p79a_done),
#endif
    K(VNDSPLY + 6u, vndsply_terminate),
    K(VNDSPLY + 7u, vndsply_proceed),
    K(VNDSPLY + 010u, vndsply_recycle),
#if !LUMINARY099
    {RTRN_KEY, basic_rtrn},
#endif
};
#undef K

#if LUMINARY099
const agc_routine_table lm_stable_orbit_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_stable_orbit_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_stable_orbit_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_stable_orbit_basic = {basic_keys, COUNT(basic_keys)};
#endif
