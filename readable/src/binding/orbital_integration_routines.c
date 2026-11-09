/* SPDX-License-Identifier: MIT */
/* The readable ORBITAL_INTEGRATION (src/orbital_integration/) as routines
 * of each mission's flight program, under the table names and keys of the
 * faithful port's (c64-port/src/lm_orbital_integration.c and
 * cm_orbital_integration.c): <m>_orbital_integration_basic and
 * <m>_orbital_integration_interpretive. The keys come from the faithful
 * tables, in their order (orbital_integration_keys.inc and
 * cm_orbital_integration_keys.inc, which tools/p20_p25_keys.py writes from
 * them). The keys no part has written yet run the faithful port's
 * routines (renamed faithful_) until their parts are written.
 * Partial file: Luminary099/ORBITAL_INTEGRATION.agc and
 * Comanche055/ORBITAL_INTEGRATION.agc.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (POODOO). An order pair is one pass of the readable interpreter
 * over the listing's words (interpret_listing()), in the part of the code
 * LOC is in. KEPPREP's BOVB goes to TCDANZIG only to reset OVFIND: there
 * the TC DANZIG, and the pass goes on with the pair's right-hand order, as
 * the faithful pair does. The file has no RTB.
 *
 * The states the faithful routines take: FB the key's bank, ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range,
 * every erasable word an ordinary one, NEWJOB +0 or a core set and FIXLOC
 * one of the VAC areas (but for the CM's basic code, which takes any
 * NEWJOB and FIXLOC); at an order pair also Z at NEWOPS, BANKSET the BBANK
 * and INTBIT15 the bank's bit 15. A run the readable code refuses part way
 * leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "../orbital_integration/orbital_integration.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099
#define ROPE            0u   /* the LM's rope */
#define BASIC_WORK_AREA true /* the LM's basic keys check the work area too */
#else
#define ROPE            1u /* the CM's */
#define BASIC_WORK_AREA false
#endif

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The bank of the file's code (ORBITAL), and TCDANZIG's (FB), from their
 * FCADRs. */
#define ORBITAL_BANK  CODE_BANK(I_KEPPREP)
#define TCDANZIG_BANK ((word)(I_TCDANZIG & 076000u))

/* The faithful tables, renamed. */
#if LUMINARY099
extern const agc_routine_table faithful_lm_orbital_integration_basic,
    faithful_lm_orbital_integration_interpretive;
#define faithful_basic        faithful_lm_orbital_integration_basic
#define faithful_interpretive faithful_lm_orbital_integration_interpretive
#else
extern const agc_routine_table faithful_cm_orbital_integration_basic,
    faithful_cm_orbital_integration_interpretive;
#define faithful_basic        faithful_cm_orbital_integration_basic
#define faithful_interpretive faithful_cm_orbital_integration_interpretive
#endif

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
 * basic_sane(), which leaves out NEWJOB and the work area). */
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
    /* Part 1 */
    {KEY(ORBITAL_BANK, INT_ABRT + 1u), int_abrt_after_exit},
};

static const struct basic_entry *basic_entry(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].key == key) return &basic_entries[i];
    return NULL;
}

/* ---- The order pairs ---- */

/* The parts of the code written so far. */
static const struct interpretive_code *const codes[] = {
    &KEPPREP_CODE,
    &ACCOMP_CODE,
};

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

/* ENDT(X) (TIME_OF_FREE_FALL.agc), under the name TCDANZIG: TC DANZIG,
 * which resets OVFIND and goes on with the pair's right-hand order. */
static void tcdanzig(void)
{
    tc_at(TCDANZIG);
    continue_at(DANZIG);
}

/* One pass; after KEPPREP's BOVB to TCDANZIG, the TC DANZIG and the rest
 * of the pair. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 mission_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != TCDANZIG || FBANK != TCDANZIG_BANK) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    tcdanzig();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each basic key's routine: the readable one, or the faithful port's until
 * its part is written. A run it refuses part way leaves the state as it
 * was. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    mission_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_basic.entries[index].call(program, state, device);
    if (!program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (FBANK != (word)((key / 1024u) << 10) || !sane(BASIC_WORK_AREA)) return EXEC_OUT_OF_SCOPE;

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

/* Each order pair's routine: one pass of the readable interpreter, or the
 * faithful port's until its part is written. */
static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    word at = (word)(02000u + key % 1024u);
    mission_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, at);
    enum exec_status status;

    if (!code) return faithful_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io || !bind(program, state, io)) return EXEC_OUT_OF_SCOPE;
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || LOC != at || BANKSET != BBANK ||
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
bool orbital_integration_readable(uint16_t key, bool order_pair)
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
#if LUMINARY099
#include "orbital_integration_keys.inc"
#else
#include "cm_orbital_integration_keys.inc"
#endif
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#if LUMINARY099
#include "orbital_integration_keys.inc"
#else
#include "cm_orbital_integration_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#if LUMINARY099
#include "orbital_integration_keys.inc"
#else
#include "cm_orbital_integration_keys.inc"
#endif
};
#undef PAIR
#undef BASIC

#if LUMINARY099
const agc_routine_table lm_orbital_integration_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_orbital_integration_basic = {basic_keys, COUNT(basic_keys)};
#else
const agc_routine_table cm_orbital_integration_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_orbital_integration_basic = {basic_keys, COUNT(basic_keys)};
#endif
