/* SPDX-License-Identifier: MIT */
/* The readable CM P20-P25 (src/p20_p25_cm/) as routines of the CM's flight
 * program, under the table names and keys of the faithful port's
 * (c64-port/src/cm_p20_p25.c): cm_p20_p25_basic (the basic code) and
 * cm_p20_p25_interpretive (the order pairs).
 *
 * The file is readable part by part, so the tables carry every key of the
 * faithful ones, in their order (cm_p20_p25_keys.inc, which
 * tools/p20_p25_keys.py writes from them): a key a readable part has
 * written runs the readable routine; any other runs the faithful port's,
 * built into the program as faithful_cm_p20_p25_basic and
 * faithful_cm_p20_p25_interpretive (tests/x86_readable.py, the
 * Makefile's cm-p20-p25 tests).
 *
 * Plumbing only. A basic routine runs from its key until control leaves
 * the file (BANKCALL, 2PHSCHNG, PRIOCHNG, UPFLAG, DOWNFLAG, FINDVAC,
 * ENDOFJOB, ...), reaches a label the faithful port keys a routine at, or
 * reaches NEWOPS through a TC INTPRET. An order pair is one pass of the
 * readable interpreter over the listing's words (interpret_listing()); a
 * pass that ends at RTB LOADTIME or TPMODE runs the routine
 * (src/rtb_op_codes/) and the DANZIG it ends at, as the faithful pair does. A pass that ends at an
 * EXIT or a CALL out of its table stops there, as the faithful pair does.
 *
 * The device is the CM's io record (cm_native_io), and the program must
 * have its hardware record (channels 30-33), as the faithful routines
 * want. RANGERD reads and writes channel 13 (in the io record) and
 * VHFREAD reads channel 33 (in the hardware record).
 *
 * The states the faithful routines take, at every key: FB the key's bank
 * (in banks 30-37 the superbank bit clear; banks 40-43 are FB 30-33 with
 * it set); at an order pair Z at NEWOPS, BANKSET the BBANK and INTBIT15
 * the bank's bit 15. Always ZERO +0, the bank registers agreeing, the
 * interrupt and superbank latches in range, and every erasable word an
 * ordinary one. At an order pair and at the basic keys the faithful port
 * marks so (its at_basic), NEWJOB +0 or a core set and FIXLOC one of the
 * VAC areas; at the other basic keys (its at_basic_nv) NEWJOB may be -0
 * too, and FIXLOC anything. A run the readable code refuses part way
 * leaves the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "rope_image.h"
#include "cm_p20_p25.h"
#include "../p20_p25_cm/p20_p25_cm.h"
#include "../interpreter/interpreter.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if COMANCHE055

/* The faithful tables, renamed. */
extern const agc_routine_table faithful_cm_p20_p25_basic, faithful_cm_p20_p25_interpretive;

/* A key: the bank, and the address in it (or in fixed-fixed memory). */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))

/* LOADTIME's and TPMODE's banks (FB), from their FCADRs. */
#define LOADTIME_BANK ((word)(I_LOADTIME & 076000u))
#define TPMODE_BANK   ((word)(I_TPMODE & 076000u))
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The run's io record (channels 11-14) and hardware record (channels
 * 30-33). */
static cm_native_io *io_now;
static agc_hardware *hardware_now;

/* RANGERD's channel 13 and VHFREAD's channel 33, as the faithful port
 * reads them. */
static word channel_in(unsigned number)
{
    if (number == 013u) return (word)io_now->channel13;
    if (number == 033u && hardware_now) return (word)hardware_now->channel33;
    not_modelled(); /* the readable parts read no other channel */
    return 0;
}

/* RANGERD's WAND and WOR of channel 13. As yaAGC writes it, bits 12-14
 * set the HANDRUPT traps and are not kept. */
static void channel_out(unsigned number, word value)
{
    if (number != 013u) not_modelled(); /* the readable parts write no other channel */
    if (value & 004000u) io_now->trap31a = 1u;
    if (value & 010000u) io_now->trap31b = 1u;
    if (value & 020000u) io_now->trap32 = 1u;
    io_now->channel13 = (agc_word)(value & 043777u);
}

/* The interpreter's fixed operands are the rope's. */
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

/* The key's bank selected: FB, with the superbank bit clear in banks
 * 30-37, and set for banks 40-43 (FB 30-33). */
static bool in_bank(unsigned bank)
{
    if (bank >= 040u)
        return FBANK == (word)((bank - 010u) << 10) && (*agc.superbank & SUPERBANK_BIT);
    return FBANK == (word)(bank << 10) && (bank < 030u || !(*agc.superbank & SUPERBANK_BIT));
}

/* ---- The basic code ---- */

static const struct basic_entry {
    uint16_t key;
    void (*routine)(void);
    bool with_work_area; /* the faithful routine's at_basic (else at_basic_nv) */
} basic_entries[] = {
    /* Part 1: P20 */
    {KEY(037u, PROG20), prog20, false},
    {KEY(037u, PROG20 + 2u), prog20_after_r02both, false},
    {KEY(037u, PROG20 + 7u), prog20_after_prftrkat, false},
    {KEY(037u, PROG20 + 011u), prog20_after_vehupflg, false},
    {KEY(037u, PROG20 + 013u), prog20_after_trackflg, false},
    {KEY(037u, PROG20 + 015u), prog20_after_updatflg, false},
    {KEY(037u, PROG20 + 017u), prog20_after_rndvzflg, false},
    {KEY(037u, PROG20 + 023u), prog20_after_2phschng, true},
    {KEY(037u, PROG20 + 055u), prog20_after_exit, false},
    {KEY(037u, PROG20 + 061u), prog20_after_findvac, false},
    {KEY(037u, PIKUP20), pikup20, false},
    {KEY(037u, PIKUP20 + 2u), pikup20_after_priochng, false},
    {KEY(037u, PIKUP20 + 016u), pikup20_after_r61csm, false},
    {KEY(037u, P20R52JB), p20r52jb, true},
    {KEY(037u, P20R52JB + 4u), p20r52jb_after_exit, false},
    {KEY(037u, P20R52JB + 6u), p20r52jb_after_mkrlees, false},
    /* Part 2: P22 */
    {KEY(030u, PROG22), prog22, false},
    {KEY(030u, PROG22 + 2u), prog22_after_rndvzflg, false},
    {KEY(030u, PROG22 + 4u), prog22_after_r02both, true},
    {KEY(030u, PROG22 + 023u), prog22_after_exit, false},
    {KEY(030u, PROG22 + 026u), prog22_terminate, false},
    {KEY(030u, PROG22 + 027u), prog22_proceed, true},
    {KEY(030u, PROG22 + 030u), prog22_enter, false},
    {KEY(030u, PROG22 + 031u), prog22_immediate_return, false},
    {KEY(030u, PROG22 + 033u), prog22_after_blanket, false},
    {KEY(030u, CALLR52 + 1u), callr52_after_exit, false},
    {KEY(030u, CALLR52 + 5u), callr52_after_2phschng, false},
    {KEY(030u, CALLR52 + 011u), callr52_after_targ2flg, false},
    {KEY(030u, CALLR52 + 013u), callr52_after_targ1flg, true},
    {KEY(030u, P22OVER + 1u), p22over_after_exit, false},
    {KEY(030u, P22OVER + 3u), p22over_after_phaschng, true},
    {KEY(030u, P22SUBRB + 2u), p22subrb_after_exit, false},
    {KEY(030u, P22SUBRB + 5u), p22subrb_terminate, false},
    {KEY(030u, P22SUBRB + 6u), p22subrb_proceed, true},
    {KEY(030u, P22SUBRB + 7u), p22subrb_enter, true},
    {KEY(030u, P22SUBRB + 010u), p22subrb_check, true},
    {KEY(030u, N89ERRX + 1u), n89errx_after_exit, false},
    {KEY(030u, N89ERRX + 2u), n89errx_after_falton, false},
    {KEY(030u, S22DSPPA), s22dsppa, false},
    {KEY(030u, S22DSPPA + 3u), s22dsppa_terminate, false},
    {KEY(030u, S22DSPPA + 4u), s22dsppa_proceed, false},
    {KEY(030u, S22DSPPA + 5u), s22dsppa_recycle, false},
    {KEY(030u, S22DSPPA + 6u), s22dsppa_immediate_return, false},
    {KEY(030u, S22DSPPA + 010u), s22dsppa_after_blanket, false},
    {KEY(030u, S22DSPPA + 021u), s22dsppa_known, false},
    {KEY(030u, S22DSPPA + 023u), s22dsppa_after_knownflg, true},
    {KEY(030u, S22DSPP + 1u), s22dspp_after_falton, true},
    {KEY(030u, S22DSPP + 2u), s22dspp_plus2, true},
    {KEY(030u, S22DSPP + 3u), s22dspp_plus3, true},
    {KEY(030u, S22DSPP + 4u), s22dspp_known_landmark, false},
    {KEY(030u, S22DSPP + 6u), s22dspp_after_knownflg, true},
    {KEY(030u, S22ABDE), s22abde, true},
    {KEY(030u, DE_GR_50 + 4u), de_gr_50_after_2phschng, false},
    {KEY(030u, DE_GR_50 + 010u), de_gr_50_after_targ2flg, false},
    {KEY(030u, DE_GR_50 + 012u), de_gr_50_after_targ1flg, true},
    /* Part 3: R57, R21 and R23 */
    {KEY(037u, R57 + 2u), r57_after_exit, false},
    {KEY(037u, R57 + 010u), r57_not_set, false},
    {KEY(037u, R57A + 2u), r57a_after_v59flag, false},
    {KEY(037u, R57A + 5u), r57a_terminate, false},
    {KEY(037u, R57A + 6u), r57a_proceed, false},
    {KEY(037u, R57A + 7u), r57a_enter, false},
    {KEY(037u, R57A + 010u), r57a_immediate_return, false},
    {KEY(037u, R57A + 012u), r57a_after_blanket, false},
    {KEY(037u, MARKDISP), markdisp, false},
    {KEY(037u, MARKDISP + 3u), markdisp_terminate, false},
    {KEY(037u, MARKDISP + 4u), markdisp_proceed, false},
    {KEY(037u, MARKDISP + 5u), markdisp_enter, false},
    {KEY(037u, MARKDISP + 6u), markdisp_immediate_return, false},
    {KEY(037u, MARKDISP + 010u), markdisp_after_blanket, false},
    {KEY(037u, ENDR57 + 2u), endr57_after_v59flag, false},
    {KEY(037u, ENDR57 + 010u), endr57_after_novac, true},
    {KEY(037u, R21CSM), r21csm, false},
    {KEY(037u, R23CSM), r23csm, false},
    {KEY(037u, R23CSM + 012u), r23csm_terminate, false},
    {KEY(037u, R23CSM + 013u), r23csm_proceed, false},
    {KEY(037u, R23CSM + 014u), r23csm_enter, false},
    {KEY(037u, R23CSM1), r23csm1, false},
    {KEY(037u, R23CSM1 + 3u), r23csm1_terminate, false},
    {KEY(037u, R23CSM1 + 4u), r23csm1_proceed, false},
    {KEY(037u, R23CSM1 + 5u), r23csm1_mark, false},
    {KEY(037u, R23CSM1 + 011u), r23csm1_after_gentran, false},
    {KEY(037u, R21C1), r21c1, false},
    {KEY(037u, R21C1 + 3u), r21c1_terminate, false},
    {KEY(037u, R21C1 + 4u), r21c1_proceed, false},
    {KEY(037u, R21C1 + 5u), r21c1_recycle, false},
    {KEY(037u, R21CSM1 + 3u), r21csm1_after_gentran, false},
    {KEY(037u, R21END + 2u), r21end_after_kleenex, false},
    {KEY(037u, R21END + 4u), r21end_after_r21mark, false},
    /* Part 4: R61CSM */
    {KEY(037u, R61CSM), r61csm, true},
    {KEY(037u, R61CSM + 4u), r61csm_after_makecadr, true},
    {KEY(037u, R61CSM + 7u), r61csm_do_r61, true},
    {KEY(037u, R61CSM + 010u), r61csm_leave, true},
    {KEY(037u, R61CSM + 011u), r61csm_interpret, true},
    {KEY(037u, R61CSM + 015u), r61csm_after_exit, false},
    {KEY(037u, R61CSM + 017u), r61csm_no_maneuver, false},
    {KEY(037u, R61C1), r61c1, false},
    {KEY(037u, R61C1 + 2u), r61c1_after_3axisflg, false},
    {KEY(037u, R61C1 + 6u), r61c1_after_pdspflag, false},
    {KEY(037u, R61C1 + 010u), r61c1_after_r60csm, false},
    {KEY(037u, R61C1 + 012u), r61c1_after_pdspflag_reset, false},
    {KEY(037u, R61C1 + 014u), r61c1_after_phaschng, false},
    {KEY(037u, R61END), r61end, false},
    /* Part 5: R22 up to REND12 */
    {KEY(034u, R22), r22, true},
    {KEY(034u, R22 + 3u), r22_after_priochng, true},
    {KEY(034u, REND1A + 1u), rend1a_after_exit, true},
    {KEY(034u, REND2 + 4u), rend2_after_gentran, true},
    /* Part 6: R22 from REND12, and GOTOV56 */
    {KEY(034u, RENDISP + 1u), rendisp_after_exit, false},
    {KEY(034u, RENDISP + 7u), rendisp_optics, false},
    {KEY(034u, RENDISP + 015u), rendisp_after_phaschng, false},
    {KEY(034u, RENDISP7), rendisp7, true},
    {KEY(034u, RENDISP2), rendisp2, false},
    {KEY(034u, RENDISP2 + 3u), rendisp2_terminate, false},
    {KEY(034u, RENDISP2 + 4u), rendisp2_proceed, false},
    {KEY(034u, RENDISP3 + 4u), rendisp3_after_exit, true},
    {KEY(02u, GOTOV56), gotov56, false},
    /* Part 7: R22's subroutines in bank 27 */
    {KEY(027u, REDOR22), redor22, false},
    {KEY(027u, REDOR22 + 2u), redor22_after_phaschng, false},
    {KEY(027u, REDOR22 + 4u), redor22_after_priochng, false},
    {KEY(027u, WAITONE + 3u), waitone_after_exit, false},
    {KEY(027u, WAITONE + 6u), waitone_after_delayjob, true},
    {KEY(027u, RANGERD + 1u), rangerd_after_exit, false},
    {KEY(027u, RANGERD + 016u), rangerd_after_skipvhf, false},
    {KEY(027u, RANGERD + 020u), rangerd_bad, true},
    {KEY(027u, RANGERD + 021u), rangerd_good, false},
    {KEY(027u, RANGERD1), rangerd1, true},
    {KEY(027u, RANGERD1 + 2u), rangerd1_plus_zero, true},
    {KEY(027u, RANGERD1 + 3u), rangerd1_negative, true},
    {KEY(027u, RANGERD1 + 4u), rangerd1_minus_zero, true},
    {KEY(027u, RANGERD4), rangerd4, true},
    {KEY(027u, LIGHTON + 4u), lighton_after_trfailon, true},
    {KEY(027u, VHFREAD), vhfread, false},
    {KEY(027u, VHFGOOD), vhfgood, false},
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
    &PROG20_CODE,   &P20R52JB_CODE,       &PROG22_CODE,    &PROG22A_CODE,       &CALLR52_CODE,
    &S22LSITE_CODE, &P22SUBRB_CHECK_CODE, &N89ERRX_3_CODE, &S22ABDE_CODE,       &DE_GR_50_CODE,
    &R57_CODE,      &ENDR57_CODE,         &R61CSM_CODE,    &BVECTORS_CODE,      &GETUM_CODE,
    &R22_CODE,      &REND2_CODE,          &REND3_CODE,     &REND4_CODE,         &REND6_CODE,
    &REND9_CODE,    &REND10_CODE,         &REND10A_CODE,   &REND12_CODE,        &REND14_CODE,
    &CMPOS_CODE,    &RENDISP_CODE,        &RENDISP7_CODE,  &RENDISP3_CODE,      &RENDISP4_CODE,
    &SETINTG_CODE,  &CNTCHK_CODE,         &WAITONE_CODE,   &WAITONE_AGAIN_CODE, &RANGERD_CODE,
    &RANGERD4_CODE, &RANGERD3_CODE,       &LIGHTON_CODE,   &SHIFTNDX_CODE,      &INITIALW_CODE,
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

/* One pass; after RTB LOADTIME or TPMODE, the routine and its DANZIG. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_native_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z == TPMODE && FBANK == TPMODE_BANK) { /* (the faithful pair takes any state) */
        tpmode();
        return interpret_listing(program, state, io, code);
    }
    if (Z != LOADTIME || FBANK != LOADTIME_BANK) return EXEC_OK;
    if (!sane(true)) return EXEC_OUT_OF_SCOPE; /* (as the faithful RTB takes it) */
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* Each key's routine: the readable one, or the faithful port's. */
static enum exec_status basic(unsigned index, uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    const struct basic_entry *entry = basic_entry(key);
    cm_native_io *io = device, kept;
    apollo_overlay_state before;

    if (!entry) return faithful_cm_p20_p25_basic.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    /* (fixed-fixed code, banks 02 and 03, runs under any FB) */
    if ((key / 1024u >= 4u && !in_bank(key / 1024u)) || !sane(entry->with_work_area))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    if (setjmp(back)) {
        *state = before;
        *io = kept;
        return EXEC_OUT_OF_SCOPE;
    }
    entry->routine();
    return EXEC_OK;
}

/* PROG22C's SLOAD* QPRET,2 and GETUM's VLOAD* 1,1 load from the mark data
 * (an index register its address's complement): the faithful port takes
 * those words from erasable memory only, ADDRWD below 02000. */
static bool erasable_only(uint16_t key)
{
    return key == KEY(030u, PROG22C) || key == KEY(023u, GETUM + 3u);
}

static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    unsigned bank = key / 1024u;
    cm_native_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_cm_p20_p25_interpretive.entries[index].call(program, state, device);
    if (!program || !program->hardware || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || !in_bank(bank) || !code || BANKSET != BBANK ||
        INTBIT15 != (FBANK & 040000u) || !sane(true))
        return EXEC_OUT_OF_SCOPE;

    before = *state;
    kept = *io;
    status = one_pass(program, state, io, code);
    if (status == EXEC_OK && erasable_only(key) && ADDRWD >= 02000u) status = EXEC_OUT_OF_SCOPE;
    if (status != EXEC_OK) {
        *state = before;
        *io = kept;
    }
    return status;
}

/* For the side-by-side test: whether a key runs readable code. */
bool cm_p20_p25_readable(uint16_t key, bool order_pair)
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
#include "cm_p20_p25_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_p20_p25_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_p20_p25_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_p20_p25_basic = {basic_keys, COUNT(basic_keys)};

#endif
