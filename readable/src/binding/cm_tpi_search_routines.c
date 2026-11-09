/* SPDX-License-Identifier: MIT */
/* The readable TPI_SEARCH of the CM (src/tpi_search/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_tpi_search.c): cm_tpi_search_basic (the basic
 * code) and cm_tpi_search_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_tpi_search_keys.inc, which tools/p20_p25_keys.py writes from them).
 * Every basic key runs a readable routine, and every order pair the
 * readable interpreter on the listing's code.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (AVFLAGA, AVFLAGP, P20FLGON, VNPOOH, BANKCALL, BLANKET, ALARM,
 * ENDOFJOB, GOTOPOOH, or a return saved in QSAVED) or reaches NEWOPS
 * through a TC INTPRET. An order pair is one pass of the interpreter,
 * from NEWOPS with LOC the key. The states both take, as the faithful
 * routines': FB the key's bank (with the superbank bit clear), ZERO +0,
 * the bank registers agreeing, the interrupt and superbank latches in
 * range, every erasable word an ordinary one, NEWJOB +0 or positive and
 * FIXLOC a VAC area; an order pair, too, Z at NEWOPS, BANKSET the BBANK
 * and INTBIT15 the bank's bit 15. A run the readable code refuses part
 * way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_tpi_search.h"
#include "../tpi_search/tpi_search.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* Channel 7's superbank bit. */
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
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
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane()). */
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

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* P17 and P77 */
    {KEY(035u, P17), p17},
    {KEY(035u, P17 + 1u), p17_after_avflaga},
    {KEY(035u, P77), p77},
    /* P17.1 */
    {KEY(035u, P17_1), p17_1},
    {KEY(035u, P17_1 + 1u), p17_1_after_p20flgon},
    {KEY(035u, P17_1 + 3u), p17_1_after_vnpooh},
    {KEY(035u, P17_1 + 020u), p17_1_after_exit},
    {KEY(035u, P17_1 + 022u), p17_1_after_vncomp17},
    /* P17.3 */
    {KEY(035u, P17_3), p17_3},
    {KEY(035u, P17_3 + 2u), p17_3_after_vncomp17},
    {KEY(035u, P17_3 + 5u), p17_3_terminate},
    {KEY(035u, P17_3 + 6u), p17_3_proceed},
    {KEY(035u, P17_3 + 7u), p17_3_recycle},
    {KEY(035u, P17_3 + 010u), p17_3_immediate_return},
    {KEY(035u, P17_3 + 012u), p17_3_after_blanket},
    /* VNCOMP17 */
    {KEY(035u, VNCOMP17 + 6u), vncomp17_terminate},
    {KEY(035u, VNCOMP17 + 7u), vncomp17_proceed},
    {KEY(035u, VNCOMP17 + 010u), vncomp17_recycle},
    /* ALARUMS */
    {KEY(035u, ALARUMS + 2u), alarums_after_exit},
    {KEY(035u, ALARUMS + 4u), alarums_after_alarm},
    {KEY(035u, ALARUMS + 6u), alarums_after_vncomp17},
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
    /* Bank 35 */
    &P17_1_CODE,
    &P17_1_OPTION_CODE,
    &ALARUMS_CODE,
    /* Bank 36 */
    &S17_1_CODE,
    &S17_2_CODE,
    &BIS_CODE,
    &CONCAUL_CODE,
    &JUNCT3_CODE,
    &JUNCT1_CODE,
    &INTERP_CODE,
    &HALFSAFE_CODE,
    &LRGRDVO_CODE,
    &RNGETEST_CODE,
    &USEKAY_CODE,
    /* Bank 07 */
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

/* ---- The keys, in the faithful tables' order ---- */

/* Each basic key's routine, the readable one. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane()) return EXEC_OUT_OF_SCOPE;

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
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, (word)(02000u + key % 1024u));
    enum exec_status status;

    if (!code || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
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
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
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
