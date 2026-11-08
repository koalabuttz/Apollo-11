/* SPDX-License-Identifier: MIT */
/* The readable AUTOMATIC_MANEUVERS (src/cm_dap) as routines of the CM's
 * flight program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_auto_man.c): cm_auto_man_basic (the basic code; the
 * file has no interpretive code, and cm_auto_man_interpretive no keys) and
 * cm_auto_man_hot_routines, the keys one pass reaches, which the flight
 * store packs together (the same routines).
 *
 * The table carries every key of the faithful one, in its order
 * (cm_auto_man_keys.inc, which tools/p20_p25_keys.py writes from it), and
 * each runs its readable routine. The side-by-side test
 * (tests/cm_auto_man_cases.c) checks them against the faithful port's.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (REINIT, STICKCHK, T6PROGM, SMALLMP, RESUME) or reaches a
 * key of another
 * part of the faithful port's code, where its run stops. The file has no
 * TC Q, so a run never goes on at a key of its own part.
 *
 * The file reads channels 31 and 32, in the program's hardware record, and
 * writes none. The faithful routines take that record, and without one
 * refuse, so the binding does too. The states they take: FB bank 21, any
 * EBANK, ZERO +0, the bank registers agreeing, the interrupt and superbank
 * latches in range and every erasable word an ordinary one. A run the
 * readable code refuses part way leaves the state as it was. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "agc_transfer.h"
#include "cm_auto_man.h"
#include "../cm_dap/cm_dap.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* The file's bank. */
#define AUTO_MAN_BANK 021u

static jmp_buf back;

/* The run's hardware record. */
static agc_hardware *hardware_now;

static void leave(void)
{
    longjmp(back, 1);
}

/* The channels the file reads, where the faithful port keeps them: 31
 * (the autopilot's switches and the RHC) and 32 (the RHC's minimum
 * impulse bits), in the hardware record. */
static word channel_in(unsigned number)
{
    switch (number) {
    case 031u: return (word)hardware_now->channel31;
    case 032u: return (word)hardware_now->channel32;
    default: not_modelled(); return 0u;
    }
}

static void channel_out(unsigned number, word value)
{
    (void)number;
    (void)value;
    not_modelled(); /* the file writes no channel */
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
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take: the bank registers, the latches,
 * and every erasable word an ordinary one. */
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

/* The words a function starts at: every key of the faithful table. */
static const struct basic_entry {
    word at;
    void (*routine)(void);
} basic_entries[] = {
    {AHFNOROT, ahfnorot},
    {FREECONT + 012u, freecont_after_stickchk},
    {ATTHOLD, atthold},
    {ATTHOLD + 041u, atthold_bias},
    {ATTHOLD + 042u, atthold_bias_errorx},
    {HOLDFUNC, holdfunc},
    {GRABANG, grabang},
    {JETS, jets},
    {JLOOP, jloop},
    {INHOLD, inhold},
    {SIGNCK1, signck1},
    {POSVEL, posvel},
    {NEGVEL, negvel},
    {NJ22 + 2u, nj22_slope},
    {SIGNCK3 - 1u, tcf_nj23},
    {SIGNCK3, signck3},
    {J18, j18},
    {J20, j20},
    {J21, j21},
    {JN, jn},
    {JP, jp},
    {J22, j22},
    {J22 + 1u, tcf_jn},
    {J22 + 2u, tcf_signck5},
    {J22 + 3u, tcf_jp},
    {SIGNCK5, signck5},
    {SIGNCK5 + 1u, tcf_jn},
    {SIGNCK5 + 2u, tcf_jn},
    {SIGNCK5 + 3u, tcf_jp},
    {SIGNCK5 + 4u, tcf_jp},
    {J23, j23},
    {J24, j24},
    {J24 + 5u, j24_kmpac},
    {JTIME, jtime},
    {JTIME + 3u, jtime_bit11},
    {JTIME + 5u, jtime_ccs},
    {DOJET + 2u, dojet_next},
    {T6PROG, t6prog},
    {T6PROG + 2u, t6prog_t5loc},
};

static const struct basic_entry *basic_entry(word at)
{
    unsigned i;

    for (i = 0; i < COUNT(basic_entries); ++i)
        if (basic_entries[i].at == at) return &basic_entries[i];
    return NULL;
}

/* ---- The keys, in the faithful table's order ---- */

/* Each key's routine, from a state the faithful one takes. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry((word)(02000u + key % 1024u));
    apollo_overlay_state before;

    if (!entry) return EXEC_OUT_OF_SCOPE;
    if (!program || !program->hardware || !state || !device) return EXEC_OUT_OF_SCOPE;
    bind(program, state, device);
    if (FBANK != (word)(AUTO_MAN_BANK << 10) || !basic_sane()) return EXEC_OUT_OF_SCOPE;

    before = *state;
    if (setjmp(back)) {
        *state = before;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_auto_man_readable(uint16_t key)
{
    return key / 1024u == AUTO_MAN_BANK && basic_entry((word)(02000u + key % 1024u)) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)
#include "cm_auto_man_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_auto_man_keys.inc"
};
#undef PAIR
#undef BASIC

/* The keys one pass reaches, as the faithful hot table lists them. */
static const agc_routine_entry hot_keys[] = {
    {KEY(AUTO_MAN_BANK, 03250u), basic_0},  {KEY(AUTO_MAN_BANK, 03441u), basic_7},
    {KEY(AUTO_MAN_BANK, 03445u), basic_8},  {KEY(AUTO_MAN_BANK, 03463u), basic_9},
    {KEY(AUTO_MAN_BANK, 03704u), basic_30}, {KEY(AUTO_MAN_BANK, 03757u), basic_37},
};

const agc_routine_table cm_auto_man_interpretive = {NULL, 0};
const agc_routine_table cm_auto_man_basic = {basic_keys, COUNT(basic_keys)};
const agc_routine_table cm_auto_man_hot_routines = {hot_keys, COUNT(hot_keys)};

#endif
