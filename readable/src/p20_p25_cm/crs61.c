/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 8 (Comanche055 pp. 590-597): CRS61.1, the CSM's
 * preferred tracking attitude, and R63, which works out the desired CDU
 * angles for it (bank 34).
 *
 * R61CSM calls CRS61.1, which leaves in MPAC what it did ("RETURNS: ALL TO
 * L+1"): 0, the "NORMAL EXIT", the "3 SETS OF INPUTS FED TO DAP" (the
 * desired CDU angles in CDUXD, CDUYD and CDUZD, their increments for a
 * tenth of a second in DELCDUX, DELCDUY and DELCDUZ, and the line of
 * sight's rate in WBODY); 1, the desired angles in CPHI for KALCMANU,
 * when an actual CDU angle is more than 10 degrees from its desired one;
 * and 0 too when the autopilot is not in auto, or when someone is on the
 * rotational hand controller (STIKFLAG), with the UPACTY light on.
 *
 * R63 brings both state vectors to the time in P21TIME by conics, and
 * gives the line of sight, the +X axis tracking attitude (CPHIX, for noun
 * 96) and the preferred one (PRAXIS, for noun 95: the track axis 55
 * degrees from +X), and in THETAD the one PRFTRKAT picks.
 *
 * Each call out of the file (BANKCALL) leaves Z at the routine and Q at
 * the word after the TC, where the routine comes back to. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void step2ck(void);
static void cduloop(void);

/* CRS61.1 -- pp. 592-593: the return in Q611, the push-down pointer 0;
 * the time now (LOADTIME), STORT: "STORE CLOCK TIME FOR SUBR R63", and
 * R63, "SUBR TO CALC DCDU (T=PRESENT,PASS1)"; "SAVE DCDU(T) FROM CALCDCDU
 * FOR STEP4" in SAVEDCDU, and out of the interpreter. */
static const word crs61_1_words[] = {
    ORDERS(STQ, SETPD),
    ADDRESS(Q611),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(RTB),
    /* "LOAD CLOCK TIME2,1 INTO MPAC." */
    ADDRESS(LOADTIME),
    /* STORT: "STORE CLOCK TIME FOR SUBR R63" */
    STCALL(P21TIME),
    ADDRESS(R63),
    ORDER(TLOAD),
    /* "SAVE DCDU(T) FROM CALCDCDU FOR STEP4." */
    ADDRESS_PLUS_ONE(THETAD),
    STORE(SAVEDCDU),
    ORDER(EXIT),
};

/* CRS61.1 +013 -- after the EXIT: STEP2CK. */
void crs61_1_after_exit(void)
{
    tc_at(CRS61_1 + 013u);
    step2ck();
}

/* AUTOCK: "AUTO MODE SELECTED (BITS 15-13=011)" in channel 31 (RXOR with
 * PRIO30, 030000, and FURST3, 070000): "YES -- CONTINUE", DAPCK; no, ASET. */
void autock(void)
{
    set_A(PRIO30);
    rxor_channel(031);
    mask(FURST3);
    if (A_is_zero()) { /* EXTEND; BZF DAPCK */
        dapck();
        return;
    }
    tc_at(AUTOCK + 6u);
    aset();
}

/* DAPCK: "IS STIKFLAG SET (I.E., IS SOMEONE ON RHC)": no, STEP3CK (CS
 * FLAGWRD1 leaves the bit set when the flag is clear); yes, ASET. (A word
 * masked by STIKBIT is never negative, for the CCS's other two words.) */
void dapck(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(STIKBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(DAPCK + 3u);
        step3ck();
        break;
    case IS_PLUS_ZERO: aset(); break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* ASET: MPAC +0, and in interpretive code to "EXIT CRS61.1". (+0 leaves no
 * overflow for the TS to skip on.) */
void aset(void)
{
    set_A(ZERO);
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    tc_at(ASET + 2u);
    intpret();
}

/* ASET +3: back through Q611. */
static const word aset_words[] = {
    ORDER(GOTO),
    ADDRESS(Q611),
};

/* STEP2CK: the UPACTY light off (UPACTOFF), */
static void step2ck(void)
{
    tc_to(STEP2CK, BANKCALL); /* CADR UPACTOFF */
}

/* STEP2CK +2: and each CDU angle checked, the third first: "SET
 * TEMPORARY INDEX DTHETASM = 2". */
void step2ck_after_upactoff(void)
{
    set_A(TWO);
    cduloop();
}

/* CDULOOP: "SET A = ACTUAL CDU (ACDU)", and "A = DIFF = ACDU - DCDU" (MSU:
 * the CDUs' two's complement), to MPAC, and in interpretive code "FOR 10
 * DEGREE CK". Not modelled: an index in DTHETASM that would make the CA or
 * the MSU another instruction, or address fixed memory. (No A here
 * overflows, for a TS to skip on.) */
static void cduloop(void)
{
    word to;

    if (ts(ADDRESS_OF(DTHETASM))) not_modelled();
    to = indexed_instruction(read_word(CDULOOP + 2u), fetch(ADDRESS_OF(DTHETASM)));
    if (OPERATION_CODE(to) != OPERATION_CODE(read_word(CDULOOP + 2u)) ||
        ADDRESS_FIELD(to) >= 02000u)
        not_modelled();
    ca(ADDRESS_FIELD(to)); /* INDEX DTHETASM; CA CDUX */
    to = indexed_instruction(read_word(CDULOOP + 5u), fetch(ADDRESS_OF(DTHETASM)));
    if (OPERATION_CODE(to) != OPERATION_CODE(read_word(CDULOOP + 5u)) ||
        ADDRESS_FIELD(to) >= 02000u)
        not_modelled();
    msu_at(ADDRESS_FIELD(to)); /* EXTEND; INDEX DTHETASM; MSU THETAD */
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    tc_at(CDULOOP + 7u);
    intpret();
}

/* CDULOOP +010: "IS (ACDU - DCDU) MORE THAN 10 DEGREES": "NO -- OK,
 * CONTINUE CHECKING OTHER ANGLES"; yes, STKTEST. */
static const word cduloop_words[] = {
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(DEGREE10),
    ORDERS(BPL, EXIT),
    /* "TEST STICK FLAG" */
    ADDRESS(STKTEST),
};

/* CDULOOP +014 -- after the EXIT: "HAVE ALL 3 ANGLE DIFFS BEEN CHECKED."
 * "NO -- DIM COUNT, CHECK NEXT ANGLE DIFF."; yes, AUTOCK. DTHETASM -0
 * runs the word after STKTEST's EXIT. Not modelled: DTHETASM negative,
 * which would run the EXIT as an instruction. */
void cduloop_after_exit(void)
{
    switch (ccs_at(ADDRESS_OF(DTHETASM))) {
    case POSITIVE:
        tc_at(CDULOOP + 015u);
        cduloop();
        break;
    case IS_PLUS_ZERO: cduloop_done(); break;
    case NEGATIVE: not_modelled(); break;
    case IS_MINUS_ZERO: stktest_after_exit(); break;
    }
}

/* CDULOOP +016: TC AUTOCK. */
void cduloop_done(void)
{
    tc_at(CDULOOP + 016u);
    autock();
}

/* STKTEST: an angle more than 10 degrees out: out of the interpreter, */
static const word stktest_words[] = {
    ORDER(EXIT),
};

/* STKTEST +1 -- after the EXIT: "STIKFLAG IS NOT SET (DO R63)",
 * MANUEXIS; set, the UPACTY light on. (A word masked by STIKBIT is never
 * negative.) */
void stktest_after_exit(void)
{
    cs(ADDRESS_OF(FLAGWRD1));
    mask(STIKBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc_at(STKTEST + 4u);
        manuexis();
        break;
    case IS_PLUS_ZERO: stktest_upacty(); break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* STKTEST +5: "STIKFLG IS SET": "TURN ON UPACTY LIGHT" (channel 11 bit
 * 3), and "EXIT AND SET R61CNTR" (ASET). */
void stktest_upacty(void)
{
    set_A(BIT3);
    wor_dsalmout();
    tc_at(STKTEST + 010u);
    aset();
}

/* STEP3CK: in interpretive code, */
void step3ck(void)
{
    tc_at(STEP3CK);
    intpret();
}

/* STEP3CK +1: "NOW HAVE DCDUS STORED IN T(SAVEDCDU)." "GO CALC OTHER DAP
 * INPUTS (DELCDU,WBODY)". CRS61.2: the line of sight's rate at right
 * angles to it, "ORTHO LOS RATE (OMEGA TH, B-15,X1)": the relative
 * velocity ("DV = VL - VC", DCDU holds VL) crossed with the unit line of
 * sight (SAVEPOS), times its length over the line of sight's (P21TIME),
 * each normalized and the difference of the shifts kept in X1 and Q6111.
 * "OBTAIN RATE IN SM COORDS (OMEGTHSM)" in 20D, a tenth of a second of it
 * in DTHETASM, and SMCDURES gives the CDU increments ("DELCDU"), in 14D
 * "FOR AUTOPILOT". Then the rate in body axes (*SMNB*), and in control
 * axes (MBDYTCTL, 7.25 degrees about X), "REVS SCALED AT 450 DEGREES", in
 * MPAC. CRS61.2A: out of the interpreter. */
static const word step3ck_words[] = {
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    /* CRS61.2 */
    ORDERS(VLOAD, VSU),
    ADDRESS_PLUS_ONE(DCDU),
    /* "DV = VL - VC" */
    ADDRESS_PLUS_ONE(SAVEVEL),
    /* "V(MPAC)=-UNITDV. VAC36D=ABSDV." */
    ORDERS(UNIT, VCOMP),
    /* "(-UNITDV)CROSS(UNITLOS)." */
    ORDERS(VXV, VXSC),
    ADDRESS_PLUS_ONE(SAVEPOS),
    ADDRESS_PLUS_ONE(RVCS_RDS),
    /* "HOLD CROSS IN PUSHLIST0.  SCALED B6." */
    ORDER(PUSH),
    /* "OBTAIN ABS VALUE OF LOS." */
    ORDERS(DLOAD, NORM),
    /* "P21TIME IS TEMP STORE FOR ABSLOS." */
    ADDRESS_PLUS_ONE(P21TIME),
    ADDRESS_PLUS_ONE(X1),
    /* "NORM ABSLOS(DENOM) AND HOLD IN PUSH1." */
    ORDER(PUSH),
    ORDERS(DLOAD, NORM),
    /* "NORM ABS VALUE OF DV(NUM)." */
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS_PLUS_ONE(S1),
    /* "X1 = X1(N DENOM)-S1(N NUM)." */
    ORDERS(XSU_1, SR1),
    ADDRESS(S1),
    /* "ABSDV(MPAC)/ABSLOS(PUSH1) = QUOT." */
    ORDERS(DDV, VXSC),
    ORDER(SXA_1),
    /* "SAVE SCALE OF RESULT (R-15,1X)." */
    ADDRESS(Q6111),
    /* "OBTAIN RATE IN SM COORDS (OMEGTHSM)" */
    ORDERS(MXV, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    /* "OMEGTHSM = VAC20D" */
    STORE_D(20),
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(TENTH),
    /* "STORE SM INCREM ANGLES FOR SMCDURES." */
    STORE(DTHETASM),
    ORDER(CALL),
    /* "OBTAIN SIN,COSCDUS FOR SMCDURES." */
    ADDRESS(CDUTRIG),
    /* "SMCDURES USES PUSH" */
    ORDERS(SETPD, CALL),
    ADDRESS_PLUS_ONE_D(0),
    /* "OBTAIN DELCDU IN V(DCDU)." */
    ADDRESS(SMCDURES),
    /* "RELOAD X1" */
    ORDER(LXA_1),
    ADDRESS(Q6111),
    /* "RECOVER SCALE." */
    ORDERS(VLOAD, VSL_STAR),
    ADDRESS_PLUS_ONE(DCDU),
    /* "EQUALS B-17D,1 TO OBTAIN HALFREVS B0." */
    SL_X1(-17),
    /* "HOLD DELS IN V(VAC14D) FOR AUTOPILOT." */
    STORE_D(14),
    /* "COMPUTES SINES AND COSINES FOR *SMNB*" */
    ORDER(CALL),
    ADDRESS(CDUTRIG),
    /* "LOAD VECTOR AND CALL TRANSFORMATION" */
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE_D(20),
    /* "OBTAIN ANG. RATE REFERRED TO NB (BODY)" */
    ADDRESS(STAR_SMNB_STAR),
    ORDER(MXV),
    /* "CONVERT RATE(OMEGA) TO CONTROL COORDS." */
    ADDRESS_PLUS_ONE(MBDYTCTL),
    /* "MULT. BY 0.8 TO RESCALE REVS TO 450 DEG." */
    ORDER(VXSC),
    ADDRESS_PLUS_ONE(POINT8),
    /* "RELOAD X1 TO RECOVER NORMALIZ." */
    ORDERS(LXA_1, VSL_STAR),
    ADDRESS(Q6111),
    SL_X1(-14),
    /* CRS61.2A */
    ORDER(EXIT),
};

/* CRS61.2A +1 -- after the EXIT, with interrupts held: the DAP's inputs,
 * the "DESIRED GIMBAL ANGLES" from SAVEDCDU to CDUXD, CDUYD and CDUZD
 * (double words, the low +0), the "OMEGA CONTROL (ANG. LOS RATE)" from
 * MPAC to WBODY, WBODY1 and WBODY2, and the "CDU INCREMENTS" from the work
 * area's 14D to DELCDUX, DELCDUY and DELCDUZ; HOLDFLAG -1, and ASET. (A
 * word of erasable memory, or -1, leaves no overflow for a TS to skip
 * on.) */
void crs61_2a_after_exit(void)
{
    inhint();
    set_A(ZERO);
    if (ts(ADDRESS_OF(CDUXD) + 1u)) not_modelled();
    if (ts(ADDRESS_OF(CDUYD) + 1u)) not_modelled();
    if (ts(ADDRESS_OF(CDUZD) + 1u)) not_modelled();
    ca(ADDRESS_OF(SAVEDCDU));
    if (ts(ADDRESS_OF(CDUXD))) not_modelled();
    ca(ADDRESS_OF(SAVEDCDU) + 1u);
    if (ts(ADDRESS_OF(CDUYD))) not_modelled();
    ca(ADDRESS_OF(SAVEDCDU) + 2u);
    if (ts(ADDRESS_OF(CDUZD))) not_modelled();
    dca(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(WBODY));
    dca(ADDRESS_OF(MPAC) + 3u);
    dxch(ADDRESS_OF(WBODY1));
    dca(ADDRESS_OF(MPAC) + 5u);
    dxch(ADDRESS_OF(WBODY2));
    dca(indexed(D(14), FIXLOC)); /* EXTEND; INDEX FIXLOC; DCA 14D */
    dxch(ADDRESS_OF(DELCDUX));
    dca(indexed(D(16), FIXLOC));
    dxch(ADDRESS_OF(DELCDUY));
    dca(indexed(D(18), FIXLOC));
    dxch(ADDRESS_OF(DELCDUZ));
    set_A(negate(ONE)); /* CS ONE */
    if (ts(ADDRESS_OF(HOLDFLAG))) not_modelled();
    relint();
    tc_at(CRS61_2A + 044u);
    aset();
}

/* MANUEXIS: in interpretive code, */
void manuexis(void)
{
    tc_at(MANUEXIS);
    intpret();
}

/* MANUEXIT -- p. 596: "ACDU-DCDU EXCEEDS 10 DEG.": "STORE DCDU(T) IN
 * CPHI,CTHETA, CPSI FOR KALCMANU", and the "SPECIAL RETURN (MPAC+0 = 1)"
 * through Q611. */
static const word manuexit_words[] = {
    ORDER(TLOAD),
    ADDRESS_PLUS_ONE(SAVEDCDU),
    STORE(CPHI),
    /* "SPECIAL RETURN (MPAC+0 = 1)" */
    ORDERS(SLOAD, GOTO),
    ADDRESS_PLUS_ONE(LOONE),
    ADDRESS(Q611),
};

/* R63 -- pp. 596-597, the "SUBR TO CALC DCDUS(T)": the return in Q6111;
 * the CSM's state vector to P21TIME (CSMCONIC), HOLDATT: its position and
 * velocity held in SAVEPOS and SAVEVEL. CALCLEM: the LM's
 * ("EXTRAPOLATE LEM STATE VECTOR TO SAVE TIME AS CSM USING LEMCONIC"),
 * its velocity in DCDU; "LOS = RL RC" in SAVEPOS as a unit vector, and in
 * POINTVSM in stable member axes ("CONVERT TO STABLE MEMBER"), its length
 * in P21TIME. VECPOINT gives the attitude with an axis (SCAXIS) along it:
 * +X, "FOR +X-AXIS TRACKING ATTITUDE", in CPHIX, and PRFUNIT's, the
 * "55 DEG TRACK AXIS UNIT VECTOR", in PRAXIS. CRSTOR: the preferred
 * attitude to THETAD, or with PRFTRKAT clear (CRSTOR1) the +X one, with
 * SCAXIS +X; and back through Q6111. */
static const word r63_words[] = {
    ORDERS(STQ, DLOAD),
    ADDRESS(Q6111),
    ADDRESS_PLUS_ONE(P21TIME),
    STCALL(TDEC1),
    ADDRESS(CSMCONIC),
    /* HOLDATT: "HOLD EXTRAPOLATED CSM POSITION AND" */
    ORDER(VLOAD),
    /* "VELOCITY" */
    ADDRESS_PLUS_ONE(RATT),
    STOVL(SAVEPOS),
    ADDRESS_PLUS_ONE(VATT),
    STORE(SAVEVEL),
    /* CALCLEM: "EXTRAPOLATE LEM STATE VECTOR TO SAVE" */
    ORDER(DLOAD),
    /* "TIME AS CSM USING LEMCONIC" */
    ADDRESS_PLUS_ONE(P21TIME),
    STCALL(TDEC1),
    ADDRESS(LEMCONIC),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(VATT),
    /* "STORE VATT IN DCDU TEMPORARILY" */
    STOVL(DCDU),
    /* "LOS = RL RC" */
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(VSU, UNIT),
    ADDRESS_PLUS_ONE(SAVEPOS),
    /* "SAVE UNITLOS FOR CRS61.2 RATE CALC." */
    STORE(SAVEPOS),
    ORDERS(MXV, VSL1),
    /* "CONVERT TO STABLE MEMBER" */
    ADDRESS_PLUS_ONE(REFSMMAT),
    STODL(POINTVSM),
    /* "HOLD ABS VAL OF LOS (VAC 36D)" */
    ADDRESS_PLUS_ONE_D(36),
    /* "IN D(P21TIME) FOR CRS61.2 RATE CALC." */
    STORE(P21TIME),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNITX),
    /* "TRACK AXIS UNIT VECTOR" */
    STCALL(SCAXIS),
    /* "FOR +X-AXIS TRACKING ATTITUDE" */
    ADDRESS(VECPOINT),
    /* "STORE ANGLES FOR N96 DISPLAY" */
    STORE(CPHIX),
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(PRFUNIT),
    STCALL(SCAXIS),
    ADDRESS(VECPOINT),
    /* "STORE ANGLES FOR N95 DISPLAY" */
    STORE(PRAXIS),
    ORDER(BOFF),
    SWITCH(BOF, PRFTRKAT),
    ADDRESS(CRSTOR1),
    /* CRSTOR: "STORE ANGLES FOR N18 DISPLAY" */
    STORE(THETAD),
    ORDER(GOTO),
    ADDRESS(Q6111),
    /* CRSTOR1 */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(UNITX),
    STORE(SCAXIS),
    ORDERS(TLOAD, GOTO),
    ADDRESS_PLUS_ONE(CPHIX),
    ADDRESS(CRSTOR),
};

/* ---- The interpretive code ---- */

const struct interpretive_code CRS61_1_CODE = {034u, CRS61_1, COUNT(crs61_1_words), crs61_1_words};
const struct interpretive_code ASET_CODE = {034u, ASET + 3u, COUNT(aset_words), aset_words};
const struct interpretive_code CDULOOP_CODE = {034u, CDULOOP + 010u, COUNT(cduloop_words),
                                               cduloop_words};
const struct interpretive_code STKTEST_CODE = {034u, STKTEST, COUNT(stktest_words), stktest_words};
const struct interpretive_code STEP3CK_CODE = {034u, STEP3CK + 1u, COUNT(step3ck_words),
                                               step3ck_words};
const struct interpretive_code MANUEXIT_CODE = {034u, MANUEXIT, COUNT(manuexit_words),
                                                manuexit_words};
const struct interpretive_code R63_CODE = {034u, R63, COUNT(r63_words), r63_words};

#endif
