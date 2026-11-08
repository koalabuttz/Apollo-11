/* SPDX-License-Identifier: MIT */
/* The readable TVCSTROKETEST (src/tvc/tvcstroketest.c) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_tvc_stroke.c): cm_tvc_stroke_basic (the basic
 * code; the file has no interpretive code, and cm_tvc_stroke_interpretive
 * no keys).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_tvc_stroke_keys.inc, which tools/p20_p25_keys.py writes from it),
 * and every key runs a readable routine. The side-by-side test
 * (tests/cm_tvc_stroke_cases.c) checks that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (ENDOFJOB, WAITLIST, TC BUNKER). BUNKER, E6,1657, is a
 * key too: the program runs the word HACK and HACKWLST keep there as TC
 * <word>, the DAP's return or TC TASKOVER.
 *
 * The device is the CM's io record (cm_native_io); the faithful routines
 * of the bank take the program's hardware record, and without one refuse,
 * so the binding does too; the word at BUNKER needs none. A burst
 * releases the pitch outcounter in channel 14 (WOR), the io record's. The
 * states the faithful routines take: FB bank 20 (at BUNKER any bank, in
 * banks 30 to 37 with the superbank bit clear), ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_tvc_stroke.h"
#include "../tvc/tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define TVC_STROKE_BANK 020u

static jmp_buf back;

/* The run's io record. */
static cm_native_io *io_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channel 14, which a burst sets the pitch bit of (WOR), as the faithful
 * port keeps it. */
static word channel_in(unsigned number)
{
    if (number == 014u) return (word)io_now->channel14;
    not_modelled(); /* the file uses no other channel */
    return 0;
}

static void channel_out(unsigned number, word value)
{
    if (number == 014u)
        io_now->channel14 = (agc_word)value;
    else
        not_modelled();
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
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: the bank registers, the
 * latches, and every erasable word an ordinary one. */
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

/* ---- The basic code ---- */

/* The words a function starts at: the keys. */
static const struct basic_entry {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {STRKTSTI, strktsti},
    {HACK, hack},
    {HACK + 6u, hack_after_waitlist},
    {HACKWLST, hackwlst},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* BUNKER's key (E6,1657), and the words HACK and HACKWLST keep there: the
 * pitch and yaw DAPs' returns from TC HACK (PSTROKER +2 or +4, YSTROKER +2
 * or +4), and TC TASKOVER. */
#define BUNKER_KEY (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + 03257u)

static bool a_bunker_word(word instruction)
{
    return instruction == PSTROKER + 2u || instruction == PSTROKER + 4u ||
           instruction == YSTROKER + 2u || instruction == YSTROKER + 4u || instruction == TCTSKOVR;
}

/* ---- The keys, in the faithful table's order ---- */

/* A readable routine, from a state the faithful one takes. */
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
    const struct basic_entry *entry = basic_entry((word)(02000u + key % 1024u));
    cm_native_io *io = device;

    if (!entry || !program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (FBANK != (word)(TVC_STROKE_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* BUNKER, run as TC: Z at E6,1657, one of the words kept there. */
static enum exec_status basic_bunker(const agc_native_program *program, apollo_overlay_state *state,
                                     void *device)
{
    cm_native_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if ((Z & 07777u) != ADDRESS_OF(BUNKER) || (EBANK & 03400u) != 03000u || !basic_sane() ||
        (FBANK >= 060000u && (*agc.superbank & 0100u)) || !a_bunker_word(agc.memory[03257u]))
        return EXEC_OUT_OF_SCOPE;
    return run_basic(bunker_return, state, io);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_tvc_stroke_readable(uint16_t key)
{
    return key == BUNKER_KEY ||
           (key / 1024u == TVC_STROKE_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL);
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#define BASIC_ERASABLE(i, address)
#include "cm_tvc_stroke_keys.inc"
#undef BASIC
#undef PAIR
#undef BASIC_ERASABLE

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
#define BASIC_ERASABLE(i, address)                                                                 \
    {(uint16_t)(AGC_TRANSFER_ERASABLE_BASE + (address)), basic_bunker},
static const agc_routine_entry basic_keys[] = {
#include "cm_tvc_stroke_keys.inc"
};
#undef PAIR
#undef BASIC
#undef BASIC_ERASABLE

const agc_routine_table cm_tvc_stroke_interpretive = {NULL, 0};
const agc_routine_table cm_tvc_stroke_basic = {basic_keys, COUNT(basic_keys)};

#endif
