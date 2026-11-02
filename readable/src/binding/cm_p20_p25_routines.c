/* SPDX-License-Identifier: MIT */
/* The readable CM P20-P25 (src/p20_p25_cm/) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p20_p25.c): cm_p20_p25_basic (the basic code) and
 * cm_p20_p25_interpretive (the order pairs).
 *
 * The file is readable part by part, so the tables carry every key of the
 * faithful ones, in their order (cm_p20_p25_keys.inc, which
 * tools/p20_p25_keys.py writes from them): a key a readable part has
 * written runs the readable routine; any other runs the faithful port's,
 * built into the program as faithful_cm_p20_p25_basic and
 * faithful_cm_p20_p25_interpretive (tests/x86_readable.py, the
 * Makefile's cm-p20-p25 tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, 2PHSCHNG, PRIOCHNG, UPFLAG, DOWNFLAG, FINDVAC,
 * ENDOFJOB, ...), reaches a label the faithful port keys a routine at, or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()); a
 * pass that ends at RTB LOADTIME runs LOADTIME (src/rtb_op_codes/) and the
 * DANZIG it ends at, as the faithful pair does. A pass that ends at an
 * EXIT or a CALL out of its table stops there, as the faithful pair does.
 *
 * The device is the CM's io record (cm_native_io), and the program must
 * have its hardware record (channels 30-33), as the faithful routines
 * want. The parts written so far read and write no channel.
 *
 * The states the faithful routines take, at every key: FB the key's bank
 * (in banks 30-37 the superbank bit clear; banks 40-43 are FB 30-33 with
 * it set); at an order pair Z at NEWOPS, BANKSET the BBANK and INTBIT15
 * the bank's bit 15. Always ZERO +0, the bank registers agreeing, the
 * interrupt and superbank latches in range, and every erasable word an
 * ordinary one. At an order pair and at the basic keys the faithful port
 * marks so (its at_basic), NEWJOB +0 or a core set and FIXLOC one of the
 * VAC areas; at the other basic keys (its at_basic_nv) NEWJOB may be -0
 * too, and FIXLOC anything. A run the readable code refuses part way
 * leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p20_p25.h"
#include "../p20_p25_cm/p20_p25_cm.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if COMANCHE055

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p20_p25_basic, faithful_cm_p20_p25_interpretive;

/* A key: the bank, and the address in it (or in fixed-fixed memory). */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static word channel_in(unsigned number)
{
    (void)number;
    not_modelled(); /* the readable parts read no channel yet */
    return 0;
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* the readable parts write no channel yet */
}

/* The interpreter's fixed operands are the rope's. */
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

/* The states the faithful routines take (its sane(); without a work area,
 * its sane_nv()). */
static bool sane(bool with_work_area)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u && (with_work_area || newjob != MINUS_ZERO)) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (with_work_area && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The key's bank selected: FB, with the superbank bit clear in banks
 * 30-37, and set for banks 40-43 (FB 30-33). */
static bool in_bank(unsigned bank)
{
    if (bank >= 040u)
        return FBANK == (word)((bank - 010u) << 10) && (*agc.superbank & SUPERBANK_BIT);
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* Part 1: P20 */
    {KEY(037u, PROG20), prog20, false},
    {KEY(037u, PROG20 + 2u), prog20_after_r02both, false},
    {KEY(037u, PROG20 + 7u), prog20_after_prftrkat, false},
    {KEY(037u, PROG20 + 011u), prog20_after_vehupflg, false},
    {KEY(037u, PROG20 + 013u), prog20_after_trackflg, false},
    {KEY(037u, PROG20 + 015u), prog20_after_updatflg, false},
    {KEY(037u, PROG20 + 017u), prog20_after_rndvzflg, false},
    {KEY(037u, PROG20 + 023u), prog20_after_2phschng, true},
    {KEY(037u, PROG20 + 055u), prog20_after_exit, false},
    {KEY(037u, PROG20 + 061u), prog20_after_findvac, false},
    {KEY(037u, PIKUP20), pikup20, false},
    {KEY(037u, PIKUP20 + 2u), pikup20_after_priochng, false},
    {KEY(037u, PIKUP20 + 016u), pikup20_after_r61csm, false},
    {KEY(037u, P20R52JB), p20r52jb, true},
    {KEY(037u, P20R52JB + 4u), p20r52jb_after_exit, false},
    {KEY(037u, P20R52JB + 6u), p20r52jb_after_mkrlees, false},
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
    &PROG20_CODE,
    &P20R52JB_CODE,
};

/* The table of bank that address is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after RTB LOADTIME, LOADTIME and its DANZIG. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p20_p25_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    /* (fixed-fixed code, banks 02 and 03, runs under any FB) */
    if ((key / 1024u >= 4u && !in_bank(key / 1024u)) || !sane(entry->with_work_area))
        return EXEC_OUT_OF_SCOPE;

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
        return faithful_cm_p20_p25_interpretive.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
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

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p20_p25_readable(uint16_t key, bool order_pair)
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
#include "cm_p20_p25_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p20_p25_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p20_p25_basic = {basic_keys, COUNT(basic_keys)};

#endif
