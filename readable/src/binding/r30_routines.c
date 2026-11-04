/* SPDX-License-Identifier: MIT */
/* The readable R30 (src/r30/) as routines of each mission's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_r30.c and cm_r30.c): lm_r30_basic and cm_r30_basic, the
 * basic code, and lm_r30_interpretive and cm_r30_interpretive, the order
 * pairs. The LM's in bank 22, the CM's in bank 23.
 *
 * Plumbing only. Each readable routine ends where its code leaves it (at a
 * TC out of the file, a TC INTPRET or a BZF ENDEXT); a TC INTPRET goes on
 * into the interpreter as far as NEWOPS, as the faithful routine does. An
 * order pair is one pass of the readable interpreter over the listing's
 * words (interpret_listing()), which ends at the pair's DANZIG, at the
 * EXIT, or where a CALL leaves its table.
 *
 * The states the faithful routines take: FB the file's bank (the
 * superbank latch any value in range), ZERO +0, the bank registers
 * agreeing, the interrupt and superbank latches in range and every
 * erasable word an ordinary one; NEWJOB +0 or positive (for the basic code
 * that neither enters the interpreter nor reads NEWJOB, -0 too); and where
 * the routine may enter the interpreter, FIXLOC one of the VAC areas or a
 * NOVAC job's (MPAC -36D). At an order pair, too, Z at NEWOPS, BANKSET the
 * BBANK and INTBIT15 the bank's bit 15. Both missions' routines take only
 * the latches from the io record. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <string.h>
#include "binding.h"
#include "rope_image.h"
#include "../r30/r30.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* Keys: the file's bank, and the address in it. */
#if LUMINARY099
#define R30_BANK 022u
#define ROPE     0u /* the LM's rope */
#else
#define R30_BANK 023u
#define ROPE     1u /* the CM's */
#endif
#define KEY(z)        (uint16_t)(R30_BANK * 1024u + ((z) - 02000u))
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

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

/* The work area: a VAC area's, or a NOVAC job's (MPAC -36D). */
static bool work_area(void)
{
    if (FIXLOC == ADDRESS_OF(MPAC) - 36u) return true;
    return FIXLOC >= VACNUSE_ADDRESS(1u) + 1u && FIXLOC <= VACNUSE_ADDRESS(5u) + 1u &&
           (FIXLOC - VACNUSE_ADDRESS(1u) - 1u) % 054u == 0u;
}

/* The states every routine takes (the faithful sane() and sane_nv()):
 * with the interpreter, NEWJOB +0 or positive and a work area; without
 * it, NEWJOB -0 too and any FIXLOC. */
static bool sane(bool interpreter)
{
    unsigned i;

    if (FBANK != (word)(R30_BANK << 10) || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u ||
        (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (NEWJOB > 037777u && (interpreter || NEWJOB != MINUS_ZERO)) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (interpreter && !work_area()) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The routine, then for a TC INTPRET on into the interpreter. */
static enum exec_status basic(void (*routine)(void), bool interpreter,
                              const agc_native_program *program, apollo_overlay_state *state,
                              void *device)
{
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!sane(interpreter)) return EXEC_OUT_OF_SCOPE;
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

/* A key whose routine may enter the interpreter or reads NEWJOB (I), and
 * one whose routine does neither (B). */
#define I(name)                                                                                    \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return basic(name, true, program, state, device);                                          \
    }
#define B(name)                                                                                    \
    static enum exec_status basic_##name(const agc_native_program *program,                        \
                                         apollo_overlay_state *state, void *device)                \
    {                                                                                              \
        return basic(name, false, program, state, device);                                         \
    }
I(v82call)
B(v82goff_options)
B(v82goff_terminate)
B(v82goff_proceed)
B(v82goff_recycle)
B(v82goff_ticktest)
B(v82goflp_relint)
B(v82stall_relint)
B(v82stall_wait)
B(v82stall_again)
B(flaggon)
B(flaggon_terminate)
B(flaggon_proceed)
B(flaggon_recycle)
I(v82goff1)
I(v82goff1_vehicle)
B(tickpart_tper)
B(tickpart_tff)
B(ticktest)
B(ticktest_end)
B(ticktest_taskover)
B(dotick_flags)
B(dotick_taskover)
B(tfftick)
B(v82gon_findvac)
I(v82gon_relint)
B(v82redsp)
B(v82redsp_terminate)
B(v82redsp_proceed)
I(v82redsp_recycle)
I(v82gon1)
I(v82gon3_again)
#if LUMINARY099
B(v82gon3)
#else
I(bothpad_modreg)
B(v82gon2_checkmm)
I(v82gon2_not_11)
I(candel)
I(splret_modreg)
#endif
#undef I
#undef B

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    &V82CALL_CODE, &V82GOFF1_CODE, &OTHSHIP_CODE, &THISSHIP_CODE, &EARTHPAD_CODE,
#if !LUMINARY099
    &SPLRET1_CODE,
#endif
    &TICKTFF_CODE, &V82GON_CODE,   &V82GON1_CODE,
#if !LUMINARY099
    &CANDEL_CODE,
#endif
    &SR30_1_CODE,
};

/* The table LOC is in, or none. */
static const struct interpretive_code *code_at(void)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (LOC >= codes[i]->first && LOC < codes[i]->first + codes[i]->count) return codes[i];
    return NULL;
}

/* One pass; after RTB LOADTIME, the routine and its DANZIG, on through
 * the rest of the pair. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
}

/* BOTHSHIP's DLOAD*s by X2 ("1/RTMUE,2", "MINPERE,2"): the faithful port
 * carries only the two words each can read, the earth's (X2 +0 or -0) and
 * the moon's (X2 2), and refuses a load from anywhere else; so does the
 * binding. */
static bool outside_constants(void)
{
    word bothship = (word)(I_BOTHSHIP % 02000u + 02000u), x2;

    if (LOC != bothship + 5u && LOC != bothship + 010u) return false;
    x2 = agc.memory[(FIXLOC + I_X2) & 03777u];
    return x2 != PLUS_ZERO && x2 != MINUS_ZERO && x2 != 2u;
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
    if (Z != NEWOPS || !code || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) || !sane(true) ||
        outside_constants())
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

/* Each interpretive key: the word a pass starts at. */
#define P(z) {KEY(z), pair}
static const agc_routine_entry pairs[] = {
#if LUMINARY099
    P(03243u), P(03247u),                       /* V82CALL +1, V82GOFF */
    P(03322u), P(03324u), P(03325u), P(03326u), /* V82GOFF1 +1 */
    P(03334u), P(03336u), P(03340u), P(03342u), P(03343u), P(03345u), P(03346u), P(03350u),
    P(03351u), P(03354u), /* OTHSHIP +1, BOTHSHIP */
    P(03357u), P(03361u), /* THISSHIP +1 */
    P(03367u), P(03373u), P(03375u), P(03377u), P(03401u), P(03404u), P(03405u), P(03410u),
    P(03413u), P(03414u),            /* EARTHPAD ... TICKTPER */
    P(03420u), P(03423u), P(03424u), /* TICKTFF */
    P(03465u),                       /* V82GON */
    P(03504u), P(03507u), P(03511u), P(03512u), P(03516u), P(03521u), P(03523u), P(03525u),
    P(03527u), P(03532u), P(03534u), P(03536u), P(03540u), /* V82GON1 +1 ... V82GON2 */
    P(03551u), P(03554u), P(03560u), P(03561u), P(03563u), P(03564u), P(03565u), P(03567u),
    P(03571u), P(03573u), P(03576u), P(03600u), P(03602u), P(03604u), P(03605u), P(03610u),
    P(03612u), P(03614u), P(03617u), P(03622u), P(03625u), P(03626u), P(03630u), P(03633u),
    P(03634u), P(03636u), P(03641u), P(03643u), /* SR30.1 ... MAXCHK */
#else
    P(02333u), P(02337u),                       /* V82CALL +1, V82GOFF */
    P(02412u), P(02414u), P(02415u), P(02416u), /* V82GOFF1 +1 */
    P(02424u), P(02426u), P(02430u), P(02432u), P(02433u), P(02435u), P(02436u), P(02440u),
    P(02441u), P(02444u),                                             /* OTHSHIP +1, BOTHSHIP */
    P(02447u), P(02451u),                                             /* THISSHIP +1 */
    P(02463u), P(02467u), P(02471u), P(02473u), P(02475u),            /* EARTHPAD ... BOTHPAD */
    P(02502u), P(02505u), P(02506u), P(02511u), P(02514u), P(02515u), /* SPLRET1 +1 */
    P(02521u), P(02524u), P(02525u),                                  /* TICKTFF */
    P(02566u),                                                        /* V82GON */
    P(02605u), P(02610u), P(02612u), P(02613u), P(02617u), P(02622u), P(02624u), P(02626u),
    P(02630u), P(02633u), P(02635u), P(02637u), P(02641u), /* V82GON1 +1 ... V82GON2 */
    P(02646u), P(02650u), P(02653u),                       /* CANDEL +1, SPLRET */
    P(02667u), P(02672u), P(02676u), P(02677u), P(02701u), P(02702u), P(02703u), P(02705u),
    P(02707u), P(02711u), P(02714u), P(02716u), P(02720u), P(02722u), P(02723u), P(02726u),
    P(02730u), P(02732u), P(02735u), P(02740u), P(02743u), P(02744u), P(02746u), P(02751u),
    P(02752u), P(02754u), P(02757u), P(02761u), /* SR30.1 ... MAXCHK */
#endif
};
#undef P

#define K(z, name) {KEY(z), basic_##name}
static const agc_routine_entry basic_keys[] = {
    K(V82CALL, v82call),
    K(V82GOFF + 1u, v82goff_options),
    K(V82GOFF + 8u, v82goff_terminate),
    K(V82GOFF + 9u, v82goff_proceed),
    K(V82GOFF + 10u, v82goff_recycle),
    K(V82GOFF + 11u, v82goff_ticktest),
    K(V82GOFF + 15u, v82goflp_relint),
    K(V82GOFLP + 8u, v82stall_relint),
    K(V82STALL + 4u, v82stall_wait),
    K(V82STALL + 7u, v82stall_again),
    K(FLAGGON, flaggon),
    K(FLAGGON + 3u, flaggon_terminate),
    K(FLAGGON + 4u, flaggon_proceed),
    K(FLAGGON + 5u, flaggon_recycle),
    K(V82GOFF1, v82goff1),
    K(V82GOFF1 + 6u, v82goff1_vehicle),
#if !LUMINARY099
    K(BOTHPAD + 3u, bothpad_modreg),
#endif
    K(TICKTPER + 5u, tickpart_tper),
    K(TICKTFF + 5u, tickpart_tff),
    K(TICKTEST, ticktest),
    K(TICKTEST + 4u, ticktest_end),
    K(TICKTEST + 8u, ticktest_taskover),
    K(DOTICK + 4u, dotick_flags),
    K(DOTICK + 8u, dotick_taskover),
    K(TFFTICK, tfftick),
    K(V82GON + 1u, v82gon_findvac),
    K(V82GON + 5u, v82gon_relint),
    K(V82REDSP, v82redsp),
    K(V82REDSP + 3u, v82redsp_terminate),
    K(V82REDSP + 4u, v82redsp_proceed),
    K(V82REDSP + 5u, v82redsp_recycle),
    K(V82GON1, v82gon1),
#if LUMINARY099
    K(V82GON3, v82gon3),
#else
    K(V82GON2 + 3u, v82gon2_checkmm),
    K(V82GON2 + 5u, v82gon2_not_11),
    K(CANDEL, candel),
    K(CANDEL + 7u, splret_modreg), /* SPLRET +1 */
#endif
    K(V82GON3 + 7u, v82gon3_again),
};
#undef K

#if LUMINARY099
const agc_routine_table lm_r30_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table lm_r30_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_r30_interpretive = {pairs, COUNT(pairs)};
const agc_routine_table cm_r30_basic = {basic_keys, COUNT(basic_keys)};
#endif
