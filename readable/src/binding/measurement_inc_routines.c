/* SPDX-License-Identifier: MIT */
/* The readable MEASUREMENT_INCORPORATION (src/measurement_inc/) as
 * routines of each mission's flight program, under the table names and
 * keys of the faithful port's (c64-port/src/lm_measurement_inc.c and
 * cm_measurement_inc.c): <m>_measurement_inc_basic (INCORP2's basic code)
 * and <m>_measurement_inc_interpretive (the order pairs, the LM's in banks
 * 22 and 23, the CM's in banks 14, 35 and 36). The keys come from the
 * faithful tables, in their order (measurement_inc_keys.inc and
 * cm_measurement_inc_keys.inc, which tools/p20_p25_keys.py writes from
 * them), and every key runs a readable routine.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (PHASCHNG, UPFLAG, POSTJUMP) or reaches NEWOPS through a TC
 * INTPRET. An order pair is one pass of the readable interpreter over the
 * listing's words (interpret_listing()), in the part of the code LOC is
 * in; after an RTB TPMODE, or an RTB of one of the state vector moves of
 * INTEGRATION_INITIALIZATION (src/integration_init/), the routine and the
 * rest of the pair, as the faithful pair runs them. (A BOVB to TCDANZIG
 * ends the pair there, as the faithful one does.)
 *
 * The states the faithful routines take: FB the key's bank (the CM's
 * banks 35 and 36 with the superbank bit off), ZERO +0, NEWJOB +0 or a
 * core set, the bank registers agreeing, FIXLOC one of the VAC areas, the
 * interrupt and superbank latches in range and every erasable word an
 * ordinary one; at an order pair also Z at NEWOPS, BANKSET the BBANK and
 * INTBIT15 the bank's bit 15. A move takes, as the faithful one does, the
 * latch, ZERO and EBANK in range and Q an ordinary word too. A run the
 * readable code refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../measurement_inc/measurement_inc.h"
#include "../integration_init/integration_init.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE 0u /* the LM's rope */
#else
#define ROPE 1u /* the CM's */
#endif

/* A bank's number as FB holds it. */
#define BANK(n) (word)((n) << 10)

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* INCORP2's bank, and the banks (FB) of TPMODE and of the moves, from
 * their FCADRs. */
#define INCORP2_BANK ((unsigned)(I_INCORP2 >> 10))
#define TPMODE_BANK  ((word)(I_TPMODE & 076000u))
#define MOVES_BANK   ((word)(I_MOVEACSM & 076000u))

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

/* The states the faithful routines take (their sane(), and the CM's
 * sb_ok()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || agc.memory[ADDRESS_OF(NEWJOB)] > 037777u ||
        EBANK > 03400u || (EBANK & 0377u) || BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if ((FBANK & 060000u) == 060000u && (*agc.superbank & 0100u)) return false; /* banks 30-37 */
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The states the faithful moves take, after the RTB: the interrupt latch,
 * ZERO and EBANK in range, and every word from Q on an ordinary one. */
static bool move_sane(void)
{
    unsigned i;

    if (*agc.interrupts > 1u || agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u))
        return false;
    for (i = Q_ADDRESS; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(INCORP2_BANK, INCORP2 + 020u), incorp2_after_exit},
    {KEY(INCORP2_BANK, FAZA), faza},
    {KEY(INCORP2_BANK, FAZA + 2u), faza_after_phaschng},
    {KEY(INCORP2_BANK, FAZA1), faza1},
    {KEY(INCORP2_BANK, FAZB1), fazb1},
    {KEY(INCORP2_BANK, FAZAB5 + 4u), fazab5_after_exit},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

/* The part of the code an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < MEASUREMENT_INC_PARTS; ++i)
        if (MEASUREMENT_INC_CODE[i].bank == bank && address >= MEASUREMENT_INC_CODE[i].first &&
            address < MEASUREMENT_INC_CODE[i].first + MEASUREMENT_INC_CODE[i].count)
            return &MEASUREMENT_INC_CODE[i];
    return NULL;
}

/* The move an RTB has gone to, or none. */
static void (*move_at(void))(void)
{
    if (FBANK != MOVES_BANK) return NULL;
    if (Z == MOVEACSM) return moveacsm;
    if (Z == MOVEPCSM) return movepcsm;
    if (Z == MOVEALEM) return movealem;
    if (Z == MOVEPLEM) return moveplem;
    return NULL;
}

/* One pass; after RTB TPMODE or an RTB of a move, the routine and its
 * DANZIG, which goes on with the pair's other order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    void (*routine)(void);

    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == TPMODE && FBANK == TPMODE_BANK) {
        routine = tpmode;
    } else {
        routine = move_at();
        if (!routine) return EXEC_OK;
        if (!move_sane()) return EXEC_OUT_OF_SCOPE;
    }
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    routine();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each basic key's routine, the readable one. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != BANK(key / 1024u) || !sane()) return EXEC_OUT_OF_SCOPE;

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
    if (Z != NEWOPS || FBANK != BANK(bank) || LOC != at || BANKSET != BBANK ||
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
bool measurement_inc_readable(uint16_t key, bool order_pair)
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
#include "measurement_inc_keys.inc"
#else
#include "cm_measurement_inc_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "measurement_inc_keys.inc"
#else
#include "cm_measurement_inc_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "measurement_inc_keys.inc"
#else
#include "cm_measurement_inc_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_measurement_inc_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_measurement_inc_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_measurement_inc_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_measurement_inc_basic = {basic_keys, COUNT(basic_keys)};
#endif
