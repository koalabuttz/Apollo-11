/* SPDX-License-Identifier: MIT */
/* The readable ASCENT_GUIDANCE (src/ascent/) as routines of the LM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_ascent.c): lm_ascent_basic (the basic code, banks 14,
 * 30 and 34) and lm_ascent_interpretive (the order pairs, banks 7, 14,
 * 27, 30 and 34). The tables carry every key of the faithful ones, in
 * their order (ascent_keys.inc, which tools/p20_p25_keys.py writes from
 * them), and every key runs a readable routine.
 *
 * Plumbing only. A basic routine runs from its key until its code leaves
 * the file (PHASCHNG, TPAGREE, TWIDDLE, IBNKCALL, BANKCALL, POLY, ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()),
 * which ends at the pair's DANZIG, at an EXIT, or where a CALL or branch
 * leaves it; a pass that ends at ENGOFF's RTB LOADTIME runs LOADTIME and
 * goes on, as the faithful pair does.
 *
 * The states the faithful routines take, at every key: FB the key's bank,
 * with the superbank bit clear in banks 30 and 34; at an order pair Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO
 * +0, the bank registers agreeing, the interrupt and superbank latches in
 * range, and every erasable word an ordinary one. At an order pair and at
 * the basic keys the faithful port marks so (its at_basic: those that
 * reach a TC INTPRET), NEWJOB +0 or a core set and FIXLOC one of the VAC
 * areas; at the other basic keys (its at_basic_nv) NEWJOB may be -0 too,
 * and FIXLOC anything. A run the readable code refuses part way leaves the
 * state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../ascent/ascent.h"
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
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* ENGOFF, ENGOFF1 and CUTOFF (bank 14) */
    {KEY(014u, ENGOFF + 6u), engoff_after_exit, false},
    {KEY(014u, ENGOFF + 7u), engoff_after_tpagree, false},
    {KEY(014u, ENGOFF + 022u), engoff_after_twiddle, false},
    {KEY(014u, ENGOFF + 024u), engoff_restart_words, false},
    {KEY(014u, ENGOFF + 027u), engoff_intpret, true},
    {KEY(014u, ENGOFF1), engoff1, false},
    {KEY(014u, ENGOFF1 + 2u), engoff1_after_enginof2, false},
    {KEY(014u, ENGOFF1 + 6u), engoff1_after_findvac, false},
    {KEY(014u, ENGOFF1 + 013u), engoff1_after_phaschng, false},
    {KEY(014u, CUTOFF), cutoff, false},
    {KEY(014u, CUTOFF + 2u), cutoff_after_upflag, false},
    {KEY(014u, CUTOFF + 5u), cutoff_terminate, false},
    {KEY(014u, CUTOFF + 6u), cutoff_proceed, false},
    {KEY(014u, CUTOFF + 7u), cutoff_enter, false},
    {KEY(014u, CUTOFF1 + 3u), cutoff1_after_zatteror, false},
    {KEY(014u, CUTOFF1 + 5u), cutoff1_after_setmindb, false},
    /* ASCTERM, CUTOFF2, TERMASC and LOGSUB (bank 30) */
    {KEY(030u, ASCTERM + 1u), ascterm_after_exit, true},
    {KEY(030u, ASCTERM1 + 1u), ascterm1_after_exit, false},
    {KEY(030u, ASCTERM1 + 014u), ascterm1_after_godspr, false},
    {KEY(030u, ASCTERM2 + 1u), ascterm3, false}, /* ASCTERM3 */
    {KEY(030u, ASCTERM4 + 1u), ascterm4_after_exit, false},
    {KEY(030u, ASCTERM4 + 4u), ascterm4_after_zatteror, false},
    {KEY(030u, CUTOFF2), cutoff2, false},
    {KEY(030u, CUTOFF2 + 2u), cutoff2_after_phaschng, false},
    {KEY(030u, CUTOFF2 + 5u), cutoff2_terminate, false},
    {KEY(030u, CUTOFF2 + 6u), cutoff2_proceed, false},
    {KEY(030u, CUTOFF2 + 7u), cutoff2_enter, false},
    {KEY(030u, TERMASC), termasc, false},
    {KEY(030u, TERMASC + 2u), termasc_after_phaschng, false},
    {KEY(030u, TERMASC + 5u), termasc_after_restordb, false},
    {KEY(030u, TERMASC + 7u), termasc_after_downflag, false},
    {KEY(030u, LOGSUB + 4u), logsub_after_exit, false},
    {KEY(030u, LOGSUB + 026u), logsub_after_poly, false},
    {KEY(030u, LOGSUB + 036u), logsub_after_shortmp, true},
    /* ATMAG (bank 34) */
    {KEY(034u, ATMAG), atmag, false},
    {KEY(034u, ATMAG + 2u), atmag_after_phaschng, true},
    {KEY(034u, ATMAG + 021u), atmag_after_exit, true},
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
    &CHECKYAW_CODE, &KEEPVR_CODE, &ENGOFF_CODE,       &ENGOFF_SET_CODE, &ANG1CHEK_CODE,
    &YCOMP_CODE,    &ASCENT_CODE, &ASCTERM_CALL_CODE, &ASCTERM2_CODE,   &ASCTERM4_CODE,
    &CHECKALT_CODE, &LOGSUB_CODE, &LOGSUB_RVQ_CODE,   &ATMAG_CODE,      &ATMAG_AFTER_EXIT_CODE};

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
    if (!in_bank(key / 1024u) || !sane(entry->with_work_area)) return EXEC_OUT_OF_SCOPE;

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

/* One pass; after ENGOFF's RTB LOADTIME, LOADTIME and the DANZIG it ends
 * at, on through the rest of the pair, as the faithful pair does. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 p66_bias_io *io, const struct interpretive_code *code)
{
    enum exec_status status = interpret_listing(program, state, io, code);

    if (status != EXEC_OK) return status;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
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
bool ascent_readable(uint16_t key, bool order_pair)
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
#include "ascent_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "ascent_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "ascent_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_ascent_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_ascent_basic = {basic_keys, COUNT(basic_keys)};

#endif
