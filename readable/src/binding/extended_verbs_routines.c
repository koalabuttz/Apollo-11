/* SPDX-License-Identifier: MIT */
/* The readable extended verbs (src/extended_verbs/) as routines of the
 * flight program. The faithful port keeps each part of EXTENDED_VERBS in
 * a module of its own (c64-port/src/lanec_ev_<part>.c), which its routine
 * tables (lanec_ev_routines.c) call for every key of the part, with the
 * state and a record of the channels (lanec_io, c64-port/include/lanec.h)
 * and the mission. The readable parts take the module's place under its
 * name: part 1, the front end, is lanec_ev_front_dispatch; part 2, the
 * short verbs, lanec_ev_short_dispatch.
 *
 * Plumbing only: point the machine at the state and the channel record,
 * and take the states the faithful module takes: the mission this object
 * is built for, bank 43 selected (FBANK's bank 33 with the superbank bit),
 * the bank registers agreeing, Z a 12-bit address and ZERO +0. A step the
 * readable code does not model leaves the state as it was.
 *
 * The front end's module runs the one routine at Z. The short verbs'
 * goes on from entry to entry while Z is one of its own, and so does its
 * readable part: a TC from one short verb to another (LEMVEC's TC
 * NOUPDOWN, the CM's TC CHKPOOH) or EXDAPOFF's TC Q back to its caller
 * runs on. */
#include <setjmp.h>
#include <stddef.h>
#include "lanec_ev_front.h"
#include "lanec_ev_short.h"
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

/* The extended verbs written so far read no channel and no fixed word
 * (GOLOADLV's WRITE SUPERBNK is channel 7, agc.superbank). */
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

/* Run the routine at Z, if it is one of the entries; with run_on, then
 * the entry at Z after it, while there is one. */
static enum lanec_status run(const struct entry *entries, size_t count, bool run_on,
                             apollo_overlay_state *state, lanec_io *io)
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
    do entry->routine();
    while (run_on && (entry = entry_at_z(entries, count)) != NULL);
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
    return run(front, sizeof front / sizeof front[0], false, state, io);
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
    if (mission != THIS_MISSION || !state || !io) return LANEC_OUT_OF_SCOPE;
    return run(short_verbs, sizeof short_verbs / sizeof short_verbs[0], true, state, io);
}
