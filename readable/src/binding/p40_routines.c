/* SPDX-License-Identifier: MIT */
/* The readable P40-P47 (src/p40_p47/) as routines of the LM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/lm_p40_p47.c): lm_p40_p47_basic (the basic code) and
 * lm_p40_p47_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (p40_keys.inc, which tools/p20_p25_keys.py writes from them), and every
 * key runs a readable routine: the file was written part by part, and is
 * whole. The side-by-side test (tests/p40_cases.c) checks that no key is
 * left without one.
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file's code (PHASCHNG, BANKCALL, IBNKCALL, FINDVAC, BURNBABY, ...,
 * or a TC P40/RET) or reaches NEWOPS through a TC INTPRET. An order pair
 * is one pass of the readable interpreter over the listing's words
 * (interpret_listing()), which ends at the pair's DANZIG, at an EXIT, or
 * where a CALL, GOTO or branch leaves it.
 *
 * The device is the LM's io record with the input channels after it
 * (lm_channel_io). ENGINOF3 reads and writes channel 11 (DSALMOUT) and
 * sets a bit of channel 14, and TRIMGIMB drives the gimbal through
 * channel 12: the program's hardware record's.
 *
 * The states the faithful routines take, at every key: FB the key's bank,
 * 27 or 36, with the superbank bit clear in bank 36; at an order pair Z at NEWOPS, BANKSET
 * the BBANK and INTBIT15 the bank's bit 15. Always ZERO +0, the bank
 * registers agreeing, the interrupt and superbank latches in range, and
 * every erasable word an ordinary one. At an order pair and at the basic
 * keys the faithful port marks so (its at_basic: those that can reach a TC
 * INTPRET), NEWJOB +0 or a core set and FIXLOC one of the VAC areas; at
 * the other basic keys (its at_basic_nv) NEWJOB may be -0 too, and FIXLOC
 * anything. A run the readable code refuses part way leaves the state, io
 * record and channels as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "../p40_p47/p40_p47.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#if LUMINARY099

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define SUPERBANK_BIT      0100u

static agc_hardware *hardware_now, hardware_kept; /* (the latches before a basic run) */
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* Channels 11, 12 and 14, the hardware's latches; any other is not
 * modelled, nor a run with no hardware record. */
static agc_word *latch(unsigned number)
{
    if (!hardware_now) not_modelled();
    switch (number) {
    case 011u: return &hardware_now->channel11;
    case 012u: return &hardware_now->channel12;
    case 014u: return &hardware_now->channel14;
    default: not_modelled(); return NULL;
    }
}

static word channel_in(unsigned number)
{
    return (word)*latch(number);
}

static void channel_out(unsigned number, word value)
{
    *latch(number) = value;
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

/* The key's bank selected: FB, and in banks 30 to 37 the superbank bit
 * clear (with it set, FB 30 to 37 would be banks 40 to 47). */
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
    /* Part 1: P40 */
    {KEY(036u, P40LM), p40lm, false},
    {KEY(036u, P40LM + 2u), p40lm_after_phaschng, true},
    {KEY(036u, P40LM + 012u), p40lm_after_r02both, true},
    {KEY(036u, P40LM + 037u), p40lm_after_exit, false},
    {KEY(036u, P40LM + 042u), p40lm_after_pflitedb, false},
    {KEY(036u, P40LM + 043u), p40lm_burn, false},
    {KEY(036u, P40SXT4 + 5u), p40sxt4_after_downflag, false},
    {KEY(036u, P40SXT4 + 7u), p40sxt4_after_r60lem, false},
    {KEY(036u, POSTBURN), postburn, false},
    {KEY(036u, POSTBURN + 010u), postburn_terminate, false},
    {KEY(036u, POSTBURN + 011u), postburn_proceed, false},
    {KEY(036u, POSTBURN + 012u), postburn_enter, false},
    {KEY(036u, P40PHS1), p40phs1, false},
    {KEY(036u, P40PHS1 + 2u), p40phs1_after_phaschng, false},
    {KEY(036u, TIGNOW), tignow, false},
    {KEY(036u, TIGNOW + 3u), tignow_after_zatteror, false},
    {KEY(036u, TIGNOW + 5u), tignow_after_setmindb, false},
    {KEY(036u, TIGNOW + 011u), tignow_terminate, false},
    {KEY(036u, TIGNOW + 012u), tignow_proceed, false},
    {KEY(036u, TIGNOW + 013u), tignow_enter, false},
    {KEY(036u, TIGNOW + 014u), tignow_immediate, false},
    {KEY(036u, TERM40 + 012u), term40_after_restordb, false},
    /* Part 1: P41 */
    {KEY(036u, P41LM), p41lm, false},
    {KEY(036u, P41LM + 4u), p41lm_after_r02both, true},
    {KEY(036u, P41LM + 023u), p41lm_after_exit, false},
    {KEY(036u, P41LM + 026u), p41lm_after_zatteror, false},
    {KEY(036u, P41LM + 030u), p41lm_after_setmindb, true},
    {KEY(036u, P41LM + 031u), p41lm_after_p40sxt4, true},
    {KEY(036u, P41LM + 037u), p41lm_after_vgbody, false},
    {KEY(036u, P41LM + 042u), p41lm_after_godspret, false},
    {KEY(036u, P41LM + 047u), p41lm_after_findvac, false},
    {KEY(036u, P41LM + 052u), p41lm_burn, false},
    {KEY(036u, REDO6_7), redo6_7, false},
    {KEY(036u, REDO6_7 + 7u), redo6_7_after_godspret, false},
    {KEY(036u, DYNMDISP), dynmdisp, true},
    {KEY(036u, DYNMDISP + 011u), dynmdisp_after_exit, false},
    {KEY(036u, DYNMDISP + 014u), dynmdisp_after_delayjob, true},
    {KEY(036u, CALCN85), calcn85, true},
    {KEY(036u, CALCN85 + 010u), calcn85_after_exit, false},
    /* Part 1: P42 */
    {KEY(036u, P42LM), p42lm, false},
    {KEY(036u, P42LM + 2u), p42lm_after_phaschng, true},
    {KEY(036u, P42STAGE), p42stage, false},
    {KEY(036u, P42STAGE + 2u), p42stage_after_r02both, true},
    /* Part 1: P47 */
    {KEY(036u, P47LM), p47lm, false},
    {KEY(036u, P47LM + 2u), p47lm_after_r02both, true},
    {KEY(036u, P47LM + 5u), p47lm_after_midtoav2, false},
    {KEY(036u, P47LM + 010u), p47lm_after_twiddle, false},
    {KEY(036u, STARTP47), startp47, false},
    {KEY(036u, STARTP47 + 3u), startp47_after_phaschng, false},
    {KEY(036u, STARTP47 + 012u), startp47_after_findvac, false},
    {KEY(036u, CALCN83), calcn83, true},
    {KEY(036u, CALCN83 + 011u), calcn83_after_exit, false},
    {KEY(036u, CALCN83 + 013u), calcn83_after_phaschng, true},
    {KEY(036u, CALCN83 + 020u), calcn83_after_delvctl, false},
    {KEY(036u, P47BOD + 3u), p47bod_terminate, false},
    {KEY(036u, P47BOD + 4u), p47bod_proceed, false},
    {KEY(036u, P47BOD + 5u), p47bod_enter, true},
    {KEY(036u, P47BOD + 6u), p47bod_immediate, false},
    {KEY(036u, P47BODY), p47body, true},
    {KEY(036u, P47BODY + 6u), p47body_after_exit, false},
    /* Part 1: the engine off routines */
    {KEY(036u, IMPLBURN), implburn, false},
    {KEY(036u, IMPLBURN + 2u), implburn_after_getdt, false},
    {KEY(036u, IMPLBURN + 4u), implburn_after_twiddle, false},
    {KEY(036u, IMPLBURN + 6u), implburn_after_ignflag, false},
    {KEY(036u, IMPLBURN + 010u), implburn_after_astnflag, false},
    {KEY(036u, IMPLBURN + 012u), implburn_after_impulsw, false},
    {KEY(036u, IMPLBURN + 014u), implburn_after_phaschng, false},
    {KEY(036u, IMPLBURN + 016u), implburn_after_fixdelay, false},
    {KEY(036u, IMPLBURN + 017u), implburn_after_noullage, false},
    {KEY(036u, ENGOFTSK), engoftsk, false},
    {KEY(036u, ENGOFTSK + 2u), engoftsk_after_enginoff, false},
    {KEY(036u, ENGINOFF), enginoff, false},
    {KEY(036u, ENGINOF2), enginof2, false},
    {KEY(036u, ENGINOF1), enginof1, false},
    {KEY(036u, ENGINOF4), enginof4, false},
    {KEY(036u, ENGINOF3), enginof3, false},
    {KEY(036u, COASTSET), coastset, false},
    {KEY(036u, COASTSET + 2u), coastset_after_allcoast, false},
    /* Part 1: steering */
    {KEY(036u, ONE_EIGHTY_SETUP + 1u), updatevg_after_exit, true},
    {KEY(036u, ONE_EIGHTY_SETUP + 010u), updatevg_after_findvac, false},
    {KEY(036u, ENDSTEER), endsteer, true},
    {KEY(036u, STEERING), steering, true},
    {KEY(036u, NSTEER), nsteer, false},
    {KEY(036u, NSTEER + 017u), nsteer_after_stoprate, false},
    {KEY(036u, NSTEER + 021u), nsteer_after_impulsw, false},
    {KEY(036u, NSTEER + 023u), nsteer_after_idleflag, false},
    {KEY(036u, NSTEER + 033u), nsteer_after_tpagree, false},
    {KEY(036u, NSTEER + 035u), nsteer_after_getdt, false},
    {KEY(036u, NSTEER + 037u), nsteer_after_twiddle, false},
    {KEY(036u, NSTEER + 042u), nsteer_after_2phschng, false},
    /* Part 2: S40.8's alarm, S40.13 and S40.9 */
    {KEY(027u, ALARMIT + 1u), alarmit_after_exit, false},
    {KEY(027u, ALARMIT + 3u), alarmit_after_alarm, true},
    {KEY(027u, S40_13), s40_13, true},
    {KEY(027u, S40_132_STAR), s40_132_star, false},
    {KEY(027u, S40_132_STAR + 1u), s40_132_star_after_tpagree, false},
    {KEY(027u, S40_134), s40_134, false},
    {KEY(027u, S40_134 + 2u), s40_134_after_phaschng, false},
    {KEY(027u, S40_130V + 5u), s40_130v_after_exit, false},
    {KEY(027u, STORETGO + 2u), storetgo_after_exit, false},
    {KEY(027u, S40_9), s40_9, true},
    {KEY(027u, ENDS40_9 + 1u), ends40_9_after_exit, false},
    {KEY(027u, ENDS40_9 + 3u), ends40_9_after_phaschng, false},
    /* Part 3: TRIMGIMB */
    {KEY(027u, TRIMGIMB), trimgimb, false},
    {KEY(027u, TRIMGIMB + 2u), trimgimb_after_downflag, false},
    {KEY(027u, TRIMGIMB + 012u), trimgimb_after_fixdelay, false},
    {KEY(027u, TRIMGIMB + 023u), trimgimb_after_twiddle, false},
    {KEY(027u, TRIMGIMB + 025u), trimgimb_after_vardelay, false},
    {KEY(027u, ROLLOVER + 010u), rollover_after_novac, false},
    {KEY(027u, PITCHOFF), pitchoff, false},
    {KEY(027u, PITCHOFF + 6u), pitchoff_after_upflag, false},
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
    &P40_CODE,      &P41_CODE,      &P41_VGBODY_CODE, &DYNMDISP_CODE,        &CALCN85_CODE,
    &P42_CODE,      &P47_CODE,      &CALCN83_CODE,    &CALCN83_DELVCTL_CODE, &P47BODY_CODE,
    &UPDATEVG_CODE, &ENDSTEER_CODE, &NO_9_CODE,       &STEERING_CODE,        &S40_1_CODE,
    &S40_2_3_CODE,  &S40_8_CODE,    &ALARMIT_CODE,    &S40_13_CODE,          &S40_13D_CODE,
    &S40_130V_CODE, &S40_138_CODE,  &APSTGO_CODE,     &S40_9_CODE,           &RASTEER1_CODE,
    &S41_1_CODE,
};

/* The table an address is in, in a bank, or none. */
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

/* Each key's routine. A basic routine runs from its key, whatever Z
 * holds (the trampoline calls it there). */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry || !program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (!in_bank(key / 1024u) || !sane(entry->with_work_area)) return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (hardware_now) hardware_kept = *hardware_now;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        if (hardware_now) *hardware_now = hardware_kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

static enum exec_status pair(uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    lm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!program || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = interpret_listing(program, state, &io->io, code);
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool p40_p47_readable(uint16_t key, bool order_pair)
{
    return order_pair ? code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL
                      : basic_entry(key) != NULL;
}

#define BASIC(i, bank, address)                                                                    \
    static enum exec_status basic_##i(const agc_native_program *program,                           \
                                      apollo_overlay_state *state, void *device)                   \
    {                                                                                              \
        return basic(KEY(bank, address), program, state, device);                                  \
    }
#define PAIR(i, bank, address)                                                                     \
    static enum exec_status pair_##i(const agc_native_program *program,                            \
                                     apollo_overlay_state *state, void *device)                    \
    {                                                                                              \
        return pair(KEY(bank, address), program, state, device);                                   \
    }
#include "p40_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "p40_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "p40_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table lm_p40_p47_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table lm_p40_p47_basic = {basic_keys, COUNT(basic_keys)};

#endif
