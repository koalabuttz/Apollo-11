/* SPDX-License-Identifier: MIT */
/* P37_P70.agc of the CM, part 1 (Comanche055 pp. 890-895, bank 36): P37's
 * main line, "RETURN TO EARTH": the crew's inputs, then the conic
 * trajectory home, found by iteration.
 *
 * The crew loads the time of ignition (V06N33, "INPUT TIG STORED IN
 * SPRTETIG") and, if they want, the entry angle and the change in speed
 * (V06N60, "INPUT REENTRY ANGLE IN GAMMAEI" "AND DESIRED DELTA V IN
 * RTEDVD"; each "PROGRM COMPUTED IF 0"). INVC100 brings the state to the
 * time of ignition; then the conic: a guess at X(T2) (the cotangent of
 * the entry angle at the far end, T2), V2T100 the transfer for it,
 * TMRAD100 the time to the entry radius; and again with X(T2) corrected
 * by the secant method until both the entry radius (RCON) and X(T2) have
 * settled. P37E shows the solution (RTEVN) and PREC100 does the precision
 * trajectory, which P37G shows in turn. */
#include "p37_p70.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- P37 -- pp. 890-891 ---- */

/* P37: group 4 made inactive (PHASCHNG, OCT 4: phase 0, "INACTIVE, WILL
 * NOT PERMIT A GROUP G RESTART"), for "P37 IS NOT RESTARTABLE". A
 * recycle from any of the program's displays, and the end of an alarm
 * ("RECYCLE AFTER ALARM DISPLAY", RTEALRM's EXIT), comes back here (TCF
 * P37). */
void p37(void)
{
    tc_to(P37, PHASCHNG); /* OCT 4 */
}

/* P37 +2: into the interpreter, */
void p37_after_phaschng(void)
{
    tc_at(P37 + 2u);
    intpret();
}

/* where ECSTEER, the "CROSS PRODUCT STEERING CONSTANT", is set to 1
 * (04000, scaled by 4) through X1, and VPRED and GAMMAEI start at +0
 * (ZEROVECS): so a display of them the crew does not load asks for the
 * program's own. */
static const word p37_words[] = {
    /* P37 +3 */ ORDERS(AXT_1, SXA_1), NUMBER(04000), ADDRESS(ECSTEER), ORDER(DLOAD),
    ADDRESS_PLUS_ONE(ZEROVECS),        STORE(VPRED),  STORE(GAMMAEI),   ORDER(EXIT),
};

const struct interpretive_code P37_CODE = {036u, P37 + 3u, COUNT(p37_words), p37_words};

/* P37 +013, after the EXIT: the time of ignition flashed for the crew
 * (V06N33, "OVERLAYED WITH TIG"); a recycle (TCF -2) shows it again,
 * "DISPLAY NEW DATA". */
void p37_tig(void)
{
    set_A(V6N33RTE);
    p370gof(P37 + 014u);
}

/* P37 +016, its proceed: the entry angle and the change in speed (V06N60,
 * through P37GFRB1, which blanks R1); a recycle shows them again,
 * "DISPLAY NEW DATA". */
void p37_gamma(void)
{
    set_A(V6N60RTE);
    p37gfrb1(P37 + 017u);
}

/* RTE299, its proceed: into the interpreter. */
void rte299(void)
{
    tc_at(RTE299);
    intpret();
}

/* The overflow indicator cleared (SSP OVFIND 0). The crew's VPRED is the
 * desired change in speed, RTEDVD, and GAMMAEI the entry angle, RTEGAM2D;
 * CONICX1 1 (1RTEB13) and MAMAX1 C4RTE, for INVC100, which brings the
 * state to the time of ignition ("GET R(T1)/,V(T1)/,UR1/,UH/"). SLOWFLG
 * clear; a negative RTEDVD is made positive, and then if the radius at
 * ignition, R(T1), is K1RTE or more, SLOWFLG set. */
static const word rte299_words[] = {
    /* RTE299 +1 */ ORDERS(SSP, DLOAD),
    ADDRESS_PLUS_ONE(OVFIND),
    NUMBER(0),
    ADDRESS_PLUS_ONE(VPRED),
    STODL(RTEDVD),
    ADDRESS_PLUS_ONE(GAMMAEI),
    STODL(RTEGAM2D),
    ADDRESS_PLUS_ONE(1RTEB13),
    STODL(CONICX1),
    ADDRESS_PLUS_ONE(C4RTE),
    STCALL(MAMAX1),
    ADDRESS(INVC100), /* "GET R(T1)/,V(T1)/,UR1/,UH/" */
    ORDERS(CLEAR, DLOAD),
    SWITCH(CLEAR, SLOWFLG),
    ADDRESS_PLUS_ONE(RTEDVD),
    ORDERS(BPL, ABS),
    ADDRESS(RTE317),
    STORE(RTEDVD),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(R_T1),
    ADDRESS_PLUS_ONE(K1RTE),
    ORDERS(BMN, SET),
    ADDRESS(RTE317),
    SWITCH(SET, SLOWFLG),
    /* RTE317 */ ORDERS(DLOAD, EXIT),
    ADDRESS_PLUS_ONE(R_T1),
};

const struct interpretive_code RTE299_CODE = {036u, RTE299 + 1u, COUNT(rte299_words), rte299_words};

/* ---- RTE317 to RTE369 -- pp. 891-893 ---- */

/* RTE317 +2, after the EXIT: TC POLY, a cubic in R(T1) whose degree and
 * coefficients are the words after the TC (DEC 2, then four 2DECs); POLY
 * returns past them, to RTE317 +014. */
void rte317_after_exit(void)
{
    tc_to(RTE317 + 2u, POLY);
}

/* RTE317 +014: into the interpreter, */
void rte317_after_poly(void)
{
    tc_at(RTE317 + 014u);
    intpret();
}

/* where the cubic is MAMAX2 ("C0+C1*R+C2*R**2+C3*R**3=MAMAX2 B30"); the
 * iterations counted in NN1A, from M9RTEB28 up; and the entry radius
 * RCON a first K2RTE ("RCON=K2").
 *
 * RTE320: the first X(T2). With no entry angle loaded ("GOTORTE340 IF
 * REENTRY ANGLE NOT INPUT"), K4RTE if R(T1) is below K1RTE ("X(T2)=K4"),
 * else K3RTE ("X(T2)=K3"); with one, its cotangent ("X(T2)=COT(GAM2D)"),
 * as the cosine over the sine of a quarter turn (1RTEB2) less the angle.
 *
 * RTE360: V2T100, the conic transfer for X(T2) (+0 back if it found one,
 * else to RTEALRM); TMRAD100 the time from R(T1)/ and V2(T1)/ to the
 * radius RCON, and T2 that time after T1. With an entry angle loaded,
 * X(T2) stays the one the crew asked for (RTE372); without, at RTE369, a
 * cubic in the speed at T2, |V(T2)/|, after the EXIT. */
static const word rte317_poly_words[] = {
    /* RTE317 +015 */ ORDER(SL1),
    STODL(MAMAX2), /* "C0+C1*R+C2*R**2+C3*R**3=MAMAX2 B30" */
    ADDRESS_PLUS_ONE(M9RTEB28),
    STODL(NN1A),
    ADDRESS_PLUS_ONE(K2RTE),
    /* RTE320 */ STODL(RCON), /* "RCON=K2" */
    ADDRESS_PLUS_ONE(RTEGAM2D),
    ORDERS(BZE, BDSU),
    ADDRESS(RTE340), /* "GOTORTE340 IF REENTRY ANGLE NOT INPUT" */
    ADDRESS_PLUS_ONE(1RTEB2),
    ORDERS(PUSH, COS),
    ORDERS(PDDL, SIN),
    ORDERS(BDDV, STADR),
    AFTER_STADR(STCALL(X_T2)), /* "X(T2)=COT(GAM2D)" */
    ADDRESS(RTE360),
    /* RTE340 */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(R_T1),
    ADDRESS_PLUS_ONE(K1RTE),
    ORDERS(BMN, DLOAD),
    ADDRESS(RTE350),
    ADDRESS_PLUS_ONE(K4RTE),
    STCALL(X_T2), /* "X(T2)=K4" */
    ADDRESS(RTE360),
    /* RTE350 */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(K3RTE),
    STORE(X_T2), /* "X(T2)=K3" */
    /* RTE360 */ ORDER(CALL),
    ADDRESS(V2T100),
    ORDERS(BZE, GOTO),
    ADDRESS(RTE367),
    ADDRESS(RTEALRM),
    /* RTE367 */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(R_T1_),
    STODL(RVEC),
    ADDRESS_PLUS_ONE(RCON),
    STOVL(RDESIRED),
    ADDRESS_PLUS_ONE(V2_T1_),
    STCALL(VVEC),
    ADDRESS(TMRAD100),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(T1),
    STODL(T2),
    ADDRESS_PLUS_ONE(RTEGAM2D),
    ORDERS(BZE, GOTO),
    ADDRESS(RTE369),
    ADDRESS(RTE372),
    /* RTE369 */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(V_T2_),
    ORDER(EXIT),
};

const struct interpretive_code RTE317_POLY_CODE = {036u, RTE317 + 015u, COUNT(rte317_poly_words),
                                                   rte317_poly_words};

/* RTE369 +3, after the EXIT: TC POLY, a cubic in the speed (DEC 2, then
 * four 2DECs, the first +0), back at RTE369 +015. */
void rte369_after_exit(void)
{
    tc_to(RTE369 + 3u, POLY);
}

/* RTE369 +015: into the interpreter, */
void rte369_after_poly(void)
{
    tc_at(RTE369 + 015u);
    intpret();
}

/* where RTED1 is added and the sum shifted: the X(T2) the speed calls
 * for, "X(T2),=D1+D2V2+D3V2**2+D4V2**3". (RTE372: with an entry angle
 * loaded, "X(T2),=X(T2)".)
 *
 * RTE373: the miss in X(T2) to the push-down list; RCON, the entry radius
 * at T2's place, from GETERAD (the earth's radius under the unit R(T2)/)
 * plus E3RTE: "RCON,=(E1/1+E2BETA11)**.5)+E3". If it is within EPC2RTE
 * of RCON (RTE374) and the miss within EPC3RTE, the conic is found: P37E.
 * Else (RTE375) one more iteration, unless NN1A has counted up to +0:
 * alarm 605 ("TOO MANY ITERATIONS"). The first correction (RTE385) is
 * the miss itself, "DX(T2)=X(T2)ERR"; later ones by the secant through
 * the last two tries, "DX(T2)=X(T2)ERR(Z2/Z1)". RTE390: RCON gets RCON,
 * ("RCON=RCON,"; an overflow goes round again at once), DRCON and RPRE'
 * keep this try's miss and X(T2), and X(T2) moves on by the correction:
 * "REITERATE".
 *
 * P37E: "DISPLAY CONIC SOLUTION" (RTEVN). PHI2, which side of the orbit
 * the entry is on: RCON against PCON times BETA1, "ENTRY NEAR APOGEE"
 * (+1RTEB2) when RCON is the greater, else "ENTRY NEAR PERIGEE" (its
 * negative). Then PREC100, the "PRECISION TRAJECTORY COMPUTATION": +0
 * back goes on to P37G; else an alarm (RTEALRM, through P370ALRM). */
static const word rte369_poly_words[] = {
    /* RTE369 +016 */ ORDER(DAD),
    ADDRESS_PLUS_ONE(RTED1),
    ORDERS(SL3, GOTO), /* "X(T2),=D1+D2V2+D3V2**2+D4V2**3" */
    ADDRESS(RTE373),
    /* RTE372 */ ORDER(DLOAD), /* "X(T2),=X(T2)" */
    ADDRESS_PLUS_ONE(X_T2),
    /* RTE373 */ ORDERS(DSU, PUSH),
    ADDRESS_PLUS_ONE(X_T2),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(R_T2_),
    STCALL(ALPHAV),
    ADDRESS(GETERAD),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(E3RTE),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(RCON),
    ORDERS(ABS, DSU),
    ADDRESS_PLUS_ONE(EPC2RTE),
    ORDERS(BMN, GOTO),
    ADDRESS(RTE374),
    ADDRESS(RTE375),
    /* RTE374 */ ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(EPC3RTE),
    ADDRESS(P37E),
    /* RTE375 */ ORDERS(DLOAD, DAD),
    ADDRESS_PLUS_ONE(NN1A),
    ADDRESS_PLUS_ONE(1RTEB28),
    ORDERS(BMN, SLOAD),
    ADDRESS(RTE380),
    ADDRESS_PLUS_ONE(OCT605),
    ORDER(GOTO),
    ADDRESS(RTEALRM), /* "TOO MANY ITERATIONS" */
    /* RTE380 */ STORE(NN1A),
    ORDERS(DSU, BZE),
    ADDRESS_PLUS_ONE(M8RTEB28),
    ADDRESS(RTE385),
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(DRCON),
    ORDERS(NORM, PDDL),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(RPRE_PRIME),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE(X_T2),
    ORDERS(DMP, SL_STAR), /* "DX(T2)=X(T2)ERR(Z2/Z1)" */
    ADDRESS_PLUS_ONE_D(0),
    SL_X1(0),
    ORDER(GOTO),
    ADDRESS(RTE390),
    /* RTE385 */ ORDER(DLOAD), /* "DX(T2)=X(T2)ERR" */
    ADDRESS_PLUS_ONE_D(0),
    /* RTE390 */ STODL_D(16),
    ORDER(STADR),
    AFTER_STADR(STODL(RCON)), /* "RCON=RCON," */
    ORDER(BOV),
    ADDRESS(RTE360),
    STODL(DRCON), /* "X(T2)ERR,=X(T2)ERR" */
    ADDRESS_PLUS_ONE(X_T2),
    STODL(RPRE_PRIME), /* "X(T2)PRI=X(T2)" */
    ADDRESS_PLUS_ONE_D(16),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(X_T2),
    STCALL(X_T2),           /* "X(T2)=X(T2)+DX(T2)" */
    ADDRESS(RTE360),        /* "REITERATE" */
    /* P37E */ ORDER(CALL), /* "DISPLAY CONIC SOLUTION" */
    ADDRESS(RTEVN),
    /* RTE505 */ ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE(PCON),
    ADDRESS_PLUS_ONE(BETA1),
    ORDERS(BDSU, BZE),
    ADDRESS_PLUS_ONE(RCON),
    ADDRESS(RTE510),
    ORDERS(BMN, DLOAD),
    ADDRESS(RTE510),
    ADDRESS_PLUS_ONE(1RTEB2),
    ORDER(GOTO), /* "ENTRY NEAR APOGEE" */
    ADDRESS(RTE515),
    /* RTE510 */ ORDERS(DLOAD, DCOMP), /* "ENTRY NEAR PERIGEE" */
    ADDRESS_PLUS_ONE(1RTEB2),
    /* RTE515 */ STCALL(PHI2),
    ADDRESS(PREC100), /* "PRECISION TRAJECTORY COMPUTATION" */
    /* RTE625 */ ORDER(BZE),
    ADDRESS(P37G),
    /* RTEALRM */ ORDER(CALL),
    ADDRESS(P370ALRM),
    ORDER(EXIT),
};

const struct interpretive_code RTE369_POLY_CODE = {036u, RTE369 + 016u, COUNT(rte369_poly_words),
                                                   rte369_poly_words};

#endif
