/* SPDX-License-Identifier: MIT */
/* The readable P11 of the CM (src/p11/) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p11.c): cm_p11_basic (the basic code) and
 * cm_p11_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_p11_keys.inc, which tools/p20_p25_keys.py writes from them). Every
 * basic key runs a readable routine, and every order pair of bank 34 the
 * readable interpreter on the listing's code; bank 37's order pairs run
 * the faithful port's routines (renamed faithful_) until part 3.
 *
 * Partial file: Comanche055/P11.agc (COVERAGE counts it when whole).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, IBNKCALL, PHASCHNG, WAITLIST, FINDVAC, POWRSERS,
 * DANZIG, RESUME, ...) or reaches NEWOPS through a TC INTPRET; an order
 * pair is one pass of the interpreter, from NEWOPS with LOC the key.
 *
 * The device is the CM's io record (cm_native_io), which keeps channel
 * 12 (the Saturn takeover's bit 9); channels 30 (the S/C control of
 * Saturn switch) and 31 (the hand controller) are the program's hardware
 * record's. The faithful routines take that record, and without one
 * refuse, so the binding does too. The states they take: FB the key's
 * bank (in bank 34 with the superbank bit clear, for bank 43 FB 33 with
 * it set), ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range, and every erasable word an ordinary one;
 * an order pair, too, NEWJOB +0 or positive and FIXLOC a VAC area. A run
 * the readable code refuses part way leaves the state and io record as
 * they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p11.h"
#include "../p11/p11.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* Channel 7's superbank bit. */
#define SUPERBANK_BIT 0100u

static jmp_buf back;

/* The run's io record and hardware record. */
static cm_native_io *io_now;
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* ATERJOB's channel 30 (RAND), SATSTICK's channel 31 (RXOR), and the
 * takeover's channel 12 (WOR, WAND), as the faithful port keeps them. */
static word channel_in(unsigned number)
{
    if (number == 012u) return (word)io_now->channel12;
    if (number == 030u) return (word)hardware_now->channel30;
    if (number == 031u) return (word)hardware_now->channel31;
    not_modelled(); /* the file reads no other channel */
    return 0;
}

static void channel_out(unsigned number, word value)
{
    if (number != 012u) not_modelled(); /* the file writes no other channel */
    io_now->channel12 = (agc_word)value;
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
    io_now = io;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful basic routines take (its basic_sane()). */
static bool basic_sane(void)
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

/* And the order pairs (its sane()): NEWJOB and FIXLOC too. */
static bool sane(void)
{
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    return basic_sane();
}

/* The key's bank selected: FB, and in banks 30 to 37 the superbank bit
 * clear (with it set, FB 30 to 37 would be banks 40 to 47); banks 40 to
 * 43 are FB 30 to 33 with SUPERBNK 0100. */
static bool in_bank(unsigned bank)
{
    if (bank >= 040u)
        return FBANK == (word)((bank - 010u) << 10) && (*agc.superbank & 0160u) == SUPERBANK_BIT;
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* Part 1: P11, REP11, VHHDOT, ATERTASK (p11.c) */
    {KEY(034u, P11), p11},
    {KEY(034u, REP11A), rep11a},
    {KEY(034u, REP11A + 015u), rep11a_after_phaschng},
    {KEY(034u, REP11A + 027u), rep11a_after_preread1},
    {KEY(034u, REP11A + 032u), rep11a_start_display},
    {KEY(034u, REP11A + 036u), rep11a_after_waitlist},
    {KEY(034u, REP11A + 040u), rep11a_after_newmodex},
    {KEY(034u, REP11A + 042u), rep11a_after_upflag},
    {KEY(034u, REP11A + 046u), rep11a_after_cleandsp},
    {KEY(034u, REP11A + 051u), rep11a_after_2phschng},
    {KEY(034u, MATRXJOB), matrxjob},
    {KEY(034u, MATRXJOB + 0101u), matrxjob_after_exit},
    {KEY(034u, MATRXJOB + 0103u), matrxjob_after_phaschng},
    {KEY(034u, MATRXJOB + 0127u), matrxjob_after_waitlist},
    {KEY(034u, MATRXJOB + 0132u), matrxjob_after_2phschng},
    {KEY(034u, REP11), rep11},
    {KEY(034u, VHHDOT), vhhdot},
    {KEY(034u, VHHDOT + 4u), vhhdot_after_exit},
    {KEY(034u, VHHDOT + 6u), vhhdot_after_phaschng},
    {KEY(034u, ATERTASK), atertask},
    {KEY(034u, ATERTASK + 4u), atertask_after_findvac},
    {KEY(034u, ATERTASK + 011u), atertask_after_needler},
    /* Part 1: ATERJOB to TAKEON, DELSTOR, RLTST, NOPOLY (aterjob.c) */
    {KEY(034u, ATERJOB), aterjob},
    {KEY(034u, ATTDISP + 022u), attdisp_after_powrsers},
    {KEY(034u, ATTDISP2 + 030u), attdisp2_after_exit},
    {KEY(034u, STEERSAT + 6u), steersat_after_exit},
    {KEY(034u, ATERSET), aterset},
    {KEY(034u, ATERSET + 3u), aterset_after_delayjob},
    {KEY(034u, ATRESET + 5u), atreset_after_needler},
    {KEY(034u, ATRESET + 010u), atreset_after_delayjob},
    {KEY(034u, DELSTOR), delstor},
    {KEY(034u, RLTST), rltst},
    {KEY(034u, NOPOLY), nopoly},
    /* Part 1: SATSTKON, REDOSAT, SATSTICK (satstick.c) */
    {KEY(023u, REDOSAT), redosat},
    {KEY(023u, REDOSAT + 010u), redosat_after_needler},
    {KEY(023u, SATSTICK), satstick},
    {KEY(023u, SATSTICK + 015u), satstick_after_stickchk},
    {KEY(023u, SATSTICK + 030u), satstick_after_needler},
    {KEY(043u, SATSTKON), satstkon},
    {KEY(043u, SATSTKON + 013u), satstkon_after_zerojet},
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
    /* Part 2: bank 34 */
    &P11_CODE,      &MATRXJOB_CODE, &VHHDOT_CODE, &GETDOWN_CODE, &ATTDISP_CODE,
    &ATTDISP2_CODE, &STEERSAT_CODE, &S11_1_CODE,  &NOPOLY_CODE,
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

/* Each basic key's routine, the readable one. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !basic_sane()) return EXEC_OUT_OF_SCOPE;

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

/* The faithful order pairs, renamed. */
extern const agc_routine_table faithful_cm_p11_interpretive;

/* Each order pair's routine: one pass of the readable interpreter where
 * its code is written, the faithful port's routine where it is not yet. */
static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code = code_at(bank, (word)(02000u + key % 1024u));
    enum exec_status status;

    if (!code) return faithful_cm_p11_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != NEWOPS || !in_bank(bank) || LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane())
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
bool cm_p11_readable(uint16_t key, bool order_pair)
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
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#include "cm_p11_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p11_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p11_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p11_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p11_basic = {basic_keys, COUNT(basic_keys)};

#endif
