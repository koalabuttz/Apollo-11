/* SPDX-License-Identifier: MIT */
/* The readable P20-P25 (src/p20_p25/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_p20_p25.c): lm_p20_p25_basic (the basic code) and
 * lm_p20_p25_interpretive (the order pairs).
 *
 * The file is readable part by part, so the tables carry every key of the
 * faithful ones, in their order (p20_p25_keys.inc, which
 * tools/p20_p25_keys.py writes from them): a key a readable part has
 * written runs the readable routine; any other runs the faithful port's,
 * built into the program as faithful_lm_p20_p25_basic and
 * faithful_lm_p20_p25_interpretive (tests/x86_readable.py, the
 * Makefile's p20-p25 tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, PHASCHNG, UPFLAG, ENDOFJOB, ...), reaches a label the
 * faithful port keys a routine at, or reaches NEWOPS through a TC INTPRET.
 * An order pair is one pass of the readable interpreter over the listing's
 * words (interpret_listing()); a pass that ends at RTB LOADTIME runs
 * LOADTIME (src/rtb_op_codes/) and the DANZIG it ends at, as the faithful
 * pair does. A pass that ends at an EXIT or a CALL out of its table stops
 * there, as the faithful pair does.
 *
 * The device is the LM's io record with the input channels after it
 * (lm_channel_io): P20LEMB7 and R22 read channel 33. Channel 12 (R22
 * reads it, R21 clears its bit 14) is the program's hardware record's.
 *
 * The states the faithful routines take, at every key: FB the key's bank
 * (in banks 30-37, the superbank bit clear); at an order pair Z at NEWOPS,
 * BANKSET the BBANK and INTBIT15 the bank's bit 15. Always ZERO +0, the
 * bank registers agreeing, the interrupt and superbank latches in range,
 * and every erasable word an ordinary one. At an order pair and at the
 * basic keys the faithful port marks so (its at_basic), NEWJOB +0 or a core
 * set and FIXLOC one of the VAC areas; at the other basic keys (its
 * at_basic_nv) NEWJOB may be -0 too, and FIXLOC anything. A run the
 * readable code refuses part way leaves the state and io record as they
 * were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "lm_p20_p25.h"
#include "../p20_p25/p20_p25.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if LUMINARY099

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_lm_p20_p25_basic, faithful_lm_p20_p25_interpretive;

/* A key: the bank, and the address in it (or in fixed-fixed memory). */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* LOADTIME's bank (FB), from its FCADR. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define SUPERBANK_BIT 0100u

static const agc_input_channels *inputs_now;
static agc_hardware *hardware_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 30-33, as the run latched them. */
static word channel_in(unsigned number)
{
    switch (number) {
    case 030u: return inputs_now->channel30;
    case 031u: return inputs_now->channel31;
    case 032u: return inputs_now->channel32;
    case 033u: return inputs_now->channel33;
    case 012u:
        if (!hardware_now) not_modelled();
        return hardware_now->channel12;
    default: not_modelled(); return 0;
    }
}

/* Channel 12 only (R21's WAND). */
static void channel_out(unsigned number, word value)
{
    if (number != 012u || !hardware_now) not_modelled();
    hardware_now->channel12 = value;
}

/* The interpreter's fixed operands are the rope's. */
static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(0u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, lm_channel_io *io)
{
    bind_machine(program, state, &io->io);
    inputs_now = &io->in;
    hardware_now = program->hardware;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its sane(); without a work area,
 * its sane_nv()). */
static bool sane(bool with_work_area)
{
    word newjob = agc.memory[ADDRESS_OF(NEWJOB)];
    unsigned i;

    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (newjob > 037777u && (with_work_area || newjob != MINUS_ZERO)) return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (with_work_area && (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u))
        return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* The key's bank selected: FB, and in banks 30-37 the superbank bit clear
 * (with it, FB 30-33 are banks 40-43). */
static bool in_bank(unsigned bank)
{
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* Part 1: P20 and P25 */
    {KEY(024u, PROG20), prog20, false},
    {KEY(024u, PROG20 + 4u), prog20_after_2phschng, false},
    {KEY(024u, PROG20 + 5u), prog20_on_surface, false},
    {KEY(024u, PROG20 + 6u), prog20_in_orbit, false},
    {KEY(024u, ORBCHGO), orbchgo, false},
    {KEY(024u, ORBCHGO + 2u), orbchgo_after_upflag, false},
    {KEY(024u, ORBCHGO + 7u), orbchgo_terminate, false},
    {KEY(024u, ORBCHGO + 010u), orbchgo_proceed, false},
    {KEY(024u, ORBCHGO + 011u), orbchgo_enter, false},
    {KEY(024u, ORBCHG1), orbchg1, false},
    {KEY(024u, ORBCHG1 + 7u), orbchg1_terminate, false},
    {KEY(024u, ORBCHG1 + 010u), orbchg1_proceed, true},
    {KEY(024u, ORBCHG1 + 011u), orbchg1_enter, true},
    {KEY(024u, ORBCHG2), orbchg2, true},
    {KEY(024u, PROG20A), prog20a, false},
    {KEY(024u, PROG20A + 2u), prog20a_after_r02both, false},
    {KEY(024u, PROG20A + 4u), prog20a_after_updatflg, false},
    {KEY(024u, PROG20A + 6u), prog20a_after_trackflg, false},
    {KEY(024u, PROG20A + 010u), prog20a_after_rndvzflg, false},
    {KEY(024u, PROG20A + 012u), prog20a_after_srchoptn, false},
    {KEY(024u, PROG20A + 014u), prog20a_after_acmodflg, false},
    {KEY(024u, PROG20A + 016u), prog20a_after_r04flag, false},
    {KEY(024u, PROG20A + 020u), prog20a_after_norrmon, false},
    {KEY(024u, PROG20A + 022u), prog20a_after_loscmflg, true},
    {KEY(024u, P20LEM1), p20lem1, false},
    {KEY(024u, P20LEM1 + 2u), p20lem1_after_phaschng, true},
    {KEY(024u, P20LEM1 + 014u), p20lem1_after_exit, false},
    {KEY(024u, P20LEM1 + 016u), p20lem1_in_range, false},
    {KEY(024u, FIVE_TWENTY_SIX_ALARM), five_twenty_six_alarm, false},
    {KEY(024u, FIVE_TWENTY_SIX_ALARM + 3u), five_twenty_six_alarm_terminate, false},
    {KEY(024u, FIVE_TWENTY_SIX_ALARM + 4u), five_twenty_six_alarm_proceed, false},
    {KEY(024u, FIVE_TWENTY_SIX_ALARM + 5u), five_twenty_six_alarm_enter, false},
    {KEY(024u, FIVE_TWENTY_SIX_ALARM + 6u), five_twenty_six_alarm_immediate_return, false},
    {KEY(024u, P20LEMA + 2u), p20lema_after_phaschng, false},
    {KEY(024u, P20LEMA + 3u), p20lema_on_surface, false},
    {KEY(024u, P20LEMA + 4u), p20lema_in_orbit, false},
    {KEY(024u, P20LEMB), p20lemb, false},
    {KEY(024u, P20LEMB + 3u), p20lemb_after_phaschng, false},
    {KEY(024u, P20LEMB + 5u), p20lemb_after_priochng, true},
    {KEY(024u, P20LEMB5 + 012u), p20lemb5_terminate, false},
    {KEY(024u, P20LEMB5 + 013u), p20lemb5_proceed, false},
    {KEY(024u, P20LEMB5 + 014u), p20lemb5_enter, false},
    {KEY(024u, P20LEMB5 + 015u), p20lemb5_immediate_return, false},
    {KEY(024u, P20LEMB6 + 3u), p20lemb6_terminate, true},
    {KEY(024u, P20LEMB6 + 4u), p20lemb6_proceed, true},
    {KEY(024u, P20LEMB6 + 5u), p20lemb6_enter, true},
    {KEY(024u, P20LEMB6 + 6u), p20lemb6_on_surface, true},
    {KEY(024u, P20LEMB6 + 7u), p20lemb6_in_orbit, true},
    {KEY(024u, P20LEMB1), p20lemb1, false},
    {KEY(024u, P20LEMB1 + 2u), p20lemb1_after_upflag, true},
    {KEY(024u, P20LEMB2), p20lemb2, false},
    {KEY(024u, P20LEMB2 + 1u), p20lemb2_after_falton, true},
    {KEY(024u, P20LEMB3), p20lemb3, true},
    {KEY(024u, P20LEMB3 + 012u), p20lemb3_after_srchoptn, false},
    {KEY(024u, P20LEMB3 + 014u), p20lemb3_after_acmodflg, true},
    {KEY(024u, P20LEMB4), p20lemb4, false},
    {KEY(024u, P20LEMB4 + 3u), p20lemb4_after_delayjob, true},
    {KEY(024u, P20LEMC3), p20lemc3, true},
    {KEY(024u, P20LEMC), p20lemc, false},
    {KEY(024u, P20LEMC + 2u), p20lemc_after_phaschng, false},
    {KEY(024u, P20LEMWT), p20lemwt, false},
    {KEY(024u, P20LEMWT + 3u), p20lemwt_after_twiddle, false},
    {KEY(024u, P20LMWT1 + 2u), p20lmwt1_after_phaschng, false},
    {KEY(024u, P20LEMC1), p20lemc1, false},
    {KEY(024u, P20LEMC1 + 014u), p20lemc1_after_findvac, false},
    {KEY(024u, P20LEMC2 + 2u), p20lemc2_after_fixdelay, false},
    {KEY(024u, P20LEMD), p20lemd, false},
    {KEY(024u, P20LEMD + 3u), p20lemd_after_twiddle, false},
    {KEY(024u, P20LEMD1), p20lemd1, false},
    {KEY(024u, P20LEMD1 + 6u), p20lemd1_after_fixdelay, false},
    {KEY(024u, P20LEMD2), p20lemd2, false},
    {KEY(024u, P20LEMD2 + 4u), p20lemd2_after_findvac, false},
    {KEY(024u, PROG25), prog25, false},
    {KEY(024u, PROG25 + 4u), prog25_after_2phschng, false},
    {KEY(024u, PROG25 + 6u), prog25_after_r02both, false},
    {KEY(024u, PROG25 + 010u), prog25_after_trackflg, false},
    {KEY(024u, P25LEM1), p25lem1, false},
    {KEY(024u, P25LEM1 + 2u), p25lem1_after_phaschng, false},
    {KEY(024u, P25LEM1 + 016u), p25lem1_after_r65lem, false},
    {KEY(024u, P25LEMWT), p25lemwt, false},
    {KEY(024u, P25LMWT1), p25lmwt1, false},
    {KEY(024u, P25LMWT1 + 3u), p25lmwt1_after_twiddle, false},
    {KEY(024u, P25LEM2), p25lem2, false},
    {KEY(024u, P25LEM2 + 4u), p25lem2_after_findvac, false},
    {KEY(032u, ORBCHG3 + 0145u), orbchg3_after_exit, false},
    /* Part 2: R22 and R21 */
    {KEY(024u, R22LEM), r22lem, false},
    {KEY(024u, R22LEM + 2u), r22lem_after_phaschng, false},
    {KEY(024u, R22LEM + 025u), r22lem_after_p20lemb5, false},
    {KEY(024u, R22LEM + 033u), r22lem_read, false},
    {KEY(024u, R22LEM + 035u), r22lem_after_read, false},
    {KEY(024u, R22LEM + 037u), r22lem_good_data, false},
    {KEY(024u, R22LEM + 040u), r22lem_no_data, false},
    {KEY(024u, R22LEM + 041u), r22lem_los_alarm, false},
    {KEY(024u, R22LEM + 044u), r22lem_los_alarm_terminate, false},
    {KEY(024u, R22LEM + 045u), r22lem_los_alarm_proceed, false},
    {KEY(024u, R22LEM + 046u), r22lem_los_alarm_enter, false},
    {KEY(024u, R22LEM + 047u), r22lem_los_alarm_immediate_return, false},
    {KEY(024u, R22LEM1 + 2u), r22lem1_after_phaschng, false},
    {KEY(024u, R22LEM1 + 5u), r22lem1_terminate, true},
    {KEY(024u, R22LEM1 + 6u), r22lem1_proceed, true},
    {KEY(024u, R22LEM1 + 7u), r22lem1_enter, true},
    {KEY(024u, R22LEM2), r22lem2, false},
    {KEY(024u, R22LEM2 + 2u), r22lem2_after_phaschng, true},
    {KEY(024u, R22LEM2 + 3u), r22lem2_on_surface, true},
    {KEY(024u, R22LEM2 + 4u), r22lem2_in_orbit, false},
    {KEY(024u, R22LEM2 + 012u), r22lem2_after_boresight, true},
    {KEY(024u, R22LEM2 + 014u), r22lem2_boresight_in, true},
    {KEY(024u, R22LEM2 + 015u), r22lem2_boresight_out, false},
    {KEY(024u, R22LEM2 + 017u), r22lem2_after_r61lem, true},
    {KEY(024u, R22LEM3), r22lem3, true},
    {KEY(024u, R22LEM93 + 1u), r22lem93_after_exit, false},
    {KEY(024u, R22LEM93 + 3u), r22lem93_after_phaschng, false},
    {KEY(024u, R22LEM96 + 1u), r22lem96_after_exit, false},
    {KEY(024u, R22LEM96 + 5u), r22lem96_after_phaschng, false},
    {KEY(024u, R22LEM96 + 011u), r22lem96_after_novac, true},
    {KEY(024u, R22LEM96 + 020u), r22lem96_recycle, false},
    {KEY(024u, R22LEM96 + 021u), r22lem96_on_surface, false},
    {KEY(024u, R22LEM96 + 022u), r22lem96_in_orbit, false},
    {KEY(024u, R22LEM44 + 2u), r22lem44_on_surface, false},
    {KEY(024u, R22LEM44 + 3u), r22lem44_in_orbit, false},
    {KEY(024u, R22LEM42), r22lem42, false},
    {KEY(024u, R22LEM42 + 1u), r22lem42_on_surface, false},
    {KEY(024u, R22LEM42 + 2u), r22lem42_in_orbit, false},
    {KEY(024u, R22LEM45 + 3u), r22lem45_after_r65lem, false},
    {KEY(024u, R22WAIT), r22wait, false},
    {KEY(024u, R22LEM46), r22lem46, false},
    {KEY(024u, R22LEM46 + 3u), r22lem46_after_delayjob, false},
    {KEY(024u, N49DSP), n49dsp, false},
    {KEY(024u, N49DSP + 3u), n49dsp_terminate, false},
    {KEY(024u, N49DSP + 4u), n49dsp_proceed, false},
    {KEY(024u, R22RSTRT), r22rstrt, false},
    {KEY(024u, R22RSTRT + 2u), r22rstrt_after_phaschng, false},
    {KEY(024u, R22RSTRT + 4u), r22rstrt_after_rrrdot, false},
    {KEY(024u, R22RSTRT + 6u), r22rstrt_bad_read, false},
    {KEY(024u, R22RSTRT + 7u), r22rstrt_good_read, false},
    {KEY(024u, R21LEM), r21lem, false},
    {KEY(024u, R21LEM + 4u), r21lem_on_surface, false},
    {KEY(024u, R21LEM + 5u), r21lem_in_orbit, false},
    {KEY(024u, R21LEM5), r21lem5, false},
    {KEY(024u, R21LEM5 + 4u), r21lem5_mode2_center, false},
    {KEY(024u, R21LEM6 + 2u), r21lem6_after_downflag, false},
    {KEY(024u, R21LEM6 + 4u), r21lem6_after_rrdesnb, true},
    {KEY(024u, R21LEM6 + 5u), r21lem6_stall, false},
    {KEY(024u, R21LEM6 + 7u), r21lem6_bad_return, true},
    {KEY(024u, R21LEM10), r21lem10, false},
    {KEY(024u, R21LEM10 + 2u), r21lem10_after_upflag, true},
    {KEY(024u, R21LEM2), r21lem2, true},
    {KEY(024u, R21LEM3), r21lem3, false},
    {KEY(024u, R21LEM3 + 2u), r21lem3_after_upflag, false},
    {KEY(024u, R21LEM3 + 4u), r21lem3_after_downflag, true},
    {KEY(024u, R21LEM3 + 010u), r21lem3_not_in_coverage, true},
    {KEY(024u, R21LEM3 + 011u), r21lem3_maneuver, true},
    {KEY(024u, R21LEM3 + 012u), r21lem3_designating, false},
    {KEY(024u, R21LEM3 + 014u), r21lem3_bad_end, true},
    {KEY(024u, R21LEM3 + 015u), r21lem3_lockon, true},
    {KEY(024u, R21_503), r21_503, false},
    {KEY(024u, R21_503 + 3u), r21_503_terminate, true},
    {KEY(024u, R21_503 + 4u), r21_503_proceed, true},
    {KEY(024u, R21_503 + 5u), r21_503_enter, true},
    {KEY(024u, R21_503 + 6u), r21_503_immediate_return, false},
    {KEY(024u, R21END + 2u), r21end_after_downflag, false},
    {KEY(024u, R21SRCH), r21srch, false},
    {KEY(024u, R21SRCH + 2u), r21srch_after_phaschng, false},
    {KEY(024u, R21LEM4 + 4u), r21lem4_after_upflag, false},
    {KEY(024u, R21LEM4 + 6u), r21lem4_after_downflag, true},
    {KEY(024u, R21LEM12 + 012u), r21lem12_not_in_coverage, true},
    {KEY(024u, R21LEM12 + 013u), r21lem12_maneuver, false},
    {KEY(024u, R21LEM12 + 014u), r21lem12_in_coverage, false},
    {KEY(024u, R21LEM12 + 016u), r21lem12_after_killtask, true},
    {KEY(024u, R21LEM13 + 2u), r21lem13_tried_enough, true},
    {KEY(024u, R21LEM7), r21lem7, true},
    {KEY(024u, R21LEM8 + 5u), r21lem8_after_exit, false},
    {KEY(024u, R21LEM8 + 7u), r21lem8_after_cdesflag, false},
    {KEY(024u, R21LEM8 + 011u), r21lem8_after_lokonsw, false},
    {KEY(024u, R21LEM8 + 013u), r21lem8_after_norrmon, false},
    {KEY(024u, R21LEM8 + 015u), r21lem8_after_rrdesnb, true},
    {KEY(024u, R21LEM8 + 016u), r21lem8_delta_time, true},
    {KEY(024u, R21LEM8 + 024u), r21lem8_after_delta_time, false},
    {KEY(024u, R21LEM8 + 031u), r21lem8_after_longcall, false},
    {KEY(024u, R21LEM9), r21lem9, false},
    {KEY(024u, R21LEM9 + 2u), r21lem9_after_killtask, false},
    {KEY(024u, R21LEM9 + 3u), r21lem9_after_clradmod, false},
    {KEY(024u, R21LEM9 + 7u), r21lem9_after_findvac, false},
    {KEY(024u, R21LEM11 + 3u), r21lem11_terminate, false},
    {KEY(024u, R21LEM11 + 4u), r21lem11_proceed, false},
    {KEY(024u, R21LEM11 + 5u), r21lem11_enter, false},
    {KEY(024u, R21LEM11 + 6u), r21lem11_immediate_return, false},
    {KEY(024u, R21DISP + 2u), r21disp_after_phaschng, false},
    {KEY(024u, R21DISP + 5u), r21disp_terminate, false},
    {KEY(024u, R21DISP + 6u), r21disp_proceed, false},
    {KEY(024u, R21DISP + 7u), r21disp_enter, false},
    {KEY(024u, R21DISP + 010u), r21disp_immediate_return, false},
    {KEY(024u, R21DISP + 012u), r21disp_after_linus, false},
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
    &ORBCHG2_CODE,
    &P20LEM1_CODE,
    &P20LEMC3_CODE,
    &UPPSV_CODE,
    &ORBCHG3_CODE,
    &R22LEM3_CODE,
    &R22LEM96_CODE,
    &R22LEM96_WAIT_CODE,
    &R22LEM7_CODE,
    &R21LEM1_CODE,
    &R21LEM3_CODE,
    &R21LEM12_CODE,
    &R21LEM12_FIRST_PASS_CODE,
    &R21LEM7_CODE,
    &R21LEM8_CODE,
};

/* The table of bank that address is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* Whether an order word ends with a switch order (BOF, CLRGO, ...: code
 * 0162): its right order, or its left if it has one. */
static bool ends_with_switch(word w)
{
    unsigned left = (~w & 0177u), right = ((~w >> 7) & 0177u);

    return (right ? right : left) == ORDER_CLRGO + 1u;
}

/* Whether store code w would store past the end of the E-bank window
 * (01777): the AGC then writes into fixed memory and reads it back into A
 * and L, which the readable machine does not model. DOSTORE's index (X1
 * or X2) and STORE's addressing, from w - 1 (NEWOPS's CCS); MODE says how
 * many words (DP, TP or vector). The faithful port's other interpretive
 * routines refuse such a store too (lmi_store_spills). */
static bool store_spills(word w)
{
    unsigned v = (unsigned)w - 1u, kind = (v >> 11) & 7u, a = v & 03777u, n;

    if (kind == 1u || kind == 2u)
        a = (a + (~agc.memory[(FIXLOC + K_X1 + kind - 1u) & 03777u] & 077777u)) % 077777u;
    if (a <= 45u) return false; /* the work area, or word 45 itself */
    n = MODE == 0u || MODE == 077777u ? 1u : (MODE & 040000u) ? 5u : 2u;
    return (a & 0377u) + n > 0377u;
}

/* Whether a switch order's branch went to a store code, which the
 * faithful pair goes on through: R21LEM12's CLRGO to R21LEM7 +1, a TC
 * INTPRET read as a store code. */
static bool switched_to_store_code(unsigned bank, word start)
{
    const struct interpretive_code *code = code_at(bank, LOC);

    if (Z != NEWOPS || !ends_with_switch(start) || !code) return false;
    return !(code->words[LOC - code->first] & 040000u);
}

/* One pass; after RTB LOADTIME, LOADTIME and its DANZIG; after a switch
 * order's branch to a store code, the store code. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 lm_channel_io *io, const struct interpretive_code *code)
{
    unsigned bank = (BANKSET >> 10) & 037u;
    word start = code->words[LOC - code->first];

    if (interpret_listing(program, state, &io->io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (switched_to_store_code(bank, start)) {
        const struct interpretive_code *target = code_at(bank, LOC);

        if (store_spills(target->words[LOC - target->first])) return EXEC_OUT_OF_SCOPE;
        return interpret_listing(program, state, &io->io, target);
    }
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, &io->io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    agc_word channel12;

    if (!entry) return faithful_lm_p20_p25_basic.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane(entry->with_work_area)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    channel12 = hardware_now ? hardware_now->channel12 : 0u;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        if (hardware_now) hardware_now->channel12 = channel12;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_lm_p20_p25_interpretive.entries[index].call(program, state, device);
    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool p20_p25_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
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
#include "p20_p25_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_p20_p25_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_p20_p25_basic = {basic_keys, COUNT(basic_keys)};

#endif
