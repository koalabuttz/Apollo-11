/* SPDX-License-Identifier: MIT */
/* P32-P35_P72-P75.agc (Luminary099 pp. 618-631 and 650): the programs'
 * own code (bank 35). P32 and P72, "COELLIPTIC SEQUENCE INITIATION
 * (CSI) PROGRAMS", and P33 and P73, "CONSTANT DELTA HEIGHT (CDH)
 * PROGRAMS"; AVFLAGA, AVFLAGP and P20FLGON, which set the flags; DISDVLVC,
 * which shows a delta V in local vertical coordinates; and COMPTGO, which
 * starts the clock display.
 *
 * P32's purpose, as its description has it: "TO CALCULATE PARAMETERS
 * ASSOCIATED WITH THE TIME FOLLOWING CONCENTRIC FLIGHT PLAN MANEUVERS --
 * THE CO-ELLIPTIC SEQUENCE INITIATION (CSI) MANEUVER AND THE CONSTANT
 * DELTA ALTITUDE (CDH) MANEUVER." P33's: "TO CALCULATE PARAMETERS
 * ASSOCIATED WITH THE CONSTANT DELTA ALTITUDE MANEUVER (CDH)." Each is
 * "SELECTED BY THE ASTRONAUT BY DSKY ENTRY": P32 or P33 "IF THIS VEHICLE
 * IS ACTIVE VEHICLE", P72 or P73 "IF THIS VEHICLE IS THE PASSIVE
 * VEHICLE".
 *
 * Each call out of the file (VNPOOH, BANKCALL, ALARM, VARALARM, UPFLAG,
 * DOWNFLAG, WAITLIST, 2PHSCHNG) leaves Z at the routine and Q at the word
 * after the TC (after its ADRES, its CADR, its 2CADR or its OCTs), where
 * the routine comes back to; GOTOPOOH doesn't come back. A display
 * through GOFLASH comes back to the word after its CADR on terminate, the
 * next on proceed and the next on enter (recycle). VNPOOH, P34-35_P74-
 * 75.agc's, comes back on proceed only. */
#include "p32_p35.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* TC to an erasable word that holds a return: the AGC runs the word as an
 * instruction, a TC when the program has saved an address there. Any
 * other word (another instruction, or a register's address) is not
 * modelled. */
static void tc_return_in(unsigned address)
{
    word to = fetch(address);

    if (OPERATION_CODE(to) != 0u || to < 010u) not_modelled();
    tc_through(address);
}

/* ---- P32 and P72 -- pp. 621-623 ---- */

/* P32: AVFLAGA, the LM the active vehicle, then on at P32STRT. */
void p32(void)
{
    tc_at(P32);
    avflaga();
}

void p32_after_avflaga(void)
{
    tc_at(P32 + 1u);
    p32strt();
}

/* P72: AVFLAGP, the CSM the active vehicle, and on at P32STRT. */
void p72(void)
{
    tc_at(P72);
    avflagp();
}

/* P32STRT: CENTANG zero (DCA P30ZERO, DXCH), then P32/P72A. */
void p32strt(void)
{
    dca(P30ZERO); /* EXTEND; DCA */
    dxch(ADDRESS_OF(CENTANG));
    tc_at(P32STRT + 3u);
    p32_p72a();
}

/* ALMXIT, after its EXIT: the alarm, its code from ALARM/TB (VARALARM
 * with it in A), and verb 05 noun 09 flashed (GOFLASH). */
void almxit_after_exit(void)
{
    ca(ADDRESS_OF(MPAC));
    tc_to(ALMXIT + 5u, VARALARM);
}

void almxit_after_varalarm(void)
{
    set_A(V05N09);
    tc_to(ALMXIT + 7u, BANKCALL); /* CADR GOFLASH */
}

/* ALMXIT +011: terminate. */
void almxit_terminate(void)
{
    tc_to(ALMXIT + 011u, GOTOPOOH);
}

/* ALMXIT +012: proceed, the display again (TC -4). Enter goes on at
 * P32/P72A. */
void almxit_proceed(void)
{
    tc_at(ALMXIT + 012u);
    almxit_after_varalarm();
}

/* P32/P72A: P20FLGON, the update and track flags set, */
void p32_p72a(void)
{
    tc_at(P32_P72A);
    p20flgon();
}

/* then NN +1, TCSI and TCSI +1 zero (A +0, from CAF P30ZERO, has no
 * overflow for a TS to skip on), and at VN0611 TCSI asked for: verb 06
 * noun 11 (VNPOOH). */
void p32_p72a_after_p20flgon(void)
{
    ca(P30ZERO);
    ts(ADDRESS_OF(NN) + 1u);
    ts(ADDRESS_OF(TCSI));
    ts(ADDRESS_OF(TCSI) + 1u);
    vn0611_again();
}

/* VN0611 +2: to the interpreter, for VN0611_CODE. */
void vn0611_after_vnpooh(void)
{
    tc_at(VN0611 + 2u);
    intpret();
}

/* VN0611 (and VN0655 -1, after an EXIT): TCSI, "TIME OF THE CSI
 * MANEUVER", verb 06 noun 11. */
void vn0611_again(void)
{
    set_A(V06N11);
    tc_to(VN0611 + 1u, VNPOOH);
}

/* After VN0611_CODE's first EXIT, back to VN0611. */
static void after_tcsi_found(void)
{
    tc_at(VN0655 - 1u);
    vn0611_again();
}

/* VN0611_CODE: with TCSI keyed in (positive: DCOMP, BMN), on at VN0655.
 * Else TCSI the time of apocenter: the active vehicle's state at TETLEM
 * (PRECSET's RACT3 and VACT3), shifted by X2 (VSR* 0,2) into RVEC and
 * VVEC, to TIMERAD (CONIC_SUBROUTINES) with RVSW set (the time alone) and
 * DPPOSMAX as RDESIRED: a radius beyond apocenter, for which TIMERAD
 * returns the apocenter solution; plus TDEC2, the state's time. Then back
 * to VN0611 (EXIT, TC VN0611) to show it. */
static const word vn0611_words[] = {
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(TCSI),
    ORDERS(BMN, DLOAD),
    ADDRESS(VN0655),
    ADDRESS_PLUS_ONE(TETLEM),
    STCALL(TDEC1),
    ADDRESS(PRECSET),
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(RACT3),
    SR_X2(0),
    STOVL(RVEC),
    ADDRESS_PLUS_ONE(VACT3),
    ORDERS(VSR_STAR, SET),
    SR_X2(0),
    SWITCH(SET, RVSW),
    STODL(VVEC),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STCALL(RDESIRED),
    ADDRESS(TIMERAD),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(TDEC2),
    STORE(TCSI),
    ORDER(EXIT),
};

void vn0611_after_exit(void)
{
    after_tcsi_found();
}

/* VN0655's EXIT, */
static const word vn0655_words[] = {
    ORDER(EXIT),
};

/* and verb 06 noun 55 flashed: "NN, ELEV(RGLOS)" (GOFLASH). */
void vn0655_after_exit(void)
{
    set_A(V06N55);
    tc_to(VN0655 + 2u, BANKCALL); /* CADR GOFLASH */
}

/* VN0655 +4: terminate. */
void vn0655_terminate(void)
{
    tc_to(VN0655 + 4u, GOTOPOOH);
}

/* VN0655 +5: proceed, on to TTPI (TC +2). */
void vn0655_proceed(void)
{
    tc_at(VN0655 + 5u);
    vn0637();
}

/* VN0655 +6: recycle, the display again (TC -5). */
void vn0655_recycle(void)
{
    tc_at(VN0655 + 6u);
    vn0655_after_exit();
}

/* VN0655 +7: TTPI, "TIME OF THE TPI MANEUVER", verb 06 noun 37
 * (VNPOOH). */
void vn0637(void)
{
    set_A(V06N37);
    tc_to(VN0655 + 010u, VNPOOH);
}

/* VN0655 +011: to the interpreter, for VN0637_CODE. */
void vn0637_after_vnpooh(void)
{
    tc_at(VN0655 + 011u);
    intpret();
}

/* VN0637_CODE: TIG is TCSI (SELECTMU picks the moon's or the earth's mu).
 * P32/P72B: ADVANCE; the passive vehicle's state at TCSI, and TTPI with
 * TWOPI on the push-down list, through INTINT and PASSIVE; then CSI/A,
 * the computation (bank 34). P32/P72C: with FINALFLG clear, the update
 * flag set. P32/P72D to P32/P72F: T1TOT2 and T2TOT3, "DELTA TIME FROM
 * CSI TO CDH" and "FROM CDH TO TPI", each less 60MIN (360,000
 * centiseconds) while it stays positive, for the display. */
static const word vn0637_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TCSI),
    STCALL(TIG),
    ADDRESS(SELECTMU),
    /* P32/P72B */
    ORDER(CALL),
    ADDRESS(ADVANCE),
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VPASS1),
    ORDERS(PDVL, PDDL),
    ADDRESS_PLUS_ONE(RPASS1),
    ADDRESS_PLUS_ONE(TCSI),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TTPI),
    ADDRESS_PLUS_ONE(TWOPI),
    ORDERS(PUSH, CALL),
    ADDRESS(INTINT),
    ORDER(CALL),
    ADDRESS(PASSIVE),
    ORDER(CALL),
    ADDRESS(CSI_A),
    /* P32/P72C */
    ORDERS(BON, SET),
    SWITCH(BON, FINALFLG),
    ADDRESS(P32_P72D),
    SWITCH(SET, UPDATFLG),
    /* P32/P72D */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(T1TOT2),
    /* P32/P72E */
    STORE(T1TOT2),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(60MIN),
    ADDRESS(P32_P72E),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(T2TOT3),
    /* P32/P72F */
    STORE(T2TOT3),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(60MIN),
    ADDRESS(P32_P72F),
    ORDER(EXIT),
};

/* P32/P72F +5: verb 06 noun 75 (VNPOOH), "DELTA ALTITUDE CDH" (DIFFALT)
 * and the two times, T1TOT2 and T2TOT3 (PINBALL_NOUN_TABLES). */
void p32_p72_n75(void)
{
    set_A(V06N75);
    tc_to(P32_P72F + 6u, VNPOOH);
}

/* P32/P72F +7: to the interpreter, for P32_P72_N75_CODE. */
void p32_p72_n75_after_vnpooh(void)
{
    tc_at(P32_P72F + 7u);
    intpret();
}

/* P32_P72_N75_CODE: DELVEET1 (S32/33.1), RACT2 to RACT1, the delta V
 * shown in local vertical coordinates (DISDVLVC with verb 06 noun 82 in
 * X1), TTPI kept in TTPIO for a recycle, VN1645, and again
 * from P32/P72B. */
static const word p32_p72_n75_words[] = {
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(DELVEET1),
    ADDRESS(S32_33_1),
    STOVL(DELVEET1),
    ADDRESS_PLUS_ONE(RACT2),
    STOVL(RACT1),
    ADDRESS_PLUS_ONE(DELVEET2),
    ORDERS(AXT_1, CALL),
    NUMBER(01522), /* VN 0682 */
    ADDRESS(DISDVLVC),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPI),
    STCALL(TTPIO),
    ADDRESS(VN1645),
    ORDER(GOTO),
    ADDRESS(P32_P72B),
};

/* ALMXITA and ALMXIT: the alarm's number to X1 from CSIALRM (SXA,2 first,
 * at ALMXITA), and its code (SLOAD* ALARM/TB -1,1) to MPAC, then EXIT. */
static const word almxita_words[] = {
    ORDER(SXA_2),
    ADDRESS(CSIALRM),
    /* ALMXIT */
    ORDER(LXC_1),
    ADDRESS(CSIALRM),
    ORDERS(SLOAD_STAR, EXIT),
    ADDRESS_PLUS_ONE_AT_X1(ALARM_TB, -1),
};

/* ---- P33 and P73 -- pp. 627-628 ---- */

/* P33: AVFLAGA, the LM the active vehicle, then on at P33/P73A. */
void p33(void)
{
    tc_at(P33);
    avflaga();
}

void p33_after_avflaga(void)
{
    tc_at(P33 + 1u);
    p33_p73a();
}

/* P73: AVFLAGP, the CSM the active vehicle. */
void p73(void)
{
    tc_at(P73);
    avflagp();
}

/* P33/P73A: P20FLGON, */
void p33_p73a(void)
{
    tc_at(P33_P73A);
    p20flgon();
}

/* then TCDH, "TIME OF THE CDH MANEUVER", asked for: verb 06 noun 13
 * (VNPOOH). */
void p33_p73a_after_p20flgon(void)
{
    set_A(V06N13);
    tc_to(P33_P73A + 2u, VNPOOH);
}

/* P33/P73A +3: to the interpreter, for P33_P73A_CODE. */
void p33_p73a_after_vnpooh(void)
{
    tc_at(P33_P73A + 3u);
    intpret();
}

/* P33_P73A_CODE: TTPI from TTPIO, TIG TCDH (SELECTMU). P33/P73B: ADVANCE,
 * CDHMVR (the CDH maneuver, bank 34), the active vehicle's state after it
 * (VACT3 and RACT2 through INTINT3P and ACTIVE) and the passive vehicle's
 * (VPASS2, RPASS2, INTINT3P, PASSIVE). NOMTPI +0 and ITSWICH set,
 * S33/34.1 (P34-35_P74-75.agc) iterates on the TPI time: with MPAC back
 * zero (it ends at TIMEX, ZEROVECS), on at P33/P73C; else (too many
 * iterations, or no solution) alarm 611 (EXIT). */
static const word p33_p73a_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPIO),
    STODL(TTPI),
    ADDRESS_PLUS_ONE(TCDH),
    STCALL(TIG),
    ADDRESS(SELECTMU),
    /* P33/P73B */
    ORDER(CALL),
    ADDRESS(ADVANCE),
    ORDER(CALL),
    ADDRESS(CDHMVR),
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VACT3),
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(RACT2),
    ADDRESS(INTINT3P),
    ORDER(CALL),
    ADDRESS(ACTIVE),
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VPASS2),
    ORDERS(PDVL, CALL),
    ADDRESS_PLUS_ONE(RPASS2),
    ADDRESS(INTINT3P),
    ORDER(CALL),
    ADDRESS(PASSIVE),
    ORDERS(DLOAD, SET),
    ADDRESS_PLUS_ONE(P30ZERO),
    SWITCH(SET, ITSWICH),
    STCALL(NOMTPI),
    ADDRESS(S33_34_1),
    ORDERS(BZE, EXIT),
    ADDRESS(P33_P73C),
};

/* After that EXIT: alarm 611 (ALARM, OCT 611), */
void p33_p73_alarm(void)
{
    tc_to(P33_P73B + 033u, ALARM); /* OCT 611 */
}

/* and verb 05 noun 09 flashed (GOFLASH). */
void p33_p73_after_alarm(void)
{
    set_A(V05N09);
    tc_to(P33_P73B + 036u, BANKCALL); /* CADR GOFLASH */
}

/* P33/P73B +040: terminate. */
void p33_p73_terminate(void)
{
    tc_to(P33_P73B + 040u, GOTOPOOH);
}

/* P33/P73B +041: proceed (TC +2), on with NOMTPI zero. */
void p33_p73_proceed(void)
{
    tc_at(P33_P73B + 041u);
    p33_p73_after_proceed();
}

/* P33/P73B +042: recycle, TCDH asked for again (TC P33/P73A). */
void p33_p73_recycle(void)
{
    tc_at(P33_P73B + 042u);
    p33_p73a();
}

/* P33/P73B +043: to the interpreter, for P33_P73_ALARM_CODE. */
void p33_p73_after_proceed(void)
{
    tc_at(P33_P73B + 043u);
    intpret();
}

/* P33_P73_ALARM_CODE: NOMTPI zero. P33/P73C: with FINALFLG clear, the
 * update flag set. P33/P73D: TTPI moved by NOMTPI, "DELTA TIME FROM
 * NOMINAL TPI TO COMPUTED TPI"; T1TOT2, TTPI less TCDH taken down by
 * 60MIN while it stays positive (P33/P73E) and 60MIN back on; T2TOT3, TTPI
 * less TTPIO, its size taken down the same way (P33/P73F) and its sign
 * put back (SIGN, STADR), for the display. */
static const word p33_p73_alarm_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(P30ZERO),
    STORE(NOMTPI),
    /* P33/P73C */
    ORDERS(BON, SET),
    SWITCH(BON, FINALFLG),
    ADDRESS(P33_P73D),
    SWITCH(SET, UPDATFLG),
    /* P33/P73D */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(NOMTPI),
    ADDRESS_PLUS_ONE(TTPI),
    STORE(TTPI),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(TCDH),
    /* P33/P73E */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(60MIN),
    ADDRESS(P33_P73E),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(60MIN),
    STODL(T1TOT2),
    ADDRESS_PLUS_ONE(TTPI),
    ORDERS(DSU, PUSH),
    ADDRESS_PLUS_ONE(TTPIO),
    /* P33/P73F */
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(60MIN),
    ORDERS(BPL, DAD),
    ADDRESS(P33_P73F),
    ADDRESS_PLUS_ONE(60MIN),
    ORDERS(SIGN, STADR),
    AFTER_STADR(STORE(T2TOT3)),
    ORDER(EXIT),
};

/* P33/P73F +010: verb 06 noun 75 (VNPOOH), "DELTA ALTITUDE CDH" and the
 * two times. */
void p33_p73_n75(void)
{
    set_A(V06N75);
    tc_to(P33_P73F + 011u, VNPOOH);
}

/* P33/P73F +012: to the interpreter, for P33_P73_N75_CODE. */
void p33_p73_n75_after_vnpooh(void)
{
    tc_at(P33_P73F + 012u);
    intpret();
}

/* P33_P73_N75_CODE: DELVEET2 (S32/33.1), VN1645, and again from
 * P33/P73B. */
static const word p33_p73_n75_words[] = {
    /* the delta V at CDH */
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(DELVEET2),
    ADDRESS(S32_33_1),
    STCALL(DELVEET2),
    ADDRESS(VN1645),
    /* and again */
    ORDER(GOTO),
    ADDRESS(P33_P73B),
};

/* ---- AVFLAGA, AVFLAGP and P20FLGON -- p. 629 ---- */

/* AVFLAGA: "AVFLAG = LEM": the return in SUBEXIT (QXCH), AVFLAG set
 * (UPFLAG), and back through SUBEXIT. */
void avflaga(void)
{
    qxch(ADDRESS_OF(SUBEXIT));   /* EXTEND; QXCH */
    tc_to(AVFLAGA + 2u, UPFLAG); /* ADRES AVFLAG */
}

void avflaga_after_upflag(void)
{
    tc_at(AVFLAGA + 4u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* AVFLAGP: "AVFLAG = CSM" (DOWNFLAG). */
void avflagp(void)
{
    qxch(ADDRESS_OF(SUBEXIT));     /* EXTEND; QXCH */
    tc_to(AVFLAGP + 2u, DOWNFLAG); /* ADRES AVFLAG */
}

void avflagp_after_downflag(void)
{
    tc_at(AVFLAGP + 4u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* P20FLGON: "SET UPDATFLG", "SET TRACKFLG" (UPFLAG twice). */
void p20flgon(void)
{
    qxch(ADDRESS_OF(SUBEXIT));    /* EXTEND; QXCH */
    tc_to(P20FLGON + 2u, UPFLAG); /* ADRES UPDATFLG */
}

void p20flgon_after_updatflg(void)
{
    tc_to(P20FLGON + 4u, UPFLAG); /* ADRES TRACKFLG */
}

void p20flgon_after_trackflg(void)
{
    tc_at(P20FLGON + 6u);
    tc_return_in(ADDRESS_OF(SUBEXIT));
}

/* ---- DISDVLVC -- p. 630 ---- */

/* DISDVLVC, CALLed with the delta V (reference coordinates) in MPAC and
 * its verb and noun in X1: the delta V to DELVLVC, the return to NORMEX;
 * S32/33.X leaves the local vertical frame's matrix at 0D, and DELVLVC
 * times it (MXV), doubled, is the delta V in local vertical coordinates;
 * the verb and noun to VERBNOUN; EXIT. */
static const word disdvlvc_words[] = {
    /* DISDVLVC */
    STORE(DELVLVC),
    ORDERS(STQ, CALL),
    ADDRESS(NORMEX),
    ADDRESS(S32_33_X),
    /* in local vertical coordinates */
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(DELVLVC),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(VSL1, SXA_1),
    ADDRESS(VERBNOUN),
    STORE(DELVLVC),
    ORDER(EXIT),
};

/* DISDVLVC +013: shown, verb and noun from VERBNOUN (VNPOOH), */
void disdvlvc_after_exit(void)
{
    ca(ADDRESS_OF(VERBNOUN));
    tc_to(DISDVLVC + 014u, VNPOOH);
}

/* DISDVLVC +015: and back through NORMEX (DISDVLVC_EXIT_CODE). */
void disdvlvc_after_vnpooh(void)
{
    tc_at(DISDVLVC + 015u);
    intpret();
}

static const word disdvlvc_exit_words[] = {
    ORDER(GOTO),
    ADDRESS(NORMEX),
};

/* ---- COMPTGO -- p. 650 ---- */

/* COMPTGO: the return in RTRN (QXCH), DISPDEX zero (A +0, from CAF ZERO,
 * has no overflow for a TS to skip on), and CLOKTASK in 2 centiseconds
 * (WAITLIST, interrupts inhibited), */
void comptgo(void)
{
    qxch(ADDRESS_OF(RTRN)); /* EXTEND; QXCH */
    set_A(ZERO);
    ts(ADDRESS_OF(DISPDEX));
    set_A(BIT2);
    inhint();
    tc_to(COMPTGO + 6u, WAITLIST); /* 2CADR CLOKTASK */
}

/* then the restart phase (2PHSCHNG: OCT 40036, OCT 05024, OCT 13000), */
void comptgo_after_waitlist(void)
{
    tc_to(COMPTGO + 011u, TWO_PHSCHNG);
}

/* and back through RTRN. */
void comptgo_after_2phschng(void)
{
    tc_at(COMPTGO + 015u);
    tc_return_in(ADDRESS_OF(RTRN));
}

/* ---- The interpretive code ---- */

const struct interpretive_code ALMXITA_CODE = {035u, ALMXIT - 2u, COUNT(almxita_words),
                                               almxita_words};
const struct interpretive_code VN0611_CODE = {035u, VN0611 + 3u, COUNT(vn0611_words), vn0611_words};
const struct interpretive_code VN0655_CODE = {035u, VN0655, COUNT(vn0655_words), vn0655_words};
const struct interpretive_code VN0637_CODE = {035u, VN0655 + 012u, COUNT(vn0637_words),
                                              vn0637_words};
const struct interpretive_code P32_P72_N75_CODE = {035u, P32_P72F + 010u, COUNT(p32_p72_n75_words),
                                                   p32_p72_n75_words};
const struct interpretive_code P33_P73A_CODE = {035u, P33_P73A + 4u, COUNT(p33_p73a_words),
                                                p33_p73a_words};
const struct interpretive_code P33_P73_ALARM_CODE = {
    035u, P33_P73B + 044u, COUNT(p33_p73_alarm_words), p33_p73_alarm_words};
const struct interpretive_code P33_P73_N75_CODE = {035u, P33_P73F + 013u, COUNT(p33_p73_n75_words),
                                                   p33_p73_n75_words};
const struct interpretive_code DISDVLVC_CODE = {035u, DISDVLVC, COUNT(disdvlvc_words),
                                                disdvlvc_words};
const struct interpretive_code DISDVLVC_EXIT_CODE = {
    035u, DISDVLVC + 016u, COUNT(disdvlvc_exit_words), disdvlvc_exit_words};

#endif
