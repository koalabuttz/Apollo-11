/* SPDX-License-Identifier: MIT */
/* The readable P37_P70 of the CM (src/p37_p70/) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p37_p70.c): cm_p37_p70_basic (the basic code) and
 * cm_p37_p70_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_p37_p70_keys.inc, which tools/p20_p25_keys.py writes from them). The
 * file is written part by part: a key a part has written runs the
 * readable routine, any other the faithful port's (renamed faithful_).
 * The side-by-side test (tests/cm_p37_p70_cases.c) checks which.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (PHASCHNG, BANKCALL, POLY, BLANKET, ENDOFJOB, GOTOPOOH), comes
 * back to a word it is keyed at (TC SPRTEX, INDEX SPRTEX; TCF ...) or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()).
 *
 * The device is the CM's io record (cm_native_io); the file reads and
 * writes no channel. The states the faithful routines take: FB the key's
 * bank (in banks 30-37 with the superbank bit off), ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range, every
 * erasable word an ordinary one, NEWJOB +0 or a core set and FIXLOC one
 * of the VAC areas; at an order pair also Z at NEWOPS, BANKSET the BBANK
 * and INTBIT15 the bank's bit 15. They need no hardware record. A run the
 * readable code refuses part way leaves the state and io record as they
 * were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p37_p70.h"
#include "../p37_p70/p37_p70.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

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

/* The interpreter's fixed operands, and the file's constants, are the
 * rope's. */
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

/* The states the faithful routines take (its sane() and sb_ok()). */
static bool sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if ((FBANK & 060000u) == 060000u && (*agc.superbank & 0100u)) return false; /* banks 30-37 */
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The basic code ---- */

/* The words a function starts at: the keys. A recycle that goes back to
 * an earlier word (TCF -2, TCF P37) runs that word's function. */
static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P37's main line (p37.c) */
    {KEY(036u, P37), p37},
    {KEY(036u, P37 + 2u), p37_after_phaschng},
    {KEY(036u, P37 + 013u), p37_tig},
    {KEY(036u, P37 + 015u), p37_tig}, /* P370GOF's recycle: TCF -2 */
    {KEY(036u, P37 + 016u), p37_gamma},
    {KEY(036u, P37 + 020u), p37_gamma}, /* P37GFRB1's recycle: TCF -2 */
    {KEY(036u, RTE299), rte299},
    {KEY(036u, RTE317 + 2u), rte317_after_exit},
    {KEY(036u, RTE317 + 014u), rte317_after_poly},
    {KEY(036u, RTE369 + 3u), rte369_after_exit},
    {KEY(036u, RTE369 + 015u), rte369_after_poly},
    {KEY(036u, RTEVN - 1u), p37}, /* RTEALRM +3: TCF P37 */
    /* Part 1: the displays (p37_displays.c) */
    {KEY(036u, RTEVN + 4u), rtevn_after_exit},
    {KEY(036u, RTEVN + 6u), rtevn_blank_r3},
    {KEY(036u, RTEVN + 010u), rtevn_v6n60}, /* TCF +5 */
    {KEY(036u, RTEVN + 011u), p37},         /* GOFLASHR's recycle: TCF P37 */
    {KEY(036u, RTEVN + 012u), rtevn_v6n39},
    {KEY(036u, RTEVN + 014u), p37}, /* P370GOF's recycle: TCF P37 */
    {KEY(036u, RTEVN + 015u), rtevn_v6n60},
    {KEY(036u, RTEVN + 017u), p37}, /* P37GFRB1's recycle: TCF P37 */
    {KEY(036u, RTEVN + 020u), rtevn_v6n81},
    {KEY(036u, RTEVN + 022u), p37}, /* P370GOF's recycle: TCF P37 */
    {KEY(036u, RTEVN + 023u), rtevn_return},
    {KEY(036u, P37N), p37n_after_exit},
    {KEY(036u, P37N + 6u), p37n_option}, /* P370GOF's recycle: TCF -2 */
    {KEY(036u, P37N + 7u), p37n_proceed},
    {KEY(036u, P37T + 3u), p37t_after_exit},
    {KEY(036u, P37T + 013u), p37t_after_poly},
    {KEY(036u, P37T + 026u), p37t_tig},
    {KEY(036u, P37T + 030u), p37t_tig}, /* P370GOF's recycle: TCF -2 */
    {KEY(036u, P37T + 031u), p37t_proceed},
    {KEY(036u, K_37BLANK), blank37},
    {KEY(036u, K_37BLANK + 2u), blank37_after_blanket},
    {KEY(036u, K_37BLANK + 3u), blank37_recycle},
    {KEY(036u, K_37BLANK + 4u), blank37_proceed},
    {KEY(036u, P370GOFR + 4u), p370gofr_terminate},
    {KEY(036u, P370GOFR + 5u), p370gofr_proceed},
    {KEY(036u, P370GOFR + 6u), p370gofr_recycle},
    {KEY(036u, P370GOFR + 7u), p370gofr_immediate},
    {KEY(036u, P370GOF + 4u), p370gof_terminate},
    {KEY(036u, P370GOF + 5u), p370gof_proceed},
    {KEY(036u, P370GOF + 6u), p370gof_recycle},
    /* Part 2: P370ALRM (rtedisp.c) */
    {KEY(032u, P370ALRM + 2u), p370alrm_after_exit},
    {KEY(032u, P370ALRM + 4u), p370alrm_after_varalarm},
    {KEY(032u, P370ALRM + 7u), p370alrm_terminate},
    {KEY(032u, P370ALRM + 010u), p370alrm_after_varalarm}, /* GOFLASH's proceed: TCF -4 */
    {KEY(032u, P370ALRM + 011u), p370alrm_recycle},
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
    /* Part 1 */
    &P37_CODE,
    &RTE299_CODE,
    &RTE317_POLY_CODE,
    &RTE369_POLY_CODE,
    &RTEVN_CODE,
    &RTEVN_RETURN_CODE,
    &P37N_CODE,
    &P37T_POLY_CODE,
    &P37T_TIG_CODE,
    /* Part 2 */
    &P370ALRM_CODE,
    &P370ALRM_GOTO_CODE,
    &TMRAD100_CODE,
    &RTD18_CODE,
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

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p37_p70_basic, faithful_cm_p37_p70_interpretive;

/* A readable basic routine, from a state the faithful one takes. */
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
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device;

    if (!entry) return faithful_cm_p37_p70_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
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
        return faithful_cm_p37_p70_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p37_p70_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return basic_entry(key) != NULL;
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
#include "cm_p37_p70_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p37_p70_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p37_p70_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p37_p70_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p37_p70_basic = {basic_keys, COUNT(basic_keys)};

#endif
