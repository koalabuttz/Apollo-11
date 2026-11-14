/* SPDX-License-Identifier: MIT */
/* The readable RTB_OP_CODES (RT8_OP_CODES in the CM; src/rtb_op_codes/)
 * as routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_rtb_op_codes.c and
 * cm_rtb_op_codes.c): <m>_rtb_op_codes_basic, every routine an RTB or a
 * BOVB reaches and every word a call returns to (24 keys in the LM, 32 in
 * the CM), and <m>_rtb_op_codes_interpretive, the CM's QUALITY1 and
 * QUALITY2 (12 order pairs; the LM's table is empty).
 *
 * Plumbing only. A basic key runs its routine until control leaves the
 * file (SLOAD2, NEWMODE, VMODE, DANZIG, TPAGREE, BANKCALL, POSTJUMP, or a
 * return through Q). An order pair is one pass of the readable
 * interpreter over QUALITY1's or QUALITY2's words.
 *
 * The states the faithful routines take: FB the file's bank (an order
 * pair's for QUALITY1 and QUALITY2), ZERO +0, the bank registers
 * agreeing, NEWJOB +0 or a core set, the interrupt and superbank latches
 * in range and every erasable word an ordinary one; FIXLOC one of the VAC
 * areas for CDUINC and PULSEIMU, which reach X1 of the work area (CDUINC
 * when an overflow runs it on into PULSEIMU), for the CM's INCRCDUS up to
 * its last CDUINC, and for the order pairs; at
 * an order pair also Z at NEWOPS, LOC the key's word, BANKSET the BBANK
 * and INTBIT15 the bank's bit 15. A run the readable code refuses part
 * way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"
#include "../agc/names.h"

#if LUMINARY099
#include "lm_rtb_op_codes.h"
#define ROPE 0u /* the LM's rope */
#else
#include "cm_rtb_op_codes.h"
#define ROPE 1u /* the CM's */
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define RTB_BANK           (I_LOADTIME >> 10)
#define IN_FILE(address)   KEY(RTB_BANK, address)

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* 1TO2SUB's and CDUINC's INDEX A; CAF LIMITS read the rope. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    if (!bind_machine(program, state, io)) return false;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    return true;
}

/* The states the faithful routines take (their sane(), and sane_nv(),
 * which leaves out the work area). */
static bool sane(bool work_area)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (work_area && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic keys ---- */

static void cduinc_key(void)
{
    (void)cduinc();
}

static enum exec_status basic(const agc_native_program *program, apollo_overlay_state *state,
                              void *device, bool work_area, void (*routine)(void))
{
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != (word)(RTB_BANK << 10) || !sane(work_area)) return EXEC_OUT_OF_SCOPE;

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

#define BASIC(name, work_area)                                                                     \
    static enum exec_status key_##name(const agc_native_program *program,                          \
                                       apollo_overlay_state *state, void *device)                  \
    {                                                                                              \
        return basic(program, state, device, work_area, name);                                     \
    }
BASIC(loadtime, false)
BASIC(cdulogic, false)
BASIC(sgnagree, false)
BASIC(sgnagree_after_tpagree, false)
BASIC(one_sto2s, false)
BASIC(one_sto2s_plus1, false)
BASIC(v1sto2s, false)
BASIC(v1sto2s_plus1, false)
BASIC(v1sto2s_plus4, false)
BASIC(v1sto2s_plus8, false)
BASIC(tpmode, false)
BASIC(two_v1sto2s, false)
BASIC(two_v1sto2s_plus1, false)
BASIC(two_v1sto2s_plus4, false)
BASIC(cduinc_key, true)
BASIC(pulseimu, true)
BASIC(pulseimu_after_imupulse, false)
BASIC(signmpac, false)
BASIC(dpmode, false)
BASIC(normunx1, false)
BASIC(normunit, false)
BASIC(normunit_after_vecagree, false)
BASIC(vecsgnag, false)
BASIC(vecsgnag_after_vecagree, false)
#if !LUMINARY099
BASIC(readpips, false)
BASIC(vecmode, false)
BASIC(incrcdus, true)
BASIC(incrcdus_plus4, true)
BASIC(incrcdus_plus7, true)
BASIC(incrcdus_plus10, false)
BASIC(transp1, false)
BASIC(transp2, false)
#endif

/* By physical address, as the faithful table has them. */
static const agc_routine_entry basic_keys[] = {
    {IN_FILE(LOADTIME), key_loadtime},
    {IN_FILE(CDULOGIC), key_cdulogic},
#if !LUMINARY099
    {IN_FILE(READPIPS), key_readpips},
    {IN_FILE(VECMODE), key_vecmode},
#endif
    {IN_FILE(SGNAGREE), key_sgnagree},
    {IN_FILE(SGNAGREE + 1u), key_sgnagree_after_tpagree},
    {IN_FILE(ONE_STO2S), key_one_sto2s},
    {IN_FILE(ONE_STO2S + 1u), key_one_sto2s_plus1},
    {IN_FILE(V1STO2S), key_v1sto2s},
    {IN_FILE(V1STO2S + 1u), key_v1sto2s_plus1},
    {IN_FILE(V1STO2S + 4u), key_v1sto2s_plus4},
    {IN_FILE(V1STO2S + 010u), key_v1sto2s_plus8},
    {IN_FILE(TPMODE), key_tpmode},
    {IN_FILE(TWO_V1STO2S), key_two_v1sto2s},
    {IN_FILE(TWO_V1STO2S + 1u), key_two_v1sto2s_plus1},
    {IN_FILE(TWO_V1STO2S + 4u), key_two_v1sto2s_plus4},
#if !LUMINARY099
    {IN_FILE(INCRCDUS), key_incrcdus},
    {IN_FILE(INCRCDUS + 4u), key_incrcdus_plus4},
    {IN_FILE(INCRCDUS + 7u), key_incrcdus_plus7},
    {IN_FILE(INCRCDUS + 012u), key_incrcdus_plus10},
#endif
    {IN_FILE(CDUINC), key_cduinc_key},
    {IN_FILE(PULSEIMU), key_pulseimu},
    {IN_FILE(PULSEIMU + 4u), key_pulseimu_after_imupulse},
#if !LUMINARY099
    {IN_FILE(TRANSP1), key_transp1},
    {IN_FILE(TRANSP2), key_transp2},
#endif
    {IN_FILE(SIGNMPAC), key_signmpac},
    {IN_FILE(DPMODE), key_dpmode},
    {IN_FILE(NORMUNX1), key_normunx1},
    {IN_FILE(NORMUNIT), key_normunit},
    {IN_FILE(NORMUNIT + 5u), key_normunit_after_vecagree},
    {IN_FILE(VECSGNAG), key_vecsgnag},
    {IN_FILE(VECSGNAG + 2u), key_vecsgnag_after_vecagree},
};

/* ---- The CM's order pairs, QUALITY1 and QUALITY2 ---- */

#if !LUMINARY099
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    static const struct interpretive_code *const codes[] = {&QUALITY2_CODE, &QUALITY1_CODE};
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    enum exec_status status;

    if (!code || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || LOC != at || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
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

#define QUALITY1_BANK (I_QUALITY1 >> 10)
#define QUALITY2_BANK (I_QUALITY2 >> 10)
/* Their first words, in the bank. */
#define QUALITY1_Z (02000u + (I_QUALITY1 & 01777u))
#define QUALITY2_Z (02000u + (I_QUALITY2 & 01777u))
#define PAIR(bank, label, offset)                                                                  \
    static enum exec_status pair_##label##_##offset(const agc_native_program *program,             \
                                                    apollo_overlay_state *state, void *device)     \
    {                                                                                              \
        return pair(KEY(bank, label##_Z + offset##u), program, state, device);                     \
    }
PAIR(QUALITY2_BANK, QUALITY2, 0)
PAIR(QUALITY2_BANK, QUALITY2, 2)
PAIR(QUALITY2_BANK, QUALITY2, 3)
PAIR(QUALITY2_BANK, QUALITY2, 6)
PAIR(QUALITY2_BANK, QUALITY2, 8)
PAIR(QUALITY2_BANK, QUALITY2, 10)
PAIR(QUALITY2_BANK, QUALITY2, 12)
PAIR(QUALITY2_BANK, QUALITY2, 14)
PAIR(QUALITY2_BANK, QUALITY2, 15)
PAIR(QUALITY2_BANK, QUALITY2, 17)
PAIR(QUALITY1_BANK, QUALITY1, 0)
PAIR(QUALITY1_BANK, QUALITY1, 4)
#undef PAIR

#define PAIR(bank, label, offset) {KEY(bank, label##_Z + offset##u), pair_##label##_##offset}
static const agc_routine_entry pair_keys[] = {
    PAIR(QUALITY2_BANK, QUALITY2, 0),  PAIR(QUALITY2_BANK, QUALITY2, 2),
    PAIR(QUALITY2_BANK, QUALITY2, 3),  PAIR(QUALITY2_BANK, QUALITY2, 6),
    PAIR(QUALITY2_BANK, QUALITY2, 8),  PAIR(QUALITY2_BANK, QUALITY2, 10),
    PAIR(QUALITY2_BANK, QUALITY2, 12), PAIR(QUALITY2_BANK, QUALITY2, 14),
    PAIR(QUALITY2_BANK, QUALITY2, 15), PAIR(QUALITY2_BANK, QUALITY2, 17),
    PAIR(QUALITY1_BANK, QUALITY1, 0),  PAIR(QUALITY1_BANK, QUALITY1, 4),
};
#undef PAIR
#endif

#if LUMINARY099
const agc_routine_table lm_rtb_op_codes_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table lm_rtb_op_codes_interpretive = {NULL, 0};
#else
const agc_routine_table cm_rtb_op_codes_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_rtb_op_codes_interpretive = {pair_keys, COUNT(pair_keys)};
#endif
