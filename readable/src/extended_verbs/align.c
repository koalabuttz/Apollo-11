/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 3: alignment and radar designate -- LM pp.
 * 265-281, CM pp. 239-250. The faithful C is c64-port/src/lanec_ev_align.c.
 *
 * The verbs that move the IMU's gimbals and, in the LM, the rendezvous
 * radar's:
 *   - V40, zero the CDUs (VBZERO): the IMU's with noun 20, the RR's with
 *     noun 72 (the CM has no radar: its noun 91 is an operator error);
 *   - V41, coarse align (VBCOARK): the IMU to angles the crew loads, or
 *     the LM's RR to a trunnion and shaft angle (RRDESNBK, then the job
 *     RRDESK2 that designates), or the CM's optics (OPTCOARK);
 *   - V42, fine align: gyro torquing to the angles loaded (IMUFINEK);
 *   - V43, load the attitude error needles (IMUATTCK);
 *   - the LM's V44, end the continuous designate (RRDESEND), and V60, the
 *     landing radar to position 2 (LRPOS2K);
 *   - and the checks they call: OP/INERT (which noun), the LM's RDRUSECK
 *     (is a radar in use?) and the CM's CKLFTBTS (has the CM lifted off?).
 *
 * Most of the work is a BANKCALL to a routine elsewhere (IMUZERO, IMUCOARS,
 * RRDESNB, ...) and a stall until it is done (IMUSTALL, RADSTALL), or a
 * flashing verb and noun (GOXDSPF) that returns to one of three words: the
 * crew's terminate (V34), proceed (V33) or enter. Each such call ends a C
 * function with Q and Z set (tc() below, or continue_at()), and the word it
 * returns to is an entry of its own; the binding goes on from one entry of
 * this part to the next while Z is one. A routine that falls into the next
 * label calls it.
 *
 * All of it is basic code in bank 43, but for the LM's AURLOKON, NOLOKON
 * and AURLKON1 in bank 23. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q: back to the caller. A return into fixed memory is taken whole (Z
 * the return, Q 3, as the TC through register Q leaves it); any other Q
 * stops at Z = 2, register Q, for the flight program to run, as the
 * faithful module does. */
static void return_to_q(void)
{
    if (Q >= 02000u && Q <= 07777u)
        tc_through(Q_ADDRESS);
    else
        continue_at(Q_ADDRESS);
}

/* ---- V40 and V41: which noun ---- */

/* VBZERO -- LM p. 265, CM p. 239.
 * Verb 40, zero the CDUs: OP/INERT returns to the next word for noun 20
 * (the IMU's CDUs), to the one after for the LM's noun 72 (the RR's) or
 * the CM's noun 91 (the optics'). */
void vbzero(void)
{
    tc(VBZERO, OP_INERT);
}

/* VBZERO +1 -- LM p. 265, CM p. 239: "RETURN HERE IF NOUN = ICDU(20)". */
void vbzero_after_op_inert_imu(void)
{
    tc(VBZERO + 1u, IMUZEROK);
}

#if LUMINARY099
/* VBZERO +2 -- LM p. 265: "RETURN HERE IF NOUN = RCDU(72)". */
void vbzero_after_op_inert_rr(void)
{
    tc(VBZERO + 2u, RRZEROK);
}
#else
/* VBZERO +2 -- CM p. 239: "RETURN HERE IF NOUN = OCDU(91) (NOT IN USE
 * YET)": the operator error light. */
void vbzero_after_op_inert_optics(void)
{
    tc(VBZERO + 2u, ALM_END);
}
#endif

/* VBCOARK -- LM p. 267, CM p. 240.
 * Verb 41, coarse align: as VBZERO, by noun. */
void vbcoark(void)
{
    tc(VBCOARK, OP_INERT);
}

/* VBCOARK +1 -- LM p. 267, CM p. 240: noun 20, the IMU. */
void vbcoark_after_op_inert_imu(void)
{
    tc(VBCOARK + 1u, IMUCOARK);
}

#if LUMINARY099
/* VBCOARK +2 -- LM p. 267: noun 72, the RR. */
void vbcoark_after_op_inert_rr(void)
{
    tc(VBCOARK + 2u, RRDESNBK);
}
#else
/* VBCOARK +2 -- CM p. 240: noun 91, the optics. */
void vbcoark_after_op_inert_optics(void)
{
    tc(VBCOARK + 2u, OPTCOARK);
}
#endif

/* OP/INERT -- LM pp. 267-268, CM p. 239.
 * NOUNREG less 20 (OCT24): +0 for noun 20, back to the caller (TCQ, a TC
 * Q). The LM then adds RRIMUDIF (-52): +0 for noun 72, to the caller's
 * second word (Q+1). The CM counts Q on first (INCR Q) and adds OPIMDIFF
 * (-71): +0 for noun 91, to the word after the caller's (XACT0Q, a TC Q).
 * Any other noun is an operator error. */
void op_inert(void)
{
    set_A(negate(OCT24)); /* CS OCT24 */
    ad_at(ADDRESS_OF(NOUNREG));
    if (A_is_zero()) { /* EXTEND; BZF: "IF = 20." */
#if LUMINARY099
        continue_at(TCQ);
#else
        continue_at(XACT0Q);
#endif
        return;
    }
#if LUMINARY099
    ad(RRIMUDIF);
    if (A_is_zero()) {
        continue_at(Q_PLUS_1);
        return;
    }
    tc(OP_INERT + 7u, ALM_END); /* "ILLEGAL." */
#else
    incr(Q_ADDRESS);
    ad(OPIMDIFF);
    if (A_is_zero()) {
        continue_at(XACT0Q);
        return;
    }
    tc(OP_INERT + 8u, ALM_END); /* "ILLEGAL." */
#endif
}

#if COMANCHE055
/* XACT0Q -- CM p. 250. TC Q. */
void xact0q(void)
{
    return_to_q();
}
#endif

/* ---- V40: zero the CDUs ---- */

/* IMUZEROK -- LM p. 265, CM p. 239.
 * "KEYBOARD REQ FOR ISS CDUZERO": only with no mode switching under way
 * (CKMODCAD), then IMUZERO, a stall until it is done (IMUSTALL), and back
 * to PINBALL whether it went well or not. */
void imuzerok(void)
{
    tc(IMUZEROK, CKMODCAD);
}

/* IMUZEROK +1 -- LM p. 265, CM p. 239. */
void imuzerok_after_ckmodcad(void)
{
    tc(IMUZEROK + 1u, BANKCALL); /* CADR IMUZERO */
}

/* IMUZEROK +3 -- LM p. 265, CM p. 239: "STALL". */
void imuzerok_after_imuzero(void)
{
    tc(IMUZEROK + 3u, BANKCALL); /* CADR IMUSTALL */
}

/* IMUZEROK +5 -- LM p. 265, CM p. 239: IMUSTALL's bad return, TC +1 on to
 * the good one. */
void imuzerok_after_imustall_bad(void)
{
    tc(IMUZEROK + 5u, IMUZEROK + 6u);
}

/* IMUZEROK +6 -- LM p. 265, CM p. 239. */
void imuzerok_after_imustall_good(void)
{
    tc(IMUZEROK + 6u, GOPIN);
}

#if LUMINARY099
/* RRZEROK -- LM p. 265.
 * The RR's CDUs: only with neither radar in use (RDRUSECK), then RRZERO
 * and a stall until it is done (RADSTALL, from RWAITK), and back to
 * PINBALL either way. */
void rrzerok(void)
{
    tc(RRZEROK, RDRUSECK);
}

/* RRZEROK +1 -- LM p. 265. */
void rrzerok_after_rdruseck(void)
{
    tc(RRZEROK + 1u, BANKCALL); /* CADR RRZERO */
}

/* RWAITK -- LM p. 265: where RRZERO returns. */
void rwaitk(void)
{
    tc(RWAITK, BANKCALL); /* CADR RADSTALL */
}

/* RWAITK +2 -- LM p. 265: RADSTALL's bad return, TCF +1 on to the good
 * one. */
void rwaitk_after_radstall_bad(void)
{
    continue_at(RWAITK + 3u);
}

/* RWAITK +3 -- LM p. 265. */
void rwaitk_after_radstall_good(void)
{
    tc(RWAITK + 3u, GOPIN);
}

/* ---- V60: the landing radar to position 2 ---- */

/* LRPOS2K -- LM p. 265.
 * "COMMAND LANDING RADAR TO POSITION 2": with neither radar in use, LRPOS2
 * and a stall until it is done; alarm 523 if position 2 does not show in
 * time (LRP2ALM). */
void lrpos2k(void)
{
    tc(LRPOS2K, RDRUSECK);
}

/* LRPOS2K +1 -- LM p. 265. */
void lrpos2k_after_rdruseck(void)
{
    tc(LRPOS2K + 1u, BANKCALL); /* CADR LRPOS2 */
}

/* LRPOS2K +3 -- LM p. 265. */
void lrpos2k_after_lrpos2(void)
{
    tc(LRPOS2K + 3u, BANKCALL); /* CADR RADSTALL */
}

/* LRPOS2K +5 -- LM p. 265: RADSTALL's bad return. */
void lrpos2k_after_radstall_bad(void)
{
    tc(LRPOS2K + 5u, LRP2ALM);
}

/* LRPOS2K +6 -- LM p. 265: the good one. */
void lrpos2k_after_radstall_good(void)
{
    tc(LRPOS2K + 6u, GOPIN);
}

/* LRP2ALM -- LM p. 265. Alarm 523, then PINBALL. */
void lrp2alm(void)
{
    tc(LRP2ALM, ALARM); /* OCT 523 */
}

/* LRP2ALM +2 -- LM p. 265. */
void lrp2alm_after_alarm(void)
{
    tc(LRP2ALM + 2u, GOPIN);
}
#endif

/* ---- V41: coarse align ---- */

/* IMUCOARK -- LM p. 268, CM p. 240.
 * "COARSE ALIGN FROM KEYBOARD": with no mode switching under way, claim
 * the display (TESTXACT) and flash V25 N22 for the angles (VNLODCDU). */
void imucoark(void)
{
    tc(IMUCOARK, CKMODCAD);
}

/* IMUCOARK +1 -- LM p. 268, CM p. 240. */
void imucoark_after_ckmodcad(void)
{
    tc(IMUCOARK + 1u, TESTXACT);
}

/* IMUCOARK +2 -- LM p. 268, CM p. 240: "CALL FOR THETAD LOAD". */
void imucoark_after_testxact(void)
{
    set_A(VNLODCDU);
    tc(IMUCOARK + 3u, BANKCALL); /* CADR GOXDSPF */
}

/* IMUCOARK +5 -- LM p. 268, CM p. 240: terminate, the display released. */
void imucoark_after_terminate(void)
{
    tc(IMUCOARK + 5u, TERMEXTV);
}

/* IMUCOARK +6 -- LM p. 268, CM p. 240: proceed, to the angles THETAD
 * holds (TCF +1, to ICORK2, where enter returns too). */
void imucoark_after_proceed(void)
{
    continue_at(ICORK2);
}

/* ICORK2 -- LM p. 268, CM p. 241.
 * "RE-DISPLAY COARSE ALIGN VERB.", then IMUCOARS, the mode switching, and
 * a stall until it is done; the display released either way. */
void icork2(void)
{
    set_A(IMUCOARV);
    tc(ICORK2 + 1u, BANKCALL); /* CADR EXDSPRET */
}

/* ICORK2 +3 -- LM p. 268, CM p. 241: "CALL MODE SWITCHING PROG". */
void icork2_after_exdspret(void)
{
    tc(ICORK2 + 3u, BANKCALL); /* CADR IMUCOARS */
}

/* ICORK2 +5 -- LM p. 268, CM p. 241: "STALL". */
void icork2_after_imucoars(void)
{
    tc(ICORK2 + 5u, BANKCALL); /* CADR IMUSTALL */
}

/* ICORK2 +7 and +8 -- LM p. 268, CM p. 241: IMUSTALL's bad and good
 * returns. */
void icork2_after_imustall_bad(void)
{
    tc(ICORK2 + 7u, ENDEXTVB);
}

void icork2_after_imustall_good(void)
{
    tc(ICORK2 + 8u, ENDEXTVB);
}

#if LUMINARY099
/* RRDESNBK -- LM p. 269.
 * "DESIGNATE TO DESIRED GIMBAL ANGLES.": with neither radar in use, claim
 * the display; an operator error if P20 runs (RNDVZBIT). Else end any
 * designate under way (RADMODES's bits 15 and 10, OCT41000, cleared, with
 * interrupts held: "RELINT DONE IN GOXDSPF") and ask for the angles. */
void rrdesnbk(void)
{
    tc(RRDESNBK, RDRUSECK);
}

/* RRDESNBK +1 -- LM p. 269. */
void rrdesnbk_after_rdruseck(void)
{
    tc(RRDESNBK + 1u, TESTXACT);
}

/* RRDESNBK +2 -- LM p. 269: "IS P20 RUNNING?" */
void rrdesnbk_after_testxact(void)
{
    set_A(RNDVZBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(XACTALM); /* "OPERADOR ERROR IF IN P20" */
        return;
    }
    set_A(negate(OCT41000)); /* CS OCT41000: "TERMINATE PRESENT DESIGNATION" */
    inhint();
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    rrdesnbk_plus10();
}

/* RRDESNBK +10 -- LM p. 269: "ASK FOR GIMBAL ANGLES.", flashing V24 N73
 * (VNLDRCDU). */
void rrdesnbk_plus10(void)
{
    set_A(VNLDRCDU);
    tc(RRDESNBK + 11u, BANKCALL); /* CADR GOXDSPF */
}

/* RRDESNBK +13 -- LM p. 269: terminate. */
void rrdesnbk_after_terminate(void)
{
    tc(RRDESNBK + 13u, TERMEXTV);
}

/* RRDESNBK +14 -- LM p. 269: proceed (V33) asks again (TCF -4). */
void rrdesnbk_after_proceed(void)
{
    continue_at(RRDESNBK + 10u);
}

/* RRDESNBK +15 -- LM p. 269: enter. "ASK OP FOR LOCK ON REQUIREMENTS."
 * (AURLOKON, in bank 23). */
void rrdesnbk_after_enter(void)
{
    tc(RRDESNBK + 15u, BANKCALL); /* CADR AURLOKON */
}

/* RRDESNBK +17 -- LM p. 269: "RE-DISPLAY OUR OWN VERB" (OPTCOARV, V41). */
void rrdesnbk_after_aurlokon(void)
{
    set_A(OPTCOARV);
    tc(RRDESNBK + 18u, BANKCALL); /* CADR EXDSPRET */
}

/* RRDESNBK +20 -- LM p. 269: the job RRDESK2, at priority 20, does the
 * designate. */
void rrdesnbk_after_exdspret(void)
{
    set_A(PRIO20);
    tc(RRDESNBK + 21u, FINDVAC); /* EBANK= LOSCOUNT; 2CADR RRDESK2 */
}

/* RRDESNBK +24 -- LM p. 269: "FREES DISPLAY". */
void rrdesnbk_after_findvac(void)
{
    continue_at(TERMEXTV); /* TCF TERMEXTV */
}

/* RRDESK2 -- LM pp. 269-270.
 * The designate job: RRDESNB, which returns to the next word or, with its
 * INCR (DESRETRN), the one after. */
void rrdesk2(void)
{
    tc(RRDESK2, BANKCALL); /* CADR RRDESNB */
}

/* RRDESK2 +2 -- LM p. 269: "DUMMY NEEDED SINCE DESRETRN DOES INCR". */
void rrdesk2_after_rrdesnb(void)
{
    tc(RRDESK2 + 2u, RRDESK2 + 3u); /* TC +1 */
}

/* RRDESK2 +3 -- LM p. 269.
 * "RELEASE THIS JOBS VAC AREA.": PRIORITY's low nine bits hold the
 * address of the job's VAC area's use word plus one (+0 for a job with no
 * VAC area). CCS leaves the address in A, and INDEX A; TS A stores it in
 * that word: a free area's use word holds its own address. (With +0 the
 * CCS skips the INDEX, and TS A changes nothing.) COM and ADS PRIORITY
 * take that address off PRIORITY, leaving its low nine bits 1 (or +0):
 * "INSURE ENDOFJOB DOES A NOVAC END (BZMF)", ENDJOB1's test of them
 * against FAKEPRET, which releases no VAC area. Then wait for the
 * designate (RADSTALL).
 *
 * A use word is never a central or counter register (below 061): the
 * readable routine does not model a store there. */
void rrdesk2_plus3(void)
{
    ca(ADDRESS_OF(PRIORITY));
    mask(LOW9);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        unsigned vac_use = A; /* INDEX A */
        if (vac_use < 061u) not_modelled();
        ts(vac_use); /* TS A */
    }
    com();
    ads(ADDRESS_OF(PRIORITY));
    tc(RRDESK2 + 10u, BANKCALL); /* CADR RADSTALL: "WAIT FOR COMPLETION OF DESIGNATE" */
}

/* RRDESK2 +12 -- LM p. 270: "BADEND-NO LOCKON OR OUT OF LIMITS" (TC +2,
 * to the alarm). */
void rrdesk2_after_radstall_bad(void)
{
    tc(RRDESK2 + 12u, RRDESK2 + 14u);
}

/* RRDESK2 +13 -- LM p. 270: "GOODEND-LOCKON ACHIEVED". */
void rrdesk2_after_radstall_good(void)
{
    tc(RRDESK2 + 13u, ENDOFJOB);
}

/* RRDESK2 +14 -- LM p. 270: alarm 503, "DESIGNATE FAIL". */
void rrdesk2_plus14(void)
{
    tc(RRDESK2 + 14u, ALARM); /* OCT 503 */
}

/* RRDESK2 +16 -- LM p. 270. */
void rrdesk2_after_alarm(void)
{
    tc(RRDESK2 + 16u, ENDOFJOB);
}

/* ---- V44: end the continuous designate ---- */

/* RRDESEND -- LM p. 270.
 * "TERMINATE CONTINOUS DESIGNATE ONLY": only with RADMODES's bit 15 set
 * (negative; -0 skips onto the same word); else back to PINBALL. Then
 * RADMODES's designate bits cleared, interrupts held ("RELINT DONE IN
 * DOWNFLAG"), the radar modes cleared (CLRADMOD), a second's wait
 * (DELAYJOB) and the RR gimbal monitor, R25, allowed again (NORRMON
 * down). */
void rrdesend(void)
{
    switch (ccs_at(ADDRESS_OF(RADMODES))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(GOPIN); return; /* TCF GOPIN */
    case NEGATIVE:
    case IS_MINUS_ZERO: break; /* TCF +1 */
    }
    set_A(negate(OCT41000)); /* CS OCT41000: "BEGDES GOES TO ENDRADAR" */
    inhint();
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    tc(RRDESEND + 8u, CLRADMOD);
}

/* RRDESEND +9 -- LM p. 270. */
void rrdesend_after_clradmod(void)
{
    set_A(ONE_SEC);
    tc(RRDESEND + 10u, BANKCALL); /* CADR DELAYJOB */
}

/* RRDESEND +12 -- LM p. 270: "ENABLE R25 GIMBAL MONITOR". */
void rrdesend_after_delayjob(void)
{
    tc(RRDESEND + 12u, DOWNFLAG); /* ADRES NORRMON */
}

/* RRDESEND +14 -- LM p. 270. */
void rrdesend_after_downflag(void)
{
    continue_at(GOPIN); /* TCF GOPIN */
}

/* ---- AURLOKON (bank 23): lock on, or a continuous designate ---- */

/* AURLOKON -- LM p. 271.
 * Asks the crew whether the RR is to lock on: V04 N12 with option code 6
 * and R2 00002 (a continuous designate) flashing. The caller's return,
 * as a CADR (MAKECADR), waits in DESRET. */
void aurlokon(void)
{
    tc(AURLOKON, MAKECADR);
}

/* AURLOKON +1 -- LM p. 271.
 * An overflow in A skips CAF TWO, and TS leaves +1 or -1 in OPTIONX +1. */
void aurlokon_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(DESRET))) set_A(TWO);
    ts(ADDRESS_OF(OPTIONX) + 1u);
    set_A(SIX); /* "OPTION CODE FOR V04N12" */
    ts(ADDRESS_OF(OPTIONX));
    aurlokon_plus6();
}

/* AURLOKON +6 -- LM p. 271 (the listing's -5): "R2 00001 LOCK-ON". */
void aurlokon_plus6(void)
{
    set_A(V04N1272);
    tc(AURLOKON + 7u, BANKCALL); /* CADR GOMARKFR */
}

/* AURLOKON +9 -- LM p. 271: V34, terminate. */
void aurlokon_after_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* AURLOKON +10 -- LM p. 271: V33, proceed (TCF +5). */
void aurlokon_after_proceed(void)
{
    continue_at(AURLOKON + 15u);
}

/* AURLOKON +11 -- LM p. 271: V32, recycle, asks again (TCF -5). */
void aurlokon_after_recycle(void)
{
    continue_at(AURLOKON + 6u);
}

/* AURLOKON +12 -- LM p. 271: a load: blank R3 (BIT3) and wait for the
 * response. */
void aurlokon_after_enter(void)
{
    set_A(BIT3);
    tc(AURLOKON + 13u, BLANKET);
}

/* AURLOKON +14 -- LM p. 271. */
void aurlokon_after_blanket(void)
{
    tc(AURLOKON + 14u, ENDOFJOB);
}

/* AURLOKON +15 -- LM p. 271 (the listing's +5).
 * OPTIONX +1's bit 2 set (R2 00002): no lock-on. Else LOKONSW up and on. */
void aurlokon_plus15(void)
{
    ca(ADDRESS_OF(OPTIONX) + 1u);
    mask(BIT2);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(NOLOKON); /* TCF NOLOKON */
        return;
    }
    tc(AURLOKON + 19u, UPFLAG); /* ADRES LOKONSW */
}

/* AURLOKON +21 -- LM p. 271. */
void aurlokon_after_upflag(void)
{
    continue_at(AURLKON1); /* TCF AURLKON1 */
}

/* NOLOKON -- LM p. 271.
 * "IF NO LOCK-ON, SET BIT15 OF RADMADES TO INDICATE THAT CONTINUOUS
 * DESIGNATION IS WANTED (TO BE TERMINATED BY V44.)": LOKONSW down,
 * CDESFLAG up, and NORRMON up ("DISABLE R25 RR GIMBAL MONITOR IN
 * T4RUPT"). UPFLAG's last return is AURLKON1. */
void nolokon(void)
{
    tc(NOLOKON, DOWNFLAG); /* ADRES LOKONSW */
}

/* NOLOKON +2 -- LM p. 271. */
void nolokon_after_downflag(void)
{
    tc(NOLOKON + 2u, UPFLAG); /* ADRES CDESFLAG */
}

/* NOLOKON +4 -- LM p. 271. */
void nolokon_after_upflag(void)
{
    tc(NOLOKON + 4u, UPFLAG); /* ADRES NORRMON */
}

/* AURLKON1 -- LM p. 271. Interrupts allowed again, and back to the
 * caller, through its CADR (BANKJUMP). */
void aurlkon1(void)
{
    relint();
    ca(ADDRESS_OF(DESRET));
    continue_at(BANKJUMP); /* TCF BANKJUMP */
}
#else
/* OPTCOARK -- CM p. 242.
 * "TEMPORARY ROUTINE TO RUN THE OPTICS CDUS FROM THE KEYBOARD": with no
 * optics mode switching under way (OPTCADR, to CKMODCAD +1), claim the
 * display. */
void optcoark(void)
{
    ca(ADDRESS_OF(OPTCADR));
    tc(OPTCOARK + 1u, CKMODCAD + 1u);
}

/* OPTCOARK +2 -- CM p. 242. */
void optcoark_after_ckmodcad(void)
{
    tc(OPTCOARK + 2u, TESTXACT);
}

/* OPTCOARK +3 -- CM p. 242.
 * E-bank 5, for SAC and PAC (OPTC1). "SEE IF SWITCH AT COMPUTER": with
 * SWSAMPLE positive the optics are under the computer's control (TC +5,
 * on); else the operator error light (FALTON) and alarm 115 (ALARM, where
 * FALTON returns). A -0 goes to the alarm alone. */
void optcoark_after_testxact(void)
{
    set_A(EBANK5);
    ts(EBANK_ADDRESS);
    switch (ccs_at(ADDRESS_OF(SWSAMPLE))) {
    case POSITIVE: tc(OPTCOARK + 6u, OPTCOARK + 11u); return; /* "SWITCH AT COMPUTER" */
    case IS_PLUS_ZERO:
        tc_at(OPTCOARK + 7u); /* TC +1: "NOT ON COMPUTER" */
    /* fall through */
    case NEGATIVE: tc(OPTCOARK + 8u, FALTON); return; /* "TURN ON OPERATOR ERR" */
    case IS_MINUS_ZERO: tc(OPTCOARK + 9u, ALARM); return;
    }
}

/* OPTCOARK +9 -- CM p. 242: "AND ALARM", OCT 00115. */
void optcoark_after_falton(void)
{
    tc(OPTCOARK + 9u, ALARM);
}

/* OPTCOARK +11 -- CM p. 242.
 * "SEE IF OPTICS AVAILABLE": OPTIND anything but -0, OPTC1 (the three
 * TCs, "IN USE"); -0, alarm 117, "OPTICS RESERVED (OPTIND=-0)", and the
 * display released. */
void optcoark_plus11(void)
{
    switch (ccs_at(ADDRESS_OF(OPTIND))) {
    case POSITIVE: tc(OPTCOARK + 12u, OPTC1); return;
    case IS_PLUS_ZERO: tc(OPTCOARK + 13u, OPTC1); return;
    case NEGATIVE: tc(OPTCOARK + 14u, OPTC1); return;
    case IS_MINUS_ZERO: tc(OPTCOARK + 15u, ALARM); return; /* OCT 00117 */
    }
}

/* OPTCOARK +17 -- CM p. 242. */
void optcoark_after_alarm(void)
{
    tc(OPTCOARK + 17u, ENDEXT);
}

/* OPTC1 -- CM p. 242.
 * "VERB-NOUN TO LOAD OPTICS CDUS": flashing V24 N92 (VNLD0CDU). On
 * proceed or enter, the angles loaded (SAC, PAC) become the optics'
 * desired ones (DESOPTS, DESOPTT), V41 shows again and OPTIND +1 says
 * "COARS WORKING". */
void optc1(void)
{
    set_A(VNLD0CDU);
    tc(OPTC1 + 1u, BANKCALL); /* CADR GOXDSPF */
}

/* OPTC1 +3 -- CM p. 242: terminate. */
void optc1_after_terminate(void)
{
    tc(OPTC1 + 3u, TERMEXTV);
}

/* OPTC1 +4 -- CM p. 242: "PROCEED" (TC +1). */
void optc1_after_proceed(void)
{
    tc(OPTC1 + 4u, OPTC1 + 5u);
}

/* OPTC1 +5 -- CM p. 242: enter. */
void optc1_after_enter(void)
{
    ca(ADDRESS_OF(SAC));
    ts(ADDRESS_OF(DESOPTS));
    ca(ADDRESS_OF(PAC));
    ts(ADDRESS_OF(DESOPTT));
    set_A(OPTCOARV);           /* "RE-DISPLAY OUR OWN VERB" */
    tc(OPTC1 + 10u, BANKCALL); /* CADR EXDSPRET */
}

/* OPTC1 +12 -- CM p. 242: "SET COARS WORKING". */
void optc1_after_exdspret(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(OPTIND));
    tc(OPTC1 + 14u, ENDEXTVB);
}

/* OPTC1 +15 -- CM p. 242. */
void optc1_plus15(void)
{
    tc(OPTC1 + 15u, ENDEXTVB);
}
#endif

/* ---- V42: fine align ---- */

/* IMUFINEK -- LM p. 273, CM p. 243.
 * "FINE ALIGN WITH GYRO TORQUING": with no mode switching under way, claim
 * the display and flash V25 N93 for the gyro angles (VNLODGYR). Proceed
 * goes on without a load. Then V42 again, IMUFINE (the mode switching), a
 * stall; if that went well, the gyros torqued (FINEK2), else the display
 * released. */
void imufinek(void)
{
    tc(IMUFINEK, CKMODCAD);
}

/* IMUFINEK +1 -- LM p. 273, CM p. 243. */
void imufinek_after_ckmodcad(void)
{
    tc(IMUFINEK + 1u, TESTXACT);
}

/* IMUFINEK +2 -- LM p. 273, CM p. 243: "CALL FOR LOAD OF GYRO COMMANDS". */
void imufinek_after_testxact(void)
{
    set_A(VNLODGYR);
    tc(IMUFINEK + 3u, BANKCALL); /* CADR GOXDSPF */
}

/* IMUFINEK +5 -- LM p. 273, CM p. 243: terminate. */
void imufinek_after_terminate(void)
{
    tc(IMUFINEK + 5u, TERMEXTV);
}

/* IMUFINEK +6 -- LM p. 273, CM p. 243: "PROCEED WITHOUT A LOAD" (TC +1). */
void imufinek_after_proceed(void)
{
    tc(IMUFINEK + 6u, IMUFINEK + 7u);
}

/* IMUFINEK +7 -- LM p. 273, CM p. 243: enter. "RE-DISPLAY OUR OWN VERB". */
void imufinek_after_enter(void)
{
    set_A(IMUFINEV);
    tc(IMUFINEK + 8u, BANKCALL); /* CADR EXDSPRET */
}

/* IMUFINEK +10 -- LM p. 273, CM p. 243: "CALL MODE SWITCH PROG". */
void imufinek_after_exdspret(void)
{
    tc(IMUFINEK + 10u, BANKCALL); /* CADR IMUFINE */
}

/* IMUFINEK +12 -- LM p. 273, CM p. 243: "HIBERNATION". */
void imufinek_after_imufine(void)
{
    tc(IMUFINEK + 12u, BANKCALL); /* CADR IMUSTALL */
}

/* IMUFINEK +14 -- LM p. 273, CM p. 243: IMUSTALL's bad return. The good
 * one is FINEK2. */
void imufinek_after_imustall_bad(void)
{
    tc(IMUFINEK + 14u, ENDEXTVB);
}

/* FINEK2 -- LM p. 273, CM p. 243.
 * "PINBALL LEFT COMMANDS IN OGC REGISTERS": IMUPULSE torques the gyros by
 * them (LGYROBIN, their ECADR), then a stall "FOR PULSES TO GET OUT", and
 * the display released. */
void finek2(void)
{
    set_A(LGYROBIN);
    tc(FINEK2 + 1u, BANKCALL); /* CADR IMUPULSE */
}

/* FINEK2 +3 -- LM p. 273, CM p. 243. */
void finek2_after_imupulse(void)
{
    tc(FINEK2 + 3u, BANKCALL); /* CADR IMUSTALL */
}

/* FINEK2 +5 and +6 -- LM p. 273, CM p. 243: IMUSTALL's bad and good
 * returns. */
void finek2_after_imustall_bad(void)
{
    tc(FINEK2 + 5u, ENDEXTVB);
}

void finek2_after_imustall_good(void)
{
    tc(FINEK2 + 6u, ENDEXTVB);
}

#if LUMINARY099
/* ---- Is a radar in use? ---- */

/* RDRUSECK -- LM p. 280.
 * An operator error (ALM/END) if R29 is on (NR29FLG down), R77 runs, or
 * SERVICER runs (V37FLAG down) with R12 on (LRBYPASS down), "POSSIBLY
 * R12 USING THE LR"; then CHECKRR. */
void rdruseck(void)
{
    cs(ADDRESS_OF(FLAGWRD3)); /* "IS R29 ON?" */
    mask(NR29FBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc(RDRUSECK + 3u, ALM_END); /* "YES" */
        return;
    }
    ca(ADDRESS_OF(FLAGWRD5)); /* "IS R77 RUNNING?" */
    mask(R77FLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc(RDRUSECK + 7u, ALM_END); /* "YES." */
        return;
    }
    cs(ADDRESS_OF(FLAGWRD7)); /* "IS SERVICER RUNNING ..." */
    mask(V37FLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(CHECKRR); /* "NO" */
        return;
    }
    /* "YES, IS R12 ON?" LRBYBIT is bit 15, so the masked word is +0 or
     * negative, and CCS of a negative one skips two words, past CHECKRR's
     * CS FLAGWRD1: MASK TRACKBIT then masks what the CCS left in A. */
    cs(ADDRESS_OF(FLGWRD11));
    mask(LRBYBIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: tc(RDRUSECK + 15u, ALM_END); return; /* "YES" */
    case IS_PLUS_ZERO: checkrr(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: checkrr_plus1(); return;
    }
}

/* CHECKRR -- LM p. 280.
 * "IS THE TRACK FLAG SET AND HENCE POSSIBLY P20 USING THE RR?" If not,
 * CHECKP22. */
void checkrr(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    checkrr_plus1();
}

/* CHECKRR +1 -- LM p. 280. */
void checkrr_plus1(void)
{
    mask(TRACKBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(CHECKP22); /* "NO, CHECK FOR P22." */
        return;
    }
    ckrndbit();
}

/* CKRNDBIT -- LM p. 280: "YES, BUT IS IT P25?" An operator error if not
 * (RNDVZBIT, P20's, set). */
void ckrndbit(void)
{
    ca(ADDRESS_OF(FLAGWRD0));
    mask(RNDVZBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc(CKRNDBIT + 3u, ALM_END);
        return;
    }
    checkp22();
}

/* CHECKP22 -- LM p. 280: an operator error in P22, else back to the
 * caller: neither radar is in use. */
void checkp22(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(DEC22);
    if (A_is_zero()) { /* EXTEND; BZF */
        continue_at(ALM_END);
        return;
    }
    return_to_q();
}
#endif

/* ---- V43: the attitude error needles ---- */

/* IMUATTCK -- LM p. 281, CM p. 249.
 * "LOAD IMU ATTITUDE ERROR METERS": only in P00 (CHKPOOH), and with the
 * IMU's coarse align and zero bits of channel 12 off. The LM wants the
 * needles off (channel 31's bits 13 and 14, the mode selected, both set);
 * the CM, after liftoff, the display (CKLFTBTS, TESTXACT) and the
 * coarse align and error counter enables off. Then flashing V25 N22 for
 * the needles' angles. */
void imuattck(void)
{
    tc(IMUATTCK, CHKPOOH);
}

/* IMUATTCK +1 -- LM p. 281, CM p. 249. */
void imuattck_after_chkpooh(void)
{
#if LUMINARY099
    set_A(BITS4_5); /* "SEE IF COARSE ALIGN ENABLE AND ZERO IMU CDUS BITS ARE ON" */
#else
    set_A(OCTAL30); /* "CHECK IF IMU ZERO AND IMU COARSE ARE ON" */
#endif
    rand_channel(012);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(ALM_END); /* "NOT ALLOWED IF IMU COARSE OR IMU ZERO ON" */
        return;
    }
#if LUMINARY099
    set_A(BIT13_14); /* "BOTH BITS 13 AND 14 MUST BE 1 INDICATING THE MODE SELECTED IS OFF." */
    rxor_channel(031);
    mask(BIT13_14);
    if (A_is_zero()) {               /* EXTEND; BZF +2 */
        continue_at(IMUATTCK + 13u); /* "NEEDLES IS OFF." */
        return;
    }
    continue_at(ALM_END); /* "EXIT. NEEDLES IS ON." */
#else
    tc(IMUATTCK + 6u, CKLFTBTS); /* "IS IT BEFORE OR AFTER LIFTOFF" */
#endif
}

#if LUMINARY099
/* IMUATTCK +13 -- LM p. 281. */
void imuattck_plus13(void)
{
    tc(IMUATTCK + 13u, TESTXACT);
}

/* IMUATTCK +14 -- LM p. 281. */
void imuattck_after_testxact(void)
{
    set_A(VNLODCDU);
    tc(IMUATTCK + 15u, BANKCALL); /* CADR GOXDSPF */
}

/* IMUATTCK +17 -- LM p. 281: V34, terminate. */
void imuattck_after_terminate(void)
{
    tc(IMUATTCK + 17u, ENDEXT);
}

/* IMUATTCK +18 -- LM p. 281: proceed (TC +1). */
void imuattck_after_proceed(void)
{
    tc(IMUATTCK + 18u, IMUATTCK + 19u);
}

/* IMUATTCK +19 -- LM p. 281: enter. "REDISPLAY OUR VERB." */
void imuattck_after_enter(void)
{
    set_A(V43K);
    tc(IMUATTCK + 20u, BANKCALL); /* CADR EXDSPRET */
}

/* IMUATTCK +22 -- LM p. 281.
 * "ENABLE ERROR COUNTERS." (channel 12's bit 6), and the task ATTCK2, two
 * centiseconds on, to "PUT OUT COMMANDS" (".32 SECONDS", the listing's
 * comment says). */
void imuattck_after_exdspret(void)
{
    set_A(BIT6);
    wor_channel(012);
    set_A(TWO);
    tc(IMUATTCK + 26u, WAITLIST); /* EBANK= THETAD; 2CADR ATTCK2 */
}

/* IMUATTCK +29 -- LM p. 281. */
void imuattck_after_waitlist(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}
#else
/* IMUATTCK +7 -- CM p. 249: "AFTER" liftoff, the display claimed. */
void imuattck_after_cklftbts_yes(void)
{
    tc(IMUATTCK + 7u, TESTXACT);
}

/* IMUATTCK +8 -- CM p. 249: TESTXACT's return, and CKLFTBTS's before
 * liftoff. "REMOVE COARSE AND ECTR ENABLE." (channel 12's bits 4 and 6,
 * OCT50), then the angles asked for. */
void imuattck_after_testxact(void)
{
    set_A(negate(OCT50)); /* CS OCT50 */
    wand_channel(012);
    set_A(VNLODCDU);
    tc(IMUATTCK + 12u, BANKCALL); /* CADR GOXDSPF */
}

/* IMUATTCK +14 -- CM p. 249: terminate. */
void imuattck_after_terminate(void)
{
    continue_at(TRMATTCK); /* TCF TRMATTCK */
}

/* IMUATTCK +15 -- CM p. 249: proceed (TC +1). */
void imuattck_after_proceed(void)
{
    tc(IMUATTCK + 15u, IMUATTCK + 16u);
}

/* IMUATTCK +16 -- CM p. 249: enter. "SET E6 FOR NEEDLES.", and NEEDLE11
 * "INITIALIZE CURRENT DAC AND COMMAND VALUES". */
void imuattck_after_enter(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    tc(IMUATTCK + 18u, BANKCALL); /* CADR NEEDLE11 */
}

/* IMUATTCK +20 -- CM p. 249: "ENABLE ERROR COUNTERS." */
void imuattck_after_needle11(void)
{
    tc(IMUATTCK + 20u, BANKCALL); /* CADR NEEDLER2 */
}

/* IMUATTCK +22 -- CM p. 249: the task ATTCK1 in "4 MS MIN.". */
void imuattck_after_needler2(void)
{
    set_A(TWO);
    tc(IMUATTCK + 23u, WAITLIST); /* EBANK= AK; 2CADR ATTCK1 */
}

/* TRMATTCK -- CM p. 249.
 * Terminate: after liftoff the display released, before it back to
 * PINBALL. */
void trmattck(void)
{
    tc(TRMATTCK, CKLFTBTS); /* "IS IT BEFORE OR AFRER LIFTOFF" */
}

/* TRMATTCK +1 -- CM p. 249: "AFTER". */
void trmattck_after_cklftbts_yes(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* TRMATTCK +2 -- CM p. 249: before. */
void trmattck_after_cklftbts_no(void)
{
    tc(TRMATTCK + 2u, GOPIN);
}

/* ATTCK1 -- CM p. 250.
 * A WAITLIST task: "TRANSFER LOADED VALUES TO DESIRED REGS." (THETAD's
 * three angles to AK's) and NEEDLES, which "SENDS COMMANDS LIMITED TO +,-
 * 384 PULSES AND LEAVES ERROR COUNTERS ENABLED." */
void attck1(void)
{
    dca(ADDRESS_OF(THETAD)); /* EXTEND; DCA THETAD */
    dxch(ADDRESS_OF(AK));
    ca(ADDRESS_OF(THETAD) + 2u); /* CAE THETAD +2 */
    ts(ADDRESS_OF(AK) + 2u);
    tc(ATTCK1 + 5u, IBNKCALL); /* CADR NEEDLES */
}

/* ATTCK1 +7 -- CM p. 250. */
void attck1_after_needles(void)
{
    tc(ATTCK1 + 7u, TASKOVER);
}

/* CKLFTBTS -- CM p. 250.
 * "HAS LIFTOFF OCCURRED": GRRBKBIT of FLAGWRD5 set, or channel 30's bit 5
 * (liftoff, a 0 when it is on) +0: back to the caller (TC Q, "YES"). Else
 * to the word after (Q+1). */
void cklftbts(void)
{
    set_A(GRRBKBIT);
    mask_at(ADDRESS_OF(FLAGWRD5));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        return_to_q(); /* "YES" */
        return;
    }
    set_A(BIT5);
    rand_channel(030);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(Q_PLUS_1); /* TCF Q+1 */
        return;
    }
    xact0q(); /* "YES" */
}
#endif
