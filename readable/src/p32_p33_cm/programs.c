/* SPDX-License-Identifier: MIT */
/* P32-P33_P72-P73.agc (Comanche055 pp. 649-662): the programs' own code.
 * P32 and P72, "COELLIPTIC SEQUENCE INITIATION (CSI) PROGRAMS", and P33
 * and P73, "CONSTANT DELTA HEIGHT (CDH) PROGRAMS" (bank 4, with a stretch
 * of each in banks 14 and 12); DISDVLVC, which shows a delta V in local
 * vertical coordinates (bank 37).
 *
 * Each "IS SELECTED BY THE ASTRONAUT BY DSKY ENTRY": P32 or P33 "IF THIS
 * VEHICLE IS ACTIVE VEHICLE", P72 or P73 "IF THIS VEHICLE IS PASSIVE
 * VEHICLE". The code matches the LM's (src/p32_p35/programs.c) but for
 * the CM's own: P32STRT and P32/P72A zero CENTANG and NN in interpretive
 * code; TCSI is only keyed in (no time of apocenter when it is zero); the
 * conic's angle is 2PISC; and the SETLOCs move P32/P72E to bank 14 and
 * P33/P73C and P33/P73D to bank 12.
 *
 * Each call out of the file (AVFLAGA, AVFLAGP, P20FLGON, VNPOOH,
 * BANKCALL, ALARM, VARALARM) leaves Z at the routine and Q at the word
 * after the TC (after its CADR or its OCT), where the routine comes back
 * to; GOTOPOOH doesn't come back. A display through GOFLASH comes back to
 * the word after its CADR on terminate, the next on proceed and the next
 * on enter (recycle). VNPOOH comes back on proceed only. */
#include "p32_p33_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P32 and P72 -- pp. 652-655 ---- */

/* P32: AVFLAGA, the CM the active vehicle, then on at P32STRT. */
void p32(void)
{
    tc_to(P32, AVFLAGA);
}

void p32_after_avflaga(void)
{
    tc_at(P32 + 1u);
    p32strt();
}

/* P72: AVFLAGP, the LM the active vehicle, and on at P32STRT. */
void p72(void)
{
    tc_to(P72, AVFLAGP);
}

/* P32STRT: to the interpreter, for P32STRT_CODE: CENTANG zero. */
void p32strt(void)
{
    tc_at(P32STRT);
    intpret();
}

static const word p32strt_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(CENTANG),
    ORDER(EXIT),
};

/* After its EXIT, on at P32/P72A. */
void p32strt_after_exit(void)
{
    tc_at(P32STRT + 5u);
    p32_p72a();
}

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
    tc_to(P32_P72A, P20FLGON);
}

/* then to the interpreter, for P32_P72A_CODE: NN zero. */
void p32_p72a_after_p20flgon(void)
{
    tc_at(P32_P72A + 1u);
    intpret();
}

static const word p32_p72a_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(NN),
    ORDER(EXIT),
};

/* After its EXIT: TCSI, "TIME OF THE CSI MANEUVER", asked for: verb 06
 * noun 11 (VNPOOH). */
void vn0611(void)
{
    set_A(V06N11);
    tc_to(P32_P72A + 7u, VNPOOH);
}

/* On proceed: verb 06 noun 55 flashed, NN and ELEV (GOFLASH). */
void vn0655(void)
{
    set_A(V06N55);
    tc_to(P32_P72A + 011u, BANKCALL); /* CADR GOFLASH */
}

/* P32/P72A +013: terminate. */
void vn0655_terminate(void)
{
    tc_to(P32_P72A + 013u, GOTOPOOH);
}

/* P32/P72A +014: proceed, on to TTPI (TC +2). */
void vn0655_proceed(void)
{
    tc_at(P32_P72A + 014u);
    vn0637();
}

/* P32/P72A +015: recycle, the display again (TC -5). */
void vn0655_recycle(void)
{
    tc_at(P32_P72A + 015u);
    vn0655();
}

/* P32/P72A +016: TTPI, "TIME OF THE TPI MANEUVER", verb 06 noun 37
 * (VNPOOH). */
void vn0637(void)
{
    set_A(V06N37);
    tc_to(P32_P72A + 017u, VNPOOH);
}

/* P32/P72A +020: to the interpreter, for VN0637_CODE. */
void vn0637_after_vnpooh(void)
{
    tc_at(P32_P72A + 020u);
    intpret();
}

/* VN0637_CODE: TIG is TCSI (SELECTMU picks the moon's or the earth's mu).
 * P32/P72B: ADVANCE; the passive vehicle's state at TCSI, and TTPI with
 * 2PISC (doubled, SL2) on the push-down list, through INTINT and PASSIVE;
 * then CSI/A, the computation. P32/P72C: with FINALFLG clear, the update
 * flag set. P32/P72D: T1TOT2 to bank 14's P32/P72E. */
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
    ADDRESS_PLUS_ONE(2PISC),
    ORDERS(SL2, PUSH),
    ORDER(CALL),
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
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(T1TOT2),
    ADDRESS(P32_P72E),
    /* P32/P72F: T2TOT3 less 60MIN while it stays positive, and EXIT. */
    STORE(T2TOT3),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(60MIN),
    ADDRESS(P32_P72F),
    ORDER(EXIT),
};

/* P32/P72E (bank 14): T1TOT2 and T2TOT3, "DELTA TIME FROM CSI TO CDH" and
 * "DELTA TIME FROM CDH TO TPI", each less 60MIN (360,000 centiseconds)
 * while it stays positive, for the display: T1TOT2 here, then T2TOT3 back
 * at P32/P72F. */
static const word p32_p72e_words[] = {
    /* P32/P72E */
    STORE(T1TOT2),
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(60MIN),
    ADDRESS(P32_P72E),
    /* then T2TOT3 */
    ORDERS(DLOAD, GOTO),
    ADDRESS_PLUS_ONE(T2TOT3),
    ADDRESS(P32_P72F),
};

/* P32/P72F +5: verb 06 noun 75 (VNPOOH), the delta altitude and the two
 * times, T1TOT2 and T2TOT3. */
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
 * X1), TTPI kept in TTPIO for a recycle, VN1645, and again from
 * P32/P72B. */
static const word p32_p72_n75_words[] = {
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(DELVEET1),
    ADDRESS(S32_33_1),
    STOVL(DELVEET1),
    ADDRESS_PLUS_ONE(RACT2),
    STOVL(RACT1),
    ADDRESS_PLUS_ONE(DELVEET2),
    ORDERS(AXT_1, CALL),
    NUMBER(06u * 0200u + 82u), /* VN 0682: verb 06, noun 82 */
    ADDRESS(DISDVLVC),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TTPI),
    STCALL(TTPIO),
    ADDRESS(VN1645),
    ORDER(GOTO),
    ADDRESS(P32_P72B),
};

/* ---- P33 and P73 -- pp. 658-659 ---- */

/* P33: AVFLAGA, the CM the active vehicle, then on at P33/P73A. */
void p33(void)
{
    tc_to(P33, AVFLAGA);
}

void p33_after_avflaga(void)
{
    tc_at(P33 + 1u);
    p33_p73a();
}

/* P73: AVFLAGP, the LM the active vehicle. */
void p73(void)
{
    tc_to(P73, AVFLAGP);
}

/* P33/P73A: P20FLGON, */
void p33_p73a(void)
{
    tc_to(P33_P73A, P20FLGON);
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
 * CDHMVR (the CDH maneuver), the active vehicle's state after it (VACT3
 * and RACT2 through INTINT3P and ACTIVE) and the passive vehicle's
 * (VPASS2, RPASS2, INTINT3P, PASSIVE). NOMTPI +0 and ITSWICH set,
 * S33/34.1 iterates on the TPI time: with MPAC back zero, on at bank 12's
 * P33/P73C; else (too many iterations, or no solution) alarm 611
 * (EXIT). */
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
    ADDRESS_PLUS_ONE(ZEROVEC),
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

/* P33_P73_ALARM_CODE: NOMTPI zero, and to bank 12's P33/P73C. */
static const word p33_p73_alarm_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STCALL(NOMTPI),
    ADDRESS(P33_P73C),
};

/* P33/P73C (bank 12): with FINALFLG clear, the update flag set.
 * P33/P73D: TTPI moved by NOMTPI, "DELTA TIME FROM NOMINAL TPI TO
 * COMPUTED TPI"; TTPI less TCDH, to bank 4's P33/P73E. */
static const word p33_p73c_words[] = {
    ORDERS(BON, SET),
    SWITCH(BON, FINALFLG),
    ADDRESS(P33_P73D),
    SWITCH(SET, UPDATFLG),
    /* P33/P73D */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(NOMTPI),
    ADDRESS_PLUS_ONE(TTPI),
    STORE(TTPI),
    ORDERS(DSU, GOTO),
    ADDRESS_PLUS_ONE(TCDH),
    ADDRESS(P33_P73E),
};

/* P33/P73E: T1TOT2, that taken down by 60MIN while it stays positive and
 * 60MIN back on; T2TOT3, TTPI less TTPIO, its size taken down the same
 * way (P33/P73F) and its sign put back (SIGN, STADR), for the display. */
static const word p33_p73e_words[] = {
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

/* P33/P73F +010: verb 06 noun 75 (VNPOOH), the delta altitude and the
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

/* ---- DISDVLVC -- p. 662 ---- */

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

/* ---- The interpretive code ---- */

const struct interpretive_code P32STRT_CODE = {04u, P32STRT + 1u, COUNT(p32strt_words),
                                               p32strt_words};
const struct interpretive_code ALMXITA_CODE = {04u, ALMXIT - 2u, COUNT(almxita_words),
                                               almxita_words};
const struct interpretive_code P32_P72A_CODE = {04u, P32_P72A + 2u, COUNT(p32_p72a_words),
                                                p32_p72a_words};
const struct interpretive_code VN0637_CODE = {04u, P32_P72A + 021u, COUNT(vn0637_words),
                                              vn0637_words};
const struct interpretive_code P32_P72E_CODE = {014u, I_P32_P72E % 02000u + 02000u,
                                                COUNT(p32_p72e_words), p32_p72e_words};
const struct interpretive_code P32_P72_N75_CODE = {04u, P32_P72F + 010u, COUNT(p32_p72_n75_words),
                                                   p32_p72_n75_words};
const struct interpretive_code P33_P73A_CODE = {04u, P33_P73A + 4u, COUNT(p33_p73a_words),
                                                p33_p73a_words};
const struct interpretive_code P33_P73_ALARM_CODE = {
    04u, P33_P73B + 044u, COUNT(p33_p73_alarm_words), p33_p73_alarm_words};
const struct interpretive_code P33_P73C_CODE = {012u, I_P33_P73C % 02000u + 02000u,
                                                COUNT(p33_p73c_words), p33_p73c_words};
const struct interpretive_code P33_P73E_CODE = {04u, I_P33_P73E % 02000u + 02000u,
                                                COUNT(p33_p73e_words), p33_p73e_words};
const struct interpretive_code P33_P73_N75_CODE = {04u, P33_P73F + 013u, COUNT(p33_p73_n75_words),
                                                   p33_p73_n75_words};
const struct interpretive_code DISDVLVC_CODE = {037u, DISDVLVC, COUNT(disdvlvc_words),
                                                disdvlvc_words};
const struct interpretive_code DISDVLVC_EXIT_CODE = {
    037u, DISDVLVC + 016u, COUNT(disdvlvc_exit_words), disdvlvc_exit_words};

#endif
