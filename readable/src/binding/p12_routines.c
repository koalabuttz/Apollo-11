/* SPDX-License-Identifier: MIT */
/* The readable P12 (src/p12/) as routines of the LM's flight program,
 * under the table names and keys of the faithful port's
 * (c64-port/src/lm_p12.c): lm_p12_basic (the basic code, bank 30) and
 * lm_p12_interpretive (the order pairs, banks 30 and 23). The tables carry
 * every key of the faithful ones, in their order (p12_keys.inc, which
 * tools/p20_p25_keys.py writes from them).
 *
 * Plumbing only. A basic routine runs from its key until its code leaves
 * the file (PHASCHNG, BANKCALL, UPFLAG, GOTOPOOH, POSTJUMP, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or branch
 * leaves its table; a pass that ends at GUIDINIT's RTB LOADTIME runs
 * LOADTIME and goes on, as the faithful pair does.
 *
 * The states the faithful routines take: FB the key's bank, with the
 * superbank bit clear in bank 30; at an order pair Z at NEWOPS, BANKSET
 * the BBANK and INTBIT15 the bank's bit 15. Always ZERO +0, the bank
 * registers agreeing, NEWJOB +0 or a core set, FIXLOC one of the VAC
 * areas, the interrupt and superbank latches in range and every erasable
 * word an ordinary one. A run the readable code refuses part way leaves
 * the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../p12/p12.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, p66_bias_io *io)
{
    bind_machine(program, state, io);
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane()). */
static bool sane(void)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u || FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u)
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The key's bank selected: FB, and in bank 30 the superbank bit clear
 * (with it set, FB 30 would be bank 40). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    {KEY(030u, P12LM), p12lm},
    {KEY(030u, P12LM + 2u), p12lm_after_phaschng},
    {KEY(030u, P12LM + 4u), p12lm_after_r02both},
    {KEY(030u, P12LM + 6u), p12lm_after_munflag},
    {KEY(030u, P12LM + 010u), p12lm_after_acc4_2fl},
    {KEY(030u, P12LM + 012u), p12lm_after_r10flag},
    {KEY(030u, P12LM + 013u), p12lm_after_clradmod},
    {KEY(030u, P12LM + 015u), p12lm_after_rndvzflg},
    {KEY(030u, P12LM + 026u), p12lm_terminate},
    {KEY(030u, P12LM + 027u), p12lm_proceed},
    {KEY(030u, P12LM + 030u), p12lm_enter},
    {KEY(030u, P12LM + 033u), p12lm_intpret},
    {KEY(030u, NEWLOAD - 2u), p12lmb_after_exit},
    {KEY(030u, NEWLOAD), newload},
    {KEY(030u, NEWLOAD + 3u), newload_terminate},
    {KEY(030u, NEWLOAD + 4u), newload_proceed},
    {KEY(030u, NEWLOAD + 5u), newload_enter},
    {KEY(030u, NEWLOAD + 012u), newload_intpret},
    {KEY(030u, NEWLOAD + 062u), p12ret_after_exit},
    {KEY(030u, NEWLOAD + 064u), p12ret_after_phaschng},
    {KEY(030u, NEWLOAD + 066u), p12ret_after_flpi},
    {KEY(030u, NEWLOAD + 071u), p12ret_after_pflitedb},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {&P12LM_CODE, &NEWLOAD_CODE, &P12INIT_CODE,
                                                        &GUIDINIT_CODE};

/* The table an address is in, in a bank, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* ---- The keys, in the faithful tables' order ---- */

static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    p66_bias_io *io = device, kept;
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

/* One pass; after GUIDINIT's RTB LOADTIME, LOADTIME and the DANZIG it
 * ends at, on through the rest of the pair, as the faithful pair does. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 p66_bias_io *io, const struct interpretive_code *code)
{
    enum exec_status status = interpret_listing(program, state, io, code);

    if (status != EXEC_OK) return status;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    p66_bias_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
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
bool p12_readable(uint16_t key, bool order_pair)
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
#include "p12_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "p12_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "p12_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_p12_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_p12_basic = {basic_keys, COUNT(basic_keys)};

#endif
