/* SPDX-License-Identifier: MIT */
/* The readable extended verbs (src/extended_verbs/) as routines of the
 * flight program. The faithful port keeps each part of EXTENDED_VERBS in
 * a module of its own (c64-port/src/lanec_ev_<part>.c), which its routine
 * tables (lanec_ev_routines.c) call for every key of the part, with the
 * state and a record of the channels (lanec_io, c64-port/include/lanec.h)
 * and the mission. The readable parts take the module's place under its
 * name: part 1, the front end, is lanec_ev_front_dispatch; part 2, the
 * short verbs, lanec_ev_short_dispatch; part 3, alignment and radar
 * designate, lanec_ev_align_dispatch; part 4, the LM's radar test verbs,
 * lanec_ev_radar_dispatch; part 5, the DAP data load, lanec_ev_dap_dispatch.
 *
 * Plumbing only: point the machine at the state and the channel record,
 * and take the states the faithful module takes: the mission this object
 * is built for, a bank the part has entries in selected (bank 43, FBANK's
 * bank 33 with the superbank bit; for the LM's part 3 also bank 23; for
 * part 5 the LM's banks 20 and 01, the CM's 43 and 42), the bank registers
 * agreeing, Z a 12-bit address and ZERO +0. A step the readable code does not model
 * leaves the state as it was.
 *
 * The front end's module runs the one routine at Z. The others go on from
 * entry to entry while Z is one of their own in the bank they started in,
 * and so do their readable parts: a TC from one short verb to another
 * (LEMVEC's TC NOUPDOWN, the CM's TC CHKPOOH), EXDAPOFF's TC Q back to its
 * caller or VBZERO's TC OP/INERT runs on. */
#include <setjmp.h>
#include <stddef.h>
#include "lanec_ev_align.h"
#include "lanec_ev_dap.h"
#include "lanec_ev_front.h"
#include "lanec_ev_radar.h"
#include "lanec_ev_short.h"
#include "../extended_verbs/extended_verbs.h"
#include "../agc/names.h"

#if LUMINARY099
#define THIS_MISSION LANEC_LM
#else
#define THIS_MISSION LANEC_CM
#endif

/* The superbank bit (channel 7's bit 7): FBANK's banks 30-33 are then
 * banks 40-43. */
#define SUPERBANK_BIT 0100u

static jmp_buf back;

static void leave(void)
{
    longjmp(back, 1);
}

static lanec_io *channels;

/* The extended verbs written so far read channels 12, 30 and 31 (V43's
 * checks) and 33 (the radar test's) and write channel 12 (V43's error
 * counter enable, the RR auto tracker enable), and read no fixed word
 * (GOLOADLV's WRITE SUPERBNK is channel 7, agc.superbank). */
static word channel_in(unsigned number)
{
    if (number != 012u && number != 030u && number != 031u && number != 033u) not_modelled();
    return channels->channel[number];
}

static void channel_out(unsigned number, word value)
{
    if (number != 012u) not_modelled();
    channels->channel[number] = value;
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
    channels = io;
    agc.superbank = &io->superbank;
    agc.interrupts = &io->interrupt_enabled;
    agc.dsalmout = &io->channel[011];
    agc.in_interrupt = NULL;
    agc.channel_in = channel_in;
    agc.channel_out = channel_out;
    agc.fixed_memory = fixed_memory;
    agc.leave = leave;
}

/* The faithful module's states (lanec_registers_valid). */
static bool as_expected(void)
{
    return (EBANK & ~03400u) == 0u && (FBANK & ~076000u) == 0u &&
           BBANK == (word)(FBANK | (EBANK >> 8)) && Z <= 07777u && agc.memory[ZERO_ADDRESS] == 0u;
}

/* The fixed bank selected: FBANK's, or with the superbank bit 40-43 for
 * 30-33. */
static word bank_selected(void)
{
    word bank = (word)((FBANK >> 10) & 037u);
    return bank >= 030u && (*agc.superbank & SUPERBANK_BIT) ? (word)(bank + 010u) : bank;
}

struct entry {
    word at;
    void (*routine)(void);
};

/* A part's entries in one fixed bank (043 for bank 43). */
struct bank {
    word number;
    const struct entry *list;
    size_t count;
};
#define BANK(number, table) {number, table, sizeof table / sizeof table[0]}
#define BANKS(banks)        banks, sizeof banks / sizeof banks[0]

/* The entry at Z, or NULL. */
static const struct entry *entry_at_z(const struct entry *entries, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i)
        if (entries[i].at == Z) return &entries[i];
    return NULL;
}

/* The part's entries in the bank selected, or NULL. */
static const struct bank *in_this_bank(const struct bank *banks, size_t count)
{
    size_t i;

    for (i = 0; i < count; ++i)
        if (banks[i].number == bank_selected()) return &banks[i];
    return NULL;
}

/* The part's entries in the bank selected, for the run under way (a
 * static, not a local, for the longjmp out of it). */
static const struct bank *selected;

/* Run entry's routine; with run_on, then the entry at Z after it in the
 * bank selected, while there is one. */
static void go_on(const struct entry *entry, bool run_on)
{
    do entry->routine();
    while (run_on && (entry = entry_at_z(selected->list, selected->count)) != NULL);
}

/* Run the routine at Z, if it is one of the part's entries in the bank
 * selected; with run_on, then the entry at Z after it in the same bank,
 * while there is one. */
static enum lanec_status run(const struct bank *banks, size_t count, bool run_on,
                             apollo_overlay_state *state, lanec_io *io)
{
    apollo_overlay_state before;
    lanec_io io_before;
    const struct entry *entry;

    bind_lanec(state, io);
    if (!as_expected()) return LANEC_OUT_OF_SCOPE;
    selected = in_this_bank(banks, count);
    entry = selected ? entry_at_z(selected->list, selected->count) : NULL;
    if (!entry) return LANEC_OUT_OF_SCOPE;
    before = *state;
    io_before = *io;
    if (setjmp(back)) {
        *state = before;
        *io = io_before;
        return LANEC_OUT_OF_SCOPE;
    }
    go_on(entry, run_on);
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
    static const struct bank banks[] = {BANK(043u, front)};

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(BANKS(banks), false, state, io);
}

/* ---- Part 2: the short verbs ---- */

static const struct entry short_verbs[] = {
#if LUMINARY099
    {DAPATTER, dapatter},
    {DAPATTER + 2u, dapatter_after_downflag},
    {TOTATTER, totatter},
    {TOTATTER + 2u, totatter_after_upflag},
    {LRON, lron},
    {LRON + 2u, lron_after_upflag},
    {LROFF, lroff},
    {LROFF + 2u, lroff_after_downflag},
    {GOLOADLV, goloadlv},
    {GOLOADLV + 1u, goloadlv_after_flashoff},
    {V47TXACT, v47txact},
    {V47TXACT + 1u, v47txact_after_testxact},
    {V47TXACT + 5u, v47txact_after_findvac},
    {CKMODCAD, ckmodcad},
    {ALINTIME, alintime},
    {ALINTIME + 1u, alintime_after_testxact},
    {VB64, vb64},
    {VB64 + 1u, vb64_after_chkpooh},
    {VB64 + 2u, vb64_after_testxact},
    {VB64 + 6u, vb64_after_findvac},
    {V82PERF, v82perf},
    {V82PERF + 1u, v82perf_after_testxact},
    {V82PERF + 3u, v82perf_after_priochng},
    {V83PERF, v83perf},
    {V83PERF + 1u, v83perf_after_testxact},
    {V83PERF + 5u, v83perf_after_waitlist},
    {V89PERF, v89perf},
    {V89PERF + 1u, v89perf_after_chkpooh},
    {V89PERF + 2u, v89perf_after_testxact},
    {V89PERF + 6u, v89perf_after_findvac},
    {V90PERF, v90perf},
    {V90PERF + 1u, v90perf_after_testxact},
    {V90PERF + 5u, v90perf_after_findvac},
    {MINIMP, minimp},
    {NOMINIMP, nominimp},
    {NOMINIMP + 6u, nominimp_after_ibnkcall},
    {CREWMANU, crewmanu},
    {CREWMANU + 1u, crewmanu_after_chkpooh},
    {CREWMANU + 2u, crewmanu_after_testxact},
    {CREWMANU + 6u, crewmanu_after_findvac},
    {TRMTRACK, trmtrack},
    {TRMTRACK + 6u, trmtrack_after_downflag},
    {TRMTRACK + 8u, trmtrack_after_second_downflag},
    {TRMTRACK + 10u, trmtrack_after_third_downflag},
    {DNEDUMP, dnedump},
    {LEMVEC, lemvec},
    {LEMVEC + 2u, lemvec_after_downflag},
    {CSMVEC, csmvec},
    {NOUPDOWN, noupdown},
    {NOUPDOWN + 2u, noupdown_after_downflag},
    {UPDATOFF, updatoff},
    {UPDATOFF + 2u, updatoff_after_upflag},
    {SYSTEST, systest},
    {SYSTEST + 1u, systest_after_chkpooh},
    {SYSTEST + 2u, systest_after_testxact},
    {SYSTEST + 6u, systest_after_findvac},
    {WMATRXNG, wmatrxng},
    {SHOWSUM, showsum},
    {SHOWSUM + 1u, showsum_after_chkpooh},
    {SHOWSUM + 2u, showsum_after_testxact},
    {SHOWSUM + 4u, showsum_after_priochng},
    {SDISPLAY, sdisplay},
    {NOKILL, nokill},
    {NOKILL + 5u, nokill_after_terminate},
    {NOKILL + 6u, nokill_after_proceed},
    {NOKILL + 7u, nokill_after_enter},
    {NOKILL + 8u, nokill_plus8},
    {ENDSUMS, endsums},
    {DAPDISP, dapdisp},
    {DAPDISP + 1u, dapdisp_after_testxact},
    {DAPDISP + 3u, dapdisp_after_priochng},
    {ATTACHED, attached},
    {ATTACHED + 4u, attached_after_findvac},
    {VERB96, verb96},
    {VERB96 + 2u, verb96_after_upflag},
    {V67, v67},
    {V67 + 1u, v67_after_testxact},
    {V67 + 5u, v67_after_findvac},
    {SNUFFOUT, snuffout},
    {SNUFFOUT + 2u, snuffout_after_upflag},
    {OUTSNUFF, outsnuff},
    {OUTSNUFF + 2u, outsnuff_after_downflag},
    {VERB85, verb85},
    {VERB85 + 1u, verb85_after_testxact},
#else
    {CKMODCAD, ckmodcad},
    {CKMODCAD + 1u, ckmodcad_plus1},
    {GOLOADLV, goloadlv},
    {GOLOADLV + 1u, goloadlv_after_flashoff},
    {V60, v60},
    {V61, v61},
    {V61 + 2u, v61_after_downflag},
    {V62, v62},
    {V62 + 2u, v62_after_upflag},
    {V62 + 4u, v62_after_second_upflag},
    {V63, v63},
    {V63 + 2u, v63_after_upflag},
    {V63 + 4u, v63_after_downflag},
    {SYSTEST, systest},
    {SYSTEST + 1u, systest_after_chkpooh},
    {V92CONT, v92cont},
    {V92CONT + 1u, v92cont_after_exdapoff},
    {V92CONT + 5u, v92cont_after_findvac},
    {CKOPTVB, ckoptvb},
    {CKOPTVB + 2u, ckoptvb_after_checkmm02_no},
    {CKOPTVB + 3u, ckoptvb_after_checkmm02_yes},
    {CKOPTVB + 8u, ckoptvb_after_findvac},
    {CHAZFOGC, chazfogc},
    {CHAZFOGC + 2u, chazfogc_after_checkmm02_no},
    {CHAZFOGC + 3u, chazfogc_after_checkmm02_yes},
    {CHAZFOGC + 7u, chazfogc_after_findvac},
    {CHAZFOGC + 9u, chazfogc_after_phaschng},
    {VB64, vb64},
    {VB64 + 1u, vb64_after_chkpooh},
    {VB64 + 2u, vb64_after_testxact},
    {VB64 + 7u, vb64_after_findvac},
    {ENATMA, enatma},
    {ENATMA + 2u, enatma_after_downflag},
    {STROKON, strokon},
    {STROKON + 8u, strokon_after_novac},
    {STABLISH, stablish},
    {STABLISH + 12u, stablish_plus12},
    {STABLISH + 13u, stablish_plus13},
    {CREWMANU, crewmanu},
    {CREWMANU + 1u, crewmanu_after_chkpooh},
    {CREWMANU + 2u, crewmanu_after_testxact},
    {CREWMANU + 6u, crewmanu_after_findvac},
    {V82PERF, v82perf},
    {V82PERF + 1u, v82perf_after_testxact},
    {V82PERF + 3u, v82perf_after_priochng},
    {V83PERF, v83perf},
    {V83PERF + 1u, v83perf_after_testxact},
    {V83PERF + 5u, v83perf_plus5},
    {V83PERF + 9u, v83perf_after_novac},
    {V85PERF, v85perf},
    {V85PERF + 1u, v85perf_after_testxact},
    {GOTOR21, gotor21},
    {GOTOR21 + 2u, gotor21_after_downflag},
    {GOTOR23, gotor23},
    {GOTOR23 + 2u, gotor23_after_upflag},
    {GOTOR23 + 3u, gotor23_after_testxact},
    {GOTOR23 + 19u, gotor23_after_novac},
    {REGR21, regr21},
    {REGR21 + 4u, regr21_after_novac},
    {R22ALARM, r22alarm},
    {R22ALARM + 2u, r22alarm_after_alarm},
    {V86PERF, v86perf},
    {LEMVEC, lemvec},
    {LEMVEC + 2u, lemvec_after_downflag},
    {CSMVEC, csmvec},
    {CSMVEC + 2u, csmvec_after_upflag},
    {DNEDUMP, dnedump},
    {LFTFLGON, lftflgon},
    {LFTFLGON + 2u, lftflgon_after_upflag},
    {CHKPOOH, chkpooh},
    {EXDAPOFF, exdapoff},
    {V89PERF, v89perf},
    {V89PERF + 1u, v89perf_after_chkpooh},
    {V89PERF + 2u, v89perf_after_testxact},
    {V89PERF + 7u, v89perf_after_findvac},
    {WMATRXNG, wmatrxng},
    {WMATRXNG + 2u, wmatrxng_after_downflag},
    {WMATRXNG + 4u, wmatrxng_after_second_downflag},
    {SHOWSUM, showsum},
    {SHOWSUM + 1u, showsum_after_chkpooh},
    {SHOWSUM + 2u, showsum_after_testxact},
    {SDISPLAY, sdisplay},
    {NOKILL, nokill},
    {NOKILL + 5u, nokill_after_terminate},
    {NOKILL + 6u, nokill_after_proceed},
    {NOKILL + 7u, nokill_after_enter},
    {NOKILL + 8u, nokill_plus8},
    {ENDSUMS, endsums},
    {SETPRFLG, setprflg},
    {SETPRFLG + 2u, setprflg_after_upflag},
    {RESETPRF, resetprf},
    {RESETPRF + 2u, resetprf_after_downflag},
    {ATTACHED, attached},
    {ATTACHED + 4u, attached_after_findvac},
    {VERB94, verb94},
    {VERB94 + 6u, verb94_after_downflag},
    {VERB94 + 8u, verb94_after_checkmm23_no},
    {VERB94 + 9u, verb94_after_checkmm23_yes},
    {VERB94 + 11u, verb94_after_phaschng},
    {V90PERF, v90perf},
    {V90PERF + 1u, v90perf_after_testxact},
    {V90PERF + 5u, v90perf_after_findvac},
    {VERB96, verb96},
    {VERB96 + 2u, verb96_after_upflag},
    {VERB96 + 4u, verb96_after_second_upflag},
    {V52, v52},
    {V52 + 2u, v52_after_checkmm22_no},
    {V52 + 3u, v52_after_checkmm22_yes},
    {V67, v67},
    {V67 + 1u, v67_after_testxact},
    {V67 + 5u, v67_after_findvac},
    {SETSURF, setsurf},
    {SETSURF + 2u, setsurf_after_upflag},
    {RESTSRF, restsrf},
    {RESTSRF + 2u, restsrf_after_downflag},
#endif
};

enum lanec_status lanec_ev_short_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
    static const struct bank banks[] = {BANK(043u, short_verbs)};

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(BANKS(banks), true, state, io);
}

/* ---- Part 3: alignment and radar designate ---- */

static const struct entry align[] = {
    {VBZERO, vbzero},
    {VBZERO + 1u, vbzero_after_op_inert_imu},
    {VBCOARK, vbcoark},
    {VBCOARK + 1u, vbcoark_after_op_inert_imu},
    {OP_INERT, op_inert},
    {IMUZEROK, imuzerok},
    {IMUZEROK + 1u, imuzerok_after_ckmodcad},
    {IMUZEROK + 3u, imuzerok_after_imuzero},
    {IMUZEROK + 5u, imuzerok_after_imustall_bad},
    {IMUZEROK + 6u, imuzerok_after_imustall_good},
    {IMUCOARK, imucoark},
    {IMUCOARK + 1u, imucoark_after_ckmodcad},
    {IMUCOARK + 2u, imucoark_after_testxact},
    {IMUCOARK + 5u, imucoark_after_terminate},
    {IMUCOARK + 6u, imucoark_after_proceed},
    {ICORK2, icork2},
    {ICORK2 + 3u, icork2_after_exdspret},
    {ICORK2 + 5u, icork2_after_imucoars},
    {ICORK2 + 7u, icork2_after_imustall_bad},
    {ICORK2 + 8u, icork2_after_imustall_good},
    {IMUFINEK, imufinek},
    {IMUFINEK + 1u, imufinek_after_ckmodcad},
    {IMUFINEK + 2u, imufinek_after_testxact},
    {IMUFINEK + 5u, imufinek_after_terminate},
    {IMUFINEK + 6u, imufinek_after_proceed},
    {IMUFINEK + 7u, imufinek_after_enter},
    {IMUFINEK + 10u, imufinek_after_exdspret},
    {IMUFINEK + 12u, imufinek_after_imufine},
    {IMUFINEK + 14u, imufinek_after_imustall_bad},
    {FINEK2, finek2},
    {FINEK2 + 3u, finek2_after_imupulse},
    {FINEK2 + 5u, finek2_after_imustall_bad},
    {FINEK2 + 6u, finek2_after_imustall_good},
    {IMUATTCK, imuattck},
    {IMUATTCK + 1u, imuattck_after_chkpooh},
#if LUMINARY099
    {VBZERO + 2u, vbzero_after_op_inert_rr},
    {VBCOARK + 2u, vbcoark_after_op_inert_rr},
    {RRZEROK, rrzerok},
    {RRZEROK + 1u, rrzerok_after_rdruseck},
    {RWAITK, rwaitk},
    {RWAITK + 2u, rwaitk_after_radstall_bad},
    {RWAITK + 3u, rwaitk_after_radstall_good},
    {LRPOS2K, lrpos2k},
    {LRPOS2K + 1u, lrpos2k_after_rdruseck},
    {LRPOS2K + 3u, lrpos2k_after_lrpos2},
    {LRPOS2K + 5u, lrpos2k_after_radstall_bad},
    {LRPOS2K + 6u, lrpos2k_after_radstall_good},
    {LRP2ALM, lrp2alm},
    {LRP2ALM + 2u, lrp2alm_after_alarm},
    {RRDESNBK, rrdesnbk},
    {RRDESNBK + 1u, rrdesnbk_after_rdruseck},
    {RRDESNBK + 2u, rrdesnbk_after_testxact},
    {RRDESNBK + 10u, rrdesnbk_plus10},
    {RRDESNBK + 13u, rrdesnbk_after_terminate},
    {RRDESNBK + 14u, rrdesnbk_after_proceed},
    {RRDESNBK + 15u, rrdesnbk_after_enter},
    {RRDESNBK + 17u, rrdesnbk_after_aurlokon},
    {RRDESNBK + 20u, rrdesnbk_after_exdspret},
    {RRDESNBK + 24u, rrdesnbk_after_findvac},
    {RRDESK2, rrdesk2},
    {RRDESK2 + 2u, rrdesk2_after_rrdesnb},
    {RRDESK2 + 3u, rrdesk2_plus3},
    {RRDESK2 + 12u, rrdesk2_after_radstall_bad},
    {RRDESK2 + 13u, rrdesk2_after_radstall_good},
    {RRDESK2 + 14u, rrdesk2_plus14},
    {RRDESK2 + 16u, rrdesk2_after_alarm},
    {RRDESEND, rrdesend},
    {RRDESEND + 9u, rrdesend_after_clradmod},
    {RRDESEND + 12u, rrdesend_after_delayjob},
    {RRDESEND + 14u, rrdesend_after_downflag},
    {RDRUSECK, rdruseck},
    {CHECKRR, checkrr},
    {CKRNDBIT, ckrndbit},
    {CHECKP22, checkp22},
    {IMUATTCK + 13u, imuattck_plus13},
    {IMUATTCK + 14u, imuattck_after_testxact},
    {IMUATTCK + 17u, imuattck_after_terminate},
    {IMUATTCK + 18u, imuattck_after_proceed},
    {IMUATTCK + 19u, imuattck_after_enter},
    {IMUATTCK + 22u, imuattck_after_exdspret},
    {IMUATTCK + 29u, imuattck_after_waitlist},
#else
    {VBZERO + 2u, vbzero_after_op_inert_optics},
    {VBCOARK + 2u, vbcoark_after_op_inert_optics},
    {XACT0Q, xact0q},
    {OPTCOARK, optcoark},
    {OPTCOARK + 2u, optcoark_after_ckmodcad},
    {OPTCOARK + 3u, optcoark_after_testxact},
    {OPTCOARK + 9u, optcoark_after_falton},
    {OPTCOARK + 11u, optcoark_plus11},
    {OPTCOARK + 17u, optcoark_after_alarm},
    {OPTC1, optc1},
    {OPTC1 + 3u, optc1_after_terminate},
    {OPTC1 + 4u, optc1_after_proceed},
    {OPTC1 + 5u, optc1_after_enter},
    {OPTC1 + 12u, optc1_after_exdspret},
    {OPTC1 + 15u, optc1_plus15},
    {IMUATTCK + 7u, imuattck_after_cklftbts_yes},
    {IMUATTCK + 8u, imuattck_after_testxact},
    {IMUATTCK + 14u, imuattck_after_terminate},
    {IMUATTCK + 15u, imuattck_after_proceed},
    {IMUATTCK + 16u, imuattck_after_enter},
    {IMUATTCK + 20u, imuattck_after_needle11},
    {IMUATTCK + 22u, imuattck_after_needler2},
    {TRMATTCK, trmattck},
    {TRMATTCK + 1u, trmattck_after_cklftbts_yes},
    {TRMATTCK + 2u, trmattck_after_cklftbts_no},
    {ATTCK1, attck1},
    {ATTCK1 + 7u, attck1_after_needles},
    {CKLFTBTS, cklftbts},
#endif
};

#if LUMINARY099
/* AURLOKON, NOLOKON and AURLKON1, in bank 23. */
static const struct entry align_in_bank_23[] = {
    {AURLOKON, aurlokon},
    {AURLOKON + 1u, aurlokon_after_makecadr},
    {AURLOKON + 6u, aurlokon_plus6},
    {AURLOKON + 9u, aurlokon_after_terminate},
    {AURLOKON + 10u, aurlokon_after_proceed},
    {AURLOKON + 11u, aurlokon_after_recycle},
    {AURLOKON + 12u, aurlokon_immediate_return},
    {AURLOKON + 14u, aurlokon_after_blanket},
    {AURLOKON + 15u, aurlokon_plus15},
    {AURLOKON + 21u, aurlokon_after_upflag},
    {NOLOKON, nolokon},
    {NOLOKON + 2u, nolokon_after_downflag},
    {NOLOKON + 4u, nolokon_after_upflag},
    {AURLKON1, aurlkon1},
};
#endif

enum lanec_status lanec_ev_align_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
    static const struct bank banks[] = {
        BANK(043u, align),
#if LUMINARY099
        BANK(023u, align_in_bank_23),
#endif
    };

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(BANKS(banks), true, state, io);
}

/* ---- Part 4: the LM's radar test verbs ---- */

#if LUMINARY099
static const struct entry radar[] = {
    {R77, r77},
    {R77 + 1u, r77_after_rdruseck},
    {R77 + 7u, r77_after_upflag},
    {R04, r04},
    {R04 + 1u, r04_after_rdruseck},
    {R04 + 2u, r04_after_testxact},
    {R04Z, r04z},
    {R04A, r04a},
    {R04A + 2u, r04a_plus2},
    {R04A + 5u, r04a_after_terminate},
    {R04A + 6u, r04a_after_proceed},
    {R04A + 7u, r04a_after_recycle},
    {R04A + 8u, r04a_immediate_return},
    {R04A + 10u, r04a_after_blanket},
    {R04A + 11u, r04a_plus11},
    {R04X - 1u, r04x_minus1},
    {R04X, r04x},
    {R04B, r04b},
    {R04B + 5u, r04b_plus5},
    {R04B + 10u, r04b_after_terminate},
    {R04B + 11u, r04b_after_proceed},
    {R04B + 12u, r04b_after_enter},
    {R04C, r04c},
    {R04C + 4u, r04c_plus4},
    {R04C + 8u, r04c_after_waitlist},
    {R04RR, r04rr},
    {R04RR + 3u, r04rr_after_terminate},
    {R04RR + 4u, r04rr_after_proceed},
    {R04RR + 5u, r04rr_after_recycle},
    {R04RR + 6u, r04rr_plus6},
    {R04RR + 9u, r04rr_plus6_after_terminate},
    {R04RR + 10u, r04rr_plus6_after_proceed},
    {R04RR + 11u, r04rr_plus6_after_recycle},
    {R04LR, r04lr},
    {R04LR + 3u, r04lr_after_terminate},
    {R04LR + 4u, r04lr_after_proceed},
    {R04LR + 5u, r04lr_after_recycle},
    {R04LR + 6u, r04lr_plus6},
    {R04LR + 9u, r04lr_plus6_after_terminate},
    {R04LR + 10u, r04lr_plus6_after_proceed},
    {R04LR + 11u, r04lr_plus6_after_recycle},
    {R04Y, r04y},
    {R04Y + 5u, r04y_after_delayjob},
    {R04K, r04k},
    {R04L, r04l},
    {R04END, r04end},
    {R04END + 5u, r04end_after_delayjob},
    {R04END + 11u, r04end_after_downflag},
    {R77END, r77end},
    {R77END + 7u, r77end_after_delayjob},
    {R77END + 9u, r77end_after_downflag},
};
#endif

/* The CM has no radar test verbs: its object refuses them as it refuses
 * the LM's mission. */
enum lanec_status lanec_ev_radar_dispatch(apollo_overlay_state *state, lanec_io *io,
                                          enum lanec_mission mission)
{
#if LUMINARY099
    static const struct bank banks[] = {BANK(043u, radar)};

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(BANKS(banks), true, state, io);
#else
    (void)state;
    (void)io;
    (void)mission;
    return LANEC_OUT_OF_SCOPE;
#endif
}

/* ---- Part 5: the DAP data load, V48 ---- */

#if LUMINARY099
/* DAPDATA1 to DPDAT1, in bank 20. */
static const struct entry dap_in_bank_20[] = {
    {DAPDATA1, dapdata1},
    {CHKDATA1, chkdata1},
    {MSKDATR1, mskdatr1},
    {MSKDATR1 + 5u, mskdatr1_after_terminate},
    {MSKDATR1 + 6u, mskdatr1_after_proceed},
    {MSKDATR1 + 7u, mskdatr1_after_enter},
    {MSKDATR1 + 8u, mskdatr1_immediate_return},
    {MSKDATR1 + 10u, mskdatr1_after_blanket},
    {FORCEONE, forceone},
    {DPDAT1, dpdat1},
};

/* STIKLOAD to TRIMDONE, in bank 01. */
static const struct entry dap_in_bank_01[] = {
    {STIKLOAD, stikload},
    {DAPDATA2, dapdata2},
    {DAPDATA2 + 3u, dapdata2_after_terminate},
    {DAPDATA2 + 4u, dapdata2_after_proceed},
    {DAPDATA2 + 5u, dapdata2_after_enter},
    {DAPDATA2 + 6u, dapdata2_immediate_return},
    {DAPDATA2 + 8u, dapdata2_after_blanket},
    {ENDR03, endr03},
    {ENDR03 + 3u, endr03_after_ibnkcall},
    {DAPDAT2, dapdat2},
    {LEMALONE, lemalone},
    {LEMALONE + 6u, lemalone_after_ibnkcall},
    {DAPDATA3, dapdata3},
    {DAPDATA3 + 4u, dapdata3_plus4},
    {DAPDATA3 + 7u, dapdata3_after_terminate},
    {DAPDATA3 + 8u, dapdata3_after_proceed},
    {DAPDATA3 + 9u, dapdata3_after_enter},
    {DAPDATA3 + 10u, dapdata3_immediate_return},
    {DAPDATA3 + 12u, dapdata3_after_blanket},
    {DPDAT3, dpdat3},
    {DPDAT3 + 5u, dpdat3_after_waitlist},
    {TRIMDONE, trimdone},
    {TRIMDONE + 3u, trimdone_after_terminate},
    {TRIMDONE + 4u, trimdone_after_proceed},
    {TRIMDONE + 5u, trimdone_after_enter},
    {TRIMDONE + 6u, trimdone_immediate_return},
    {TRIMDONE + 8u, trimdone_after_blanket},
};
#else
/* DAPDISP, in bank 43. */
static const struct entry dap_in_bank_43[] = {
    {DAPDISP, dapdisp},
    {DAPDISP + 5u, dapdisp_plus5},
    {DAPDISP + 6u, dapdisp_after_testxact},
};

/* DAPDISP1 to MAXIN, in bank 42. */
static const struct entry dap_in_bank_42[] = {
    {DAPDISP1, dapdisp1},
    {DONOUN46, donoun46},
    {DONOUN46 + 3u, donoun46_after_terminate},
    {DONOUN46 + 4u, donoun46_after_proceed},
    {DONOUN46 + 5u, donoun46_after_enter},
    {DONOUN46 + 6u, donoun46_plus6},
    {MAXOUT, maxout},
    {DONOUN47, donoun47},
    {DONOUN47 + 3u, donoun47_after_terminate},
    {DONOUN47 + 4u, donoun47_after_proceed},
    {DONOUN47 + 5u, donoun47_after_enter},
    {DONOUN47 + 6u, donoun47_plus6},
    {DONOUN47 + 17u, donoun47_after_ibnkcall},
    {DONOUN48, donoun48},
    {DONOUN48 + 3u, donoun48_after_terminate},
    {DONOUN48 + 4u, donoun48_after_proceed},
    {DONOUN48 + 5u, donoun48_after_enter},
    {MAXIN, maxin},
    {MAXIN + 2u, maxin_after_upflag},
};
#endif

enum lanec_status lanec_ev_dap_dispatch(apollo_overlay_state *state, lanec_io *io,
                                        enum lanec_mission mission)
{
    static const struct bank banks[] = {
#if LUMINARY099
        BANK(020u, dap_in_bank_20),
        BANK(001u, dap_in_bank_01),
#else
        BANK(043u, dap_in_bank_43),
        BANK(042u, dap_in_bank_42),
#endif
    };

    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(BANKS(banks), true, state, io);
}
