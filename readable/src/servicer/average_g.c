/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 880-883 (Luminary099 only, bank 33): the average G
 * integration. CALCRVG, the usual one, updates RN and VN from the
 * measured velocity change and the gravity (CALCGRAV) at the start and
 * end of the two seconds; RVBOTH does the same for the CSM's state, and
 * MUNRVG, "A SPECIAL AVERAGE G INTEGRATION ROUTINE USED BY THRUSTING
 * PROGRAMS WHICH FUNCTION IN THE VICINITY OF AN ASSUMED SPHERICAL MOON.",
 * for the LM's, with the moon's gravity (MUNGRAV). The faithful C is
 * c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* ---- CALCGRAV, CALCRVG -- pp. 880-881 ---- */

/* CALCGRAV: the gravity of the position in MPAC, half of it times two
 * seconds (GDT1/2). UNIT/R/ ("SAVE UNIT/R/ IN PUSHLIST"); then, but near
 * the moon ("RTX2 = 0 IF EARTH ORBIT, =2 IF LUNAR.", and X1 its
 * complement), the earth's oblateness: J (20J, 2J) and UNITZ, the earth's
 * axis, give UNITGOBL, added to the unit vector ("MPAC = UNIT GRAVITY
 * VECTOR."). CALCGRV1: "PERFORM A NORMALIZATION ON RMAGSQ IN ORDER TO BE
 * ABLE TO SCALE THE MU FOR MAXIMUM PRECISION." (-MUDT, the earth's or the
 * moon's by X1, over it, shifted back by X2), times the vector: GDT1/2,
 * "SCALED AT 2(+7) M/CS". */
static const word calcgrav_words[] = {
    /* CALCGRAV */ ORDERS(UNIT, PUSH), /* "SAVE UNIT/R/ IN PUSHLIST" */
    STORE(UNIT_R_),
    ORDERS(LXC_1, SLOAD), /* "RTX2 = 0 IF EARTH ORBIT, =2 IF LUNAR." */
    ADDRESS(RTX2),
    ADDRESS_PLUS_ONE(RTX2),
    ORDERS(DCOMP, BMN),
    ADDRESS(CALCGRV1),
    ORDERS(VLOAD, DOT),
    ADDRESS_PLUS_ONE(UNITZ),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(SL1, PUSH),
    ORDERS(DSQ, BDSU),
    ADDRESS_PLUS_ONE(DP1_20),
    ORDERS(PDDL, DDV),
    ADDRESS_PLUS_ONE(RESQ),
    ADDRESS_PLUS_ONE_D(34), /* "(RN)SQ" */
    STORE_D(32),            /* "TEMP FOR (RE/RN)SQ" */
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(20J),
    ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(2J),
    ADDRESS_PLUS_ONE_D(32),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UNITZ),
    ORDERS(VAD, STADR),
    AFTER_STADR(STORE(UNITGOBL)),
    ORDERS(VAD, PUSH),                  /* "MPAC = UNIT GRAVITY VECTOR." */
    /* CALCGRV1 */ ORDERS(DLOAD, NORM), /* "PERFORM A NORMALIZATION ON RMAGSQ IN" */
    ADDRESS_PLUS_ONE_D(34),             /* "ORDER TO BE ABLE TO SCALE THE MU FOR" */
    ADDRESS_PLUS_ONE(X2),               /* "MAXIMUM PRECISION." */
    ORDERS(BDDV_STAR, SLR_STAR),
    ADDRESS_PLUS_ONE_AT_X1(MINUS_MUDT, 0),
    SLR_X2(-21),
    ORDERS(VXSC, STADR),
    AFTER_STADR(STORE(GDT1_2)), /* "SCALED AT 2(+7) M/CS" */
    ORDER(RVQ),
};

const struct interpretive_code CALCGRAV_CODE = {033u, CALCGRAV, COUNT(calcgrav_words),
                                                calcgrav_words};

/* CALCRVG: DELV in the reference frame, times KPIP1 (DELVREF); half of it
 * and "(DV-OLDGDT)/2 TO PD SCALED AT 2(+7) M/CS." (GDT/2, the old gravity);
 * the position two seconds on (VN times PGUIDE, plus RN): "TEMP STORAGE OF
 * RN SCALED 2(+29) M" (RN1), with its gravity (CALCGRAV); and the velocity,
 * VN plus the velocity change and both gravities: "TEMP STORAGE OF VN
 * SCALED 2(+7) M/CS" (VN1). Back through 31D, where STQ left the return. */
static const word calcrvg_words[] = {
    /* CALCRVG */ ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(DELV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(KPIP1),
    STORE(DELVREF),
    ORDERS(VSR1, PUSH),
    ORDERS(VAD, PUSH), /* "(DV-OLDGDT)/2 TO PD SCALED AT 2(+7) M/CS." */
    ADDRESS_PLUS_ONE(GDT_2),
    ORDERS(VAD, PDDL),
    ADDRESS_PLUS_ONE(VN),
    ADDRESS_PLUS_ONE(PGUIDE),
    ORDERS(SL, VXSC),
    SL_COUNT(6),
    ORDERS(VAD, STQ),
    ADDRESS_PLUS_ONE(RN),
    ADDRESS_D(31),
    STCALL(RN1), /* "TEMP STORAGE OF RN SCALED 2(+29) M" */
    ADDRESS(CALCGRAV),
    ORDERS(VAD, VAD),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(VN),
    STCALL(VN1), /* "TEMP STORAGE OF VN SCALED 2(+7) M/CS" */
    ADDRESS_D(31),
};

const struct interpretive_code CALCRVG_CODE = {033u, CALCRVG, COUNT(calcrvg_words), calcrvg_words};

/* ---- RVBOTH, MUNRVG, MUNGRAV -- pp. 882-883 ---- */

/* RVBOTH: the CSM's state two seconds on with its gravity, G(CSM), from
 * before and after (MUNGRAV): R1S and V1S. ... */
static const word rvboth_words[] = {
    /* RVBOTH */ ORDERS(VLOAD, PUSH),
    ADDRESS_PLUS_ONE(G_CSM),
    ORDERS(VAD, PDDL),
    ADDRESS_PLUS_ONE(V_CSM),
    ADDRESS_PLUS_ONE(PGUIDE),
    ORDERS(DDV, VXSC),
    ADDRESS_PLUS_ONE(SHIFT11),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(R_CSM),
    STCALL(R1S),
    ADDRESS(MUNGRAV),
    ORDERS(VAD, VAD),
    ADDRESS_PLUS_ONE(V_CSM),
    ORDER(STADR),
    AFTER_STADR(STORE(V1S)),
    ORDER(EXIT),
};

const struct interpretive_code RVBOTH_CODE = {033u, RVBOTH, COUNT(rvboth_words), rvboth_words};

/* ... the restart point (QUIKFAZ5), ... */
void rvboth_after_exit(void)
{
    tc_to(RVBOTH + 020u, QUIKFAZ5);
}

void rvboth_after_quikfaz5(void)
{
    tc_at(RVBOTH + 021u);
    intpret();
}

/* ... the new state the CSM's: G(CSM), R(CSM) and V(CSM); ... */
static const word rvboth_2_words[] = {
    /* RVBOTH +022 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(GDT1_2),
    STOVL(G_CSM),
    ADDRESS_PLUS_ONE(R1S),
    STOVL(R_CSM),
    ADDRESS_PLUS_ONE(V1S),
    STORE(V_CSM),
    ORDER(EXIT),
};

const struct interpretive_code RVBOTH_2_CODE = {033u, RVBOTH + 022u, COUNT(rvboth_2_words),
                                                rvboth_2_words};

/* ... the restart point again, and on to MUNRVG for the LM. */
void rvboth_after_second_exit(void)
{
    tc_to(RVBOTH + 032u, QUIKFAZ5);
}

void rvboth_after_second_quikfaz5(void)
{
    tc_at(RVBOTH + 033u);
    intpret();
}

/* MUNRVG: "THE INPUT AND OUTPUT QUANTITIES ARE REFERENCED TO THE STABLE
 * MEMBER COORDINATE SYSTEM." DELV times KPIP2 ("1ST PUSH:  DELV IN UNITS
 * OF 2(8) M/CS"), with GDT/2 ("2ND PUSH:  (DELV + GDT)/2, UNITS OF 2(7)"),
 * the position two seconds on: "STORE R SCALED AT 2(+24) M" (R1S), with
 * its gravity (MUNGRAV); the velocity, "STORE V SCALED AT 2(+7) M/CS."
 * (V1S); "STORE SPEED FOR LR AND DISPLAYS." (ABVEL); "HDOT = V. UNIT(R)*2(7)
 * M/CS." (HDOTDISP); "LUNAR ROTATION CORRECTION TERM*2(5) M/CS." (DELVS,
 * from WM); and HCALC, the altitude above /LAND/ ("FOR NOW, DISPLAY
 * WHETHER POS OR NEG"), on to MUNRETRN. */
static const word munrvg_words[] = {
    /* MUNRVG */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(DELV),
    ADDRESS_PLUS_ONE(KPIP2),
    ORDERS(PUSH, VAD), /* "1ST PUSH:  DELV IN UNITS OF 2(8) M/CS" */
    ADDRESS_PLUS_ONE(GDT_2),
    ORDERS(PUSH, VAD), /* "2ND PUSH:  (DELV + GDT)/2, UNITS OF 2(7)" */
    ADDRESS_PLUS_ONE(V),
    ORDERS(PDDL, DDV),
    ADDRESS_PLUS_ONE(PGUIDE),
    ADDRESS_PLUS_ONE(SHIFT11),
    ORDER(VXSC),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(R),
    STCALL(R1S), /* "STORE R SCALED AT 2(+24) M" */
    ADDRESS(MUNGRAV),
    ORDERS(VAD, VAD),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(V),
    STORE(V1S), /* "STORE V SCALED AT 2(+7) M/CS." */
    ORDER(ABVAL),
    STOVL(ABVEL), /* "STORE SPEED FOR LR AND DISPLAYS." */
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(V1S),
    STOVL(HDOTDISP), /* "HDOT = V. UNIT(R)*2(7) M/CS." */
    ADDRESS_PLUS_ONE(R1S),
    ORDERS(VXV, VSL2),
    ADDRESS_PLUS_ONE(WM),
    STODL(DELVS), /* "LUNAR ROTATION CORRECTION TERM*2(5) M/CS." */
    ADDRESS_PLUS_ONE_D(36),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(_LAND_),
    STCALL(HCALC), /* "FOR NOW, DISPLAY WHETHER POS OR NEG" */
    ADDRESS(MUNRETRN),
};

const struct interpretive_code MUNRVG_CODE = {033u, MUNRVG, COUNT(munrvg_words), munrvg_words};

/* MUNGRAV: the moon's gravity, "AT 36D HAVE ABVAL(R), AT 34D R.R": UNIT/R/,
 * and -MUDTMUN over R.R times the unit vector: "1/2GDT SCALED AT 2(7)
 * M/CS." (GDT1/2). */
static const word mungrav_words[] = {
    /* MUNGRAV */ ORDER(UNIT), /* "AT 36D HAVE ABVAL(R), AT 34D R.R" */
    STODL(UNIT_R_),
    ADDRESS_PLUS_ONE_D(34),
    ORDERS(SL, BDDV),
    SL_COUNT(6),
    ADDRESS_PLUS_ONE(MINUS_MUDTMUN),
    ORDERS(DMP, VXSC),
    ADDRESS_PLUS_ONE(SHIFT11),
    ADDRESS_PLUS_ONE(UNIT_R_),
    STORE(GDT1_2), /* "1/2GDT SCALED AT 2(7) M/CS." */
    ORDER(RVQ),
};

const struct interpretive_code MUNGRAV_CODE = {033u, MUNGRAV, COUNT(mungrav_words), mungrav_words};

#endif
