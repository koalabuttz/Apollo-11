/* SPDX-License-Identifier: MIT */
/* The readable Pinball (src/pinball/) as routines of the flight program.
 *
 * The faithful port runs PINBALL_GAME_BUTTONS_AND_LIGHTS through routine
 * tables (c64-port/src/lanec_pin_routines.c and the keypress groups'
 * lanec_pin_keys_*_routines.c). Every routine there calls the trampoline,
 * lanec_pin_run_lm or lanec_pin_run_cm (c64-port/src/lanec_pin_trampoline.c),
 * with the list of the port's modules that claim its entry; the trampoline
 * runs them on the mission's device record (an lm_dntm_io or a cm_dntm_io:
 * the channels and the superbank). The readable file takes the
 * trampoline's place under its name, a part at a time: at an entry of a
 * readable part the readable routine runs, and at any other the faithful
 * trampoline does, built into the readable programs as
 * faithful_lanec_pin_run_<m> (tests/x86_readable.py, the Makefile's
 * pinball tests) until the file is readable whole.
 *
 * Plumbing only: point the machine at the state and at copies of the
 * channels the record carries, take the states the trampoline takes
 * (lanec_registers_valid), run the readable routine at Z, and give the
 * channels back. A routine the table enters at SWRETURN, the way back from
 * one of the file's BANKCALLs (c64-port/src/lanec_pin_routines.c,
 * swreturn), runs the readable SWRETURN (src/interbank/) first when it
 * returns into a readable entry. A step the readable code does not model
 * leaves the state and the record as they were and the routine refused,
 * as the trampoline leaves them.
 *
 * One routine is one step. The flight program charges the AGC clock for
 * each step by the point it starts at (c64-port/platform/x86/
 * flight_dispatch.c), so a readable step must start and end where the
 * faithful one does (tests/pinball_steps.c checks it): it ends where the
 * routine leaves for another entry, and runs the instruction a return
 * cell holds as the trampoline does. A step that calls through BANKCALL or
 * POSTJUMP into a part not yet readable (CLEAR's UPDATVB) runs in the
 * faithful port instead, whose module runs on into that routine in the
 * same step; so does ENTERJMP, which is written with ENTER. */
#include <setjmp.h>
#include <stddef.h>
#include "lanec_pin_routines.h"
#include "lm_downtelemetry.h"
#include "cm_downtelemetry.h"
#include "binding.h"
#include "../pinball/pinball.h"
#include "../interbank/interbank.h"
#include "../agc/names.h"

#if LUMINARY099
#define PIN_RUN          lanec_pin_run_lm
#define FAITHFUL_PIN_RUN faithful_lanec_pin_run_lm
#define THIS_MISSION     LANEC_LM
typedef lm_dntm_io device_record;
#else
#define PIN_RUN          lanec_pin_run_cm
#define FAITHFUL_PIN_RUN faithful_lanec_pin_run_cm
#define THIS_MISSION     LANEC_CM
typedef cm_dntm_io device_record;
#endif

enum exec_status FAITHFUL_PIN_RUN(const agc_native_program *program, apollo_overlay_state *state,
                                  void *device, const lanec_pin_dispatch *modules);

/* ---- The readable entries ---- */

struct entry {
    word at;
    void (*routine)(void);
};

/* Part 1, the keys: bank 40, and SLEFT5 and RELDSPON in fixed-fixed. */
static const struct entry entries[] = {
    {CHARIN, charin},
    {EIGHTY_NINE_TEST, eighty_nine_test},
    {NUM - 2u, num_minus2},
    {NUM, num},
    {VERB, verb},
    {NOUN, noun},
    {NEGSGN, negsgn},
    {POSGN, posgn},
    {PLUS_ON, plus_on},
    {MINUS_ON, minus_on},
    {CLEAR, clear},
    {CLPASHI + 11u, clpashi_after_updatvb},
    {CLR5, clr5},
    {FIVE_BLANK - 1u, five_blank_minus1},
    {FIVE_BLANK, five_blank},
    {FIVE_BLANK + 2u, five_blank_plus2},
    {TWO_BLANK, two_blank},
    {DSPIN, dspin},
    {ELEVEN_DSPIN, eleven_dspin},
    {SLEFT5, sleft5},
    {RELDSPON, reldspon},
};
#define ENTRIES (sizeof entries / sizeof entries[0])

#define BANK_40 040u

/* An entry's physical address: in bank 40 or in fixed-fixed. */
static unsigned physical(word at)
{
    return at >= 04000u ? FF(at) : BANK_40 * 02000u + (at - 02000u);
}

/* The entry at Z, or NULL. */
static const struct entry *entry_at_z(void)
{
    unsigned i, at;

    if (Z < 02000u || Z > 07777u) return NULL;
    at = fixed_physical(Z);
    for (i = 0; i < ENTRIES; ++i)
        if (physical(entries[i].at) == at) return &entries[i];
    return NULL;
}

/* ---- The machine on the device record ---- */

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static const agc_native_program *program;

/* The program's rope: its fixed_data, or the harness's reader. */
static word rope_word(unsigned at)
{
    agc_word value = 0;
    bool read = at <= 0xFFFFu &&
                (program->fixed_data
                     ? program->fixed_data(program, (uint16_t)at, &value)
                     : lanec_fixed_read && lanec_fixed_read(THIS_MISSION, (uint16_t)at, &value));

    if (!read) not_modelled();
    return (word)(value & MINUS_ZERO);
}

/* The readable parts written so far use only channel 11 (agc.dsalmout). */
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

/* The record's channel 11, superbank and interrupt switch, as the
 * routines change them. */
static uint16_t dsalmout, superbank, interrupts;

static void bind_record(apollo_overlay_state *state)
{
    agc.memory = state->words;
    agc.superbank = &superbank;
    agc.interrupts = &interrupts;
    agc.dsalmout = &dsalmout;
    agc.in_interrupt = NULL;
    agc.fixed_memory = rope_word;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.leave = leave;
}

#if LUMINARY099
static bool take_record(const agc_native_program *p, device_record *x)
{
    if (!p->hardware) return false;
    superbank = x->control.superbank;
    interrupts = x->control.interrupt_enabled;
    dsalmout = p->hardware->channel11;
    return true;
}

static void give_record(const agc_native_program *p, device_record *x)
{
    x->control.superbank = superbank;
    x->control.interrupt_enabled = interrupts;
    p->hardware->channel11 = x->channel11 = dsalmout;
}
#else
static bool take_record(const agc_native_program *p, device_record *x)
{
    (void)p;
    superbank = x->io.superbank;
    interrupts = x->io.interrupt_enabled;
    dsalmout = x->io.channel11;
    return true;
}

static void give_record(const agc_native_program *p, device_record *x)
{
    (void)p;
    x->io.superbank = superbank;
    x->io.interrupt_enabled = interrupts;
    x->io.channel11 = dsalmout;
}
#endif

/* ---- The trampoline ---- */

/* The way back from a BANKCALL into a readable entry: Z at SWRETURN, its
 * buffer BUF2 holding the return address and BUF2 +1 the caller's bank. */
static bool swreturn_into_part(void)
{
    word bank_word = fetch(ADDRESS_OF(BUF2) + 1u);
    word at = fetch(ADDRESS_OF(BUF2));
    unsigned bank = bank_word >> 10;

    if (Z != SWRETURN || (bank_word & ~076000u) || at < 02000u || at > 07777u) return false;
    if (at < 04000u && bank >= 030u && (superbank & 0100u)) bank += 010u;
    at = at >= 04000u ? FF(at) : bank * 02000u + (at - 02000u);
    for (unsigned i = 0; i < ENTRIES; ++i)
        if (physical(entries[i].at) == at) return true;
    return false;
}

/* The step's last word, as the trampoline runs it (cell_return): a routine
 * that ended at a register or at one of the erasable words a return
 * address is kept in (lanec_pin_cells: Q's TC Q, DSEXIT's TC DSEXIT, ...)
 * runs the instruction that word holds when it is a TC (not to itself, and
 * not RELINT, INHINT or EXTEND) or a TCF into fixed memory. */
static void run_return_cell(apollo_overlay_state *state)
{
    uint16_t *w = state->words, z = w[LANEC_Z], t;
    unsigned i;

    if (z >= 010u) {
        for (i = 0; i < LANEC_PIN_CELLS && lanec_pin_cells[i] != z; ++i) continue;
        if (i == LANEC_PIN_CELLS) return;
    }
    t = w[z];
    if (t <= 07777u) { /* TC: Q the word after it */
        if (t == 3u || t == 4u || t == 6u || t == z) return;
        w[LANEC_Q] = (uint16_t)(z + 1u);
    } else if ((uint16_t)(t - 012000u) > 05777u)
        return; /* not a TCF */
    w[LANEC_Z] = (uint16_t)(t & 07777u);
}

enum exec_status PIN_RUN(const agc_native_program *p, apollo_overlay_state *state, void *device,
                         const lanec_pin_dispatch *modules)
{
    static apollo_overlay_state before;
    static device_record record_before;
    device_record *record = device;
    const struct entry *entry;

    if (!p || !state || !record || !lanec_registers_valid(state) || !take_record(p, record))
        return FAITHFUL_PIN_RUN(p, state, device, modules);
    program = p;
    bind_record(state);
    entry = entry_at_z();
    if (!entry && !swreturn_into_part()) return FAITHFUL_PIN_RUN(p, state, device, modules);

    before = *state;
    record_before = *record;
    if (setjmp(back)) {
        *state = before;
        *record = record_before;
        return EXEC_OUT_OF_SCOPE;
    }
    if (!entry) {
        swreturn();
        entry = entry_at_z();
        if (!entry) not_modelled();
    }
    entry->routine();
    if (Z == BANKCALL || Z == POSTJUMP) {
        /* A call into a part not yet readable: the faithful step runs on
         * into it. */
        *state = before;
        return FAITHFUL_PIN_RUN(p, state, device, modules);
    }
    run_return_cell(state);
    give_record(p, record);
    return EXEC_OK;
}
