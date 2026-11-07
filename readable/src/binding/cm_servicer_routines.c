/* SPDX-License-Identifier: MIT */
/* The readable SERVICER207 of the CM (src/servicer_cm/) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_servicer.c): cm_servicer_basic (the basic code)
 * and cm_servicer_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_servicer_keys.inc, which tools/p20_p25_keys.py writes from them). A
 * key the readable code has written runs the readable routine, any other
 * the faithful port's (renamed faithful_): the file is written part by
 * part (README.md).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (WAITLIST, PHASCHNG, TASKOVER, ENDOFJOB, ...) or reaches NEWOPS
 * through a TC INTPRET; where the code computes the address it goes on at
 * (TC Q, TC RUPTREG1, AVERAGEG's DXCH Z), the run goes on with the
 * function that starts at that word, as long as one does (the words the
 * faithful routine runs on from). An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()).
 *
 * The device is the CM's io record (cm_native_io); the file writes one
 * channel, DSALMOUT (channel 11, the record's), and its basic routines run
 * only in a program with its hardware record, as the faithful ones. The
 * states the faithful routines take: FB bank 37 (the superbank bit clear),
 * ZERO +0, the bank registers agreeing, the interrupt and superbank
 * latches in range and every erasable word an ordinary one; at an order
 * pair also NEWJOB +0 or a core set, FIXLOC one of the VAC areas, Z at
 * NEWOPS, BANKSET the BBANK and INTBIT15 the bank's bit 15. A run the
 * readable code refuses part way leaves the state and io record as they
 * were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_servicer.h"
#include "../servicer_cm/servicer_cm.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The longest run of computed transfers a run follows. */
#define RUN_LIMIT 256u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* DSALMOUT is the io record's (bind_machine); no other channel. */
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

/* The states the faithful basic routines take (its basic_sane()). */
static bool basic_sane(void)
{
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (FBANK >= 060000u && (*agc.superbank & 0100u)) return false;
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

/* ---- The basic code ---- */

/* The words a function starts at: the keys, and the words a computed
 * transfer may reach. */
static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
} basic_entries[] = {
    /* PREREAD and READACCS (preread.c) */
    {KEY(037u, PREREAD), preread},
    {KEY(037u, REDO5_31), redo5_31},
    {KEY(037u, REDO5_31 + 1u), redo5_31_after_preread1},
    {KEY(037u, REDO5_31 + 5u), redo5_31_after_findvac},
    {KEY(037u, REDO5_31 + 011u), redo5_31_after_waitlist},
    {KEY(037u, REDO5_31 + 014u), redo5_31_after_newphase},
    {KEY(037u, PREREAD1), preread1},
    {KEY(037u, PREREAD1 + 3u), preread1_after_pipasr},
    {KEY(037u, READACCS), readaccs},
    {KEY(037u, PIPSDONE), pipsdone},
    {KEY(037u, REDO5_5), redo5_5},
    {KEY(037u, REDO5_5 + 010u), redo5_5_entry_dap},
    {KEY(037u, REDO5_5 + 034u), redo5_5_after_quikread},
    {KEY(037u, NOSAVPIP + 6u), nosavpip_after_waitlist},
    {KEY(037u, NOSAVPIP + 011u), nosavpip_after_newphase},
    {KEY(037u, CHEKAVEG), chekaveg},
    {KEY(037u, CHEKAVEG + 4u), recall_readaccs},
    {KEY(037u, MAKESERV), makeserv},
    {KEY(037u, MAKESERV + 4u), makeserv_after_findvac},
    {KEY(037u, MAKESERV + 7u), makeserv_after_newphase},
    {KEY(037u, QUIKREAD), quikread},
    /* SERVICER to NORMLIZE (servicer.c) */
    {KEY(037u, SERVICER), servicer},
    {KEY(037u, PIPCHECK + 5u), pipcheck_no_count},
    {KEY(037u, PIPCHECK + 6u), saturation_test},
    {KEY(037u, PIPCHECK + 013u), pipcheck_after_alarm},
    {KEY(037u, PIPLOOP), piploop},
    {KEY(037u, PIPLOOP + 7u), piploop_after_phaschng},
    {KEY(037u, DVTOTUP), dvtotup},
    {KEY(037u, DVTOTUP + 5u), dvtotup_after_exit},
    {KEY(037u, AVERAGEG + 2u), averageg_after_phaschng},
    {KEY(037u, AVERAGEG + 6u), averageg_after_exit},
    {KEY(037u, AVERAGEG + 010u), averageg_copy},
    {KEY(037u, AVERAGEG + 014u), averageg_after_gentran},
    {KEY(037u, AVERAGEG + 017u), averageg_exit},
    {KEY(037u, AVGEND), avgend},
    {KEY(037u, AVGEND + 4u), avgend_after_upflag},
    {KEY(037u, AVGEND + 010u), avgend_after_2phschng},
    {KEY(037u, AVGEND + 014u), avgend_after_exit},
    {KEY(037u, AVGEND + 021u), avgend_after_pipfree},
    {KEY(037u, AVGEND + 027u), avgend_after_downflag},
    {KEY(037u, AVGEND + 031u), avgend_after_downflag_2},
    {KEY(037u, AVGEND + 040u), avgend_after_2phschng_2},
    {KEY(037u, SERVEXIT), servexit},
    {KEY(037u, SERVEXIT + 2u), servexit_after_phaschng},
    {KEY(037u, NORMLIZE), normlize},
    {KEY(037u, NORMLIZE + 4u), normlize_after_gentran},
    {KEY(037u, NORMLIZE + 015u), normlize_after_exit},
    /* PIPASR and REREADAC (pipasr.c) */
    {KEY(037u, REREADAC), rereadac},
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
    &DVTOTUP_CODE,
    &AVERAGEG_CODE,
    &AVGEND_CODE,
    &NORMLIZE_CODE,
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
extern const agc_routine_table faithful_cm_servicer_basic, faithful_cm_servicer_interpretive;

/* A readable basic routine, from a state the faithful one takes, and on
 * from each word a computed transfer reaches that a function starts at
 * (under the FB it leaves). */
static enum exec_status run_basic(void (*routine)(void), apollo_overlay_state *state,
                                  cm_native_io *io)
{
    cm_native_io kept = *io;
    apollo_overlay_state before = *state;
    const struct basic_entry *next;
    unsigned n;

    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    routine();
    for (n = 0; Z >= 02000u && (next = basic_entry((uint16_t)fixed_physical(Z))) != NULL; ++n) {
        if (n == RUN_LIMIT) not_modelled();
        next->routine();
    }
    return EXEC_OK;
}

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device;

    if (!entry) return faithful_cm_servicer_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)((key / 1024u) << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
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
        return faithful_cm_servicer_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || (*agc.superbank & 0100u) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !sane())
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, io, code) == EXEC_OK ? EXEC_OK : EXEC_OUT_OF_SCOPE;
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_servicer_readable(uint16_t key, bool order_pair)
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
#include "cm_servicer_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_servicer_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_servicer_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_servicer_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_servicer_basic = {basic_keys, COUNT(basic_keys)};

#endif
