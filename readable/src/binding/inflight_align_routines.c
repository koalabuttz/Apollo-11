/* SPDX-License-Identifier: MIT */
/* The readable INFLIGHT_ALIGNMENT_ROUTINES (src/inflight_align/) as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_inflight_align.c and
 * cm_inflight_align.c): <m>_inflight_align_basic and
 * <m>_inflight_align_interpretive, all in bank 23. The keys come from the
 * faithful tables, in their order (inflight_align_keys.inc and
 * cm_inflight_align_keys.inc, which tools/p20_p25_keys.py writes from
 * them). Every key runs a readable routine.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (ALARM, UPFLAG) or reaches NEWOPS through INTPRET. An order
 * pair is one pass of the readable interpreter over the listing's words
 * (interpret_listing()), in the part of the code LOC is in; a pass that
 * ends at RTB V1STO2S runs V1STO2S (src/rtb_op_codes/) and the DANZIG it
 * ends at, as the faithful pair does.
 *
 * The states the faithful routines take: FB bank 23, ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range, every
 * erasable word an ordinary one, NEWJOB +0 or a core set and FIXLOC one
 * of the VAC areas; at an order pair also Z at NEWOPS, BANKSET the BBANK
 * and INTBIT15 the bank's bit 15. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../inflight_align/inflight_align.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The bank of the file's code (INFLIGHT), and V1STO2S's (FB), from their
 * FCADRs. */
#define INFLIGHT_BANK CODE_BANK(I_CALCGTA)
#define V1STO2S_BANK  ((word)(I_V1STO2S & 076000u))

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

/* The states the faithful routines take (their sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u || FIXLOC < 0401u || FIXLOC > 0661u ||
        (FIXLOC - 0401u) % 054u)
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
    {KEY(INFLIGHT_BANK, GIMLOCK1 + 1u), gimlock1_after_exit},
    {KEY(INFLIGHT_BANK, GIMLOCK1 + 3u), gimlock1_after_alarm},
    {KEY(INFLIGHT_BANK, GIMLOCK1 + 5u), gimlock1_after_upflag},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

/* The file's interpretive code, in its parts. */
static const struct interpretive_code *const codes[] = {&CALCGTA_CODE, &AXISGEN_CODE};

/* The part of the code an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* One pass; after CALCGA1's RTB V1STO2S, V1STO2S and its DANZIG. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != V1STO2S || FBANK != V1STO2S_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    v1sto2s();
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
    if (FBANK != (word)((key / 1024u) << 10) || !sane()) return EXEC_OUT_OF_SCOPE;
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
    enum exec_status status;

    if (!code || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || LOC != at || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
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
bool inflight_align_readable(uint16_t key, bool order_pair)
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
#include "inflight_align_keys.inc"
#else
#include "cm_inflight_align_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "inflight_align_keys.inc"
#else
#include "cm_inflight_align_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "inflight_align_keys.inc"
#else
#include "cm_inflight_align_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_inflight_align_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_inflight_align_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_inflight_align_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_inflight_align_basic = {basic_keys, COUNT(basic_keys)};
#endif
