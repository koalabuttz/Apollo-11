/* SPDX-License-Identifier: MIT */
/* ATTITUDE_MANEUVER_ROUTINE.agc (Luminary099 pages 342-363) and the CM's
 * ANGLFIND.agc (Comanche055 pages 399-411): KALCMANU's start, the
 * maneuver's axis and angle, and the matrix routines it and its steering
 * use. Interpretive code in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs
 * (src/binding/attitude_maneuver_routines.c), and the basic code among it.
 * The faithful C is c64-port/src/lm_attitude_maneuver.c and
 * cm_attitude_maneuver.c.
 *
 * "KALCMANU IS A ROUTINE WHICH GENERATES COMMANDS FOR THE LM DAP TO CHANGE
 * THE ATTITUDE OF THE SPACECRAFT DURING FREE FALL. IT IS DESIGNED TO
 * MANEUVER THE SPACECRAFT FROM ITS INITIAL ORIENTATION TO SOME DESIRED
 * ORIENTATION SPECIFIED BY THE PROGRAM WHICH CALLS KALCMANU, AVOIDING
 * GIMBAL LOCK IN THE PROCESS." The desired orientation is three commanded
 * CDU angles, CPHI, CTHETA, CPSI (outer, inner, middle gimbal). KALCMANU
 * "DETERMINES THE DIRECTION OF THE SINGLE EQUIVALENT ROTATION (COF ALSO
 * U) AND THE MAGNITUDE OF THE ROTATION (AM) TO BRING THE S/C FROM ITS
 * INITIAL ORIENTATION TO ITS FINAL ORIENTATION": this file. Gimbal lock
 * (LOCSKIRT) and the steering, a small rotation (DEL) each second, are the
 * next two files.
 *
 * The two missions' files are mostly the same program. The tables both
 * have word for word are written once, below, with both listings' pages;
 * where the CM's differs (KALCMAN3 has no gimbal lock check, SECAD ends
 * in basic code, MXM3 multiplies by columns, TRNSPSPD is interpretive,
 * CDUTODCM packs its orders differently, and the endings), each mission
 * has its own.
 *
 * Every word is in bank 22 (the listings' SETLOCs, KALCMON1 and KALCMON2,
 * put both halves there). The constants (MINANG, MAXANG, the gimbal lock
 * constants SD to CNGL, the LM's LOCKANGL) are read from the rope where
 * the interpreter reads them. */
#include "attitude_maneuver.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* KALCMAN3 -- LM p. 351, CM p. 399.
 * "KALCMANU IS CALLED AS A HIGH PRIORITY JOB WITH ENTRY POINTS AT KALCMAN3
 * AND VECPOINT." "THE THREE DESIRED CDU ANGLES MUST BE STORED AS SINGLE
 * PRECISION TWO'S COMPLEMENT ANGLES IN THE THREE SUCCESSIVE LOCATIONS,
 * CPHI, CTHETA, CPSI." Into the interpreter. */
void kalcman3(void)
{
    tc_at(KALCMAN3);
    intpret();
}

#if LUMINARY099

/* KALCMAN3 +1 -- LM p. 351.
 * "PICK UP THE CURRENT CDU ANGLES AND COMPUTE THE MATRIX FROM INITIAL S/C
 * AXES TO FINAL S/C AXES." RTB READCDUK reads the three CDUs into MPAC;
 * they are the start, BCDU. "IF GREATER THAN 70 DEG ABORT MANEUVER": a
 * commanded middle gimbal angle (CPSI) past LOCKANGL is gimbal lock, and
 * TOOBADF gives up. Then CDUTODCM turns each set of angles into a
 * direction cosine matrix: the start's (BCDU) into MIS, "FROM INITIAL S/C
 * AXES TO STABLE MEMBER AXES", the commanded ones (CPHI, CTHETA, CPSI) into
 * MFS, final S/C axes to stable member axes. AXC,2 gives CDUTODCM the
 * matrix to fill (X2 holds its address complemented). */
static const word kalcman3_words[] = {
    ORDER(RTB),             /* "COMPUTE THE MATRIX FROM INITIAL S/C" */
    ADDRESS(READCDUK),      /* "AXES TO FINAL S/C AXES" */
    STORE(BCDU),            /* "STORE INITIAL S/C ANGLES" */
    ORDERS(SLOAD, ABS),     /* "CHECK THE MAGNITUDE OF THE DESIRED" */
    ADDRESS_PLUS_ONE(CPSI), /* "MIDDLE GIMBAL ANGLE" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(LOCKANGL), /* "IF GREATER THAN 70 DEG ABORT MANEUVER" */
    ADDRESS(TOOBADF),
    ORDERS(AXC_2, TLOAD),
    ADDRESS(MIS),
    ADDRESS_PLUS_ONE(BCDU),
    ORDER(CALL),       /* "COMPUTE THE TRANSFORMATION FROM INITIAL" */
    ADDRESS(CDUTODCM), /* "S/C AXES TO STABLE MEMBER AXES" */
    ORDERS(AXC_2, TLOAD),
    ADDRESS(MFS), /* "PREPARE TO CALCULATE ARRAY MFS" */
    ADDRESS_PLUS_ONE(CPHI),
    ORDER(CALL),
    ADDRESS(CDUTODCM),
};

/* SECAD -- LM pp. 351-352.
 * "MIS AND MFS ARRAYS CALCULATED". TMIS is MIS transposed (TRANSPOS leaves
 * it on the push-down list, and three STADRs store it back a row at a
 * time, the last row first): stable member to initial axes. MXM3 makes MFI
 * = TMIS MFS, "SCALED BY 4": final S/C axes to initial S/C axes, the whole
 * maneuver as one matrix; TMFI is its transpose.
 *
 * "CALCULATE COFSKEW": the skew symmetric part of MFI, three differences
 * of MFI and TMFI (VDEF makes them a vector), which points along the axis
 * of the rotation ("COF SCALED BY 2/SIN(AM)").
 *
 * "CALCULATE AM AND PROCEED ACCORDING TO ITS MAGNITUDE": the trace of MFI
 * gives the cosine of the angle of the rotation, CAM = (MFI0 + MFI4 + MFI8 -
 * 1)/2, and ARCCOS the angle, AM. Under MINANG (.25 degrees: "MANEUVER
 * LESS THAN .25 DEGREES ... GO DIRECTLY INTO ATTITUDE HOLD ABOUT COMMANDED
 * ANGLES") the commanded angles go straight to CDUXD, CDUYD, CDUZD (STCALL
 * stores all three, MPAC triple precision) and TOOBADI stops the rates. */
static const word secad_words[] = {
    /* SECAD */ ORDERS(AXC_1, CALL), /* "MIS AND MFS ARRAYS CALCULATED $2" */
    ADDRESS(MIS),
    ADDRESS(TRANSPOS),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STOVL_AT(TMIS, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(TMIS, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(TMIS)), /* "TMIS = TRANSPOSE(MIS) SCALED BY 2" */
    ORDERS(AXC_1, AXC_2),
    ADDRESS(TMIS),
    ADDRESS(MFS),
    ORDER(CALL),
    ADDRESS(MXM3),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STOVL_AT(MFI, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(MFI, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(MFI)), /* "MFI = TMIS MFS (SCALED BY 4)" */
    ORDERS(SETPD, CALL),     /* "TRANSPOSE MFI IN PD LIST" */
    ADDRESS_PLUS_ONE_D(18),
    ADDRESS(TRNSPSPD),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STOVL_AT(TMFI, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(TMFI, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(TMFI)), /* "TMFI = TRANSPOSE (MFI) SCALED BY 4" */
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_AT(TMFI, 2),
    ADDRESS_PLUS_ONE_AT(MFI, 2),
    ORDERS(PDDL, DSU), /* "CALCULATE COF SCALED BY 2/SIN(AM)" */
    ADDRESS_PLUS_ONE_AT(MFI, 4),
    ADDRESS_PLUS_ONE_AT(TMFI, 4),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE_AT(TMFI, 10),
    ADDRESS_PLUS_ONE_AT(MFI, 10),
    ORDER(VDEF),
    STORE(COFSKEW), /* "EQUALS MFISKEW" */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(MFI),
    ADDRESS_PLUS_ONE_AT(MFI, 16),
    ORDERS(DSU, DAD),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS_PLUS_ONE_AT(MFI, 8),
    STORE(CAM), /* "CAM = (MFI0+MFI4+MFI8-1)/2 HALF SCALE" */
    ORDER(ARCCOS),
    STORE(AM), /* "AM=ARCCOS(CAM) (AM SCALED BY 2)" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(MINANG),
    ADDRESS(CHECKMAX),
    ORDER(TLOAD),           /* "MANEUVER LESS THAN .25 DEGREES" */
    ADDRESS_PLUS_ONE(CPHI), /* "GO DIRECTLY INTO ATTITUDE HOLD" */
    STCALL(CDUXD),          /* "ABOUT COMMANDED ANGLES" */
    ADDRESS(TOOBADI),       /* "STOP RATE AND EXIT" */
};

#else

/* KALCMAN3 +1 -- CM p. 399.
 * As the LM's, without its gimbal lock check: the CDUs read (RTB
 * READCDUK) are the start, BCDU ("STORE THE INITIAL S/C ANGLES"), and
 * CDUTODCM turns each set of angles into a direction cosine matrix: BCDU
 * into MIS, "THE TRANSFORMATION FROM INITIAL S/C AXES TO STABLE MEMBER
 * AXES", and the commanded ones (CPHI, CTHETA, CPSI) into MFS, final S/C
 * axes to stable member axes. AXC,2 gives CDUTODCM the matrix to fill. */
static const word kalcman3_words[] = {
    ORDER(RTB),
    ADDRESS(READCDUK),      /* "PICK UP CURRENT CDU ANGLES" */
    STORE(BCDU),            /* "STORE THE INITIAL S/C ANGLES" */
    ORDERS(AXC_2, TLOAD),   /* "COMPUTE THE TRANSFORMATION FROM" */
    ADDRESS(MIS),           /* "INITIAL S/C AXES TO STABLE MEMBER AXES" */
    ADDRESS_PLUS_ONE(BCDU), /* "(MIS)" */
    ORDER(CALL),
    ADDRESS(CDUTODCM),
    ORDERS(AXC_2, TLOAD),   /* "COMPUTE THE TRANSFORMATION FROM" */
    ADDRESS(MFS),           /* "FINAL S/C AXES TO STABLE MEMBER AXES" */
    ADDRESS_PLUS_ONE(CPHI), /* "(MFS)" */
    ORDER(CALL),
    ADDRESS(CDUTODCM),
};

/* SECAD -- CM pp. 399-400.
 * As the LM's (TMIS, MFI, TMFI, COFSKEW, CAM and AM; the first VLOAD and
 * STADR are a word each here); but a maneuver under MINANG ends in basic
 * code: EXIT. */
static const word secad_words[] = {
    /* SECAD */ ORDERS(AXC_1, CALL), /* "MIS AND MFS ARRAYS CALCULATED $2" */
    ADDRESS(MIS),
    ADDRESS(TRANSPOS),
    ORDER(VLOAD),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(TMIS, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(TMIS, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(TMIS)), /* "TMIS = TRANSPOSE(MIS) SCALED BY 2" */
    ORDERS(AXC_1, AXC_2),
    ADDRESS(TMIS),
    ADDRESS(MFS),
    ORDER(CALL),
    ADDRESS(MXM3),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STOVL_AT(MFI, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(MFI, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(MFI)), /* "MFI = TMIS MFS (SCALED BY 4)" */
    ORDERS(SETPD, CALL),     /* "TRANSPOSE MFI IN PD LIST" */
    ADDRESS_PLUS_ONE_D(18),
    ADDRESS(TRNSPSPD),
    ORDERS(VLOAD, STADR),
    AFTER_STADR(STOVL_AT(TMFI, 12)),
    ORDER(STADR),
    AFTER_STADR(STOVL_AT(TMFI, 6)),
    ORDER(STADR),
    AFTER_STADR(STORE(TMFI)), /* "TMFI = TRANSPOSE (MFI) SCALED BY 4" */
    ORDERS(DLOAD, DSU),       /* "CALCULATE COFSKEW AND MFISYM" */
    ADDRESS_PLUS_ONE_AT(TMFI, 2),
    ADDRESS_PLUS_ONE_AT(MFI, 2),
    ORDERS(PDDL, DSU), /* "CALCULATE COF SCALED BY 2/SIN(AM)" */
    ADDRESS_PLUS_ONE_AT(MFI, 4),
    ADDRESS_PLUS_ONE_AT(TMFI, 4),
    ORDERS(PDDL, DSU),
    ADDRESS_PLUS_ONE_AT(TMFI, 10),
    ADDRESS_PLUS_ONE_AT(MFI, 10),
    ORDER(VDEF),
    STORE(COFSKEW), /* "EQUALS MFISKEW" */
    ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(MFI),
    ADDRESS_PLUS_ONE_AT(MFI, 16),
    ORDERS(DSU, DAD),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS_PLUS_ONE_AT(MFI, 8),
    STORE(CAM), /* "CAM = (MFI0+MFI4+MFI8-1)/2 HALF SCALE" */
    ORDER(ARCCOS),
    STORE(AM), /* "AM=ARCCOS(CAM) (AM SCALED BY 2)" */
    ORDERS(DSU, BPL),
    ADDRESS_PLUS_ONE(MINANG),
    ADDRESS(CHECKMAX),
    ORDER(EXIT), /* "MANEUVER LESS THAN 0.25 DEG" */
};

/* SECAD +066 -- CM p. 400. After the EXIT: "GO DIRECTLY INTO ATTITUDE
 * HOLD ABOUT COMMANDED ANGLES". HOLDFLAG -1, and LOADCDUD
 * (KALCMANU_STEERING.agc) puts the commanded angles in CDUXD, CDUYD and
 * CDUZD. */
void secad_after_exit(void)
{
    inhint();
    set_A(negate(ONE));       /* CS ONE */
    ts(ADDRESS_OF(HOLDFLAG)); /* "NOGO WILL STOP ANY RATE AND SET UP FOR A */
    tc_at(SECAD + 071u);      /* GOOD RETURN" */
    continue_at(LOADCDUD);
}

/* SECAD +072 -- CM p. 400. Back from LOADCDUD: NOGO. */
void secad_after_loadcdud(void)
{
    nogo(); /* TCF NOGO */
}

#endif

/* CHECKMAX -- LM p. 352, CM p. 400.
 * Up to MAXANG (170 degrees) the axis is COFSKEW, made a unit vector: COF,
 * "THE MANEUVER AXIS". Then "SEE IF MANEUVER GOES THRU GIMBAL LOCK"
 * (LOCSKIRT, GIMBAL_LOCK_AVOIDANCE.agc). */
static const word checkmax_words[] = {
    /* CHECKMAX */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(AM),
    ADDRESS_PLUS_ONE(MAXANG),
    ORDERS(BPL, VLOAD),
    ADDRESS(ALTCALC),          /* "UNIT" */
    ADDRESS_PLUS_ONE(COFSKEW), /* "COFSKEW" */
    ORDER(UNIT),
    STORE(COF),  /* "COF IS THE MANEUVER AXIS" */
    ORDER(GOTO), /* "SEE IF MANEUVER GOES THRU GIMBAL LOCK" */
    ADDRESS(LOCSKIRT),
};

/* ALTCALC -- LM p. 353, CM pp. 400-401.
 * "IF AM GREATER THAN 170 DEGREES" the skew part is too small to trust (it
 * goes as the sine of AM), and the symmetric part is used: MFISYM = (MFI +
 * TMFI)/2, row by row. "CALCULATE COF": each component's magnitude from
 * the diagonal, "COFZ = SQRT(MFISYM8-CAM)/(1-CAM)", and the same for Y and
 * X, then COF made a unit vector. A BOVB on the way (1 - CAM overflowing)
 * goes to SIGNMPAC, which keeps the largest value of the sign. The signs are
 * not known yet: COFMAXGO finds them. */
static const word altcalc_words[] = {
    /* ALTCALC */ ORDERS(VLOAD, VAD), /* "IF AM GREATER THAN 170 DEGREES" */
    ADDRESS_PLUS_ONE(MFI),
    ADDRESS_PLUS_ONE(TMFI),
    ORDER(VSR1),
    STOVL(MFISYM),
    ADDRESS_PLUS_ONE_AT(MFI, 6),
    ORDERS(VAD, VSR1),
    ADDRESS_PLUS_ONE_AT(TMFI, 6),
    STOVL_AT(MFISYM, 6),
    ADDRESS_PLUS_ONE_AT(MFI, 12),
    ORDERS(VAD, VSR1),
    ADDRESS_PLUS_ONE_AT(TMFI, 12),
    STORE_AT(MFISYM, 12), /* "MFISYM=(MFI+TMFI)/2 SCALED BY 4" */
    ORDERS(DLOAD, SR1),
    ADDRESS_PLUS_ONE(CAM),
    ORDERS(PDDL, DSU), /* "PDO CAM $4" */
    ADDRESS_PLUS_ONE(DPHALF),
    ADDRESS_PLUS_ONE(CAM),
    ORDERS(BOVB, PDDL), /* "PD2 1 - CAM $2" */
    ADDRESS(SIGNMPAC),
    ADDRESS_PLUS_ONE_AT(MFISYM, 16),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SQRT, PDDL),             /* "COFZ = SQRT(MFISYM8-CAM)/1-CAM)" */
    ADDRESS_PLUS_ONE_AT(MFISYM, 8), /* "$ ROOT 2" */
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SQRT, PDDL), /* "COFY = SQRT(MFISYM4-CAM)/(1-CAM) $ROOT2" */
    ADDRESS_PLUS_ONE(MFISYM),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SQRT, VDEF), /* "COFX = SQRT(MFISYM-CAM)/(1-CAM) $ROOT 2" */
    ORDER(UNIT),
    STORE(COF),
};

/* COFMAXGO, COMP12 -- LM pp. 353-354, CM pp. 401-402.
 * "DETERMINE LARGEST COF AND ADJUST ACCORDINGLY": the largest component,
 * the one whose sign is surest, decides the others. COFX largest goes to
 * METHOD1, COFZ to METHOD3, COFY on into METHOD2. */
static const word cofmaxgo_words[] = {
    /* COFMAXGO */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(COF),
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ORDERS(BMN, DLOAD), /* "COFY G COFX" */
    ADDRESS(COMP12),
    ADDRESS_PLUS_ONE(COF),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ADDRESS(METHOD3), /* "COFZ G COFX OR COFY" */
    ORDER(GOTO),
    ADDRESS(METHOD1), /* "COFX G COFY OR COFZ" */
    /* COMP12 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ORDER(BMN),
    ADDRESS(METHOD3), /* "COFZ G COFY OR COFX" */
};

/* METHOD2, U2POS, OKU21 -- LM p. 354, CM p. 402.
 * "COFY MAX": COFY takes the sign of the skew part's Y (COFSKEW +2), the
 * whole vector turned round (VCOMP) if that is negative. Then each other
 * component's sign from the symmetric part: UX UY (MFISYM +2) negative
 * means "SIGN OF UX OPPOSITE" to UY; UY UZ (MFISYM +10D), "SIGN OF UZ
 * OPPOSITE TO UY". Then LOCSKIRT. */
static const word method2_words[] = {
    /* METHOD2 */ ORDERS(DLOAD, BPL), /* "COFY MAX" */
    ADDRESS_PLUS_ONE_AT(COFSKEW, 2),  /* "UY" */
    ADDRESS(U2POS),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(COF),
    STORE(COF),
    /* U2POS */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 2), /* "UX UY" */
    ADDRESS(OKU21),
    ORDERS(DLOAD, DCOMP), /* "SIGN OF UX OPPOSITE TO UY" */
    ADDRESS_PLUS_ONE(COF),
    STORE(COF),
    /* OKU21 */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 10), /* "UY UZ" */
    ADDRESS(LOCSKIRT),
    ORDERS(DLOAD, DCOMP), /* "SIGN OF UZ OPPOSITE TO UY" */
    ADDRESS_PLUS_ONE_AT(COF, 4),
    STORE_AT(COF, 4),
    ORDER(GOTO),
    ADDRESS(LOCSKIRT),
};

/* METHOD1, U1POS, OKU12 -- LM p. 354, CM p. 402.
 * "COFX MAX": as METHOD2 with X first. */
static const word method1_words[] = {
    /* METHOD1 */ ORDERS(DLOAD, BPL), /* "COFX MAX" */
    ADDRESS_PLUS_ONE(COFSKEW),        /* "UX" */
    ADDRESS(U1POS),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(COF),
    STORE(COF),
    /* U1POS */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 2), /* "UX UY" */
    ADDRESS(OKU12),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE_AT(COF, 2), /* "SIGN OF UY OPPOSITE TO UX" */
    STORE_AT(COF, 2),
    /* OKU12 */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 4), /* "UX UZ" */
    ADDRESS(LOCSKIRT),
    ORDERS(DLOAD, DCOMP), /* "SIGN OF UZ OPPOSITE TO UY" */
    ADDRESS_PLUS_ONE_AT(COF, 4),
    STORE_AT(COF, 4),
    ORDER(GOTO),
    ADDRESS(LOCSKIRT),
};

/* METHOD3, U3POS, OKU31 -- LM p. 355, CM pp. 402-403.
 * "COFZ MAX": as METHOD2 with Z first. */
static const word method3_words[] = {
    /* METHOD3 */ ORDERS(DLOAD, BPL), /* "COFZ MAX" */
    ADDRESS_PLUS_ONE_AT(COFSKEW, 4),  /* "UZ" */
    ADDRESS(U3POS),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(COF),
    STORE(COF),
    /* U3POS */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 4), /* "UX UZ" */
    ADDRESS(OKU31),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(COF), /* "SIGN OF UX OPPOSITE TO UZ" */
    STORE(COF),
    /* OKU31 */ ORDERS(DLOAD, BPL),
    ADDRESS_PLUS_ONE_AT(MFISYM, 10), /* "UY UZ" */
    ADDRESS(LOCSKIRT),
    ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE_AT(COF, 2), /* "SIGN OF UY OPPOSITE TO UZ" */
    STORE_AT(COF, 2),
    ORDER(GOTO),
    ADDRESS(LOCSKIRT),
};

#if LUMINARY099

/* MXM3 -- LM p. 356. "MATRIX OPERATIONS" (the listing's BANK 13 is
 * overridden by its SETLOC KALCMON2: they are in bank 22 too).
 * "MXM3 MULTIPLIES 2 3X3 MATRICES AND LEAVES RESULT IN PD LIST AND MPAC":
 * the matrix X1 names times the one X2 names, each row of the first (0,1,
 * 6,1 and 12D,1) times the second (VXM*, 0,2), pushed down in turn. */
static const word mxm3_words[] = {
    /* MXM3 */ ORDERS(SETPD, VLOAD_STAR), /* "MXM3 MULTIPLIES 2 3X3 MATRICES" */
    ADDRESS_PLUS_ONE_D(0),                /* "AND LEAVES RESULT IN PD LIST" */
    ADDRESS_PLUS_ONE_X1(0),               /* "AND MPAC" */
    ORDERS(VXM_STAR, PDVL_STAR),
    ADDRESS_PLUS_ONE_X2(0),
    ADDRESS_PLUS_ONE_X1(6),
    ORDERS(VXM_STAR, PDVL_STAR),
    ADDRESS_PLUS_ONE_X2(0),
    ADDRESS_PLUS_ONE_X1(12),
    ORDERS(VXM_STAR, PUSH),
    ADDRESS_PLUS_ONE_X2(0),
    ORDER(RVQ),
};

#else

/* MXM3 -- CM p. 404. "MATRIX OPERATIONS". "MXM3 MULTIPLIES 2 3X3
 * MATRICES AND LEAVES RESULT IN PD LIST" ("ADDRESS OF 1ST MATRIX IN XR1,
 * ADDRESS OF 2ND MATRIX IN XR2"), a column at a time: each column of the
 * second made a vector (VDEF of its three words, pushed down last first)
 * and the first matrix times it (MXV*, 0,1), pushed down. The three
 * products are the result's columns; TRNSPSPD turns them into its rows
 * ("REVERSE ROWS AND COLS IN PD AND RETURN WITH M1XM2 IN PD LIST"). */
static const word mxm3_words[] = {
    /* MXM3 */ ORDER(SETPD),       /* "MXM3 MULTIPLIES 2 3X3 MATRICES" */
    ADDRESS_PLUS_ONE_D(0),         /* "AND LEAVES RESULT IN PD LIST" */
    ORDERS(DLOAD_STAR, PDDL_STAR), /* "ADDRESS OF 1ST MATRIX IN XR1" */
    ADDRESS_PLUS_ONE_X2(12),       /* "ADDRESS OF 2ND MATRIX IN XR2" */
    ADDRESS_PLUS_ONE_X2(6),
    ORDERS(PDDL_STAR, VDEF), /* "DEFINE VECTOR M2(COL 1)" */
    ADDRESS_PLUS_ONE_X2(0),
    ORDERS(MXV_STAR, PDDL_STAR), /* "M1XM2(COL 1) IN PD" */
    ADDRESS_PLUS_ONE_X1(0),
    ADDRESS_PLUS_ONE_X2(14),
    ORDERS(PDDL_STAR, PDDL_STAR),
    ADDRESS_PLUS_ONE_X2(8),
    ADDRESS_PLUS_ONE_X2(2),
    ORDERS(VDEF, MXV_STAR), /* "DEFINE VECTOR M2(COL 2)" */
    ADDRESS_PLUS_ONE_X1(0),
    ORDERS(PDDL_STAR, PDDL_STAR), /* "M1XM2(COL 2) IN PD" */
    ADDRESS_PLUS_ONE_X2(16),
    ADDRESS_PLUS_ONE_X2(10),
    ORDERS(PDDL_STAR, VDEF), /* "DEFINE VECTOR M2(COL 3)" */
    ADDRESS_PLUS_ONE_X2(4),
    ORDERS(MXV_STAR, PUSH), /* "M1XM2(COL 3) IN PD" */
    ADDRESS_PLUS_ONE_X1(0),
    ORDER(GOTO),
    ADDRESS(TRNSPSPD), /* "REVERSE ROWS AND COLS IN PD AND" */
};

#endif

/* TRANSPOS -- LM p. 356, CM p. 404.
 * "TRANSPOS TRANSPOSES A 3X3 MATRIX AND LEAVES RESULT IN PD LIST": the
 * matrix X1 names ("MATRIX ADDRESS IN XR1"), its three rows pushed down;
 * then TRNSPSPD swaps them across the diagonal. */
static const word transpos_words[] = {
    /* TRANSPOS */ ORDERS(SETPD, VLOAD_STAR), /* "TRANSPOS TRANSPOSES A 3X3 MATRIX" */
    ADDRESS_PLUS_ONE_D(0),                    /* "AND LEAVES RESULT IN PD LIST" */
    ADDRESS_PLUS_ONE_X1(0),                   /* "MATRIX ADDRESS IN XR1" */
    ORDERS(PDVL_STAR, PDVL_STAR),
    ADDRESS_PLUS_ONE_X1(6),
    ADDRESS_PLUS_ONE_X1(12),
    ORDER(PUSH), /* "MATRIX IN PD" */
};

#if LUMINARY099

/* TRNSPSPD -- LM p. 356. "ENTER WITH MATRIX AT 0 IN PD LIST": out of the
 * interpreter, to swap the words across the diagonal in basic code. */
static const word trnspspd_words[] = {
    /* TRNSPSPD */ ORDER(EXIT), /* "ENTER WITH MATRIX AT 0 IN PD LIST" */
};

/* TRNSPSPD +1 -- LM pp. 356-357.
 * The matrix on the push-down list (rows at words 0, 6D and 12D of the
 * work area, two words an element) transposed in place: each element above
 * the diagonal swapped with the one below it through A,L, by three DXCHes
 * (INDEX FIXLOC: the work area's words), the first and last on one element
 * and the middle one on the other. The listing's addresses are octal: 12
 * and 16 are 10D and 14D, elements (1,2) and (2,1); 14 and 4 are 12D and
 * 4, (2,0) and (0,2); 2 and 6 are (0,1) and (1,0). Then back into the
 * interpreter. */
static void swap_across(unsigned through, unsigned other)
{
    dxch(indexed(through, FIXLOC)); /* INDEX FIXLOC; DXCH */
    dxch(indexed(other, FIXLOC));
    dxch(indexed(through, FIXLOC));
}

void trnspspd_after_exit(void)
{
    swap_across(012u, 016u);
    swap_across(014u, 04u);
    swap_across(02u, 06u);
    tc_at(TRNSPSPD + 023u); /* TC INTPRET */
    intpret();
}

/* TRNSPSPD +024 -- LM p. 357. Back from the basic code, return. */
static const word trnspspd_rvq_words[] = {
    ORDER(RVQ),
};

/* READCDUK -- LM p. 357. An RTB routine: "INTERPRETIVE SUBROUTINE TO
 * READ THE CDU ANGLES". "LOAD T(MPAC) WITH CDU ANGLES": CDUZ to MPAC +2,
 * CDUX and CDUY to MPAC and MPAC +1, "AND CHANGE MODE TO TRIPLE PRECISION"
 * (TLOAD +6). */
void readcduk(void)
{
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(MPAC) + 2u);
    dca(ADDRESS_OF(CDUX)); /* EXTEND; DCA CDUX */
    continue_at(TLOAD + 6u);
}

/* CDUTODCM, LOOPSIN -- LM pp. 357-358.
 * Three CDU angles (in MPAC, triple precision, as TLOAD left them) to the
 * direction cosine matrix from spacecraft axes to stable member axes,
 * stored where X2 says. "SET XR1, S1, AND PD FOR LOOP": X1 3, S1 (the step
 * TIX takes from it) 1; the angles kept at 7 to 11 of the work area.
 * LOOPSIN runs three times (TIX,1): each angle through RTB CDULOGIC (two's
 * complement to the interpreter's double precision) and pushed down as its
 * sine and cosine: "LOAD PD WITH 0 SIN(PHI), 2 COS(PHI), 4 SIN(THETA), 6
 * COS(THETA), 8 SIN(PSI), 10 COS(PSI)". Then the nine elements, C0 to C8,
 * as the listing's comment below (DCMTOCDU's) writes them out, PHI the
 * outer gimbal angle, THETA the inner, PSI the middle. */
static const word cdutodcm_words[] = {
    /* CDUTODCM */ ORDERS(AXT_1, SSP),
    NUMBER(3),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(1), /* "SET XR1, S1, AND PD FOR LOOP" */
    STORE_D(7),
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    /* LOOPSIN */ ORDERS(SLOAD_STAR, RTB),
    ADDRESS_PLUS_ONE_X1(10),
    ADDRESS(CDULOGIC),
    STORE_D(10),            /* "LOAD PD WITH 0 SIN(PHI)" */
    ORDERS(SIN, PDDL),      /* "2 COS(PHI)" */
    ADDRESS_PLUS_ONE_D(10), /* "4 SIN(THETA)" */
    ORDERS(COS, PUSH),      /* "6 COS(THETA)" */
    ORDERS(TIX_1, DLOAD),   /* "8 SIN(PSI)" */
    ADDRESS(LOOPSIN),       /* "10 COS(PSI)" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(10),
    STORE_X2_D(0), /* "C0 = COS(THETA)COS(PSI)" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(4),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PDDL, DMP), /* "(PD6 SIN(THETA)SIN(PHI))" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(BDSU, SL1),
    ADDRESS_PLUS_ONE_D(12),
    STORE_X2_D(2), /* "C1=-COS(THETA)SIN(PSI)COS(PHI)" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(PDDL, DMP), /* "(PD7 COS(PHI)SIN(THETA)) SCALED 4" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SL1),
    ADDRESS_PLUS_ONE_D(14),
    STORE_X2_D(4), /* "C2=COS(THETA)SIN(PSI)SIN(PHI)" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(8),
    STORE_X2_D(6), /* "C3=SIN(PSI)" */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(2),
    STORE_X2_D(8), /* "C4=COS(PSI)COS(PHI)" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DCOMP, SL1),
    STORE_X2_D(10), /* "C5=-COS(PSI)SIN(PHI)" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(4),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(DCOMP, SL1),
    STORE_X2_D(12), /* "C6=-SIN(THETA)COS(PSI)" */
    ORDER(DLOAD),
    ORDERS(DMP, SL1), /* "(PUSH UP 7)" */
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(PDDL, DMP), /* "(PD7 COS(PHI)SIN(THETA)SIN(PSI)) SCALE 4" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SL1),            /* "(PUSH UP 7)" */
    ORDER(STADR),                /* "C7=COS(PHI)SIN(THETA)SIN(PSI)" */
    AFTER_STADR(STORE_X2_D(14)), /* "+COS(THETA)SIN(PHI)" */
    ORDER(DLOAD),
    ORDERS(DMP, SL1), /* "(PUSH UP 6)" */
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(PDDL, DMP), /* "(PD6 SIN(THETA)SIN(PHI)SIN(PSI)) SCALE 4" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DSU, SL1), /* "(PUSH UP 6)" */
    ORDER(STADR),
    AFTER_STADR(STORE_X2_D(16)), /* "C8=-SIN(THETA)SIN(PHI)SIN(PSI)" */
    ORDER(RVQ),                  /* "+COS(THETA)COS(PHI)" */
};

#else

/* TRNSPSPD -- CM pp. 404-405. "ENTER WITH MATRIX IN PD LIST": the matrix
 * on the push-down list (rows at 0, 6D and 12D, two words an element)
 * transposed in place, interpretively. For each pair across the
 * diagonal, the first is pushed down (PDDL) while the second is loaded and
 * stored in the first's place (STODL); the first, loaded back off the
 * push-down list, goes in the second's (STADR, then the store): (0,1) and
 * (1,0) at 2 and 6, (0,2) and (2,0) at 4 and 12D, (1,2) and (2,1) at 10D
 * and 14D. "RETURN WITH TRANSPOSED MATRIX IN PD LIST". */
static const word trnspspd_words[] = {
    /* TRNSPSPD */ ORDERS(DLOAD, PDDL), /* "ENTER WITH MATRIX IN PD LIST" */
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE_D(6),
    STODL_D(2),
    ORDER(STADR),
    AFTER_STADR(STODL_D(6)),
    ADDRESS_PLUS_ONE_D(4),
    ORDER(PDDL),
    ADDRESS_PLUS_ONE_D(12),
    STODL_D(4),
    ORDER(STADR),
    AFTER_STADR(STODL_D(12)),
    ADDRESS_PLUS_ONE_D(10),
    ORDER(PDDL),
    ADDRESS_PLUS_ONE_D(14),
    STODL_D(10),
    ORDER(STADR),
    AFTER_STADR(STORE_D(14)),
    ORDER(RVQ), /* "RETURN WITH TRANSPOSED MATRIX IN PD LIST" */
};

/* READCDUK -- CM p. 405. An RTB routine: "LOAD T(MPAC) WITH THE CURRENT
 * CDU ANGLES", as the LM's (CDUZ to MPAC +2, CDUX and CDUY to MPAC and
 * MPAC +1, then TLOAD +6), with interrupts held off while it reads them. */
void readcduk(void)
{
    inhint();
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(MPAC) + 2u);
    dca(ADDRESS_OF(CDUX)); /* EXTEND; DCA CDUX */
    relint();
    continue_at(TLOAD + 6u);
}

/* CDUTODCM, LOOPSIN -- CM pp. 405-407.
 * As the LM's (the same arithmetic, C0 to C8 from the sines and cosines of
 * the three angles), with its orders paired differently: most loads stand
 * alone in a word, and the shifts after the products come later. */
static const word cdutodcm_words[] = {
    /* CDUTODCM */ ORDERS(AXT_1, SSP), /* "SUBROUTINE TO COMPUTE DIRECTION COSINE */
    NUMBER(3),                         /* MATRIX RELATING S/C AXES TO STABLE */
    ADDRESS_PLUS_ONE(S1),              /* MEMBER AXES FROM 3 CDU ANGLES IN T(MPAC)" */
    NUMBER(1),                         /* "SET XR1, S1 AND PD FOR LOOP" */
    STORE_D(7),
    ORDER(SETPD),
    ADDRESS_PLUS_ONE_D(0),
    /* LOOPSIN */ ORDERS(SLOAD_STAR, RTB),
    ADDRESS_PLUS_ONE_X1(10),
    ADDRESS(CDULOGIC),
    STORE_D(10),            /* "LOAD PD WITH 0 SIN(PHI)" */
    ORDERS(SIN, PDDL),      /* "2 COS(PHI)" */
    ADDRESS_PLUS_ONE_D(10), /* "4 SIN(THETA)" */
    ORDERS(COS, PUSH),      /* "6 COS(THETA)" */
    ORDERS(TIX_1, DLOAD),   /* "8 SIN(PSI)" */
    ADDRESS(LOOPSIN),       /* "10 COS(PSI)" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(10),
    STORE_X2_D(0), /* C0 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE_D(0), /* "(PD6 SIN(THETA)SIN(PHI))" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_D(8),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SL1, BDSU),
    ADDRESS_PLUS_ONE_D(12),
    ORDER(SL1),
    STORE_X2_D(2), /* C1 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DMP, PDDL), /* "(PD7 COS(PHI)SIN(THETA)) SCALED 4" */
    ADDRESS_PLUS_ONE_D(4),
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_D(8),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(SL1),
    ORDERS(DAD, SL1),
    ADDRESS_PLUS_ONE_D(14),
    STORE_X2_D(4), /* C2 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(8),
    STORE_X2_D(6), /* C3 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(2),
    STORE_X2_D(8), /* C4 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(DMP, DCOMP),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(SL1),
    STORE_X2_D(10), /* C5 */
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DMP, DCOMP),
    ADDRESS_PLUS_ONE_D(10),
    ORDER(SL1),
    STORE_X2_D(12), /* C6 */
    ORDER(DLOAD),
    ORDERS(DMP, SL1), /* "(PUSH UP 7)" */
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(PDDL, DMP), /* "(PD7 COS(PHI)SIN(THETA)SIN(PSI)) SCALE4" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SL1),            /* "(PUSH UP 7)" */
    ORDER(STADR),                /* "C7=COS(PHI)SIN(THETA)SIN(PSI)" */
    AFTER_STADR(STORE_X2_D(14)), /* C7 */
    ORDER(DLOAD),
    ORDERS(DMP, SL1), /* "(PUSH UP 6)" */
    ADDRESS_PLUS_ONE_D(8),
    ORDERS(PDDL, DMP), /* "(PD6 SIN(THETA)SIN(PHI)SIN(PSI)) SCALE4" */
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(DSU, SL1), /* "(PUSH UP 6)" */
    ORDER(STADR),
    AFTER_STADR(STORE_X2_D(16)), /* "C8=-SIN(THETA)SIN(PHI)SIN(PSI)" */
    ORDER(RVQ),                  /* "+COS(THETA)COS(PHI)" */
};

#endif

/* DELCOMP's matrix, by the mission's name for it. */
#if LUMINARY099
#define I_DELMATRIX I_KEL
#else
#define I_DELMATRIX I_DEL
#endif

/* DELCOMP -- LM pp. 359-361, CM pp. 407-409.
 * "CALCULATION OF THE MATRIX DEL": the rotation by the angle A in MPAC
 * about the unit vector COF, U, "DEL = (IDMATRIX)COS(A)+UU (1-COS(A))+UX
 * SIN(A) SCALED 1", into DELMATRIX (the LM's KEL, the CM's DEL: both
 * lie on MFS). "COMPUTE THE DIAGONAL COMPONENTS OF DEL",
 * then "THE OFF DIAGONAL TERMS", each pair across the diagonal the same
 * product of components plus and minus one SIN(A) term. Any overflow
 * (BOVB) goes to SIGNMPAC, the largest value of the sign. */
static const word delcomp_words[] = {
    /* DELCOMP */ ORDERS(SETPD, PUSH), /* "MPAC CONTAINS THE ANGLE A" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SIN, PDDL), /* "PD0 = SIN(A)" */
    ORDERS(COS, PUSH), /* "PD2 = COS(A)" */
    ORDERS(SR2, PDDL), /* "PD2 = COS(A) $8" */
    ORDERS(BDSU, BOVB),
    ADDRESS_PLUS_ONE(DPHALF),
    ADDRESS(SIGNMPAC),
    ORDER(PDDL), /* "PD4 = 1-COS(A)" */
    ADDRESS_PLUS_ONE(COF),
    ORDERS(DSQ, DMP),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DAD, SL3),
    ADDRESS_PLUS_ONE_D(2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL(DELMATRIX), /* "UX UX(1-COS(A)) +COS(A) $1" */
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ORDERS(DSQ, DMP),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DAD, SL3),
    ADDRESS_PLUS_ONE_D(2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 8), /* "UY UY(1-COS(A)) +COS(A) $1" */
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ORDERS(DSQ, DMP),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DAD, SL3),
    ADDRESS_PLUS_ONE_D(2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STORE_AT(DELMATRIX, 16), /* "UZ UZ(1-COS(A)) +COS(A) $1" */
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(COF),
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(PDDL, DMP), /* "D6 UX UY (1-COS A) $4" */
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(PUSH, DAD), /* "D8 UZ SIN A $4" */
    ADDRESS_PLUS_ONE_D(6),
    ORDERS(SL2, BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 6),
    ORDERS(BDSU, SL2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 2),
    ADDRESS_PLUS_ONE(COF),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(SL1, PDDL), /* "D6 UX UZ (1-COS A) $4" */
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ORDERS(DMP, PUSH), /* "D8 UY SIN(A)" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SL2),
    ADDRESS_PLUS_ONE_D(6),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 4), /* "UX UZ (1-COS(A))+UY SIN(A)" */
    ORDERS(BDSU, SL2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 12), /* "UX UZ (1-COS(A))-UY SIN(A)" */
    ADDRESS_PLUS_ONE_AT(COF, 2),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_AT(COF, 4),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(SL1, PDDL), /* "D6 UY UZ (1-COS(A)) $ 4" */
    ADDRESS_PLUS_ONE(COF),
    ORDERS(DMP, PUSH), /* "D8 UX SIN(A)" */
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DAD, SL2),
    ADDRESS_PLUS_ONE_D(6),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STODL_AT(DELMATRIX, 14), /* "UY UZ(1-COS(A)) +UX SIN(A)" */
    ORDERS(BDSU, SL2),
    ORDER(BOVB),
    ADDRESS(SIGNMPAC),
    STORE_AT(DELMATRIX, 10), /* "UY UZ (1-COS(A)) -UX SIN(A)" */
    ORDER(RVQ),
};

/* DCMTOCDU, SUHALFA, CALCPHI, OKTHETA, SUHALFAP, OKPHI, VECOFANG -- LM
 * p. 362, CM pp. 409-410. "DIRECTION COSINE MATRIX TO CDU ANGLE ROUTINE": "X1 CONTAINS THE
 * COMPLEMENT OF THE STARTING ADDRESS FOR MATRIX (SCALED 2). LEAVE CDU
 * ANGLES SCALED 2PI IN V(MPAC). COS(MGA) WILL BE LEFT IN S1 (SCALED 1)."
 * The middle gimbal angle PSI is the arcsine of C3; THETA from -C6/COS(PSI)
 * and PHI from -C5/COS(PSI), each by arcsine, then moved into the right
 * half turn by the sign of C0 (THETA, "MUST CHECK THE SIGN OF COS(THETA)
 * TO DETERMINE THE PROPER QUADRANT") and of C4 (PHI). VECOFANG makes the
 * three a vector, (PHI, THETA, PSI). */
static const word dcmtocdu_words[] = {
    /* DCMTOCDU */ ORDERS(DLOAD_STAR, ARCSIN),
    ADDRESS_PLUS_ONE_X1(6),
    ORDERS(PUSH, COS), /* "PD +0 PSI" */
    ORDERS(SL1, BOVB),
    ADDRESS(SIGNMPAC),
    STORE(S1),
    ORDERS(DLOAD_STAR, DCOMP),
    ADDRESS_PLUS_ONE_X1(12),
    ORDERS(DDV, ARCSIN),
    ADDRESS_PLUS_ONE(S1),
    ORDERS(PDDL_STAR, BPL), /* "PD +2 THETA" */
    ADDRESS_PLUS_ONE_X1(0), /* "MUST CHECK THE SIGN OF COS(THETA)" */
    ADDRESS(OKTHETA),       /* "TO DETERMINE THE PROPER QUADRANT" */
    ORDERS(DLOAD, DCOMP),
    ORDERS(BPL, DAD),
    ADDRESS(SUHALFA),
    ADDRESS_PLUS_ONE(DPHALF),
    ORDER(GOTO),
    ADDRESS(CALCPHI),
    /* SUHALFA */ ORDER(DSU),
    ADDRESS_PLUS_ONE(DPHALF),
    /* CALCPHI */ ORDER(PUSH),
    /* OKTHETA */ ORDERS(DLOAD_STAR, DCOMP),
    ADDRESS_PLUS_ONE_X1(10),
    ORDERS(DDV, ARCSIN),
    ADDRESS_PLUS_ONE(S1),
    ORDERS(PDDL_STAR, BPL), /* "PUSH DOWN PHI" */
    ADDRESS_PLUS_ONE_X1(8),
    ADDRESS(OKPHI),
    ORDERS(DLOAD, DCOMP), /* "PUSH UP PHI" */
    ORDERS(BPL, DAD),
    ADDRESS(SUHALFAP),
    ADDRESS_PLUS_ONE(DPHALF),
    ORDER(GOTO),
    ADDRESS(VECOFANG),
    /* SUHALFAP */ ORDERS(DSU, GOTO),
    ADDRESS_PLUS_ONE(DPHALF),
    ADDRESS(VECOFANG),
    /* OKPHI */ ORDER(DLOAD), /* "PUSH UP PHI" */
    /* VECOFANG */ ORDERS(VDEF, RVQ),
};

#if LUMINARY099

/* TOOBADF -- LM p. 363. "ROUTINES FOR TERMINATING THE AUTOMATIC
 * MANEUVER AND RETURNING TO USER." */
static const word toobadf_words[] = {
    /* TOOBADF */ ORDER(EXIT),
};

/* TOOBADF +1 -- LM p. 363. After the EXIT: alarm 00401, the desired
 * middle gimbal angle past 70 degrees. */
void toobadf_after_exit(void)
{
    tc_at(TOOBADF + 1u);
    continue_at(ALARM); /* OCT 00401 */
}

/* TOOBADF +3 -- LM p. 363. Back from ALARM: "DO NOT ZERO ATTITUDE ERRORS". */
void toobadf_after_alarm(void)
{
    nogo(); /* TCF NOGO */
}

/* NOGO -2 -- LM p. 363. Where KALCMANU_STEERING's TCF NOGO -2 comes in:
 * "ZERO ATTITUDE ERRORS" (ZATTEROR), then NOGO. */
void nogo_minus2(void)
{
    tc_at(NOGO - 2u);
    continue_at(BANKCALL); /* CADR ZATTEROR */
}

/* NOGO -- LM p. 363. "STOP RATES" (STOPRATE). */
void nogo(void)
{
    tc_at(NOGO);
    continue_at(BANKCALL); /* CADR STOPRATE */
}

/* NOGO +2 -- LM p. 363. "ALL RETURNS ARE NOW MADE VIA GOODEND": GOODMANU
 * as a task in two centiseconds, and the job ends. */
void nogo_after_stoprate(void)
{
    set_A(TWO);
    inhint();
    tc_at(NOGO + 4u);
    continue_at(WAITLIST); /* 2CADR GOODMANU */
}

/* NOGO +7 -- LM p. 363. */
void nogo_after_waitlist(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* TOOBADI -- LM p. 363. */
static const word toobadi_words[] = {
    /* TOOBADI */ ORDER(EXIT),
};

/* TOOBADI +1 -- LM p. 363. After the EXIT: stop the rates. */
void toobadi_after_exit(void)
{
    nogo(); /* TCF NOGO */
}

/* The tables, where they lie in bank 22. */
static const struct interpretive_code KALCMAN3_CODE = {022u, 02005u, COUNT(kalcman3_words),
                                                       kalcman3_words};
static const struct interpretive_code SECAD_CODE = {022u, 02027u, COUNT(secad_words), secad_words};
static const struct interpretive_code CHECKMAX_CODE = {022u, 02117u, COUNT(checkmax_words),
                                                       checkmax_words};
static const struct interpretive_code ALTCALC_CODE = {022u, 02131u, COUNT(altcalc_words),
                                                      altcalc_words};
static const struct interpretive_code COFMAXGO_CODE = {022u, 02176u, COUNT(cofmaxgo_words),
                                                       cofmaxgo_words};
static const struct interpretive_code METHOD2_CODE = {022u, 02216u, COUNT(method2_words),
                                                      method2_words};
static const struct interpretive_code METHOD1_CODE = {022u, 02242u, COUNT(method1_words),
                                                      method1_words};
static const struct interpretive_code METHOD3_CODE = {022u, 02266u, COUNT(method3_words),
                                                      method3_words};
static const struct interpretive_code MXM3_CODE = {022u, 02312u, COUNT(mxm3_words), mxm3_words};
static const struct interpretive_code TRANSPOS_CODE = {022u, 02326u, COUNT(transpos_words),
                                                       transpos_words};
static const struct interpretive_code TRNSPSPD_CODE = {022u, 02335u, COUNT(trnspspd_words),
                                                       trnspspd_words};
static const struct interpretive_code TRNSPSPD_RVQ_CODE = {022u, 02361u, COUNT(trnspspd_rvq_words),
                                                           trnspspd_rvq_words};
static const struct interpretive_code CDUTODCM_CODE = {022u, 02410u, COUNT(cdutodcm_words),
                                                       cdutodcm_words};
static const struct interpretive_code DELCOMP_CODE = {022u, 02527u, COUNT(delcomp_words),
                                                      delcomp_words};
static const struct interpretive_code DCMTOCDU_CODE = {022u, 02654u, COUNT(dcmtocdu_words),
                                                       dcmtocdu_words};
static const struct interpretive_code TOOBADF_CODE = {022u, 02724u, COUNT(toobadf_words),
                                                      toobadf_words};
static const struct interpretive_code TOOBADI_CODE = {022u, 02742u, COUNT(toobadi_words),
                                                      toobadi_words};

const struct interpretive_code *const ATTITUDE_MANEUVER_CODES[] = {
    &KALCMAN3_CODE, &SECAD_CODE,        &CHECKMAX_CODE, &ALTCALC_CODE, &COFMAXGO_CODE,
    &METHOD2_CODE,  &METHOD1_CODE,      &METHOD3_CODE,  &MXM3_CODE,    &TRANSPOS_CODE,
    &TRNSPSPD_CODE, &TRNSPSPD_RVQ_CODE, &CDUTODCM_CODE, &DELCOMP_CODE, &DCMTOCDU_CODE,
    &TOOBADF_CODE,  &TOOBADI_CODE,
};

#else

/* NOGOM2 -- CM p. 411. "ROUTINE FOR TERMINATING AUTOMATIC MANEUVERS".
 * KALCMANU_STEERING's "BZMF NOGO -2" comes in at the word after this
 * INHINT (NOGO -2): ZEROEROR, by BANKCALL, takes the attitude errors away
 * (the desired angles become the CDUs'). Then NOGO. */
void nogom2(void)
{
    inhint();
    nogo_minus2();
}

/* NOGO -2 -- CM p. 411. TC BANKCALL, CADR ZEROEROR. */
void nogo_minus2(void)
{
    tc_at(NOGO - 2u);
    continue_at(BANKCALL); /* CADR ZEROEROR */
}

/* NOGO -- CM p. 411. STOPRATE (in fixed-fixed memory, TC'd directly)
 * stops the rates. */
void nogo(void)
{
    inhint();
    tc_at(NOGO + 1u);
    continue_at(STOPRATE);
}

/* NOGO +2 -- CM p. 411. "TERMINATE MANEUVER": ENDMANU as a task in two
 * centiseconds ("NOTE - ALL RETURNS ARE NOW MADE VIA GOODEND"). */
void nogo_after_stoprate(void)
{
    set_A(TWO); /* CAF TWO */
    tc_at(NOGO + 3u);
    continue_at(WAITLIST); /* 2CADR ENDMANU */
}

/* NOGO +6 -- CM p. 411. The job ends. */
void nogo_after_waitlist(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* The tables, where they lie in bank 22. */
static const struct interpretive_code KALCMAN3_CODE = {022u, 02001u, COUNT(kalcman3_words),
                                                       kalcman3_words};
static const struct interpretive_code SECAD_CODE = {022u, 02016u, COUNT(secad_words), secad_words};
static const struct interpretive_code CHECKMAX_CODE = {022u, 02111u, COUNT(checkmax_words),
                                                       checkmax_words};
static const struct interpretive_code ALTCALC_CODE = {022u, 02123u, COUNT(altcalc_words),
                                                      altcalc_words};
static const struct interpretive_code COFMAXGO_CODE = {022u, 02170u, COUNT(cofmaxgo_words),
                                                       cofmaxgo_words};
static const struct interpretive_code METHOD2_CODE = {022u, 02210u, COUNT(method2_words),
                                                      method2_words};
static const struct interpretive_code METHOD1_CODE = {022u, 02234u, COUNT(method1_words),
                                                      method1_words};
static const struct interpretive_code METHOD3_CODE = {022u, 02260u, COUNT(method3_words),
                                                      method3_words};
static const struct interpretive_code MXM3_CODE = {022u, 02304u, COUNT(mxm3_words), mxm3_words};
static const struct interpretive_code TRANSPOS_CODE = {022u, 02334u, COUNT(transpos_words),
                                                       transpos_words};
static const struct interpretive_code TRNSPSPD_CODE = {022u, 02343u, COUNT(trnspspd_words),
                                                       trnspspd_words};
static const struct interpretive_code CDUTODCM_CODE = {022u, 02405u, COUNT(cdutodcm_words),
                                                       cdutodcm_words};
static const struct interpretive_code DELCOMP_CODE = {022u, 02530u, COUNT(delcomp_words),
                                                      delcomp_words};
static const struct interpretive_code DCMTOCDU_CODE = {022u, 02655u, COUNT(dcmtocdu_words),
                                                       dcmtocdu_words};

const struct interpretive_code *const ATTITUDE_MANEUVER_CODES[] = {
    &KALCMAN3_CODE, &SECAD_CODE,    &CHECKMAX_CODE, &ALTCALC_CODE,  &COFMAXGO_CODE,
    &METHOD2_CODE,  &METHOD1_CODE,  &METHOD3_CODE,  &MXM3_CODE,     &TRANSPOS_CODE,
    &TRNSPSPD_CODE, &CDUTODCM_CODE, &DELCOMP_CODE,  &DCMTOCDU_CODE,
};

#endif

const unsigned ATTITUDE_MANEUVER_CODE_COUNT = COUNT(ATTITUDE_MANEUVER_CODES);
