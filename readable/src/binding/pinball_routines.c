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
 * runs in the faithful trampoline instead, from the state and the record
 * as they were (but for CLR5's, below).
 *
 * A routine is one step. The flight program charges the AGC clock for
 * each step by the point it starts at (c64-port/platform/x86/
 * flight_dispatch.c), so a readable step must start and end where the
 * faithful one does (tests/pinball_steps.c checks it). The binding runs
 * readable routines, and INTER-BANK_COMMUNICATION's between them, one
 * after another (running the instruction a return cell holds, as the
 * trampoline does) until it reaches a point where the faithful step from
 * the same start ends (pinball_stops.inc), or leaves Pinball (returns to
 * its caller). A step that reaches Pinball code not yet readable runs in
 * the faithful port instead. */
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
    unsigned at; /* physical: fixed_physical's numbering */
    void (*routine)(void);
};

#define BANK_40(at)     (040u * 02000u + ((at) - 02000u))
#define BANK_41(at)     (041u * 02000u + ((at) - 02000u))
#define FIXED_FIXED(at) ((unsigned)(at)) /* banks 2 and 3, at 04000-07777 */

/* Part 1, the keys: bank 40, and SLEFT5 and RELDSPON in fixed-fixed. */
static const struct entry entries[] = {
    {BANK_40(CHARIN), charin},
    {BANK_40(EIGHTY_NINE_TEST), eighty_nine_test},
    {BANK_40(NUM - 2u), num_minus2},
    {BANK_40(NUM), num},
    {BANK_40(VERB), verb},
    {BANK_40(NOUN), noun},
    {BANK_40(NEGSGN), negsgn},
    {BANK_40(POSGN), posgn},
    {BANK_40(PLUS_ON), plus_on},
    {BANK_40(MINUS_ON), minus_on},
    {BANK_40(CLEAR), clear},
    {BANK_40(CLPASHI + 11u), clpashi_after_updatvb},
    {BANK_40(CLR5), clr5},
    {BANK_40(FIVE_BLANK - 1u), five_blank_minus1},
    {BANK_40(FIVE_BLANK), five_blank},
    {BANK_40(FIVE_BLANK + 2u), five_blank_plus2},
    {BANK_40(TWO_BLANK), two_blank},
    {BANK_40(DSPIN), dspin},
    {BANK_40(ELEVEN_DSPIN), eleven_dspin},
    {FIXED_FIXED(SLEFT5), sleft5},
    {FIXED_FIXED(RELDSPON), reldspon},
    /* Part 2a, the word display routines: bank 40, and bank 41's octal. */
    {BANK_40(DSPSIGN), dspsign},
    {BANK_40(DSPSIGN + 8u), dspsign_after_minus_on},
    {BANK_40(DSPSIGN + 12u), dspsign_after_plus_on},
    {BANK_40(DSPRND), dsprnd},
    {BANK_40(DSPDECWD), dspdecwd},
    {BANK_40(DSPDECWD + 3u), dspdecwd_after_dspsign},
    {BANK_40(DSPDCWD1 - 1u), dspdcwd1_minus1},
    {BANK_40(DSPDCWD1), dspdcwd1},
    {BANK_40(DSPDCWD1 + 16u), dspdcwd1_after_dspin},
    {BANK_40(DSPDECNR), dspdecnr},
    {BANK_40(DSPDECNR + 3u), dspdecnr_after_dspsign},
    {BANK_40(DSPDC2NR), dspdc2nr},
    {BANK_40(DSPDC2NR + 3u), dspdc2nr_plus3},
    {BANK_40(DSP2DEC), dsp2dec},
    {BANK_40(DSP2DEC + 6u), dsp2dec_after_minus_r2},
    {BANK_40(DSP2DEC + 8u), dsp2dec_after_plus_r2},
    {BANK_40(DSP2DEC + 9u), dsp2dec_after_dspsign},
    {BANK_40(END2DEC), end2dec},
    {BANK_40(DSPDECVN), dspdecvn},
    {BANK_40(GOVNUPDT), govnupdt},
    {BANK_40(GOVNUPDT + 1u), govnupdt_after_dspdecvn},
    {BANK_40(DSPOCTIN), dspoctin},
    {BANK_40(DSPOCTIN + 1u), dspoctin_after_dspin},
    {BANK_41(DSPOCTWD), dspoctwd},
    {BANK_41(WDAGAIN), wdagain},
    {BANK_41(WDAGAIN + 5u), wdagain_plus5},
    {BANK_41(OCTBACK), octback},
    {BANK_41(DSP2BIT), dsp2bit},
    /* Part 2b: ENTER and the verb fan */
    {BANK_40(ENTERJMP), enterjmp},
    {BANK_41(NVSUBB), nvsubb},
    {BANK_41(LOADLV1), loadlv1},
    {BANK_41(ENTER), enter},
    {BANK_41(ENTPAS0), entpas0},
    {BANK_41(REQADD), reqadd},
    {BANK_41(REQADD + 8u), reqadd_after_reqdatz},
    {BANK_41(USEADD), useadd},
    {BANK_41(INTMCTBS), intmctbs},
    {BANK_41(INTMCTBS + 2u), intmctbs_plus2_entry},
    {BANK_41(INTMCTBS + 10u), intmctbs_after_dspoctwd},
    {BANK_41(VERBFAN - 2u), verbfan_minus2},
    {BANK_41(VERBFAN), verbfan},
    {BANK_41(VERBFAN + 8u), verbfan_after_reldsp},
    {BANK_41(VBFANDIR), vbfandir},
    {BANK_41(MIXNOUN), mixnoun},
    {BANK_41(REQDATX), reqdatx},
    {BANK_41(REQDATY), reqdaty},
    {BANK_41(REQDATZ), reqdatz},
    {BANK_41(ENDRQDAT - 1u), reqcom_after_5blank},
    {BANK_41(UPDATNN - 1u), updatnn_minus1},
    {BANK_41(UPDATNN), updatnn},
    {BANK_41(UPDATVB - 1u), updatvb_minus1},
    {BANK_41(UPDATVB), updatvb},
    {BANK_41(UPDAT1 + 2u), updat1_plus2},
    {BANK_41(GOALMCYC), goalmcyc},
    {BANK_41(GODSPALM), godspalm},
    {FIXED_FIXED(SETNCADR), setncadr},
    {FIXED_FIXED(SETNCADR + 1u), setncadr_plus1},
    {FIXED_FIXED(SETNADD), setnadd},
    {FIXED_FIXED(SETEBANK), setebank},
    {FIXED_FIXED(RIGHT5), right5},
    {FIXED_FIXED(LEFT5), left5},
    {FIXED_FIXED(FALTON), falton},
    {FIXED_FIXED(FLASHON), flashon},
    {FIXED_FIXED(FLASHOFF), flashoff},
    {FIXED_FIXED(RELDSP), reldsp},
    {FIXED_FIXED(RELDSP1), reldsp1},
    {FIXED_FIXED(ALMCYCLE), almcycle},
    {FIXED_FIXED(ALMCYCLE + 5u), almcycle_after_updatvb},
    {FIXED_FIXED(SFRUTMIX), sfrutmix},
};
#define ENTRIES (sizeof entries / sizeof entries[0])

/* INTER-BANK_COMMUNICATION's routines (src/interbank/), which a step runs
 * through on its way between banks. */
static const struct entry interbank[] = {
    {FIXED_FIXED(BANKCALL), bankcall},
    {FIXED_FIXED(SWRETURN), swreturn},
    {FIXED_FIXED(POSTJUMP), postjump},
    {FIXED_FIXED(BANKJUMP), bankjump},
};
#define INTERBANK (sizeof interbank / sizeof interbank[0])

/* An erasable address, in the numbering of the stops below. */
#define ERASABLE_POINT(z) (0176000u + (z))

/* Where Z is: its physical word, or for an erasable address
 * ERASABLE_POINT. */
static unsigned here(void)
{
    return Z < 02000u ? ERASABLE_POINT(Z) : fixed_physical(Z);
}

/* The routine at a point, in a table, or NULL. */
static const struct entry *find(const struct entry *table, unsigned count, unsigned at)
{
    unsigned i;

    for (i = 0; i < count; ++i)
        if (table[i].at == at) return &table[i];
    return NULL;
}

/* ---- Where a step ends ----
 * The flight program charges the AGC clock for each step by the point it
 * starts at (c64-port/platform/x86/flight_dispatch.c). The faithful port's
 * modules run a step past routine boundaries and stop at points of their
 * own: CHARIN stops at the key's routine, 89TEST runs on into NUM, CLEAR
 * runs through BANKCALL into UPDATVB. pinball_stops.inc lists, for each
 * point a faithful step starts at in the port's Pinball chains, the points
 * it ends at; a readable step ends at the same ones. pinball_points lists
 * the points of the faithful routine tables, Pinball's code as far as the
 * port enters it. */
struct pinball_stop {
    uint16_t from, to;
};
#include "pinball_stops.inc"

static bool stops_at(unsigned from, unsigned to)
{
    unsigned low = 0, high = sizeof pinball_stops / sizeof pinball_stops[0];

    while (low < high) {
        unsigned middle = (low + high) / 2u;
        const struct pinball_stop *s = &pinball_stops[middle];
        if (s->from == from && s->to == to) return true;
        if (s->from < from || (s->from == from && s->to < to))
            low = middle + 1u;
        else
            high = middle;
    }
    return false;
}

/* Whether a point is Pinball code (the faithful port's routine tables
 * enter it there). */
static bool pinball_point(unsigned at)
{
    unsigned low = 0, high = sizeof pinball_points / sizeof pinball_points[0];

    while (low < high) {
        unsigned middle = (low + high) / 2u;
        if (pinball_points[middle] == at) return true;
        if (pinball_points[middle] < at)
            low = middle + 1u;
        else
            high = middle;
    }
    return false;
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
    at = at >= 04000u ? FIXED_FIXED(at) : bank * 02000u + (at - 02000u);
    return find(entries, ENTRIES, at) != NULL;
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
    const struct entry *routine;
    unsigned start, routines;
    bool jumped;

    if (!p || !state || !record || !lanec_registers_valid(state) || !take_record(p, record))
        return FAITHFUL_PIN_RUN(p, state, device, modules);
    program = p;
    bind_record(state);
    start = here();
    routine = find(entries, ENTRIES, start);
    if (!routine) {
        if (!swreturn_into_part()) return FAITHFUL_PIN_RUN(p, state, device, modules);
        routine = find(interbank, INTERBANK, start);
    }

    before = *state;
    record_before = *record;
    if (setjmp(back)) {
        /* A step the readable routines do not model runs in the faithful
         * port, from the state it started in; but not one from CLR5, where
         * the faithful port's result differs from the original program's
         * (issue #1093): that step is refused. */
        *state = before;
        *record = record_before;
        if (start == BANK_40(CLR5)) return EXEC_OUT_OF_SCOPE;
        return FAITHFUL_PIN_RUN(p, state, device, modules);
    }
    /* The routines of the step, to the point the faithful step ends at. */
    for (routines = 0;; ++routines) {
        unsigned at;

        if (routines == 64u) not_modelled();
        /* A jump between banks (BANKCALL, POSTJUMP, BANKJUMP), not a
         * return (SWRETURN). */
        jumped = routine->routine == bankcall || routine->routine == postjump ||
                 routine->routine == bankjump;
        routine->routine();
        run_return_cell(state);
        at = here();
        if (stops_at(start, at)) break;
        routine = find(entries, ENTRIES, at);
        if (!routine) routine = find(interbank, INTERBANK, at);
        if (!routine && !pinball_point(at) && !jumped) break; /* back out of Pinball */
        if (!routine) {
            /* The faithful step runs on into Pinball code not yet
             * readable, or by a jump between banks into another file's
             * code (VERBFAN's POSTJUMP to GOEXTVB): it runs this step. */
            *state = before;
            return FAITHFUL_PIN_RUN(p, state, device, modules);
        }
    }
    give_record(p, record);
    return EXEC_OK;
}
