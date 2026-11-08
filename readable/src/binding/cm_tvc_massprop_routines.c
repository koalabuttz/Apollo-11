/* SPDX-License-Identifier: MIT */
/* The readable TVCMASSPROP (src/tvc/tvcmassprop.c) as routines of the
 * CM's flight program, under the table names and keys of the faithful
 * port's (c64-port/src/cm_tvc_massprop.c): cm_tvc_massprop_basic (the
 * basic code; the file has no interpretive code, and
 * cm_tvc_massprop_interpretive no keys).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_tvc_massprop_keys.inc, which tools/p20_p25_keys.py writes from it),
 * and every key runs a readable routine. The side-by-side test
 * (tests/cm_tvc_massprop_cases.c) checks that no key is left without one.
 *
 * Plumbing only. Both routines end in ENDMASSP's TC Q. Entered at their
 * keys, Q can hold any return; the faithful port runs on where it is one
 * of the file's keys (MASSPROP or FIXCW), and so does the binding; at any
 * other word it stops.
 *
 * The device is the CM's io record (cm_native_io); the faithful routines
 * of the bank take the program's hardware record, and without one refuse,
 * so the binding does too. The file reads and writes no channel. The
 * states the faithful routines take: FB bank 5, ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_tvc_massprop.h"
#include "../tvc/tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define TVC_MASSPROP_BANK 05u

static jmp_buf back;

/* The run's io record. */
static cm_native_io *io_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* The file reads no channel. */
static word channel_in(unsigned number)
{
    (void)number;
    not_modelled();
    return 0;
}

/* The file writes no channel. */
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
    {MASSPROP, massprop},
    {FIXCW, fixcw},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* After ENDMASSP's TC Q, to a word of the bank: on at MASSPROP or FIXCW,
 * as the faithful port does, and on again after its TC Q; at any other
 * word (or in erasable or fixed-fixed memory) the run stops there. */
static void run_on(void)
{
    const struct basic_entry *entry;

    for (;;) {
        if (Z < 02000u || Z > 03777u) return;
        entry = basic_entry(Z);
        if (!entry) return;
        entry->routine();
    }
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
    run_on();
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
    if (FBANK != (word)(TVC_MASSPROP_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_tvc_massprop_readable(uint16_t key)
{
    return key / 1024u == TVC_MASSPROP_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#include "cm_tvc_massprop_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_tvc_massprop_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_tvc_massprop_interpretive = {NULL, 0};
const agc_routine_table cm_tvc_massprop_basic = {basic_keys, COUNT(basic_keys)};

#endif
