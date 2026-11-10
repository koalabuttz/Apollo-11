/* SPDX-License-Identifier: MIT */
/* The readable TIME_OF_FREE_FALL (src/time_free_fall/) as routines of
 * each mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_tff_abi.c and cm_time_free_fall.c):
 * lm_tff_abi_basic and _interpretive for the LM, cm_time_free_fall_basic
 * and _interpretive for the CM, all in bank 27. The keys come from the
 * faithful tables, in their order (time_free_fall_keys.inc and
 * cm_time_free_fall_keys.inc, which tools/p20_p25_keys.py writes from
 * them). Every key runs a readable routine.
 *
 * Plumbing only. A basic routine (T(X), ENDT(X)) runs until control
 * leaves the file, at POLY or DANZIG. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()). A
 * pass that ends at the BOVB to TCDANZIG runs its TC DANZIG and the rest
 * of the pair, as the faithful pair does; one that ends at RTB T(X)
 * stops there, and the pair's right-hand order waits in EDOP for the key
 * after the RTB, which ENDT(X)'s DANZIG goes on with (see pending_orders).
 *
 * The states the faithful routines take: FB the key's bank, ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range
 * and every erasable word an ordinary one; at an order pair also NEWJOB
 * +0 or a core set and FIXLOC one of the VAC areas, Z at NEWOPS, BANKSET
 * the BBANK and INTBIT15 the bank's bit 15 (or, at a pending key, Z at
 * DANZIG). A run the readable code refuses part way leaves the state as
 * it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../time_free_fall/time_free_fall.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The bank of the file's code, from TFFCONIC's FCADR. */
#define TFF_BANK CODE_BANK(I_TFFCONIC)

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

/* The file's constants are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(ROPE, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static bool bind(const agc_native_program *program, apollo_overlay_state *state, mission_io *io)
{
    if (!bind_machine(program, state, io)) return false;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
    return true;
}

/* The states the faithful routines take (their sane(), and basic_sane(),
 * which leaves out the work area). */
static bool sane(bool work_area)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (work_area && (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u || FIXLOC < 0401u ||
                      FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(TFF_BANK, T_X), t_x},
    {KEY(TFF_BANK, ENDT_X), endt_x},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

/* The part of the code an address of the bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    if (TFF_CODE.bank == bank && address >= TFF_CODE.first &&
        address < TFF_CODE.first + TFF_CODE.count)
        return &TFF_CODE;
    return NULL;
}

/* The keys a pair's right-hand order waits at in EDOP while its left, RTB
 * T(X), runs the series: TFFXTEST's and TFFEL1's RTB DMPs, the word after
 * each one's T(X). ENDT(X)'s DANZIG finds the order in EDOP (its code
 * plus one) and resumes the pass at LOC. */
static const struct pending_order {
    word at;
    word edop;
} pending_orders[] = {
    {(word)(CODE_Z(I_TFFCONIC) + 0232u), ORDER_DMP + 1u},
    {(word)(CODE_Z(I_TFFCONIC) + 0315u), ORDER_DMP + 1u},
};

static const struct pending_order *pending_order(unsigned bank, word at)
{
    unsigned i;

    if (bank != TFF_BANK) return NULL;
    for (i = 0; i < COUNT(pending_orders); ++i)
        if (pending_orders[i].at == at) return &pending_orders[i];
    return NULL;
}

/* The states the faithful port resumes such a pair from: Z at DANZIG, the
 * order in EDOP, BANKSET the pair's bank (FB may be any) with no E-bank
 * bits, and INTBIT15 the bank's bit 15. */
static bool at_danzig(unsigned bank, const struct pending_order *pending)
{
    return Z == DANZIG && EDOP == pending->edop && ((BANKSET >> 10) & 037u) == bank &&
           !(BANKSET & 01770u) && INTBIT15 == ((bank << 10) & 040000u) && sane(true);
}

/* One pass; after a BOVB to TCDANZIG, its TC DANZIG, which goes on with
 * the pair's right-hand order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != TCDANZIG || FBANK != (word)(TFF_BANK << 10)) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    endt_x();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* A basic routine, run; a run it refuses part way leaves the state as it
 * was. */
static enum exec_status run(void (*routine)(void), apollo_overlay_state *state, mission_io *io)
{
    mission_io kept = *io;
    apollo_overlay_state before = *state;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    return EXEC_OK;
}

/* Each basic key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device;

    if (!entry || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != (word)((key / 1024u) << 10) || !sane(false)) return EXEC_OUT_OF_SCOPE;
    return run(entry->routine, state, io);
}

/* Each order pair's routine: one pass of the readable interpreter. */
static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    const struct pending_order *pending = pending_order(bank, at);
    enum exec_status status;
    bool started;

    if (!code || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (pending)
        started = LOC == at && at_danzig(bank, pending);
    else
        started = Z == NEWOPS && FBANK == (word)(bank << 10) && LOC == at && BANKSET == BBANK &&
                  INTBIT15 == (FBANK & 040000u) && sane(true);
    if (!started) return EXEC_OUT_OF_SCOPE;

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
bool time_free_fall_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(KEY(bank, address), program, state, device);                                   \
    }
#if LUMINARY099
#include "time_free_fall_keys.inc"
#else
#include "cm_time_free_fall_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "time_free_fall_keys.inc"
#else
#include "cm_time_free_fall_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "time_free_fall_keys.inc"
#else
#include "cm_time_free_fall_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_tff_abi_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_tff_abi_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_time_free_fall_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_time_free_fall_basic = {basic_keys, COUNT(basic_keys)};
#endif
