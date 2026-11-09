/* SPDX-License-Identifier: MIT */
/* The readable CM_ENTRY_DIGITAL_AUTOPILOT (src/entry_dap) as routines of
 * the CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_entry_dap.c): cm_entry_dap_basic (the basic
 * code; the file has no interpretive code, and cm_entry_dap_interpretive
 * no keys) and cm_entry_dap_hot_routines, the keys of the 0.1 s cycle,
 * which the flight store packs together (the same routines).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_entry_dap_keys.inc, which tools/p20_p25_keys.py writes from them).
 * A key in a part written so far runs its readable routine; the other
 * keys run the faithful port's routines (renamed faithful_) until their
 * parts are written.
 *
 * Partial file: Comanche055/CM_ENTRY_DIGITAL_AUTOPILOT.agc (COVERAGE
 * counts it when whole).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (WAITLIST, TASKOVER, PHASCHNG, 2PHSCHNG, BANKCALL,
 * DOWNFLAG, POSTJUMP, a TC Q to erasable memory) or reaches a key of
 * another part of the faithful port's code, where its run stops, or a
 * key of its own part, which the binding runs on at, as the faithful
 * port does.
 *
 * The file writes the jets' channels 5 and 6, in the program's hardware
 * record. The faithful routines take that record, and without one
 * refuse, so the binding does too. The states they take: FB the key's
 * bank (any for FLUSHJET, in fixed-fixed memory, but banks 30 to 37 with
 * the superbank bit clear), E bank 6 (any for CM/DAPON and CM/DAPIC,
 * which set it), ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one. A
 * run the readable code refuses part way leaves the state, io and
 * hardware records as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_entry_dap.h"
#include "../entry_dap/entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it (or in fixed-fixed memory). */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* Channel 7's superbank bit. */
#define SUPERBANK_BIT 0100u

static jmp_buf back;

/* The run's hardware record. */
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* The channels the file uses, where the faithful port keeps them: 5 and
 * 6 (the jets) in the hardware record. */
static agc_word *channel(unsigned number)
{
    switch (number) {
    case 05u: return &hardware_now->channel5;
    case 06u: return &hardware_now->channel6;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return (word)*channel(number);
}

static void channel_out(unsigned number, word value)
{
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
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: FB fb (in banks 30 to 37 with
 * the superbank bit clear), E bank 6 unless any_eb, the bank registers,
 * the latches, and every erasable word an ordinary one. */
static bool basic_sane(word fb, bool any_eb)
{
    unsigned i;

    if (FBANK != fb || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u) ||
        (fb >= 060000u && (*agc.superbank & SUPERBANK_BIT)))
        return false;
    if (!any_eb && EBANK != 03000u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The keys each function starts at. */
static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part A: READGYMB to ANGOVCOR, FLUSHJET (bank 15, fixed-fixed) */
    {KEY(03u, FLUSHJET), flushjet},
    {KEY(015u, READGYMB), readgymb},
    {KEY(015u, READGYMB + 012u), readgymb_after_flushjet},
    {KEY(015u, READGYM1), readgym1},
    {KEY(015u, DOBRATE_Q + 4u), dobrate_q_ok},
    {KEY(015u, DOBRATE_Q + 5u), dobrate_q_first_pass},
    {KEY(015u, DOBRATE_Q + 6u), dobrate_q_noop},
    {KEY(015u, DOBRATE_Q + 7u), dobrate_q_standby},
    {KEY(015u, DOBRATE_Q + 010u), dobrate_q_group6},
    {KEY(015u, DOBRATE_Q + 012u), dobrate_q_after_phaschng},
    {KEY(015u, DOBRATE1 + 5u), dobrate1_after_waitlist},
    {KEY(015u, DOBRATE1 + 7u), dobrate1_taskover},
    {KEY(015u, ANGOVCOR), angovcor},
    /* Part A: CM/DAPON and CM/DAPIC (bank 20) */
    {KEY(020u, CM_DAPON), cm_dapon},
    {KEY(020u, CM_DAPON + 4u), cm_dapon_after_dapbit1},
    {KEY(020u, CM_DAPON + 6u), cm_dapon_after_dapbit2},
    {KEY(020u, CM_DAPON + 010u), cm_dapon_t5loc},
    {KEY(020u, CM_DAPON + 015u), cm_dapon_after_flushjet},
    {KEY(020u, CM_DAPON + 020u), cm_dapon_plus4_tc},
    {KEY(020u, NOTYET), notyet},
    {KEY(020u, NOTYET + 1u), notyet_bankcall},
    {KEY(020u, NOTYET + 3u), notyet_plus4},
    {KEY(020u, NOTYET + 020u), notyet_alfacom},
    {KEY(020u, CM_DAPIC), cm_dapic},
    {KEY(020u, CM_DAP2C + 5u), cm_dap2c_jetem},
    {KEY(020u, CM_DAP2C + 020u), cm_dap2c_2cadr},
    {KEY(020u, CM_DAP2C + 021u), cm_dap2c_after_waitlist},
    {KEY(020u, CM_DAP2C + 025u), cm_dap2c_beta},
    {KEY(020u, CM_DAP2C + 026u), cm_dap2c_beta_ts},
    {KEY(020u, CM_DAP2C + 031u), cm_dap2c_2phschng},
    {KEY(020u, CM_DAP2C + 035u), cm_dap2c_after_2phschng},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* The first key of each part of the faithful port's code, part 0 from
 * FLUSHJET (fixed-fixed) and READGYMB on: a run that reaches a key of
 * another part stops there. */
static const uint16_t part_starts[] = {
    KEY(03u, 06000u),  KEY(015u, 02527u), KEY(015u, 02636u), KEY(015u, 02753u), KEY(015u, 03037u),
    KEY(015u, 03131u), KEY(015u, 03252u), KEY(015u, 03356u), KEY(015u, 03457u), KEY(015u, 03566u),
    KEY(015u, 03662u), KEY(020u, 03616u), KEY(020u, 03722u),
};

static unsigned part_of(uint16_t key)
{
    unsigned part = 0;

    while (part + 1u < COUNT(part_starts) && part_starts[part + 1u] <= key) ++part;
    return part;
}

/* After a TC, a jump or a TC Q, at the word Z names, in the bank FB
 * selects: on at a key of the same part, as the faithful port does; at
 * any other word (in erasable memory, or another part) the run stops
 * there. */
static void run_on(uint16_t key)
{
    const struct basic_entry *entry;
    uint16_t at;

    while (Z >= 02000u && Z <= 07777u) {
        at = (uint16_t)fixed_physical(Z);
        entry = basic_entry(at);
        if (!entry || part_of(at) != part_of(key)) return;
        entry->routine();
    }
}

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_entry_dap_basic;

/* ---- The keys, in the faithful table's order ---- */

/* Each key's routine: the readable one, and on at the keys of its part;
 * or the faithful port's until its part is written. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    agc_hardware hardware_kept;
    apollo_overlay_state before;
    word fb = (word)((key / 1024u) << 10);

    if (!entry) return faithful_cm_entry_dap_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (key / 1024u == 03u) fb = FBANK; /* FLUSHJET, under any FB */
    if (!basic_sane(fb, key == KEY(020u, CM_DAPON) || key == KEY(020u, CM_DAPIC)))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    hardware_kept = *hardware_now;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        *hardware_now = hardware_kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    run_on(key);
    return EXEC_OK;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_entry_dap_readable(uint16_t key)
{
    return basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, KEY(bank, address), program, state, device);                               \
    }
#define PAIR(i, bank, address)
#include "cm_entry_dap_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_entry_dap_keys.inc"
};
#undef PAIR
#undef BASIC

/* The keys of the 0.1 s cycle, as the faithful hot table lists them. */
static const agc_routine_entry hot_keys[] = {
    {KEY(015u, 02434u), basic_1},   {KEY(015u, 02507u), basic_10},  {KEY(015u, 02522u), basic_12},
    {KEY(015u, 02536u), basic_14},  {KEY(015u, 02540u), basic_15},  {KEY(015u, 02543u), basic_16},
    {KEY(015u, 02551u), basic_17},  {KEY(015u, 02607u), basic_20},  {KEY(015u, 02651u), basic_27},
    {KEY(015u, 02665u), basic_28},  {KEY(015u, 02676u), basic_29},  {KEY(015u, 02700u), basic_30},
    {KEY(015u, 02712u), basic_31},  {KEY(015u, 02724u), basic_32},  {KEY(015u, 02730u), basic_33},
    {KEY(015u, 02734u), basic_34},  {KEY(015u, 02741u), basic_35},  {KEY(015u, 02753u), basic_36},
    {KEY(015u, 02770u), basic_39},  {KEY(015u, 03006u), basic_43},  {KEY(015u, 03037u), basic_45},
    {KEY(015u, 03112u), basic_56},  {KEY(015u, 03123u), basic_57},  {KEY(015u, 03253u), basic_68},
    {KEY(015u, 03677u), basic_111}, {KEY(015u, 03705u), basic_114}, {KEY(015u, 03717u), basic_115},
};

const agc_routine_table cm_entry_dap_interpretive = {NULL, 0};
const agc_routine_table cm_entry_dap_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_entry_dap_hot_routines = {hot_keys, COUNT(hot_keys)};

#endif
