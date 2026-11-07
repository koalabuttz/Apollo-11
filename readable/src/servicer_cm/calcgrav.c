/* SPDX-License-Identifier: MIT */
/* SERVICER207.agc, pp. 835-836 (Comanche055, bank 37): "ROUTINE CALCRVG
 * INTEGRATES THE EQUATIONS OF MOTION BY AVERAGING THE THRUST AND
 * GRAVITATIONAL ACCELERATIONS OVER A TIME INTERVAL OF 2 SECONDS.", with
 * the gravity from CALCGRAV. "FOR THE EARTH-CENTERED GRAVITATIONAL FIELD,
 * THE PERTURBATION DUE TO OBLATENESS IS COMPUTED TO THE FIRST HARMONIC
 * COEFFICIENT J." The LM's are in src/servicer/average_g.c. The faithful
 * C is c64-port/src/cm_servicer.c. */
#include "servicer_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- CALCGRAV -- p. 835 ---- */

/* CALCGRAV: the gravity of RN ("ENTER WITH RN IN MPAC"), half of it times
 * two seconds (GDT1/2). The unit vector, UNITR, pushed. Then, but near
 * the moon (RTX2 2, its complement in X1 going negative: BMN ITISMOON),
 * the earth's oblateness from J (20J, 2J) and UNITW, "THE EARTH S UNIT
 * ROTATIONAL VECTOR": GOBL1/2, added to the unit vector. ITISMOON: R MAG
 * SQ. (34D) normalized, -MUDT(E), or by X1 -MUDT(M), over it, shifted
 * back by X2, times the vector: GDT1/2 "SCALED AT 2(+7) M/CS". */
static const word calcgrav_words[] = {
    /* CALCGRAV */ ORDERS(UNIT, PUSH), /* "ENTER WITH RN IN MPAC" */
    STORE(UNITR),
    ORDERS(LXC_1, SLOAD),
    ADDRESS(RTX2),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(BMN, VLOAD),
    ADDRESS(ITISMOON),
    ORDERS(DOT, PUSH),
    ADDRESS_PLUS_ONE(UNITW),
    ORDERS(DSQ, BDSU),
    ADDRESS_PLUS_ONE(DP1_20),
    ORDERS(PDDL, DDV),
    ADDRESS_PLUS_ONE(RESQ),
    ADDRESS_PLUS_ONE_D(34), /* "(RN)SQ" */
    STORE_D(32),            /* "TEMP FOR (RE/RN)SQ" */
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(20J),
    ORDERS(VXSC, PDDL),
    ADDRESS_PLUS_ONE(UNITR),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(2J),
    ADDRESS_PLUS_ONE_D(32),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(UNITW),
    ORDER(STADR),
    AFTER_STADR(STORE(GOBL1_2)),
    ORDERS(VAD, PUSH),
    ADDRESS_PLUS_ONE(UNITR),
    /* ITISMOON */ ORDERS(DLOAD, NORM),
    ADDRESS_PLUS_ONE_D(34),
    ADDRESS_PLUS_ONE(X2),
    ORDERS(BDDV_STAR, SLR_STAR),
    ADDRESS_PLUS_ONE_AT_X1(MMUDT_E, 0),
    SLR_X2(-21),
    ORDERS(VXSC, STADR),
    AFTER_STADR(STORE(GDT1_2)), /* "SCALED AT 2(+7) M/CS" */
    ORDER(RVQ),
};

const struct interpretive_code CALCGRAV_CODE = {037u, CALCGRAV, COUNT(calcgrav_words),
                                                calcgrav_words};

/* ---- CALCRVG -- pp. 835-836 ---- */

/* CALCRVG: DELV, times KPIP1, in the reference frame: "DELV IN REF COORDS
 * AT 2(+7)" (DELVREF). Half of it, and "(DV-OLDGDT)/2 TO PD SCALED AT
 * 2(+7)M/CS" (GDT/2, the old gravity); the position two seconds on (VN
 * times 2SEC(22), plus RN): "TEMP STORAGE OF RN SCALED 2(+29)M" (RN1),
 * with its gravity (CALCGRAV); and the velocity, VN plus the velocity
 * change and both gravities: "TEMP STORAGE OF VN SCALED 2(+7)M/CS." (VN1).
 * Back through 31D, where STQ left the return. */
static const word calcrvg_words[] = {
    /* CALCRVG */ ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(DELV),
    ADDRESS_PLUS_ONE(KPIP1),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STORE(DELVREF), /* "DELV IN REF COORDS AT 2(+7)" */
    ORDERS(VSR1, PUSH),
    ORDERS(VAD, PUSH), /* "(DV-OLDGDT)/2 TO PD SCALED AT 2(+7)M/CS" */
    ADDRESS_PLUS_ONE(GDT_2),
    ORDERS(VAD, VXSC),
    ADDRESS_PLUS_ONE(VN),
    ADDRESS_PLUS_ONE(2SEC_22),
    ORDERS(VAD, STQ),
    ADDRESS_PLUS_ONE(RN),
    ADDRESS_D(31),
    STCALL(RN1), /* "TEMP STORAGE OF RN SCALED 2(+29)M" */
    ADDRESS(CALCGRAV),
    ORDERS(VAD, VAD),
    ORDER(VAD),
    ADDRESS_PLUS_ONE(VN),
    STCALL(VN1), /* "TEMP STORAGE OF VN SCALED 2(+7)M/CS." */
    ADDRESS_D(31),
};

const struct interpretive_code CALCRVG_CODE = {037u, CALCRVG, COUNT(calcrvg_words), calcrvg_words};

#endif
