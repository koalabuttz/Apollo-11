/* SPDX-License-Identifier: MIT */
/* The readable P51-P53 of the CM (src/p51_p53_cm/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_p51_p53.c): cm_p51_p53_basic (the basic code)
 * and cm_p51_p53_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_p51_keys.inc, which tools/p20_p25_keys.py writes from them). The
 * file is written part by part: a key a part has written runs the
 * readable routine; any other runs the faithful port's, built into the
 * program as faithful_cm_p51_p53_basic and faithful_cm_p51_p53_interpretive
 * (tests/x86_readable.py, the Makefile's tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, DOWNFLAG, ENDOFJOB, GOTOPOOH, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()).
 *
 * The device is the CM's io record (cm_native_io); the file reads and
 * writes no channel. The states the faithful routines take, at every key:
 * FB the key's bank, ZERO +0, NEWJOB +0 or a core set, the bank registers
 * agreeing, the interrupt and superbank latches in range, FIXLOC one of
 * the VAC areas, and every erasable word an ordinary one; at an order pair
 * also Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. A
 * run the readable code refuses part way leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p51_p53.h"
#include "../p51_p53_cm/p51_p53_cm.h"
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

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P52 */
    {KEY(015u, PROG52), prog52},
    {KEY(015u, PROG52 + 2u), prog52_after_phaschng},
    {KEY(015u, PROG52 + 4u), prog52_after_updatflg},
    {KEY(015u, PROG52 + 6u), prog52_after_trackflg},
    {KEY(015u, PROG52 + 010u), prog52_after_r02both},
    {KEY(015u, PROG52 + 014u), prog52_no_preferred},
    {KEY(015u, P52A), p52a},
    {KEY(015u, P52B + 3u), p52b_terminate},
    {KEY(015u, P52B + 4u), p52b_proceed},
    {KEY(015u, P52B + 5u), p52b_enter},
    {KEY(015u, P52B + 6u), p52b_immediate_return},
    {KEY(015u, P52B + 010u), p52b_after_phaschng},
    {KEY(015u, P52B + 015u), p52_landing_site},
    {KEY(015u, P52B + 016u), p52_preferred},
    {KEY(015u, P52B + 017u), p52_nominal},
    {KEY(015u, P52B + 020u), p52_reference},
    {KEY(015u, P52T + 6u), p52t_terminate},
    {KEY(015u, P52T + 7u), p52t_proceed},
    {KEY(015u, P52T + 010u), p52t_enter},
    {KEY(015u, P52T + 011u), p52t_time},
    {KEY(015u, P52D + 3u), p52d_after_exit},
    {KEY(015u, P52D + 6u), p52d_terminate},
    {KEY(015u, P52D + 7u), p52d_proceed},
    {KEY(015u, P52J), p52j},
    {KEY(015u, P52J + 3u), p52j_coarse},
    {KEY(015u, P52C), p52c},
    {KEY(015u, P52C + 2u), p52c_after_phaschng},
    {KEY(015u, P52C + 5u), p52c_terminate},
    {KEY(015u, P52C + 6u), p52c_proceed},
    {KEY(015u, P52C + 7u), p52c_enter},
    {KEY(015u, P52C + 010u), p52c_stars},
    {KEY(015u, P52E), p52e},
    {KEY(015u, P52E + 2u), p52e_no_stars},
    {KEY(015u, P52F), p52f},
    {KEY(015u, ENDP50S + 1u), p52f_after_exit},
    {KEY(015u, P52I + 2u), p52i_after_alarm},
    {KEY(015u, P52I + 5u), p52i_terminate},
    {KEY(015u, P52I + 6u), p52i_proceed},
    {KEY(015u, P52I + 7u), p52i_recycle},
    {KEY(015u, COARSTYP + 3u), coarstyp_terminate},
    {KEY(015u, COARSTYP + 4u), coarstyp_normal},
    {KEY(015u, COARSTYP + 5u), coarstyp_gyro},
    {KEY(015u, GYCRS + 022u), gycrs_after_exit},
    {KEY(015u, GYCRS + 025u), gycrs_after_godspr},
    {KEY(015u, GYCRS + 030u), gycrs_after_imupulse},
    {KEY(015u, GYCRS + 032u), gycrs_torque_failed},
    {KEY(015u, GYCRS + 033u), gycrs_torqued},
    {KEY(015u, GYCRS + 035u), gycrs_after_phaschng},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    /* Part 1: P52 */
    &P52T_LANDING_SITE_CODE, &P52T_NOMINAL_CODE, &P52J_CODE,  &P52J_COARSE_CODE,
    &P52C_STARS_CODE,        &P52F_CODE,         &GYCRS_CODE, &GYCRS_DONE_CODE,
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

/* The RTB routines the pairs run, at their addresses under their banks
 * (FB, from their FCADRs). */
static const struct {
    word at, bank;
    void (*routine)(void);
} rtb_routines[] = {
    {LOADTIME, I_LOADTIME & 076000u, loadtime}, /* P52C */
    {SET1_PDT, I_SET1_PDT & 076000u, set1_pdt}, /* GYCRS (p51.c) */
};

/* One pass; after an RTB, the routine (src/rtb_op_codes/, or SET1/PDT in
 * p51.c) and the pass on from its DANZIG, as the faithful pair runs them. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    unsigned i;

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    for (i = 0; i < COUNT(rtb_routines); ++i)
        if (Z == rtb_routines[i].at && FBANK == rtb_routines[i].bank) break;
    if (i == COUNT(rtb_routines)) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    rtb_routines[i].routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p51_p53_basic, faithful_cm_p51_p53_interpretive;

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p51_p53_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
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
        return faithful_cm_p51_p53_interpretive.entries[index].call(program, state, device);
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
bool cm_p51_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
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
#include "cm_p51_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p51_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p51_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p51_p53_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p51_p53_basic = {basic_keys, COUNT(basic_keys)};

#endif
