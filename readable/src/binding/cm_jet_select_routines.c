/* SPDX-License-Identifier: MIT */
/* The readable JET_SELECTION_LOGIC (src/cm_dap) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_jet_select.c): cm_jet_select_basic (the basic code;
 * the file has no interpretive code, and cm_jet_select_interpretive no
 * keys) and cm_jet_select_hot_routines, the keys one pass reaches, which
 * the flight store packs together (the same routines).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_jet_select_keys.inc, which tools/p20_p25_keys.py writes from it). A
 * key with a readable routine runs it; any other runs the faithful
 * port's, built into the program as faithful_cm_jet_select_basic. The
 * side-by-side test (tests/cm_jet_select_cases.c) checks the readable
 * ones against it.
 *
 * Plumbing only. A basic routine runs from its key until it reaches a key
 * of another part of the faithful port's code, where its run stops, or a
 * word its part runs on to, a key of its own, run by the function named
 * for it.
 *
 * The file reads channel 31 (the translation hand controller), in the
 * program's hardware record, and sets T6's enable in channel 13, in the
 * io record. The faithful routines take the hardware record, and without
 * one refuse, so the binding does too. The states they take: FB bank 17,
 * any EBANK, ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one. A
 * run the readable code refuses part way leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_jet_select.h"
#include "../cm_dap/cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define JET_SELECT_BANK 017u

static jmp_buf back;

/* The run's io and hardware records. */
static cm_native_io *io_now;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* The channels the file uses, where the faithful port keeps them: 13
 * (T6's enable) in the io record, 31 (the hand controllers, read only)
 * in the hardware record. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 013u: return &io_now->channel13;
    case 031u: return &hardware_now->channel31;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return (word)*channel(number);
}

/* Channel 13's bits 12-14, as the faithful port's io record keeps them,
 * set the HANDRUPT traps and are not kept. */
static void channel_out(unsigned number, word value)
{
    if (number == 031u) not_modelled(); /* the file writes no input channel */
    if (number == 013u) {
        if (value & 004000u) io_now->trap31a = 1u;
        if (value & 010000u) io_now->trap31b = 1u;
        if (value & 020000u) io_now->trap32 = 1u;
        value &= 043777u;
    }
    *channel(number) = (agc_word)value;
}

/* The file's constants are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_native_io *io)
{
    bind_machine(program, state, io);
    io_now = io;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: the bank registers, the latches,
 * and every erasable word an ordinary one. */
static bool basic_sane(void)
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

/* ---- The basic code ---- */

/* The words a function starts at: the keys written so far. */
static const struct basic_entry {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {JETSLECT, jetslect},
    {TABPCOM + 5u, tabpcom_pjets},
    {TABRCOM + 1u, tabrcom_rtable},
    {BDZCHECK + 7u, bdzcheck_index},
    {BDZCHECK + 011u, bdzcheck_yztable},
    {NOBDZ, nobdz},
    {BDROLL, bdroll},
    {ACYCHECK + 014u, acycheck_rword1},
    {ACYCHECK + 020u, acycheck_net},
    {BDRACZ, bdracz},
    {BDRACZ + 2u, bdracz_rolltime},
    {TAUCHCK, tauchck},
    {NOACY, noacy},
    {PITCHTIM, pitchtim},
    {YAWTIME, yawtime},
    {YAWTIME + 1u, yawtime_to_ytaupos},
    {YAWTIME + 2u, yawtime_to_dft2},
    {YAWTIME + 3u, yawtime_to_ytauneg},
    {YAWTIME + 4u, yawtime_dft2},
    {YTAUPOS, ytaupos},
    {YTAUPOS + 010u, ytaupos_dftmax},
    {AD14MSY, ad14msy},
    {ROLLTIME, rolltime},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_jet_select_basic;

/* ---- The keys, in the faithful table's order ---- */

/* A readable routine, from a state the faithful one takes. */
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
    const struct basic_entry *entry = basic_entry((word)(02000u + key % 1024u));
    cm_native_io *io = device;

    if (!entry) return faithful_cm_jet_select_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)(JET_SELECT_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_jet_select_readable(uint16_t key)
{
    return key / 1024u == JET_SELECT_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, KEY(bank, address), program, state, device);                               \
    }
#define PAIR(i, bank, address)
#include "cm_jet_select_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_jet_select_keys.inc"
};
#undef PAIR
#undef BASIC

/* The keys one pass reaches, as the faithful hot table lists them. */
static const agc_routine_entry hot_keys[] = {
    {KEY(JET_SELECT_BANK, 02552u), basic_0},  {KEY(JET_SELECT_BANK, 02671u), basic_1},
    {KEY(JET_SELECT_BANK, 03046u), basic_6},  {KEY(JET_SELECT_BANK, 03122u), basic_12},
    {KEY(JET_SELECT_BANK, 03164u), basic_13}, {KEY(JET_SELECT_BANK, 03230u), basic_14},
    {KEY(JET_SELECT_BANK, 03274u), basic_22}, {KEY(JET_SELECT_BANK, 03372u), basic_29},
    {KEY(JET_SELECT_BANK, 03516u), basic_45}, {KEY(JET_SELECT_BANK, 03536u), basic_50},
    {KEY(JET_SELECT_BANK, 03554u), basic_51}, {KEY(JET_SELECT_BANK, 03560u), basic_52},
    {KEY(JET_SELECT_BANK, 03561u), basic_53}, {KEY(JET_SELECT_BANK, 03577u), basic_55},
    {KEY(JET_SELECT_BANK, 03603u), basic_57}, {KEY(JET_SELECT_BANK, 03617u), basic_58},
    {KEY(JET_SELECT_BANK, 03654u), basic_62}, {KEY(JET_SELECT_BANK, 03663u), basic_63},
    {KEY(JET_SELECT_BANK, 03666u), basic_64}, {KEY(JET_SELECT_BANK, 03673u), basic_66},
    {KEY(JET_SELECT_BANK, 03713u), basic_71},
};

const agc_routine_table cm_jet_select_interpretive = {NULL, 0};
const agc_routine_table cm_jet_select_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_jet_select_hot_routines = {hot_keys, COUNT(hot_keys)};

#endif
