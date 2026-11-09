/* SPDX-License-Identifier: MIT */
/* P11.agc of the CM (Comanche055 pp. 535-541, bank 34):
 * P11, its restart point REP11, VHHDOT (SERVICER's exit during the boost),
 * ATERTASK and GETDOWN.
 *
 * P11 "WILL ... ZERO CMC CLOCK AT LIFTOFF (OR UPON RECEIPT OF BACKUP)",
 * "UPDATE TEPHEM TO TIME CMC CLOCK WAS ZEROED", "INITIATE SERVICER AT
 * PREREAD1", "CHANGE MAJOR MODE TO 11", "CLEAR DSKY IN CASE OF V 75",
 * "STORE LIFTOFF IMU-CDU ANGLES FOR ATT. ERROR DISPLAY", "COMPUTE INITIAL
 * VECTORS", "COMPUTE REFSMMAT FOR PRELAUNCH ALIGNMENT", "SET REFSMMAT
 * KNOWN FLAG", and "SET AVGEXIT IN SERVICER TO VHHDOT". While it zeroes
 * the clock it keeps two words a restart (REP11) reads to tell how far
 * it got: LIFTTEMP, -0 until it holds the clock at liftoff ("FOR
 * RESTARTS"), and TLIFTOFF, which holds REP11SA (a restart phase) until
 * it holds that clock too.
 *
 * The interpretive code between the basic stretches (the earth rate
 * compensation, the REFSMMAT and the initial vectors, VHHDOT's CALL)
 * is written out as the listing has it (src/interpreter/notation.h). */
#include "p11.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P11 and REP11A -- pp. 535-537 ---- */

/* REP11A: TEPHEM, the ephemeris time of the clock's zero, moved on by
 * TLIFTOFF: TEPHEM1 gets TEPHEM + TLIFTOFF (triple precision; the ADS
 * "CORRECTFOR OVERFLOW" of the low words); then a restart point
 * (PHASCHNG, OCT 05023) before TEPHEM is replaced. */
void rep11a(void)
{
    inhint();
    dca(ADDRESS_OF(TEPHEM) + 1u);
    dxch(ADDRESS_OF(TEPHEM1) + 1u);
    ca(ADDRESS_OF(TEPHEM));
    xch(ADDRESS_OF(TEPHEM1));
    dca(ADDRESS_OF(TLIFTOFF));
    das(ADDRESS_OF(TEPHEM1) + 1u);
    ads(ADDRESS_OF(TEPHEM1));
    tc_to(REP11A + 012u, PHASCHNG); /* OCT 05023, OCT 22000 */
}

/* REP11A-1: group 3's phase from A and L "RESET PHASE"; then REP11A. */
static void rep11a_minus1(void)
{
    dxch(ADDRESS_OF(MINUS_PHASE3));
    rep11a();
}

/* REP11A-2: TLIFTOFF from A and L (the clock at liftoff); A and L get
 * what it held. */
static void rep11a_minus2(void)
{
    dxch(ADDRESS_OF(TLIFTOFF));
    rep11a_minus1();
}

/* REP11A -5: the clock zeroed (TIME2 and TIME1 +0); A and L get its
 * reading. */
static void rep11a_minus5(void)
{
    set_A(ZERO);
    zl();
    dxch(ADDRESS_OF(TIME2));
    rep11a_minus2();
}

/* P11+7: TLIFTOFF gets REP11SA (and A, L what it held); LIFTTEMP the
 * clock "FOR RESTARTS" (and A, L its -0 and second word); then the clock
 * zeroed. */
static void p11_plus7(void)
{
    dca_constant(REP11SA);
    dxch(ADDRESS_OF(TLIFTOFF));
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(LIFTTEMP));
    rep11a_minus5();
}

/* P11: "DIRECT RESTARTS TO REP11" (group 3's phase, REP11S); LIFTTEMP -0;
 * group 5 "INACTIVE GROUP 5, PRELAUNCH PROTECTION" (-PHASE5 -0, PHASE5
 * +0). */
void p11(void)
{
    set_A(EBANK3); /* EBANK= TEPHEM */
    ts(EBANK_ADDRESS);
    dca_constant(REP11S);
    dxch(ADDRESS_OF(MINUS_PHASE3));
    set_A(negate(ZERO)); /* CS ZERO */
    zl();
    ts(ADDRESS_OF(LIFTTEMP));
    dxch(ADDRESS_OF(MINUS_PHASE5));
    p11_plus7();
}

/* REP11A +015: TEPHEM replaced by TEPHEM1; then PREREAD1, "ZERO PIPS AND
 * INITIALIZE AVERAGEG" (IBNKCALL, under EBANK= DVCNTR). */
void rep11a_after_phaschng(void)
{
    inhint();
    dca(ADDRESS_OF(TEPHEM1));
    dxch(ADDRESS_OF(TEPHEM));
    ca(ADDRESS_OF(TEPHEM1) + 2u);
    xch(ADDRESS_OF(TEPHEM) + 2u);
    set_A(EBDVCNT);
    ts(EBANK_ADDRESS);
    tc_to(REP11A + 025u, IBNKCALL); /* CADR PREREAD1 */
}

/* REP11A +027: a restart point, "CONTINUE HERE ON RESTART". */
void rep11a_after_preread1(void)
{
    tc_to(REP11A + 027u, PHASCHNG); /* OCT 05023, OCT 22000 */
}

/* REP11A +032: "START ATT ERROR DISPLAY", ATERTASK "IN .5 SEC". */
void rep11a_start_display(void)
{
    set_A(HALF_SEC);
    tc_to(REP11A + 033u, WAITLIST); /* 2CADR ATERTASK */
}

/* REP11A +036: "DISPLAY MM 11". */
void rep11a_after_waitlist(void)
{
    tc_to(REP11A + 036u, NEWMODEX); /* MM 11 */
}

/* REP11A +040: NODOP01 set. */
void rep11a_after_newmodex(void)
{
    tc_to(REP11A + 040u, UPFLAG); /* ADRES NODOP01 */
}

/* REP11A +042: "SWITCH TO POWERED FLIGHT DOWNLIST" (DNLSTCOD 3); then
 * "CLEAR DSKY IN CASE OF V75". */
void rep11a_after_upflag(void)
{
    set_A(POWDNCOD);
    ts(ADDRESS_OF(DNLSTCOD));
    tc_to(REP11A + 044u, BANKCALL); /* CADR CLEANDSP */
}

/* REP11A +046: a restart point, "PROTECT ATERTASK". */
void rep11a_after_cleandsp(void)
{
    tc_to(REP11A + 046u, TWO_PHSCHNG); /* OCT 40514, OCT 00073 */
}

/* REP11A +051: QPLACES gets P11XIT, "SET EXIT FROM PROUT IN EARTHR" (to
 * P11OUT); then the interpretive code (EBANK= QPLACES). */
void rep11a_after_2phschng(void)
{
    set_A(EBQPLACE);
    ts(EBANK_ADDRESS);
    set_A(P11XIT);
    ts(ADDRESS_OF(QPLACES));
    tc_at(REP11A + 055u);
    intpret();
}

/* REP11A +056: ERCOMP, the earth rate compensation, gets THETAN (in
 * stable member axes by XSM, doubled); then EARTHR +3, the earth rate
 * loop, from TLIFTOFF, with S2 (EARTHR's return) PROUT, "RETURN FROM
 * EARTHR", which exits to P11OUT (QPLACES). */
static const word p11_words[] = {
    ORDERS(VLOAD, MXV),         ADDRESS_PLUS_ONE(THETAN),
    ADDRESS_PLUS_ONE(XSM),      ORDERS(VSL1, VAD),
    ADDRESS_PLUS_ONE(ERCOMP),   STODL(ERCOMP),
    ADDRESS_PLUS_ONE(TLIFTOFF), ORDERS(SSP, GOTO),
    ADDRESS_PLUS_ONE(S2),       ADDRESS(PROUT), /* CADR PROUT: "RETURN FROM EARTHR" */
    ADDRESS_AT(EARTHR, 3),
};

const struct interpretive_code P11_CODE = {034u, REP11A + 056u, COUNT(p11_words), p11_words};

/* ---- MATRXJOB -- pp. 537-539 ---- */

/* MATRXJOB: "STORE DP GIMBAL ANGLES FOR ATTITUDE ERROR DISPLAY AFTER
 * LIFTOFF": OGC, IGC and MGC get CDUX, CDUY and CDUZ (each the high word,
 * the low +0). Then the interpretive code. */
void matrxjob(void)
{
    zl();
    ca(ADDRESS_OF(CDUX));
    dxch(ADDRESS_OF(OGC));
    zl();
    ca(ADDRESS_OF(CDUY));
    dxch(ADDRESS_OF(IGC));
    zl();
    ca(ADDRESS_OF(CDUZ));
    dxch(ADDRESS_OF(MGC));
    tc_at(MATRXJOB + 011u);
    intpret();
}

/* MATRXJOB +012: OGC scaled B-1, and RTX2 +0 "FOR EARTH"; the pad's
 * position (PGNCSALT, PADLONG, LATITUDE: "LAT,LONG,ALT ARE CONSECUTIVE",
 * at "TIME = 0"), by LALOTORV a "POSITION VECTOR IN REF.COORDS" (RN1), and
 * by GETDOWN the down direction. Then the REFSMMAT: its third row "UNITZ
 * = UNIT(GRAV)"; VN1, "V = EARTHRATE X R" (scaled to 2(7) m/cs); "(REF3 X
 * UNITW) = EAST", "(EAST X REF3) =
 * -SOUTH", and from the launch azimuth "SIN(AZ)*EAST - COS(AZ)*SOUTH =
 * REF1", its first row; its second "(REF1 X REF3) = -REF3"
 * complemented. SATRLRT gets the sign of "1/2REV -LAVNCHAZ +AZIMUTH", "FOR
 * ROLL CALC IN FDAI ATT. ERROR DISPLAY"; "SET REFSMMAT KNOWN FLAG". */
static const word matrxjob_words[] = {
    ORDERS(VLOAD, VSR1), /* "SCALE OGC B-1" */
    ADDRESS_PLUS_ONE(OGC),
    STORE(OGC),
    ORDER(SSP), /* "ZERO RTX2 FOR EARTH" */
    ADDRESS_PLUS_ONE(RTX2),
    NUMBER(0),
    ORDERS(DLOAD, PDDL),
    ADDRESS_PLUS_ONE(PGNCSALT), /* "ALTITUDE OF PGNCS" */
    ADDRESS_PLUS_ONE(PADLONG),  /* "LONGITUDE" */
    ORDERS(PDDL, VDEF),
    ADDRESS_PLUS_ONE(LATITUDE), /* "GEODETIC LATITUDE" */
    STODL(LAT),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, ERADFLAG),
    ADDRESS(LALOTORV),
    STCALL(RN1),
    ADDRESS(GETDOWN), /* "RETURN WITH VECTOR FOR DOWN DIRECTION" */
    ORDERS(VCOMP, UNIT),
    STOVL_AT(REFSMMAT, 12), /* "UNITZ = UNIT(GRAV)" */
    ADDRESS_PLUS_ONE(RN1),
    ORDERS(VXV, VXSC),
    ADDRESS_PLUS_ONE(UNITW),    /* "SCALED AT 1" */
    ADDRESS_PLUS_ONE(MERTHRAT), /* -ERTHRAT: "V = EARTHRATE X R" */
    ORDER(VSL4),
    STOVL(VN1),
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 12),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UNITW), /* "(REF3 X UNITW) = EAST" */
    ORDERS(PUSH, VXV),
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 12), /* "(EAST X REF3) = -SOUTH" */
    ORDERS(UNIT, PDDL),
    ADDRESS_PLUS_ONE(LAUNCHAZ), /* "COS(AZ)*SOUTH" */
    ORDERS(COS, VXSC),
    ORDER(STADR),
    AFTER_STADR(STODL(REFSMMAT)), /* "TEMPORARY STORAGE" */
    ADDRESS_PLUS_ONE(LAUNCHAZ),
    ORDERS(SIN, VXSC), /* "SIN(AZ)*EAST" */
    ORDERS(VAD, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(REFSMMAT),
    ORDERS(VXV, UNIT), /* "(REF1 X REF3) = -REF3" */
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 12),
    ORDER(VCOMP),
    STORE_AT(REFSMMAT, 6),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(DPHALF), /* "1/2 REV" */
    ADDRESS_PLUS_ONE(LAUNCHAZ),
    ORDERS(DAD, PDDL),
    ADDRESS_PLUS_ONE(AZIMUTH),
    ADDRESS_PLUS_ONE(SATRLRT),
    ORDERS(SIGN, STADR),
    AFTER_STADR(STORE(SATRLRT)),
    ORDERS(SET, EXIT),
    SWITCH(SET, REFSMFLG),
};

const struct interpretive_code MATRXJOB_CODE = {034u, MATRXJOB + 012u, COUNT(matrxjob_words),
                                                matrxjob_words};

/* MATRXJOB +0101, after the REFSMMAT's EXIT: a restart point. */
void matrxjob_after_exit(void)
{
    tc_to(MATRXJOB + 0101u, PHASCHNG); /* OCT 04023 */
}

/* MATRXJOB +0103: "SET AVGEXIT" to VHHDOT, and 1/PIPADT to "2 SECONDS AT
 * 2(+8)"; under EBANK= RCSFLAGS, READACCS restarted ("RESTART READACCS 2
 * SECONDS AFTER LIFTOFF", TBASE5 -0) and started by WAITLIST, "DO
 * READACCS 2 SECONDS AFTER LIFTOFF": in 2 seconds less TIME1, or 1
 * centisecond ("CHECK TO INSURE DT IS POSITIVE"). */
void matrxjob_after_phaschng(void)
{
    dca_constant(P11SCADR);
    dxch(ADDRESS_OF(AVGEXIT));
    set_A(PRIO31);
    ts(ADDRESS_OF(1_PIPADT));
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    inhint();
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(TBASE5));
    cs(ADDRESS_OF(TIME1));
    ad(TWO_SECS);
    /* CCS A: "TIME POSITIVE" (TCF +3), and for +0 or -0 A is +0; "TIME
     * NEGATIVE - SET TO 1". */
    if (ccs_register(A_ADDRESS) == NEGATIVE) set_A(ZERO);
    ad(ONE);                           /* "RESTORE TIME  -  OR MAKE POSITIVE" */
    tc_to(MATRXJOB + 0124u, WAITLIST); /* 2CADR READACCS */
}

/* MATRXJOB +0127: a restart point, "TURN OFF GROUP 3" and "PROTECT
 * NORMLIZE AND READACCS". */
void matrxjob_after_waitlist(void)
{
    tc_to(MATRXJOB + 0127u, TWO_PHSCHNG); /* OCT 00003, OCT 00025 */
}

/* MATRXJOB +0132: "DO NORMLIZE AND ENDOFJOB". */
void matrxjob_after_2phschng(void)
{
    tc_to(MATRXJOB + 0132u, POSTJUMP); /* CADR NORMLIZE */
}

/* ---- REP11 -- p. 539 ---- */

/* REP11, the restart: with group 5 active (PHASE5 positive), nothing to
 * do (ENDOFJOB). Else, with LIFTTEMP still -0, all of P11 again from
 * P11+7. A TLIFTOFF not negative is the clock at liftoff, stored:
 * ENDREP11, the phase reset and REP11A again. A negative one is still
 * REP11SA, the clock perhaps half zeroed: "**TIME2 MUST BE NON-ZERO AT
 * LIFTOFF**", so with TIME2 positive "T2,T1 NOT YET ZEROED, GO AND DO IT"
 * (REP11A -5); with +0, "T2,T1 ZEROED, SET TLIFTOFF" from LIFTTEMP
 * (REP11A-2). (A negative TIME2 makes the CCS skip the EXTEND, so the DCA
 * LIFTTEMP runs as CA LIFTTEMP +1; a -0 skips both, leaving A +0.) */
void rep11(void)
{
    inhint();
    switch (ccs_at(ADDRESS_OF(PHASE5))) {
    case POSITIVE: tc_to(REP11 + 2u, ENDOFJOB); return;
    case IS_PLUS_ZERO:
        if (ccs_at(ADDRESS_OF(LIFTTEMP)) == IS_MINUS_ZERO) {
            p11_plus7();
            return;
        }
        break;
    default: break;
    }
    cs(ADDRESS_OF(TLIFTOFF));
    if (A_is_zero_or_minus()) { /* ENDREP11 */
        dca_constant(REP11SA);
        rep11a_minus1();
        return;
    }
    switch (ccs_at(ADDRESS_OF(TIME2))) {
    case POSITIVE: rep11a_minus5(); return;
    case IS_PLUS_ZERO: dca(ADDRESS_OF(LIFTTEMP)); break;
    case NEGATIVE: ca(ADDRESS_OF(LIFTTEMP) + 1u); break;
    case IS_MINUS_ZERO: break;
    }
    rep11a_minus2();
}

/* ---- VHHDOT, ATERTASK and GETDOWN -- pp. 540-541 ---- */

/* VHHDOT: "EXECUTED EVERY 2 SECONDS TO DISPLAY ON DSKY" V1, HDOT and H:
 * S11.1, in interpretive code, loads them. */
void vhhdot(void)
{
    tc_at(VHHDOT);
    intpret();
}

/* VHHDOT +1: "LOAD VMAGI, ALTI, HDOT FOR DISPLAY" (S11.1, in aterjob.c). */
static const word vhhdot_words[] = {
    ORDER(CALL),
    ADDRESS(S11_1),
    ORDER(EXIT),
};

const struct interpretive_code VHHDOT_CODE = {034u, VHHDOT + 1u, COUNT(vhhdot_words), vhhdot_words};

/* VHHDOT +4, after the EXIT: a restart point. */
void vhhdot_after_exit(void)
{
    tc_to(VHHDOT + 4u, PHASCHNG); /* OCT 00035 */
}

/* VHHDOT +6: "DISPLAY IN R1 R2 R3" (V06N62, REGODSP). */
void vhhdot_after_phaschng(void)
{
    set_A(V06N62);
    tc_to(VHHDOT + 7u, BANKCALL); /* CADR REGODSP */
}

/* ATERTASK, the task WAITLIST starts half a second after liftoff:
 * "ESTABLISH JOB TO DISPLAY ATT ERRORS" (ATERJOB). */
void atertask(void)
{
    set_A(PRIO1);
    tc_to(ATERTASK + 1u, FINDVAC); /* 2CADR ATERJOB */
}

/* ATERTASK +4: RCSFLAGS' bit 3 set for NEEDLER's "INITIALIZATION PASS",
 * "AND GO DO IT". */
void atertask_after_findvac(void)
{
    cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    tc_to(ATERTASK + 7u, IBNKCALL); /* CADR NEEDLER */
}

/* ATERTASK +011: SATSW +1, "SET SW FOR DISPLAY". */
void atertask_after_needler(void)
{
    set_A(BIT1);
    ts(ADDRESS_OF(SATSW));
    tc_to(ATERTASK + 013u, TASKOVER);
}

/* GETDOWN: "RETURN WITH VECTOR FOR DOWN DIRECTION". LALOTORV again from
 * +5, past its SETGAMMA and its own STQ (the return in INCORPEX here),
 * with 6D, the time, +0 and 8D, its GAMMA, DPHALF (one, B-1: no
 * flattening): a vector on the vertical of the geodetic latitude, back
 * to MATRXJOB's code. */
static const word getdown_words[] = {
    ORDERS(STQ, SETPD),         ADDRESS(INCORPEX), ADDRESS_PLUS_ONE_D(0),    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(HI6ZEROS), STODL_D(6),        ADDRESS_PLUS_ONE(DPHALF), STCALL_D(8),
    ADDRESS_AT(LALOTORV, 5),
};

const struct interpretive_code GETDOWN_CODE = {034u, GETDOWN, COUNT(getdown_words), getdown_words};

#endif
