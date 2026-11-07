/* SPDX-License-Identifier: MIT */
/* The readable TVCINITIALIZE (src/tvc/) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_tvc_init.c): cm_tvc_init_basic (the basic code; the
 * file has no interpretive code, and cm_tvc_init_interpretive no keys).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_tvc_init_keys.inc, which tools/p20_p25_keys.py writes from it), and
 * every key runs a readable routine. The side-by-side test
 * (tests/cm_tvc_init_cases.c) checks that no key is left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (RESUME, IBNKCALL, WAITLIST, POSTJUMP, TC RTRNLOC) or
 * reaches a word where the faithful port's run stops, a key of its own.
 * LOADCOEF and S40.15 end in TC Q. Entered at their keys, Q can hold any
 * return; the faithful port runs on where it is one of the file's keys
 * those two routines hold, and so does the binding; at any other word
 * both stop. RTRNLOC, E6,1613, is a key too: the program runs the word
 * SWICHOVR keeps there as TC <return>.
 *
 * The device is the CM's io record (cm_native_io); the faithful routines
 * of the bank take the program's hardware record, and without one refuse,
 * so the binding does too; the word at RTRNLOC needs none. TVCINIT4 reads
 * channel 11 (RAND DSALMOUT), the io record's. The states the faithful
 * routines take: FB bank 17 (at RTRNLOC any bank, in banks 30 to 37 with
 * the superbank bit clear), ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range and
 * every erasable word an ordinary one. A run the readable code refuses
 * part way leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_tvc_init.h"
#include "../tvc/tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define TVC_BANK 017u

static jmp_buf back;

/* The run's io record. */
static cm_native_io *io_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* TVCINIT4's channel 11 (RAND DSALMOUT), as the faithful port keeps it. */
static word channel_in(unsigned number)
{
    if (number == 011u) return (word)io_now->channel11;
    not_modelled(); /* the file reads no other channel */
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
    {TVCDAPON, tvcdapon},
    {MRCLEAN, mrclean},
    {TVCINIT1, tvcinit1},
    {TVCINIT1 + 5u, tvcinit1_after_massprop},
    {TVCINIT1 + 025u, tvcinit1_after_loadcoef},
    {TVCINIT2, tvcinit2},
    {TVCINIT2 + 021u, tvcinit2_repfrac},
    {TVCINIT4, tvcinit4},
    {TVCINIT4 + 017u, tvcinit4_after_waitlist},
    {TVCINIT4 + 021u, tvcinit4_t5},
    {PRESWTCH, preswtch},
    {PRESWTCH + 1u, preswtch_after_swichovr},
    {SWICHOVR + 5u, swichovr_restart},
    {SWICHOVR + 026u, swichovr_after_s40_15},
    {SWICHOVR + 043u, swichovr_after_loadcoef},
    {LOADCOEF, loadcoef},
    {LOADCOEF + 014u, loadcoef_dca6},
    {LOADCOEF + 017u, loadcoef_n10_6},
    {LOADCOEF + 023u, loadcoef_n10_8},
    {S40_15, s40_15},
    {S40_15 + 7u, s40_15_switchover},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* The routines that end in a TC Q, after which the faithful port runs on. */
static bool ends_in_tc_q(void (*routine)(void))
{
    return routine == loadcoef_dca6 || routine == loadcoef_n10_6 || routine == loadcoef_n10_8 ||
           routine == s40_15 || routine == s40_15_switchover;
}

/* After such a routine, to a word of the bank: on at one of these
 * routines' own keys, as the faithful port does, and on again after its
 * TC Q; at any other word (or in erasable or fixed-fixed memory) the run
 * stops there. */
static void run_on(void)
{
    const struct basic_entry *entry;

    for (;;) {
        if (Z < 02000u || Z > 03777u) return;
        entry = basic_entry(Z);
        if (!entry || !ends_in_tc_q(entry->routine)) return;
        entry->routine();
    }
}

/* RTRNLOC's key (E6,1613), and the words SWICHOVR keeps there: TC
 * PRESWTCH +1, and TVCRESTARTS's return. */
#define RTRNLOC_KEY (uint16_t)(AGC_TRANSFER_ERASABLE_BASE + 03213u)

static bool a_swichovr_return(word instruction)
{
    return instruction == PRESWTCH + 1u || instruction == 04753u;
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
    if (ends_in_tc_q(routine)) run_on();
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
    if (FBANK != (word)(TVC_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;
    return run_basic(entry->routine, state, io);
}

/* RTRNLOC, run as TC: Z at E6,1613, one of SWICHOVR's returns there. */
static enum exec_status basic_rtrnloc(const agc_native_program *program,
                                      apollo_overlay_state *state, void *device)
{
    cm_native_io *io = device;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if ((Z & 07777u) != ADDRESS_OF(RTRNLOC) || (EBANK & 03400u) != 03000u || !basic_sane() ||
        (FBANK >= 060000u && (*agc.superbank & 0100u)) || !a_swichovr_return(agc.memory[03213u]))
        return EXEC_OUT_OF_SCOPE;
    return run_basic(rtrnloc_return, state, io);
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_tvc_init_readable(uint16_t key)
{
    return key == RTRNLOC_KEY ||
           (key / 1024u == TVC_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL);
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#define BASIC_ERASABLE(i, address)
#include "cm_tvc_init_keys.inc"
#undef BASIC
#undef PAIR
#undef BASIC_ERASABLE

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
#define BASIC_ERASABLE(i, address)                                                                 \
    {(uint16_t)(AGC_TRANSFER_ERASABLE_BASE + (address)), basic_rtrnloc},
static const agc_routine_entry basic_keys[] = {
#include "cm_tvc_init_keys.inc"
};
#undef PAIR
#undef BASIC
#undef BASIC_ERASABLE

const agc_routine_table cm_tvc_init_interpretive = {NULL, 0};
const agc_routine_table cm_tvc_init_basic = {basic_keys, COUNT(basic_keys)};

#endif
