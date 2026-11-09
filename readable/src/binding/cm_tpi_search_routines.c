/* SPDX-License-Identifier: MIT */
/* The readable TPI_SEARCH of the CM (src/tpi_search/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_tpi_search.c): cm_tpi_search_basic (the basic
 * code) and cm_tpi_search_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_tpi_search_keys.inc, which tools/p20_p25_keys.py writes from them).
 * An order pair in a part written so far runs the readable interpreter
 * on the listing's code; the other keys run the faithful port's routines
 * (renamed faithful_) until their parts are written.
 *
 * Partial file: Comanche055/TPI_SEARCH.agc (COVERAGE counts it when whole).
 *
 * Plumbing only. An order pair is one pass of the interpreter, from
 * NEWOPS with LOC the key. The states it takes, as the faithful pairs':
 * FB the key's bank (with the superbank bit clear), ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range,
 * every erasable word an ordinary one, NEWJOB +0 or positive and FIXLOC
 * a VAC area. A pass the readable code refuses part way leaves the state
 * and io record as they were. */
#include <stddef.h>
#include "binding.h"
#include "cm_tpi_search.h"
#include "../tpi_search/tpi_search.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* Channel 7's superbank bit. */
#define SUPERBANK_BIT 0100u

/* The states the faithful routines take (its basic_sane() and sane()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    return FIXLOC >= 0401u && FIXLOC <= 0661u && (FIXLOC - 0401u) % 054u == 0u;
}

/* The key's bank selected: FB, and in banks 30 to 37 the superbank bit
 * clear (with it set, FB 30 to 37 would be banks 40 to 47). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    /* Part 1: bank 36 */
    &S17_1_CODE,
    &S17_2_CODE,
    &BIS_CODE,
    &CONCAUL_CODE,
    &JUNCT3_CODE,
    /* Part 2: bank 36 */
    &JUNCT1_CODE,
    &INTERP_CODE,
    &HALFSAFE_CODE,
    &LRGRDVO_CODE,
    &RNGETEST_CODE,
    &USEKAY_CODE,
    /* Part 2: bank 07 */
    &TRANSANG_CODE,
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

/* MUTABLE, read by index (TRANSANG's STODL* and PDDL*, by X1 as the pass
 * finds it): the faithful port carries only the words an index of -2 (the
 * Earth's) or -10D (the Moon's) reads, and refuses a load from anywhere
 * else; so does the binding. */
static bool outside_mutable(uint16_t key)
{
    word x1;

    if (key != KEY(07u, TRANSANG + 011u) && key != KEY(07u, TRANSANG + 016u)) return false;
    x1 = agc.memory[(FIXLOC + I_X1) & 03777u];
    return x1 != 077775u && x1 != 077765u;
}

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_tpi_search_basic, faithful_cm_tpi_search_interpretive;

/* ---- The keys, in the faithful tables' order ---- */

/* Each basic key's routine: the faithful port's until its part is
 * written. */
static enum exec_status basic(unsigned index, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    return faithful_cm_tpi_search_basic.entries[index].call(program, state, device);
}

/* Each order pair's routine: one pass of the readable interpreter, or the
 * faithful port's until its part is written. */
static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, (word)(02000u + key % 1024u));
    enum exec_status status;

    if (!code)
        return faithful_cm_tpi_search_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io || !bind_machine(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || !in_bank(bank) || LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane() || outside_mutable(key))
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

/* For the side-by-side test: whether a key runs readable code. */
bool cm_tpi_search_readable(uint16_t key, bool order_pair)
{
    return order_pair && code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(i, program, state, device);                                                   \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#include "cm_tpi_search_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_tpi_search_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_tpi_search_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_tpi_search_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_tpi_search_basic = {basic_keys, COUNT(basic_keys)};

#endif
