/* SPDX-License-Identifier: MIT */
/* The readable TVCDAPS (src/tvc/tvcdaps.c) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_tvc_daps.c): cm_tvc_daps_basic (the basic code; the
 * file has no interpretive code, and cm_tvc_daps_interpretive no keys).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_tvc_daps_keys.inc, which tools/p20_p25_keys.py writes from it), and
 * every key runs a readable routine. The side-by-side test
 * (tests/cm_tvc_daps_cases.c) checks that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (RESUME, NOQRSM, TC HACK). A copycycle re-entered at
 * PCOPY +1 or YCOPY +1 ends in TC Q, which goes on at the word Q names;
 * the faithful port runs on where that is one of the file's keys or a
 * word one of its subroutines returns to, and so does the binding; at
 * any other word it stops.
 *
 * The device is the CM's io record (cm_native_io); the faithful routines
 * of the bank take the program's hardware record, and without one refuse,
 * so the binding does too. The DAPs release their outcounters in channel
 * 14 (WOR), the io record's. The states the faithful routines take: FB
 * bank 20, ZERO +0, the bank registers agreeing, the interrupt and
 * superbank latches in range and every erasable word an ordinary one. A
 * run the readable code refuses part way leaves the state and io record
 * as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_tvc_daps.h"
#include "../tvc/tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define TVC_DAPS_BANK 020u

static jmp_buf back;

/* The run's io record. */
static cm_native_io *io_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channel 14, which the DAPs set bits of (WOR), as the faithful port
 * keeps it. */
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
    {PITCHDAP, pitchdap}, {PITCHDAP + 011u, pitchdap_after_hack},
    {PCDUDOTS, pcdudots}, {PCOPY + 1u, pcopy_reentry},
    {YAWDAP, yawdap},     {YAWDAP + 011u, yawdap_after_hack},
    {YINTEGRL, yintegrl}, {YCOPY + 1u, ycopy_reentry},
    {DAPINIT, dapinit},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* The words the file's subroutines return to (TC Q), each the word after
 * a TCR: the faithful port runs on at them after a copycycle's TC Q, and
 * so does the binding. */
static const struct basic_entry return_entries[] = {
    {PCDUDOTS + 5u, pcdudots_ydot},
    {PCDUDOTS + 013u, pcdudots_zdot},
    {PFORWARD, pforward},
    {POFFSET, poffset},
    {POUT, pout},
    {DELBARP, delbarp},
    {PDAPEND, dapend},
    {YFORWARD, yforward},
    {YOFFSET, yoffset},
    {YOUT, yout},
    {DELBARY, delbary},
    {YDAPEND, dapend},
};

/* After a copycycle's TC Q, at the word Q named: on at a key of the file,
 * or a word one of its subroutines returns to, as the faithful port does;
 * at any other word (in another bank, or fixed-fixed memory) the run
 * stops there. */
static void run_on(void)
{
    const struct basic_entry *entry;
    unsigned i;

    for (;;) {
        if (Z < 02000u || Z > 03777u) return;
        entry = basic_entry(Z);
        for (i = 0; !entry && i < COUNT(return_entries); ++i)
            if (return_entries[i].at == Z) entry = &return_entries[i];
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
    if (routine == pcopy_reentry || routine == ycopy_reentry) run_on();
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
    if (FBANK != (word)(TVC_DAPS_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_tvc_daps_readable(uint16_t key)
{
    return key / 1024u == TVC_DAPS_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#include "cm_tvc_daps_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_tvc_daps_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_tvc_daps_interpretive = {NULL, 0};
const agc_routine_table cm_tvc_daps_basic = {basic_keys, COUNT(basic_keys)};

#endif
