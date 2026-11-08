/* SPDX-License-Identifier: MIT */
/* The readable IMU_CALIBRATION_AND_ALIGNMENT of the CM (src/imu_calibration/)
 * as routines of the CM's flight program, under the table names and keys
 * of the faithful port's (c64-port/src/cm_imu_calibration.c):
 * cm_imu_calibration_basic (the basic code) and
 * cm_imu_calibration_interpretive (the order pairs).
 *
 * The tables carry every key of the faithful ones, in their order
 * (cm_imu_cal_keys.inc, which tools/p20_p25_keys.py writes from them). The
 * file is written part by part: every basic key runs the readable code,
 * and so do the order pairs of pp. 423-430; the others run the faithful
 * port's routines (renamed faithful_) until their parts are written. The
 * side-by-side test (tests/cm_imu_cal_cases.c) checks which.
 * Partial file: Comanche055/IMU_CALIBRATION_AND_ALIGNMENT.agc (COVERAGE
 * counts it when the last part is written).
 *
 * Plumbing only. A basic routine runs the file's functions one after
 * another from its key, as the faithful port's routine runs the file's
 * words: through the words a dynamic return lands on (TC QPLACE, TC
 * QPLACES, TC MPAC, TC Q, INDEX NDXCTR; TC +1), which are the points
 * below, until control leaves the file (BANKCALL, TWIDDLE, PHASCHNG, TC
 * INTPRET into NEWOPS ...) or reaches a word that is no point. It stops
 * where the AGC waits for the hardware: CHECKG1, the PIPA not yet moved,
 * and FINETIME +1, the scaler at 37777 (the faithful port's idle points).
 *
 * The device is the CM's io record with the input channels after it
 * (cm_channel_io): the scalers FINETIME reads (channels 3 and 4) and
 * channel 30, which CHKCOMED reads for liftoff. The states the faithful
 * routines take: FB the key's bank (bank 33, the superbank bit off),
 * EBANK 5 (the bank the faithful port reads the file's switched words
 * in), ZERO +0, the bank registers agreeing, the latches in range and
 * every erasable word an ordinary one; FINETIME's two keys take any FB
 * and EBANK, and PERFERAS +1 (which sets EBANK 7) any EBANK.
 * An order pair is one pass of the readable interpreter over the listing's
 * words (interpret_listing()), from NEWOPS with LOC the key, BANKSET the
 * BBANK, INTBIT15 the bank's bit 15, NEWJOB +0 or a core set and FIXLOC
 * one of the VAC areas; after RTB LOADTIME the pass goes on, as the
 * faithful pair's does. A run the readable code refuses part way leaves
 * the state and io record as they were. */
#include <setjmp.h>
#include <stddef.h>
#include "binding.h"
#include "agc_input_channels.h"
#include "rope_image.h"
#include "cm_imu_calibration.h"
#include "../imu_calibration/imu_calibration.h"
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"
#include "../rtb_op_codes/rtb_op_codes.h"
#include "../agc/names.h"

#if COMANCHE055

/* A key: the bank, and the address in it. */
#define KEY(bank, address) (uint16_t)((bank) * 1024u + ((address) & 01777u))
#define BANK33             033u
#define RUN_LIMIT          4000u

static cm_channel_io *io_now;
static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

/* The scalers (LOSCALAR, HISCALAR) and channel 30, as the caller latched
 * them for the run. */
static word channel_in(unsigned number)
{
    if (number == LOSCALAR) return io_now->in.channel4;
    if (number == HISCALAR) return io_now->in.channel3;
    if (number == ADDRESS_OF(CHAN30)) return io_now->in.channel30;
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

static word rope_word(unsigned physical)
{
    agc_word value = 0;

    if (physical > 0xFFFFu || !apollo_rope_read(1u, (uint16_t)physical, &value)) not_modelled();
    return (word)(value & MINUS_ZERO);
}

static void bind(const agc_native_program *program, apollo_overlay_state *state, cm_channel_io *io)
{
    bind_machine(program, state, &io->io);
    io_now = io;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = rope_word;
    agc.leave = leave;
}

/* The states the faithful routines take (its basic_sane()): fb the key's
 * FB, or any for a fixed-fixed key (FINETIME's), which also takes any
 * EBANK, as PERFERAS +1 does. */
static bool sane(word fb, bool fixed_fixed, bool any_eb)
{
    unsigned i;

    if (!fixed_fixed && FBANK != fb) return false;
    if (agc.memory[ZERO_ADDRESS] != 0u || EBANK > 03400u || (EBANK & 0377u) ||
        BBANK != (word)((FBANK & 076000u) | (EBANK >> 8)))
        return false;
    if (*agc.interrupts > 1u || (*agc.superbank & ~0160u)) return false;
    if (!fixed_fixed && (fb & 060000u) == 060000u && (*agc.superbank & 0100u)) return false;
    if (!any_eb && EBANK != 02400u) return false;
    for (i = 3u; i < 2048u; ++i)
        if (agc.memory[i] > 077777u) return false;
    return true;
}

/* ---- The points ---- */

/* Every word a dynamic return can land on (the faithful port's dispatch),
 * with the function that starts there. Every key is one of them too. */
static const struct point {
    uint16_t at;
    void (*routine)(void);
} points[] = {
    /* FINETIME, in fixed-fixed (drift_loop.c) */
    {KEY(02u, FINETIME), finetime},
    {KEY(02u, FINETIME + 1u), finetime_plus1},
    /* LATAZCHK, in bank 4 (optical_verification.c) */
    {KEY(04u, LATAZCHK + 6u), latazchk_after_exit},
    {KEY(04u, LATAZCHK + 012u), latazchk_flash},
    {KEY(04u, LATAZCHK + 015u), latazchk_terminate},
    {KEY(04u, LATAZCHK + 016u), latazchk_proceed},
    {KEY(04u, LATAZCHK + 017u), latazchk_enter},
    {KEY(04u, LATAZCHK + 020u), latazchk_load},
    /* IMUTEST to ZEROING, pp. 423-429 (imu_test.c) */
    {KEY(BANK33, IMUTEST), imutest},
    {KEY(BANK33, IMUTEST + 6u), imutest_after_coaalign},
    {KEY(BANK33, GEOIMUTT), geoimutt},
    {KEY(BANK33, GEOIMUTT + 2u), geoimutt_after_imuzero},
    {KEY(BANK33, IMUBACK), imuback},
    {KEY(BANK33, NBPOSPL + 4u), nbpospl_after_zeroing},
    {KEY(BANK33, NBPOSPL + 021u), nbpospl_after_exit},
    {KEY(BANK33, NBPOSPL + 023u), nbpospl_not_verifying},
    {KEY(BANK33, NBPOSPL + 024u), nbpospl_verifying},
    {KEY(BANK33, NBPOSPL + 031u), nbpospl_after_calcga},
    {KEY(BANK33, NBPOSPL + 033u), nbpospl_after_imucoars},
    {KEY(BANK33, NBPOSPL + 042u), nbpospl_after_downflag},
    {KEY(BANK33, NBPOSPL + 043u), nbpospl_after_imustllg},
    {KEY(BANK33, NBPOSPL + 045u), nbpospl_fine_align},
    {KEY(BANK33, NBPOSPL + 047u), nbpospl_after_imufine},
    {KEY(BANK33, NBPOSPL + 050u), nbpospl_settle},
    {KEY(BANK33, NBPOSPL + 055u), nbpospl_after_longcall},
    {KEY(BANK33, GOESTIMS), goestims},
    {KEY(BANK33, GOESTIMS + 2u), goestims_after_jobwake},
    {KEY(BANK33, PIPACHK), pipachk},
    {KEY(BANK33, PIPACHK + 2u), pipachk_earthr},
    {KEY(BANK33, PIPACHK + 3u), pipachk_start},
    {KEY(BANK33, PIPACHK + 015u), pipachk_after_checkg},
    {KEY(BANK33, PIPACHK + 021u), pipachk_after_twiddle},
    {KEY(BANK33, PIPATASK), pipatask},
    {KEY(BANK33, STARTPIP), startpip},
    {KEY(BANK33, STARTPIP + 4u), startpip_after_findvac},
    {KEY(BANK33, PIPJOBB), pipjobb},
    {KEY(BANK33, PIPJOBB + 2u), pipjobb_earthr},
    {KEY(BANK33, PIPJOBB + 3u), pipjobb_check_time},
    {KEY(BANK33, PIPJOBB + 012u), pipjobb_after_checkg},
    {KEY(BANK33, AINGOTN + 010u), aingotn_after_exit},
    {KEY(BANK33, AINGOTN + 012u), aingotn_show},
    {KEY(BANK33, VERTDRFT), vertdrft},
    {KEY(BANK33, PONG + 1u), pong_after_earthr_star},
    {KEY(BANK33, VALMIS), valmis},
    {KEY(BANK33, ENDTEST1), endtest1},
    {KEY(BANK33, ENDTEST1 + 2u), endtest1_after_downflag},
    {KEY(BANK33, ENDTEST1 + 4u), endtest1_after_newmodex},
    {KEY(BANK33, ENDTEST1 + 6u), endtest1_after_mkreleas},
    {KEY(BANK33, COAALIGN + 010u), coaalign_after_imucoars},
    {KEY(BANK33, COAALIGN + 012u), coaalign_bad_stall},
    {KEY(BANK33, COAALIGN + 013u), coaalign_after_imustall},
    {KEY(BANK33, CHECKG), checkg},
    {KEY(BANK33, CHECKG1), checkg1},
    {KEY(BANK33, CHECKG1 + 5u), checkg_after_chang1},
    {KEY(BANK33, CHECKG1 + 024u), checkg_after_finetime},
    /* EARTHR* to ALFLT, pp. 430-438 (drift_loop.c) */
    {KEY(BANK33, PROUT + 1u), prout_after_exit},
    {KEY(BANK33, PROUT + 2u), prout_after_imustllg},
    {KEY(BANK33, SHOW1 + 5u), show_after_terminate},
    {KEY(BANK33, SHOW1 + 6u), show_after_proceed},
    {KEY(BANK33, SHOW1 + 7u), show_after_enter},
    {KEY(BANK33, P11OUT), p11out},
    {KEY(BANK33, ESTIMS), estims},
    {KEY(BANK33, RSTGTS1), rstgts1},
    {KEY(BANK33, RSTGTS1 + 014u), rstgts1_after_zeroing},
    {KEY(BANK33, ESTIMS + 031u), estims_after_exit},
    {KEY(BANK33, ESTIMS + 033u), estims_drift_test},
    {KEY(BANK33, ESTIMS + 034u), estims_compass},
    {KEY(BANK33, ESTIMS + 040u), estims_after_erthrvse},
    {KEY(BANK33, ESTIMS + 044u), estims_after_newmodex},
    {KEY(BANK33, ESTIMS + 046u), estims_after_pipuse},
    {KEY(BANK33, SLEEPIE), sleepie},
    {KEY(BANK33, SLEEPIE + 3u), sleepie_after_phaschng},
    {KEY(BANK33, WTLISTNT), wtlistnt},
    {KEY(BANK33, WTLISTNT + 1u), wtlistnt_after_chkcomed},
    {KEY(BANK33, WTLISTNT + 2u), wtlistnt_after_setgwlst},
    {KEY(BANK33, WTGTSMPL + 2u), wtgtsmpl_after_twiddle},
    {KEY(BANK33, STOREDTA), storedta},
    {KEY(BANK33, ALLOOP), alloop},
    {KEY(BANK33, ALLOOP1), alloop1},
    {KEY(BANK33, SPECSTS), specsts},
    {KEY(BANK33, SPECSTS + 4u), specsts_after_findvac},
    {KEY(BANK33, ALFLT), alflt},
    {KEY(BANK33, ALFLT + 1u), alflt_after_storedta},
    {KEY(BANK33, ALFLT + 3u), alflt_after_phaschng},
    {KEY(BANK33, ALFLT1), alflt1},
    {KEY(BANK33, ALFLT1 + 1u), alflt1_after_loadstdt},
    {KEY(BANK33, ALFLT1 + 3u), alflt1_calibration},
    {KEY(BANK33, ALFLT1 + 4u), alflt1_compass},
    {KEY(BANK33, ALFLT1 + 5u), alflt1_after_chkcomed},
    {KEY(BANK33, NORMLOP), normlop},
    /* The drift test's end, pp. 441-442 (gyrocompass.c) */
    {KEY(BANK33, PERFERAS + 1u), perferas_after_exit},
    {KEY(BANK33, ONCEMORE), oncemore},
    {KEY(BANK33, ONCEMORE + 2u), oncemore_torqndx},
    {KEY(BANK33, ONCEMORE + 5u), oncemore_cdux},
    {KEY(BANK33, TORQINCH), torqinch},
    {KEY(BANK33, TORQINCH + 2u), torqinch_after_phaschng},
    {KEY(BANK33, TORQINCH + 5u), torqinch_after_imupulse},
    {KEY(BANK33, TORQINCH + 6u), torqinch_after_imustllg},
    {KEY(BANK33, TORQINCH + 010u), torqinch_erate},
    {KEY(BANK33, TORQINCH + 014u), torqinch_after_exit},
    /* SOMEERRR and SOMERR2, p. 442 (drift_loop.c) */
    {KEY(BANK33, SOMEERRR), someerrr},
    {KEY(BANK33, SOMEERRR + 2u), someerrr_after_alarm},
    {KEY(BANK33, SOMERR2), somerr2},
    {KEY(BANK33, SOMERR2 + 2u), somerr2_after_alarm},
    {KEY(BANK33, SOMERR2 + 4u), somerr2_after_phaschng},
    {KEY(BANK33, SOMERR2 + 5u), data_word}, /* DEC585 */
    /* The gyrocompassing, pp. 442-446 (gyrocompass.c) */
    {KEY(BANK33, GTSCPSS), gtscpss},
    {KEY(BANK33, SETUPGC + 4u), setupgc_after_zeroing},
    {KEY(BANK33, SETUPGC + 5u), setupgc_after_posn17c},
    {KEY(BANK33, POSN17C), posn17c},
    {KEY(BANK33, POSN17C + 017u), posn17c_after_exit},
    {KEY(BANK33, ENDGTSAL), endgtsal},
    {KEY(BANK33, ENDGTSAL + 2u), endgtsal_chkcomed},
    {KEY(BANK33, ENDGTSAL + 3u), endgtsal_after_chkcomed},
    {KEY(BANK33, RESTAIER), restaier},
    {KEY(BANK33, RESTAIER + 2u), restaier_after_phaschng},
    {KEY(BANK33, RESTAIER + 014u), restaier_after_exit},
    {KEY(BANK33, RESTAIER + 015u), restaier_after_earthr},
    {KEY(BANK33, RESTEST1), restest1},
    {KEY(BANK33, RESTEST1 + 5u), restest1_after_exit},
    {KEY(BANK33, RESTEST1 + 7u), restest1_phaschng},
    {KEY(BANK33, RESTEST3), restest3},
    {KEY(BANK33, RESTEST3 + 020u), restest3_after_exit},
    {KEY(BANK33, RESTEST3 + 021u), restest3_after_posn17c},
    {KEY(BANK33, RESCHNG), reschng},
    {KEY(BANK33, SPITGYRO), spitgyro},
    {KEY(BANK33, SPITGYRO + 3u), spitgyro_after_imupulse},
    {KEY(BANK33, SPITGYRO + 5u), spitgyro_bad_stall},
    {KEY(BANK33, SPITGYRO + 6u), spitgyro_after_imustall},
    {KEY(BANK33, NOAZCHGE), data_word},
    {KEY(BANK33, NOAZCHGE + 1u), noazchge_after_exit},
    /* CHKCOMED, pp. 446-447 (drift_loop.c) */
    {KEY(BANK33, CHKCOMED), chkcomed},
    {KEY(BANK33, PRELTERM + 2u), prelterm_after_priochng},
    /* The optical verification, pp. 449-454 (optical_verification.c) */
    {KEY(BANK33, GCOMPVER), gcompver},
    {KEY(BANK33, GCOMPVER + 2u), gcompver_after_phaschng},
    {KEY(BANK33, SETNBPOS), setnbpos},
    {KEY(BANK33, SETNBPOS + 1u), setnbpos_after_nbpospl},
    {KEY(BANK33, OPTDATA), optdata},
    {KEY(BANK33, OPTDATA + 013u), optdata_after_godspret},
    {KEY(BANK33, OPTDATA + 016u), optdata_terminate},
    {KEY(BANK33, OPTDATA + 017u), optdata_proceed},
    {KEY(BANK33, OPTDATA + 020u), optdata_enter},
    {KEY(BANK33, OPTDATA + 021u), data_word},
    {KEY(BANK33, TAR_EREF - 2u), data_word},
    {KEY(BANK33, TAR_EREF - 1u), tar_eref_minus1},
    {KEY(BANK33, CONTIN33 + 4u), contin33_after_targdrve},
    {KEY(BANK33, NEXTBNKS + 014u), nextbnks_after_exit},
    {KEY(BANK33, NEXBNKSS), nexbnkss},
    {KEY(BANK33, NEXBNKSS + 4u), nexbnkss_after_targdrve},
    {KEY(BANK33, GCOMP4), gcomp4},
    {KEY(BANK33, GCOMP4 + 3u), gcomp4_terminate},
    {KEY(BANK33, GCOMP4 + 4u), gcomp4_proceed},
    {KEY(BANK33, GCOMP4 + 5u), gcomp4_enter},
    {KEY(BANK33, GCOMP5), gcomp5},
    {KEY(BANK33, GCOMP5 + 2u), gcomp5_after_mkreleas},
    {KEY(BANK33, GCOMP5 + 4u), gcomp5_after_downflag},
    {KEY(BANK33, GCOMP5 + 6u), gcomp5_after_newmodex},
    {KEY(BANK33, GCOMP5 + 010u), gcomp5_after_phaschng},
    {KEY(BANK33, GTSOPTCS + 2u), gtsoptcs_after_alarm},
    {KEY(BANK33, TARGDRVE), targdrve},
    {KEY(BANK33, TARGDRVE + 014u), targdrve_after_exit},
    {KEY(BANK33, RETARG + 5u), retarg_after_sxtmark},
    {KEY(BANK33, RETARG + 7u), retarg_bad_stall},
    {KEY(BANK33, RETARG + 010u), retarg_after_optstall},
    {KEY(BANK33, RETARG + 014u), retarg_mark},
    {KEY(BANK33, AZMTHCG1), azmthcg1},
    {KEY(BANK33, AZMTHCG1 + 5u), azmthcg1_after_exit},
    {KEY(BANK33, AZMTHCG1 + 011u), azmthcg1_flash},
    {KEY(BANK33, AZMTHCG1 + 014u), azmthcg1_terminate},
    {KEY(BANK33, AZMTHCG1 + 015u), azmthcg1_proceed},
    {KEY(BANK33, AZMTHCG1 + 016u), azmthcg1_enter},
    {KEY(BANK33, AZMTHCG1 + 025u), azmthcg1_after_load},
    {KEY(BANK33, AZMTHCG1 + 031u), azmthcg1_after_phaschng},
};

static const struct point *point_at(uint16_t at)
{
    unsigned i;

    for (i = 0; i < COUNT(points); ++i)
        if (points[i].at == at) return &points[i];
    return NULL;
}

/* The physical word Z names, or 0 for an erasable Z. */
static uint16_t z_physical(void)
{
    return Z < 02000u ? 0u : (uint16_t)fixed_physical(Z);
}

/* ---- The run ---- */

enum step { GO_ON, LEFT, IDLE };

static enum step step(bool fixed_fixed_key)
{
    const struct point *point = point_at(z_physical());

    if (!point) return LEFT;
    /* (From FINETIME's keys, under any EBANK: the faithful port reads the
     * file's switched words in EBANK 5 whatever EBANK holds, which the
     * readable code does not model.) */
    if (fixed_fixed_key && EBANK != 02400u && point->at / 1024u == BANK33) not_modelled();
    point->routine();
    if (Z == FINETIME + 1u || z_physical() == KEY(BANK33, CHECKG1)) return IDLE;
    return GO_ON;
}

/* (A separate function, so that nothing setjmp returns through is a
 * local of run().) */
static enum exec_status steps(bool fixed_fixed_key)
{
    unsigned n;

    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    for (n = 0; n < RUN_LIMIT; ++n) {
        switch (step(fixed_fixed_key)) {
        case GO_ON: continue;
        case LEFT: return EXEC_OK;
        case IDLE: return EXEC_IDLE;
        }
    }
    return EXEC_OUT_OF_SCOPE;
}

/* ---- The order pairs ---- */

static const struct interpretive_code *const codes[] = {
    /* Part 3: pp. 423-430 */
    &GUESS_CODE,    &AXES_CODE,     &CALCGA_CODE, &AINGOTN_CODE,
    &OVERFFIX_CODE, &ERTHRVSE_CODE, &EARTHR_CODE, &EARTHR_STAR_CODE,
};

/* The table an address of a bank is in, or none. */
static const struct interpretive_code *code_at(unsigned bank, word address)
{
    unsigned i;

    for (i = 0; i < COUNT(codes); ++i)
        if (codes[i]->bank == bank && address >= codes[i]->first &&
            address < codes[i]->first + codes[i]->count)
            return codes[i];
    return NULL;
}

/* The states the faithful order pairs take (its sane()): the basic
 * routines' in the key's bank, and NEWJOB +0 or a core set, FIXLOC one of
 * the VAC areas. */
static bool pair_sane(word fb)
{
    if (agc.memory[ADDRESS_OF(NEWJOB)] > 037777u) return false;
    if (FIXLOC < 0401u || FIXLOC > 0661u || (FIXLOC - 0401u) % 054u) return false;
    return sane(fb, false, false);
}

/* One pass; after RTB LOADTIME (ERTHRVSE's and EARTHR's), the routine
 * (src/rtb_op_codes/) and the pass on from its DANZIG, as the faithful
 * pair runs them. The other RTBs (SGNAGREE, PULSEIMU) end the pass. */
static enum exec_status one_pass(const agc_native_program *program, apollo_overlay_state *state,
                                 cm_channel_io *io, const struct interpretive_code *code)
{
    if (interpret_listing(program, state, &io->io, code) != EXEC_OK) return EXEC_OUT_OF_SCOPE;
    bind(program, state, io);
    if (Z != LOADTIME || FBANK != (I_LOADTIME & 076000u)) return EXEC_OK;
    if (setjmp(back)) return EXEC_OUT_OF_SCOPE;
    loadtime();
    return interpret_listing(program, state, &io->io, code);
}

/* ---- The keys, in the faithful tables' order ---- */

/* The faithful order pairs, renamed. */
extern const agc_routine_table faithful_cm_imu_calibration_interpretive;

/* Each basic key's routine. */
static enum exec_status basic(uint16_t key, const agc_native_program *program,
                              apollo_overlay_state *state, void *device)
{
    static const agc_native_program none;
    const struct point *point = point_at(key);
    bool fixed_fixed = key / 1024u == 02u;
    cm_channel_io *io = device, kept;
    apollo_overlay_state before;
    enum exec_status status;

    if (!point || !state || !io) return EXEC_OUT_OF_SCOPE;
    bind(program ? program : &none, state, io);
    if (!sane((word)((key / 1024u) << 10), fixed_fixed,
              fixed_fixed || key == KEY(BANK33, PERFERAS + 1u)))
        return EXEC_OUT_OF_SCOPE;
    before = *state;
    kept = *io;
    status = steps(fixed_fixed);
    if (status == EXEC_OK || status == EXEC_IDLE) return status;
    *state = before;
    *io = kept;
    return EXEC_OUT_OF_SCOPE;
}

/* Each order pair's routine: the readable one, or the faithful port's
 * until a part writes it. */
static enum exec_status pair(unsigned index, uint16_t key, const agc_native_program *program,
                             apollo_overlay_state *state, void *device)
{
    static const agc_native_program none;
    unsigned bank = key / 1024u;
    cm_channel_io *io = device, kept;
    apollo_overlay_state before;
    const struct interpretive_code *code;
    enum exec_status status;

    if (!code_at(bank, (word)(02000u + key % 1024u)))
        return faithful_cm_imu_calibration_interpretive.entries[index].call(program, state, device);
    if (!state || !io) return EXEC_OUT_OF_SCOPE;
    if (!program) program = &none;
    bind(program, state, io);
    code = code_at(bank, LOC);
    if (Z != NEWOPS || FBANK != (word)(bank << 10) || !code ||
        LOC != (word)(02000u + key % 1024u) || BANKSET != BBANK || INTBIT15 != (FBANK & 040000u) ||
        !pair_sane(FBANK))
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
bool cm_imu_cal_readable(uint16_t key, bool order_pair)
{
    if (order_pair) return code_at(key / 1024u, (word)(02000u + key % 1024u)) != NULL;
    return point_at(key) != NULL;
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
        return pair(i, KEY(bank, address), program, state, device);                                \
    }
#include "cm_imu_cal_keys.inc"
#undef BASIC
#undef PAIR

#define BASIC(i, bank, address)
#define PAIR(i, bank, address) {KEY(bank, address), pair_##i},
static const agc_routine_entry pair_keys[] = {
#include "cm_imu_cal_keys.inc"
};
#undef PAIR
#undef BASIC

#define BASIC(i, bank, address) {KEY(bank, address), basic_##i},
#define PAIR(i, bank, address)
static const agc_routine_entry basic_keys[] = {
#include "cm_imu_cal_keys.inc"
};
#undef PAIR
#undef BASIC

const agc_routine_table cm_imu_calibration_interpretive = {pair_keys, COUNT(pair_keys)};
const agc_routine_table cm_imu_calibration_basic = {basic_keys, COUNT(basic_keys)};

#endif
