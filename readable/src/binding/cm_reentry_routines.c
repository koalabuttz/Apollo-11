/* SPDX-License-Identifier: MIT */
/* The readable REENTRY_CONTROL of the CM (src/reentry/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_reentry.c): cm_reentry_basic (the basic code)
 * and cm_reentry_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_reentry_keys.inc, which tools/p20_p25_keys.py writes from them), and
 * every key runs a readable routine: the file was written part by part,
 * and is whole. The side-by-side test (tests/cm_reentry_cases.c) checks
 * that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (PHASCHNG, PRIOCHNG, NOVAC, 2PHSCHNG, BANKCALL, CHANG1,
 * POSTJUMP) or reaches NEWOPS through a TC INTPRET. An order pair is one
 * pass of the readable interpreter over the listing's words
 * (interpret_listing()), on through TCDANZIG after a BOVB.
 *
 * The device is the CM's io record (cm_native_io); the file's basic
 * routines run only in a program with its hardware record, as the
 * faithful ones. The states the faithful routines take: at a basic key,
 * FB the key's bank, ZERO +0, the bank registers agreeing, the interrupt
 * and superbank latches in range and every erasable word an ordinary one,
 * and in banks 30-37 the superbank bit off; at an order pair the same but
 * the superbank bit, NEWJOB +0 or a core set, FIXLOC one of the VAC areas,
 * Z at NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15 (or, at
 * the two keys after an RTB, Z at DANZIG: see pending_orders). A
 * run the readable code refuses part way leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_reentry.h"
#include "../reentry/reentry.h"
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

/* The states the faithful routines take: the bank registers, the latches,
 * and every erasable word an ordinary one; ... */
static bool machine_sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ... at a basic key also the superbank bit off in banks 30-37 (its
 * basic_sane()); ... */
static bool basic_sane(void)
{
    return machine_sane() && !(FBANK >= 060000u && (*agc.superbank & 0100u));
}

/* ... and at an order pair NEWJOB and FIXLOC (its sane()). */
static bool sane(void)
{
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    return machine_sane();
}

/* ---- The basic code ---- */

/* The words a function starts at: the keys. */
static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* STARTENT to REFAZE10 (startent.c) */
    {KEY(025u, STARTENT + 1u), startent_after_exit},
    {KEY(025u, REFAZE10), refaze10},
    {KEY(025u, REFAZE10 + 2u), refaze10_after_phaschng},
    /* SIDETRAK and PRE-HUNT (ranger.c) */
    {KEY(025u, SIDETRAK + 1u), sidetrak_after_exit},
    {KEY(025u, SIDETRAK + 7u), sidetrak_after_phaschng},
    {KEY(025u, SIDETRAK + 011u), sidetrak_after_priochng},
    {KEY(025u, PRE_HUNT), pre_hunt},
    /* PREDICT3 (kep.c) */
    {KEY(025u, PREDICT3 + 5u), predict3_after_exit},
    /* OVERNOUT to STEEROFF (overnout.c) */
    {KEY(025u, OVERNOUT), overnout},
    {KEY(025u, NODISKY), nodisky},
    {KEY(025u, SERVNOUT), servnout},
    {KEY(025u, STEEROFF + 1u), steeroff_after_exit},
    {KEY(025u, STEEROFF + 7u), steeroff_after_novac},
    {KEY(025u, STEEROFF + 012u), steeroff_after_2phschng},
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
    &STARTENT_CODE,
    &STARTENT_INTERPRETIVE_CODE,
    &SCALEPOP_CODE,
    &REFAZE10_CODE,
    &TARGETNG_CODE,
    &GETETA_CODE,
    &INITROLL_CODE,
    &HUNTEST_CODE,
    &RANGER_CODE,
    &SIDETRAK_CODE,
    &PRE_HUNT_CODE,
    &GOTOUPSY_CODE,
    &UPCONTRL_CODE,
    &DCONSTD_CODE,
    &KEP_CODE,
    &PREDICT3_CODE,
    &HAVDNRNG_CODE,
    &STEEROFF_CODE,
    &STEEROFF_INTERPRETIVE_CODE,
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

/* Each key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device;

    if (!entry || !program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* The keys a pair's right-hand order waits at in EDOP while its left, an
 * RTB, runs a routine: KEP's RTB SSP (P66, then SSP GOTOADDR) and the RTB
 * DMP after SIGNMPAC. The routine ends at DANZIG, which finds the order in
 * EDOP and resumes the pass at LOC: EDOP holds the order's code plus
 * one. */
static const struct pending_order {
    uint16_t key;
    word edop;
} pending_orders[] = {
    {KEY(025u, KEP + 1u), ORDER_SSP + 1u},
    {KEY(025u, HAVDNRNG + 020u), ORDER_DMP + 1u}, /* GOMAXL/D +1 */
};

static const struct pending_order *pending_order(uint16_t key)
{
    unsigned i;

    for (i = 0; i < COUNT(pending_orders); ++i)
        if (pending_orders[i].key == key) return &pending_orders[i];
    return NULL;
}

/* The states the faithful port resumes such a pair from: Z at DANZIG, the
 * order in EDOP, BANKSET the pair's bank (FB may be any) with no E-bank
 * bits, and INTBIT15 the bank's bit 15. */
static bool at_danzig(unsigned bank, const struct pending_order *pending)
{
    return Z == DANZIG && EDOP == pending->edop && ((BANKSET >> 10) & 037u) == bank &&
           !(BANKSET & 01770u) && INTBIT15 == ((bank << 10) & 040000u) && sane();
}

/* One pass of the interpreter; after RANGER's BOVB to TCDANZIG, its TC
 * DANZIG (ENDT(X), TIME_OF_FREE_FALL.agc), which only resets OVFIND: the
 * pass goes on with the next order. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != TCDANZIG || FBANK != (word)(I_TCDANZIG & 076000u)) return EXEC_OK;
    if (!sane()) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    tc_at(TCDANZIG);
    continue_at(DANZIG);
    return interpret_listing(program, state, io, code) == EXEC_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    const struct pending_order *pending;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    pending = pending_order(key);
    if (!code || LOC != (word)(02000u + key % 1024u) ||
        (pending ? !at_danzig(bank, pending)
                 : Z != NEWOPS || FBANK != (word)(bank << 10) || BANKSET != BBANK ||
                       INTBIT15 != (FBANK & 040000u) || !sane()))
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
bool cm_reentry_readable(uint16_t key, bool order_pair)
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
#include "cm_reentry_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_reentry_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_reentry_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_reentry_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_reentry_basic = {basic_keys, COUNT(basic_keys)};

#endif
