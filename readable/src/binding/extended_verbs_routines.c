/* SPDX-License-Identifier: MIT */
/* The readable extended verbs (src/extended_verbs/) as routines of the
 * flight program. The faithful port keeps each part of EXTENDED_VERBS in
 * a module of its own (c64-port/src/lanec_ev_<part>.c), which its routine
 * tables (lanec_ev_routines.c) call for every key of the part, with the
 * state and a record of the channels (lanec_io, c64-port/include/lanec.h)
 * and the mission. The readable parts take the module's place under its
 * name: part 1, the front end, is lanec_ev_front_dispatch.
 *
 * Plumbing only: point the machine at the state and the channel record,
 * and take the states the faithful module takes: the mission this object
 * is built for, bank 43 selected (FBANK's bank 33 with the superbank bit),
 * the bank registers agreeing, Z a 12-bit address and ZERO +0. A step the
 * readable code does not model leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "lanec_ev_front.h"
#include "../extended_verbs/extended_verbs.h"
#include "../agc/names.h"

#if LUMINARY099
#define THIS_MISSION LANEC_LM
#else
#define THIS_MISSION LANEC_CM
#endif

/* Bank 43: FBANK's bank 33 with the superbank bit. */
#define BANK_33_FBANK 066000u
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The front end reads no channel and no fixed word. */
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

static word fixed_memory(unsigned physical)
{
    (void)physical;
    not_modelled();
    return 0;
}

static void bind_lanec(apollo_overlay_state *state, lanec_io *io)
{
    agc.memory = state->words;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
    agc.dsalmout = &io->channel[011];
    agc.in_interrupt = NULL;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
}

/* The faithful module's states (lanec_registers_valid, bank 43). */
static bool as_expected(void)
{
    return (EBANK & ~03400u) == 0u && (FBANK & ~076000u) == 0u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && Z <= 07777u && agc.memory[ZERO_ADDRESS] == 0u &&
           FBANK == BANK_33_FBANK && (*agc.superbank & SUPERBANK_BIT);
}

struct entry {
    word at;
    void (*routine)(void);
};

/* The entry at Z, or NULL. */
static const struct entry *entry_at_z(const struct entry *entries, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i)
        if (entries[i].at == Z) return &entries[i];
    return NULL;
}

/* Run the routine at Z, if it is one of the entries. */
static enum lanec_status run(const struct entry *entries, size_t count, apollo_overlay_state *state,
                             lanec_io *io)
{
    apollo_overlay_state before;
    lanec_io io_before;
    const struct entry *entry;

    bind_lanec(state, io);
    if (!as_expected()) return LANEC_OUT_OF_SCOPE;
    entry = entry_at_z(entries, count);
    if (!entry) return LANEC_OUT_OF_SCOPE;
    before = *state;
    io_before = *io;
    if (setjmp(back)) {
        *state = before;
        *io = io_before;
        return LANEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return LANEC_OK;
}

/* ---- Part 1: the front end ---- */

static const struct entry front[] = {
    {GOEXTVB, goextvb},
    {TESTXACT, testxact},
    {SETXTACT, setxtact},
    {SETXTACT + 5u, setxtact_after_nvsub_busy},
    {SETXTACT + 6u, setxtact_after_nvsub},
    {XACTALM, xactalm},
    {XACTALM + 1u, xactalm_after_falton},
    {XACT0, xact0},
    {ALM_END, alm_end},
    {GOPIN, gopin},
#if LUMINARY099
    {CHKPOOH, chkpooh},
#endif
};

enum lanec_status lanec_ev_front_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(front, sizeof front / sizeof front[0], state, io);
}
