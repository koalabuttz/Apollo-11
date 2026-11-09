/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part E (Comanche055 pp. 1081-1085, bank
 * 15): SETJTAG and the 2 s roll system's phase plane, CM/RCS.
 *
 * "RCS		THIS SECTION IS ENTERED EACH 2 SEC BY WAITLIST CALL FOLLOWING A
 * DELAY OF 1.2 SEC AFTER PIPUP." SETJTAG sets JETAG, and on the next 0.1 s
 * pass EXDAPIN goes on to CM/RCS.
 *
 * "PREDICTIVE ROLL SYSTEM	ENTRY STEERING PROVIDES ROLL COMMAND IN LOC
 * ROLLC.  THE FOLLOWING CALCULATES THE TRAJECTORY TO THE ORIGIN IN PHASE
 * PLANE (X,V)." X is the roll error, LCX/360, and V the roll rate, -VT/180
 * (from PREL). CM/RCS takes the shortest way round to ROLLC (or over the
 * top, while LATSW is clear), allowing for the turn it coordinates
 * (COMPAT, by COS ALFA); then WHICHALF finds the half plane: a point to
 * the left is reflected into the right (REFLECT), a point inside the dead
 * zones fires nothing (DZCALL), and the others get two burns, TON1 to the
 * drifting velocity (VDT/180, or the most, VMT/180) and TON2 back to rest,
 * with JNDX and JNDX1 their jets' directions. GETOFF (part F) times them. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void getlcx(void);
static void compat_errors(void);
static void whichalf(void);
static void maxvtest(void);
static void geton1(void);
static void geton2(void);

/* ---- SETJTAG -- p. 1081 ---- */

/* SETJTAG: "SAVE NOMINAL UPDATE TIME FOR SYNCH" (-TIME1 in TUSED), and 1
 * to JETAG ("RATHER THAN INCR, FOR SAFETY"), "TO CAUSE CM/RCS TO BE"
 * "EXECUTED AFTER NEXT BODYRATE UPDATE"; the phase (PHASCHNG, OCT
 * 00001). */
void setjtag(void)
{
    cs(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TUSED));
    set_A(ONE); /* CA ONE */
    ts(ADDRESS_OF(JETAG));
    tc_to(SETJTAG + 4u, PHASCHNG);
}

/* SETJTAG +6: the task ends. */
void setjtag_taskover(void)
{
    tc_to(SETJTAG + 6u, TASKOVER);
}

/* ---- CM/RCS -- pp. 1081-1082 ---- */

/* CM/RCS: "SET NDX FOR POS ROLL, AND CHANGE LATER" (JNDX -1); the roll
 * rate, "ROLLDOT = DELAOG + DELAIG SINM =DELR", from PREL: "-2 RDOT T/180
 * IN L", added in, "SAVE -2VT/180 HERE" (-VT/180); "SAVE (-R/180) /2" in
 * SR (an overflow at TS -VT/180 skips the CS ROLL/180: A +1 or -1). Then
 * LATSW: "ROLL OVER TOP $" "NO, TAKE SHORTEST PATH" (GETLCX); "YES,
 * ENFORCE ROLL OVER TOP.. (BIT =0)": the error "(ROLLC/180) /2" "-(R/180)
 * /2" to LCX/360, "DIFFERENT X REQD HERE.  DISCONT AT 180.", and on at
 * COMPAT (the faithful port's run stops there, at a key of another of its
 * parts). */
void cm_rcs(void)
{
    set_A(negate(ONE)); /* CS ONE */
    ts(ADDRESS_OF(JNDX));
    set_A(negate(TWO_T_TCDU)); /* CS 2T/TCDU */
    mp_at(ADDRESS_OF(PREL));   /* EXTEND; MP PREL */
    ad_at(L_ADDRESS);
    if (!ts(ADDRESS_OF(MVT_180))) cs(ADDRESS_OF(ROLL_180));
    ts(SR_ADDRESS);
    cs(ADDRESS_OF(CM_FLAGS));
    mask(BIT4);
    if (A_is_zero()) { /* EXTEND; BZF GETLCX */
        getlcx();
        return;
    }
    ads(ADDRESS_OF(CM_FLAGS));
    ca(ADDRESS_OF(ROLLC));
    ad_at(SR_ADDRESS);
    xch(ADDRESS_OF(LCX_360));
    continue_at(COMPAT); /* "POSSIBLE OVFL ABOVE." */
}

/* GETLCX: "FORM RCOM/360" (POS1/2 doubled, and ROLLC), "IGNORE POSSIBLE
 * OVFL."; "FORM -R/360" (SR, and NEG1/2 twice, "IGNORE OVFL"), and the
 * two summed: "LCX/360 = RCOM/360 -R/360  RANGE (-1,1)". Then "TAKE
 * SHORTEST ANGULAR PATH" "(BASED ON SINGLE JET ACCELERATION)": CCS
 * -VT/180, on at GETLCX +012 to +015 (keys of another of the faithful
 * port's parts). */
static void getlcx(void)
{
    set_A(POS1_2); /* CA */
    double_A();
    ad_at(ADDRESS_OF(ROLLC));
    xch(ADDRESS_OF(LCX_360));
    ca(SR_ADDRESS);
    ad(NEG1_2);
    ad(NEG1_2);
    xch(ADDRESS_OF(LCX_360));
    ads(ADDRESS_OF(LCX_360));
    switch (ccs_at(ADDRESS_OF(MVT_180))) {
    case POSITIVE: continue_at(GETLCX + 012u); break;
    case IS_PLUS_ZERO: continue_at(GETLCX + 013u); break;
    case NEGATIVE: continue_at(GETLCX + 014u); break;
    case IS_MINUS_ZERO: continue_at(GETLCX + 015u); break;
    }
}

/* GETLCX +012: -VT/180 positive: its magnitude (less 1, after CCS) plus
 * 1, */
void cm_rcs_vt_positive(void)
{
    ad(ONE);
    cm_rcs_vt_plus0();
}

/* GETLCX +013: TCF +2 (+0 for a -VT/180 of +0), */
void cm_rcs_vt_plus0(void)
{
    cm_rcs_shortest();
}

/* GETLCX +014: -VT/180 negative: plus 1, */
void cm_rcs_vt_negative(void)
{
    ad(ONE);
    cm_rcs_shortest();
}

/* GETLCX +015: "DOES SGN(-VT) (VT/180) (VT/180) (180/(4 A1 TT COSALFA)) +
 * X/360 + SGN(X) / 2	  OVFL ?": the magnitude times -VT/180, "C(-VT/180)
 * = -2 VT/180", times 1/16A1, "= 180/(16 A1 TT)", over CALFA, to L; then
 * POS1/2 by the sign of LCX/360 (+0 for either zero; CS POS1/2 for a
 * negative one, at GETLCX +027). */
void cm_rcs_shortest(void)
{
    mp_at(ADDRESS_OF(MVT_180)); /* EXTEND; MP -VT/180 */
    mp(ONE_16A1);               /* EXTEND; MP 1/16A1 */
    dv_at(ADDRESS_OF(CALFA));   /* EXTEND; DV CALFA */
    ts(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(LCX_360))) {
    case POSITIVE: set_A(POS1_2); break; /* CAF POS1/2; TCF +2 */
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    case NEGATIVE: cm_rcs_cs_pos1_2(); return;
    }
    cm_rcs_ad_lcx();
}

/* GETLCX +027: CS POS1/2, */
void cm_rcs_cs_pos1_2(void)
{
    set_A(negate(POS1_2));
    cm_rcs_ad_lcx();
}

/* GETLCX +030: LCX/360 and L added: "IS LCX/360 LESS THAN 180 DEGS  $"
 * (TS L skips on an overflow): "YES, GO ON." (COMPAT); else TRTAGXPI,
 * "NO, SHIFT X BY - SGN(X) 2 PI": INDEX A on the overflow's +1 or -1, CS
 * HALFPR ("+A YIELDS -1/2"), doubled, into LCX/360; then COMPAT. */
void cm_rcs_ad_lcx(void)
{
    ad_at(ADDRESS_OF(LCX_360));
    ad_at(L_ADDRESS);
    if (ts(L_ADDRESS)) {
        index_a_cs(HALFPR); /* TRTAGXPI: INDEX A; CS HALFPR */
        double_A();
        ads(ADDRESS_OF(LCX_360));
    }
    compat();
}

/* ---- COMPAT and WHICHALF -- pp. 1082-1083 ---- */

/* COMPAT: "CORRECT FOR ASSUMED COORD TURN." (LCX/360 times CALFA, "COS
 * ALFA", "SCALED LCX OK HERE."). CMDAPMOD, "FOUR POSSIBILITIES HERE": +1,
 * "EXIT, SETTING JETAG=0. (C(A)=0)" "ALL 3 AXES ALREADY DONE." (TC
 * DZCALL1); +0, "G LESS THAN .05.  CA POS. CONTINUE" (TC +1); -1, "G GEQ
 * .05.  CONTINUE IN CM/RCS"; -0, "CMDAPMOD=-0.  DAMPING ONLY. SET LCX=0"
 * (+0 from the CCS). */
void compat(void)
{
    ca(ADDRESS_OF(LCX_360));
    mp_at(ADDRESS_OF(CALFA)); /* EXTEND; MP CALFA */
    ts(ADDRESS_OF(LCX_360));
    switch (ccs_at(ADDRESS_OF(CMDAPMOD))) {
    case POSITIVE: tc_to(COMPAT + 5u, DZCALL1); break;
    case IS_PLUS_ZERO: compat_tc(); break;
    case NEGATIVE: compat_ca(); break;
    case IS_MINUS_ZERO: compat_errors(); break;
    }
}

/* COMPAT +6: TC +1, */
void compat_tc(void)
{
    tc_at(COMPAT + 6u);
    compat_ca();
}

/* COMPAT +7: CA LCX/360, */
void compat_ca(void)
{
    ca(ADDRESS_OF(LCX_360));
    compat_errors();
}

/* COMPAT +010: the error to LCX/360, ERRORZ, "INITIAL ROLL ERROR
 * (UNREFLECTED) FOR TM.", and PAXERR1, "SAVE LCX FOR FDAI AND EDIT.
 * (/360)". "GET - 2 VT/180" through SR, "GET -VT/180, LEAVE -VT/360 IN SR
 * FOR DZ" (and -VT/180E, "DIAGNOSTIC ****"); "NOW CONTENTS OF -VT/180 AS
 * LABELED" (XCH); its square, "B(A) = -ZVT/180", times 180/8ATT, to
 * VSQ/4API; on at WHICHALF. */
static void compat_errors(void)
{
    ts(ADDRESS_OF(LCX_360));
    ts(ADDRESS_OF(ERRORZ));
    ts(ADDRESS_OF(PAXERR1));
    ca(ADDRESS_OF(MVT_180));
    ts(SR_ADDRESS);
    ca(SR_ADDRESS);
    ts(ADDRESS_OF(MVT_180E));
    xch(ADDRESS_OF(MVT_180));
    mp_at(ADDRESS_OF(MVT_180)); /* EXTEND; MP -VT/180 */
    mp(K180_8ATT);              /* EXTEND; MP 180/8ATT */
    ts(ADDRESS_OF(VSQ_4API));
    whichalf();
}

/* WHICHALF: "IS SGN(VT) ( (180/4A1 TT) VT/180 VT/180 - .5 BUFLIM/360 )
 * -X/360 - .5 BUFLIM/360  POS?" ("FOR SECOND BURN, A1"): VSQ/4API
 * doubled, complemented, BUFLIM added, to L (an overflow skips the CCS:
 * CS L), signed by -VT/180 (+0 for either zero), LCX/360 and BUFLIM
 * added: "POINT (X,V) IN LHP." (BZMF REFLECT). "IS SGN(VT) ( (180/4A1 TT)
 * VT/180 VT/180 - .5 BUFLIM/360 ) -X/360 + .5 BUFLIM/360  NEG?": "POINT
 * (X,V) IN RHP." (BZMF DZ1, a key of another of the faithful port's
 * parts). "IS POINT WITHIN VELOCITY DZ?": "IS VSQ/4API - (VSQ/4API) MIN
 * NEG?" (BZMF DZCALL, part F). Else "POINT IS IN BUFFER ZONE.  THRUST TO
 * X AXIS.": JNDX complemented to JNDX1, and OVRLINE1 (its run stops there
 * too). */
static void whichalf(void)
{
    enum sign vt = POSITIVE;

    double_A();
    com();
    ad(BUFLIM);
    if (!ts(L_ADDRESS)) vt = ccs_at(ADDRESS_OF(MVT_180));
    if (vt == POSITIVE) cs(L_ADDRESS);
    if (vt == NEGATIVE) ca(L_ADDRESS);
    ad_at(ADDRESS_OF(LCX_360));
    ad(BUFLIM);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF REFLECT */
        reflect();
        return;
    }
    com();
    ad(BUFLIM);
    ad(BUFLIM);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DZ1 */
        continue_at(DZ1);
        return;
    }
    set_A(negate(VSQMIN)); /* CS VSQMIN */
    ad_at(ADDRESS_OF(VSQ_4API));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DZCALL */
        continue_at(DZCALL);
        return;
    }
    cs(ADDRESS_OF(JNDX));
    ts(ADDRESS_OF(JNDX1));
    tc_to(REFLECT - 1u, OVRLINE1);
}

/* ---- REFLECT, DZ1, DZ2 and MAXVTEST -- pp. 1083-1084 ---- */

/* REFLECT: "RELFECT LHP INTO RHP REL TO TERM CONTR": CS -VT/180, and on
 * at REFLECT +1 (the faithful port's run stops there, at a key of another
 * of its parts). */
void reflect(void)
{
    cs(ADDRESS_OF(MVT_180));
    continue_at(REFLECT + 1u);
}

/* REFLECT +1: to -VT/180 (an overflow skips the TS SR: A +1 or -1), */
void reflect_ts(void)
{
    if (!ts(ADDRESS_OF(MVT_180))) {
        reflect_ts_sr();
        return;
    }
    cs(ADDRESS_OF(LCX_360));
    ts(ADDRESS_OF(LCX_360));
    reflect_jndx();
}

/* REFLECT +2: "-VT/360 SAVED FOR DZ." (SR; an overflow skips the CS
 * LCX/360); LCX/360 complemented, */
void reflect_ts_sr(void)
{
    if (!ts(SR_ADDRESS)) cs(ADDRESS_OF(LCX_360));
    ts(ADDRESS_OF(LCX_360));
    reflect_jndx();
}

/* REFLECT +5: and JNDX. */
void reflect_jndx(void)
{
    cs(ADDRESS_OF(JNDX));
    ts(ADDRESS_OF(JNDX));
    dz1();
}

/* DZ1: "IS VSQ/4API - (VSQ/4API) MIN NEG  $" "YES, GO TEST FURTHER."
 * (DZ2) "NO" (MAXVTEST). DZ2: "IS X/360 - XMIN/360 -VT/360 NEG  $" ("C(SR)
 * = -VT/360"): "YES, IN DZ. EXIT SETTING JETAG=0." (DZCALL, part F); else
 * MAXVTEST. */
void dz1(void)
{
    set_A(negate(VSQMIN)); /* CS VSQMIN */
    ad_at(ADDRESS_OF(VSQ_4API));
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF DZ2; TCF MAXVTEST */
        maxvtest();
        return;
    }
    set_A(negate(XMIN_360)); /* DZ2: CS XMIN/360, "XMIN/360 = 4/360" */
    ad_at(ADDRESS_OF(LCX_360));
    ad_at(SR_ADDRESS);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DZCALL */
        continue_at(DZCALL);
        return;
    }
    maxvtest();
}

/* MAXVTEST: "IS XD/360 - VM/360K - XS/360 POS?": "NOW CAN SET JNDX1 FOR
 * TON2 JETS." (JNDX complemented); then -XS/360, "XS/360 = (XMIN -YMIN/K)
 * /360", */
static void maxvtest(void)
{
    cs(ADDRESS_OF(JNDX));
    ts(ADDRESS_OF(JNDX1));
    set_A(negate(XS_360)); /* CS XS/360 */
    maxvtest_ad();
}

/* MAXVTEST +3: VSQ/4API and LCX/360 added, "XD/360= X/360 +VSQ/4API   X
 * INTERCEPT" "BUT C(XD/360) = (XD - XS) /360" (an overflow skips the AD
 * -VM/360K, "X INTERCEPT FOR MAX V (VM)"); complemented: "YES, THRUST TO
 * VM" (BZMF MAXVTIM1); else XD/360 times KTRCS, doubled, "GO SAVE
 * PREDICTED DRIFTING VELOCITY." (TC GETON1, "INSURE THAT Q IS POS AS
 * TAG."). */
void maxvtest_ad(void)
{
    ad_at(ADDRESS_OF(VSQ_4API));
    ad_at(ADDRESS_OF(LCX_360));
    if (!ts(ADDRESS_OF(XD_360))) ad(MVM_360K);
    com();
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF MAXVTIM1 */
        maxvtim1();
        return;
    }
    ca(ADDRESS_OF(XD_360));
    mp(KTRCS); /* EXTEND; MP KTRCS */
    ddoubl();
    tc_at(MAXVTEST + 016u);
    geton1();
}

/* MAXVTIM1: "SET +Q AS TAG" (EXTEND; ZQ), and -VMT/180's complement, */
void maxvtim1(void)
{
    qxch(7u);                /* EXTEND; ZQ */
    set_A(negate(MVMT_180)); /* CS -VMT/180 */
    geton1();
}

/* ---- GETON1, OVRLINE and GETON2 -- pp. 1084-1085 ---- */

/* GETON1: "VDT/180 OR VMT/180" (an overflow skips the AD -VT/180); with
 * -VT/180 added, doubled, times 180/8ATT: "TON1 / 4T". Not positive,
 * OVRLINE; else "RESET Q POS IF CAME FROM MAXVTIM1" (TC GETON2). */
static void geton1(void)
{
    if (!ts(ADDRESS_OF(VDT_180))) ad_at(ADDRESS_OF(MVT_180));
    double_A();
    mp(K180_8ATT); /* EXTEND; MP 180/8ATT */
    ts(ADDRESS_OF(TON1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF OVRLINE */
        ovrline();
        return;
    }
    tc_at(GETON1 + 010u);
    geton2();
}

/* OVRLINE: CCS Q, the tag: positive, OVRLINE1; +0 (from MAXVTIM1),
 * MAXVTIM2, "ABOVE VM, SO THRUST DOWN": JNDX1 to JNDX (a negative Q's
 * magnitude, less 1, goes to JNDX; -0 skips both), and TON1 complemented
 * (TCF OVRLINE2 +1). */
void ovrline(void)
{
    switch (ccs_at(Q_ADDRESS)) {
    case POSITIVE: ovrline1(); return; /* TCF OVRLINE1 */
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(JNDX1));
        ts(ADDRESS_OF(JNDX));
        break;
    case NEGATIVE: ts(ADDRESS_OF(JNDX)); break;
    case IS_MINUS_ZERO: break;
    }
    cs(ADDRESS_OF(TON1));
    ts(ADDRESS_OF(TON1));
    geton2();
}

/* OVRLINE1: "DRIFT AT V" (-VT/180 complemented to VDT/180); OVRLINE2:
 * TON1 +0; on at GETON2. */
void ovrline1(void)
{
    cs(ADDRESS_OF(MVT_180));
    ts(ADDRESS_OF(VDT_180));
    set_A(ZERO); /* OVRLINE2: CA ZERO */
    ts(ADDRESS_OF(TON1));
    geton2();
}

/* GETON2: "VDT/180, OR VMT/180 OR VT/180", doubled, times 180/8ATT, and
 * doubled again, "FOR SECOND BURN, A1": "= TON2 / 4T" (an overflow skips
 * the COM). Complemented, not positive: on at GETOFF; else TON2 again
 * (an overflow there skips the CA JNDX) and JNDX to JNDX1; on at GETOFF
 * (the faithful port's run stops there, at a key of another of its
 * parts). */
static void geton2(void)
{
    ca(ADDRESS_OF(VDT_180));
    double_A();
    mp(K180_8ATT); /* EXTEND; MP 180/8ATT */
    double_A();
    if (!ts(ADDRESS_OF(TON2))) com();
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF GETOFF */
        continue_at(GETOFF);
        return;
    }
    if (!ts(ADDRESS_OF(TON2))) ca(ADDRESS_OF(JNDX));
    ts(ADDRESS_OF(JNDX1));
    continue_at(GETOFF);
}

#endif
