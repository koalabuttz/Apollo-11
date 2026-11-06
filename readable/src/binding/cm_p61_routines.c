/* SPDX-License-Identifier: MIT */
/* The readable P61-P67 of the CM (src/p61_p67/) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p61_p67.c): cm_p61_p67_basic (the basic code) and
 * cm_p61_p67_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_p61_keys.inc, which tools/p20_p25_keys.py writes from them). The
 * file is written part by part: a key a part has written runs the
 * readable routine, any other the faithful port's (renamed faithful_).
 * The side-by-side test (tests/cm_p61_cases.c) checks which.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, NEWMODEX, ENDOFJOB, GOTOPOOH, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()). One
 * basic key is in erasable memory: 60GENRET (E6,1773), where S61.1 keeps
 * its return and RETRN1 sends control (TC 60GENRET).
 *
 * The device is the CM's io record (cm_native_io); the file reads and
 * writes no channel, but its basic routines run only in a program with
 * its hardware record, as the faithful ones. The states the faithful
 * routines take: at a basic key, FB the key's bank (at 60GENRET, any),
 * ZERO +0, the bank registers agreeing, the interrupt and superbank
 * latches in range and every erasable word an ordinary one; at an order
 * pair also NEWJOB +0 or a core set, FIXLOC one of the VAC areas, Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. A run the
 * readable code refuses part way leaves the state and io record as they
 * were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p61_p67.h"
#include "../p61_p67/p61_p67.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads and writes no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled();
}

/* The interpreter's fixed operands, and the file's constants, are the
 * rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_native_io *io)
{
    bind_machine(program, state, io);
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful basic routines take (its basic_sane()). */
static bool basic_sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FBANK >= 060000u && (*agc.superbank & 0100u)) return false; /* (60GENRET's) */
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* And the order pairs (its sane()): NEWJOB and FIXLOC too. */
static bool sane(void)
{
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    return basic_sane();
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P61 and P62 (p61_p62.c) */
    {KEY(026u, P61), p61},
    {KEY(026u, P61 + 5u), p61_display},
    {KEY(026u, P61 + 010u), p61_terminate},
    {KEY(026u, P61 + 011u), p61_proceed},
    {KEY(026u, P61 + 012u), p61_enter},
    {KEY(026u, P61_3), p61_3},
    {KEY(026u, P61_3 + 2u), p61_3_after_phaschng},
    {KEY(026u, P61_1), p61_1},
    {KEY(026u, P61_1 + 1u), p61_1_display},
    {KEY(026u, P61_1 + 4u), p61_1_terminate},
    {KEY(026u, P61_1 + 5u), p61_1_proceed},
    {KEY(026u, P61_1 + 6u), p61_1_enter},
    {KEY(026u, P61_2 + 010u), p61_2_display},
    {KEY(026u, P61_2 + 013u), p61_2_terminate},
    {KEY(026u, P61_2 + 014u), p61_2_proceed},
    {KEY(026u, P61_2 + 015u), p61_2_redo},
    {KEY(026u, P62 - 2u), p62_from_p61},
    {KEY(026u, P62), p62},
    {KEY(026u, P62 + 1u), p62_after_s61_1},
    {KEY(026u, P62_2), p62_2},
    {KEY(026u, P62_2 + 6u), p62_2_terminate},
    {KEY(026u, P62_2 + 7u), p62_2_proceed},
    {KEY(026u, P62_2 + 010u), p62_2_enter},
    {KEY(026u, P62_2 + 011u), p62_2_immediate_return},
    {KEY(026u, P62_1), p62_1},
    {KEY(026u, P62_1 + 3u), p62_1_terminate},
    {KEY(026u, P62_1 + 4u), p62_1_proceed},
    {KEY(026u, P62_1 + 5u), p62_1_enter},
    {KEY(026u, P62_1 + 010u), p62_1_after_phaschng},
    {KEY(026u, P62_1 + 025u), p62_1_after_upflag},
    {KEY(026u, WAKEP62), wakep62},
    {KEY(026u, WAKEP62 + 4u), wakep62_after_novac},
    /* Part 1: P63 to P67 (p63_p67.c) */
    {KEY(026u, P63), p63},
    {KEY(026u, P63 + 2u), p63_after_newmodex},
    {KEY(026u, P63 + 012u), p63_after_phaschng},
    {KEY(026u, P63_1), p63_1},
    {KEY(026u, P63_1 + 2u), p63_1_after_phaschng},
    {KEY(026u, P64), p64},
    {KEY(026u, P64 + 2u), p64_after_newmodex},
    {KEY(026u, P65), p65},
    {KEY(026u, P65 + 2u), p65_after_newmodex},
    {KEY(026u, P65 + 6u), p65_after_novac},
    {KEY(026u, P65 + 011u), p65_after_2phschng},
    {KEY(026u, P65_1), p65_1},
    {KEY(026u, P65_1 + 2u), p65_1_display},
    {KEY(026u, P65_1 + 5u), p65_1_terminate},
    {KEY(026u, P65_1 + 6u), p65_1_proceed},
    {KEY(026u, P65_1 + 7u), p65_1_enter},
    {KEY(026u, P65_1 + 010u), p65_1_immediate_return},
    {KEY(026u, P65_1 + 013u), p65_1_after_upflag},
    {KEY(026u, P66), p66},
    {KEY(026u, P66 + 2u), p66_after_newmodex},
    {KEY(026u, P67), p67},
    {KEY(026u, P67 + 2u), p67_after_newmodex},
    {KEY(026u, KILLGRP4), killgrp4},
    {KEY(026u, KILLGRP4 + 2u), killgrp4_after_phaschng},
    {KEY(026u, P67_1), p67_1},
    {KEY(026u, P67_1 + 3u), p67_1_terminate},
    {KEY(026u, P67_1 + 4u), p67_1_proceed},
    {KEY(026u, P67_1 + 5u), p67_1_enter},
    /* Part 1: S61.1 (s61_1.c) */
    {KEY(026u, S61_1 + 4u), s61_1_after_r02both},
    {KEY(026u, S61_1 + 011u), s61_1_future_state},
    {KEY(026u, S61_1 + 016u), s61_1_after_waitlist},
    {KEY(026u, S61_1 + 020u), s61_1_after_phaschng},
    {KEY(026u, S61_1C), s61_1c},
    {KEY(026u, S61_1C + 4u), s61_1c_after_findvac},
    {KEY(026u, S61_1C + 012u), s61_1c_after_2phschng},
    {KEY(026u, S61_1C + 016u), s61_1a_job},
    {KEY(026u, RETRN3), retrn3},
    {KEY(026u, RETRN3 + 2u), retrn3_after_alarm},
    {KEY(026u, RETRN2), retrn2},
    {KEY(026u, RETRN2 + 2u), retrn2_after_alarm},
    {KEY(026u, RETRN2 + 5u), retrn2_after_godspr},
    {KEY(026u, RETRN1), retrn1},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* 60GENRET's key, and the words the file stores there: TC S61.1's
 * returns, from P61 and from P62. */
#define GENRET_KEY (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + 03373u)

static bool an_s61_1_return(word instruction)
{
    return instruction == P61 + 5u || instruction == P62 + 1u;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    /* Part 1 */
    &NEWRNVN_CODE,
    &P61_2_CODE,
    &P62_CODE,
    &P62_3_CODE,
    &P65_CODE,
    &P67_2_CODE,
    &S61_1_CODE,
    &S61_1A_CODE,
    /* Part 2 */
    &S61_2_CODE,
};

/* The table an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* TCDANZIG (ENDT(X), TIME_OF_FREE_FALL.agc, bank 27), where S61.1A's BOVB
 * goes: TC DANZIG, which only resets OVFIND; the pair goes on with its
 * right order. */
static void tcdanzig(void)
{
    tc_at(TCDANZIG);
    continue_at(DANZIG);
}

/* The basic routines a pair runs, at their addresses under their banks
 * (FB, from their FCADRs): RTB's, and the one a BOVB goes to. */
static const struct {
    word at, bank;
    void (*routine)(void);
} rtb_routines[] = {
    {LOADTIME, I_LOADTIME & 076000u, loadtime}, /* P61.2 */
    {TCDANZIG, I_TCDANZIG & 076000u, tcdanzig}, /* S61.1A */
};

/* One pass; after an RTB (or the BOVB to TCDANZIG), the routine and the
 * pass on from its DANZIG, as the faithful pair runs them. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    unsigned i;

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(rtb_routines); ++i)
        if (Z == rtb_routines[i].at && FBANK == rtb_routines[i].bank) break;
    if (i == COUNT(rtb_routines)) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    rtb_routines[i].routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p61_p67_basic, faithful_cm_p61_p67_interpretive;

/* A readable basic routine, from a state the faithful one takes. */
static enum exec_status run_basic(void (*routine)(void), apollo_overlay_state *state,
                                  cm_native_io *io)
{
    cm_native_io kept = *io;
    apollo_overlay_state before = *state;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device;

    if (!entry) return faithful_cm_p61_p67_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* 60GENRET, run as TC: Z at E6,1773, one of the file's words there. */
static enum exec_status basic_genret(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    cm_native_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if ((Z & 07777u) != ADDRESS_OF(60GENRET) || (EBANK & 03400u) != 03000u || !basic_sane() ||
        !an_s61_1_return(agc.memory[03373u]))
        return EXEC_OUT_OF_SCOPE;
    return run_basic(s61_1_return, state, io);
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_cm_p61_p67_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !sane())
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

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p61_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return key == GENRET_KEY || basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, KEY(bank, address), program, state, device);                               \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#define BASIC_ERASABLE(i, address)
#include "cm_p61_keys.inc"
#undef BASIC
#undef PAIR
#undef BASIC_ERASABLE

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
#define BASIC_ERASABLE(i, address)
static const agc_routine_entry pair_keys[] = {
#include "cm_p61_keys.inc"
};
#undef PAIR
#undef BASIC
#undef BASIC_ERASABLE

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
#define BASIC_ERASABLE(i, address)                                                                 \
    {(uint16_t)(AGC_TRANSFER_ERASABLE_BASE + (address)), basic_genret},
static const agc_routine_entry basic_keys[] = {
#include "cm_p61_keys.inc"
};
#undef PAIR
#undef BASIC
#undef BASIC_ERASABLE

const agc_routine_table cm_p61_p67_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p61_p67_basic = {basic_keys, COUNT(basic_keys)};

#endif
