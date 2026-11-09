/* SPDX-License-Identifier: MIT */
/* PLANETARY_INERTIAL_ORIENTATION.agc (Luminary099 pages 1140-1148,
 * Comanche055 pages 1243-1251): its interpretive code in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs (src/binding/planetary_routines.c). The faithful C is c64-port/src/
 * lm_planetary.c and cm_planetary.c.
 *
 * The subroutines share the job's work area: the vector in and out at 0D
 * ("504RPR"), the time at 6D, and the matrix M from 20D ("MMATRIX", three
 * rows of three DP numbers). RP-TO-R and R-TO-RP keep their return in S1
 * ("RPREXIT"), MOONMX and EARTHMX theirs in S2 ("EARTHMXX"). */
#include "planetary.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word rp_to_r[] = {
    /* RP-TO-R -- p. 1140 (CM p. 1243).
     * "SUBROUTINE TO CONVERT RP (VECTOR IN PLANETARY COORDINATE
     * SYSTEM,EITHER EARTH-FIXED OR MOON-FIXED) TO R (SAME VECTOR IN THE
     * BASIC REF. SYSTEM)": "R=MT(T)*(RP+LPXRP)", "MT= M MATRIX TRANSPOSE".
     * In: "MPAC= 0 FOR EARTH,NON-ZERO FOR MOON", "0-5D= RP VECTOR" and
     * "6-7D= TIME". Out: "MPAC= R VECTOR METERS B-29 FOR EARTH, B-27 FOR
     * MOON".
     *
     * The return to RPREXIT (STQ). For the moon, MOONMX's matrix and the
     * moon's libration vector LP, 504LM, from the launch data; for the
     * earth (RPTORA), EARTHMX's matrix and EARTHL's L turned into the
     * earth's axes by it. Then (RPTORB) RP plus LP cross RP, times M
     * transposed (VXM), and out through RPRPXXXX, in R-TO-RP, which
     * shifts it, sets the push-down list back to 0D and returns. */
    /* RP-TO-R */ ORDERS(STQ, BHIZ),
    ADDRESS(RPREXIT),
    ADDRESS(RPTORA),
    ORDER(CALL),     /* "COMPUTE M MATRIX FOR MOON" */
    ADDRESS(MOONMX), /* "LP=LM FOR MOON" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(504LM),
    /* RPTORB */ ORDERS(VXV, VAD),
    ADDRESS_PLUS_ONE(504RPR),
    ADDRESS_PLUS_ONE(504RPR),
    ORDERS(VXM, GOTO),
    ADDRESS_PLUS_ONE(MMATRIX), /* "MPAC=R=MT(T)*(RP+LPXRP)" */
    ADDRESS(RPRPXXXX),         /* "RESET PUSHLOC TO 0 BEFORE EXITING" */
    /* RPTORA */ ORDER(CALL),  /* "EARTH COMPUTATIONS" */
    ADDRESS(EARTHMX),          /* "M MATRIX B-1" */
    ORDER(CALL),
    ADDRESS(EARTHL),   /* "L VECTOR RADIANS B0" */
    ORDERS(MXV, VSL1), /* "LP=M(T)*L" */
    ADDRESS_PLUS_ONE(MMATRIX),
    ORDER(GOTO),
    ADDRESS(RPTORB),
};

static const word r_to_rp[] = {
    /* R-TO-RP -- p. 1142 (CM p. 1245).
     * "SUBROUTINE TO CONVERT R (VECTOR IN REFERENCE COORD. SYSTEM) TO RP
     * (VECTOR IN PLANETARY COORD SYSTEM) EITHER EARTH-FIXED OR
     * MOON-FIXED": "RP=M(T)*(R-LXR)". In: MPAC zero for the earth, not
     * zero for the moon, "0-5D= R VECTOR" and "6-7D= TIME". Out: "MPAC=RP
     * VECTOR METERS B-29 FOR EARTH, B-27 FOR MOON".
     *
     * As RP-TO-R, the other way round. For the moon, MOONMX's matrix, and
     * the moon's L from its LP: M transposed times 504LM. For the earth
     * (RTORPA), EARTHMX's matrix and EARTHL's L as it is. Then (RTORPB) R
     * less L cross R, times M (MXV), shifted back (RPRPXXXX), the
     * push-down list set back to 0D, and out through RPREXIT. */
    /* R-TO-RP */ ORDERS(STQ, BHIZ),
    ADDRESS(RPREXIT),
    ADDRESS(RTORPA),
    ORDER(CALL),
    ADDRESS(MOONMX),
    ORDERS(VLOAD, VXM),
    ADDRESS_PLUS_ONE(504LM), /* "LP=LM" */
    ADDRESS_PLUS_ONE(MMATRIX),
    ORDER(VSL1), /* "L=MT(T)*LP" */
    /* RTORPB */ ORDERS(VXV, BVSU),
    ADDRESS_PLUS_ONE(504RPR),
    ADDRESS_PLUS_ONE(504RPR),
    ORDER(MXV), /* "M(T)*(R-LXR) B-2" */
    ADDRESS_PLUS_ONE(MMATRIX),
    /* RPRPXXXX */ ORDERS(VSL1, SETPD),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(GOTO),
    ADDRESS(RPREXIT),
    /* RTORPA */ ORDER(CALL), /* "EARTH COMPUTATIONS" */
    ADDRESS(EARTHMX),
    ORDER(CALL),
    ADDRESS(EARTHL),
    ORDER(GOTO), /* "MPAC=L=(-AX,-AY,0) RAD B-0" */
    ADDRESS(RTORPB),

    /* MOONMX -- p. 1143 (CM p. 1246).
     * "SUBROUTINE TO COMPUTE THE TRANSFORMATION MATRIX M FOR THE MOON" at
     * "6-7D= TIME", to "MMATRIX= 3X3 M MATRIX B-1 (STORED IN VAC AREA)".
     * From the launch data: three angles that grow at steady rates, B
     * ("BSUBO,BDOT"), F and the node ("TIMSUBO,NODIO,NODDOT,FSUBO,FDOT"),
     * and "COSI= COS(I) B-1" and "SINI= SIN(I) B-1", where "I IS THE
     * ANGLE BETWEEN THE MEAN LUNAR EQUATORIAL PLANE AND THE PLANE OF THE
     * ECLIPTIC".
     *
     * The return to S2 (STQ) and the push-down list at 8D. Each angle by
     * NEWANGLE, its start at 8D and its rate at 10D on the list, and in
     * X1 the shift that scales its rate: B (its cosine and sine to COB
     * and SOB), F (to AVECTR +2 for now) and the node, NODI. From those
     * the vectors A and B on the list: A = (COS(NODI), COB*SIN(NODI),
     * SOB*SIN(NODI)), B = (-SIN(NODI), COB*COS(NODI), SOB*COS(NODI)). F
     * goes to 504F. MOONMXA goes on with them. */
    /* MOONMX */ ORDERS(STQ, SETPD),
    ADDRESS(EARTHMXX),
    ADDRESS_PLUS_ONE_D(8),
    ORDER(AXT_1), /* "B REQUIRES SL 0, SL 5 IN NEWANGLE" */
    NUMBER(5),
    ORDERS(DLOAD, PDDL), /* "8-9D=BSUBO" */
    ADDRESS_PLUS_ONE(BSUBO),
    ADDRESS_PLUS_ONE(BDOT), /* "10-11D= BDOT" */
    ORDERS(PUSH, CALL),
    ADDRESS(NEWANGLE), /* "EXIT WITH PD 8D AND MPAC= B REVS B0" */
    ORDERS(PUSH, COS),
    STODL(COB),              /* "COS(B) B-1" */
    ORDER(SIN),              /* "SIN(B) B-1" */
    STODL(SOB),              /* "SETUP INPUT FOR NEWANGLE" */
    ADDRESS_PLUS_ONE(FSUBO), /* "8-9D=FSUBO" */
    ORDERS(PDDL, PUSH),      /* "10-11D=FDOT" */
    ADDRESS_PLUS_ONE(FDOT),
    ORDERS(AXT_1, CALL), /* "F REQUIRES SL 1, SL 6 IN NEWANGLE" */
    NUMBER(4),
    ADDRESS(NEWANGLE),        /* "EXIT WITH PD 8D AND MPAC= F REVS B0" */
    STODL_AT(AVECTR, 2),      /* "SAVE F TEMP" */
    ADDRESS_PLUS_ONE(NODIO),  /* "8-9D=NODIO" */
    ORDERS(PDDL, PUSH),       /* "10-11D=NODDOT" */
    ADDRESS_PLUS_ONE(NODDOT), /* "MPAC=T" */
    ORDERS(AXT_1, CALL),      /* "NODE REQUIRES SL 0, SL 5 IN NEWANGLE" */
    NUMBER(5),
    ADDRESS(NEWANGLE), /* "EXIT WITH PD 8D AND MPAC= NODI REVS B0" */
    ORDERS(PUSH, COS), /* "8-9D= NODI REVS B0" */
    ORDER(PUSH),       /* "10-11D= COS(NODI) B-1" */
    STORE(AVECTR),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(COB),
    STODL_AT(BVECTR, 2), /* "20-25D=AVECTR= COB*SIN(NODI)" */
    ORDERS(DMP, SL1R),   /* "SOB*SIN(NODI)" */
    ADDRESS_PLUS_ONE(SOB),
    STODL_AT(BVECTR, 4),
    ORDERS(SIN, PUSH),              /* "-SIN(NODI) B-1" */
    ORDER(DCOMP),                   /* "26-31D=BVECTR= COB*COS(NODI)" */
    STODL(BVECTR),                  /* "SOB*COS(NODI)" */
    ADDRESS_PLUS_ONE_AT(AVECTR, 2), /* "MOVE F FROM TEMP LOC. TO 504F" */
    STODL(504F),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(COB),
#if LUMINARY099
    ORDER(GOTO),
    ADDRESS(MOONMXA),
#endif
};

static const word moonmxa[] = {
    /* MOONMXA -- p. 1144 (CM p. 1247; the CM's has no label: MOONMX runs
     * on into it, in the same bank).
     * The vectors C = (0, -SOB, COB) and, with the angle I between the
     * moon's equator and the ecliptic, the rows of M: M2 = B*SINI +
     * C*COSI ("M2=BVECTR*SINI+CVECTR*COSI"), D = B*COSI - C*SINI, M1 =
     * A*SINF - D*COSF ("M1= AVECTR*SINF-DVECTR*COSF") and M0 = -(A*COSF +
     * D*SINF), each shifted back to B-1. Out through S2, "M0=
     * -(AVECTR*COSF+DVECTR*SINF) B-1". */
    /* MOONMXA */ STODL_AT(AVECTR, 2),
    ADDRESS_PLUS_ONE(SINNODI), /* "8-9D=SIN(NODI) B-1" */
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(SOB),
    STODL_AT(AVECTR, 4),
    ADDRESS_PLUS_ONE(HI6ZEROS), /* "8-13D= CVECTR= -SOB B-1" */
    ORDERS(PDDL, DCOMP),
    ADDRESS_PLUS_ONE(SOB),
    ORDERS(PDDL, PDVL), /* "PD 12D THEN PD 14D" */
    ADDRESS_PLUS_ONE(COB),
    ADDRESS_PLUS_ONE(BVECTR),
    ORDERS(VXSC, PDVL), /* "BVECTR*SINI B-2" */
    ADDRESS_PLUS_ONE(SINI),
    ADDRESS_PLUS_ONE(CVECTR),
    ORDERS(VXSC, VAD), /* "CVECTR*COSI B-2" */
    ADDRESS_PLUS_ONE(COSI),
    ORDER(VSL1),
    STOVL_AT(MMATRIX, 12), /* "M2=BVECTR*SINI+CVECTR*COSI B-1" */
    ORDERS(VXSC, PDVL),
    ADDRESS_PLUS_ONE(SINI), /* "CVECTR*SINI B-2" */
    ADDRESS_PLUS_ONE(BVECTR),
    ORDERS(VXSC, VSU), /* "BVECTR*COSI B-2" */
    ADDRESS_PLUS_ONE(COSI),
    ORDERS(VSL1, PDDL),
    ADDRESS_PLUS_ONE(504F), /* "8-13D=DVECTR=BVECTR*COSI-CVECTR*SINI B-1" */
    ORDERS(COS, VXSC),
    ADDRESS_PLUS_ONE(DVECTR),
    ORDERS(PDDL, SIN), /* "14-19D= DVECTR*COSF B-2" */
    ADDRESS_PLUS_ONE(504F),
    ORDERS(VXSC, VSU), /* "AVECTR*SINF B-2" */
    ADDRESS_PLUS_ONE(AVECTR),
    ORDER(VSL1),
    STODL_AT(MMATRIX, 6), /* "M1= AVECTR*SINF-DVECTR*COSF B-1" */
    ADDRESS_PLUS_ONE(504F),
    ORDERS(SIN, VXSC),
    ORDERS(PDDL, COS), /* "8-13D=DVECTR*SINF B-2" */
    ADDRESS_PLUS_ONE(504F),
    ORDERS(VXSC, VAD), /* "AVECTR*COSF B-2" */
    ADDRESS_PLUS_ONE(AVECTR),
    ORDERS(VSL1, VCOMP),
    STCALL(MMATRIX), /* "M0= -(AVECTR*COSF+DVECTR*SINF) B-1" */
    ADDRESS(EARTHMXX),

    /* NEWANGLE -- p. 1145 (CM p. 1248).
     * "COMPUTE X=X0+(XDOT)(T+T0)": in, "8-9D= X0 (REVS B-0),PUSHLOC SET AT
     * 12D", the rate at 10-11D, "6-7D=T (CSEC B-28)", and in X1 the
     * "DIFFERENCE IN 23 AND SCALING OF XDOT". TIMSUBO is the launch data's
     * T0, "(CSEC B-42 TRIPLE PREC.)".
     *
     * T+T0 in triple precision (TAD) to TIMSUBM. Its low two words times
     * the rate, shifted by X1, plus X0; then its high word times the rate,
     * shifted 10 more by X1, added on. The BOV to the next order turns off
     * any overflow the shifts set. Out (RVQ) with "MPAC=X= X0+(XDOT)(T+T0)
     * REVS B0". */
    /* NEWANGLE */ ORDERS(DLOAD, SR), /* "ENTER PD 12D" */
    ADDRESS_PLUS_ONE_D(6),
    SR_COUNT(14),
    ORDERS(TAD, TLOAD), /* "CHANGE MODE TO TP" */
    ADDRESS_PLUS_ONE(TIMSUBO),
    ADDRESS_PLUS_ONE(MPAC),
    STODL(TIMSUBM), /* "T+T0 CSEC B-42" */
    ADDRESS_PLUS_ONE_AT(TIMSUBM, 1),
    ORDER(DMP),           /* "MULT BY XDOT IN 10-11D" */
    ORDERS(SL_STAR, DAD), /* "ADD X0 IN 8-9D AFTER SHIFTING" */
    SL_X1(5),             /* "SUCH THAT SCALING IS B-0" */
    ORDERS(PUSH, SLOAD),  /* "SAVE PARTIAL (X0+XDOT*T) IN 8-9D" */
    ADDRESS_PLUS_ONE(TIMSUBM),
    ORDERS(SL, DMP),
    SL_COUNT(9),
    ADDRESS_PLUS_ONE_D(10),    /* "XDOT" */
    ORDERS(SL_STAR, DAD),      /* "SHIFT SUCH THAT THIS PART OF X" */
    SL_X1(10),                 /* "IS SCALED REVS/CSEC B-0" */
    ORDER(BOV),                /* "TURN OFF OVERFLOW IF SET BY SHIFT" */
    ADDRESS_AT(NEWANGLE, 024), /* (+1: the RVQ) */
    ORDER(RVQ),                /* "MPAC=X= X0+(XDOT)(T+T0) REVS B0" */
};

static const word earthmx[] = {
    /* EARTHMX -- p. 1146 (CM p. 1249).
     * "SUBROUTINE TO COMPUTE THE TRANSFORMATION MATRIX M FOR THE EARTH" at
     * "6-7D= TIME CSEC B-28", to "MMATRIX= 3X3 M MATRIX B-1". From the
     * launch data, AZO, the earth's angle at the start ("REVS B-0"), and
     * the rate it turns at, WEARTH.
     *
     * The return to S2 (STQ) and the push-down list at 8D. The angle AZ
     * by NEWANGLE (X1 0), to 18D ("504AZ"); then the matrix pushed on the
     * list from 20D, row by row: (COS(AZ), SIN(AZ), 0), (-SIN(AZ),
     * COS(AZ), 0), (0, 0, 1/2), a turn about the earth's axis. Out through
     * S2. */
    /* EARTHMX */ ORDERS(STQ, SETPD), /* "SET 8-9D=AZO" */
    ADDRESS(EARTHMXX),
    ADDRESS_PLUS_ONE_D(8), /* "10-11D=WEARTH" */
    ORDER(AXT_1),          /* "FOR SL 5, AND SL 10 IN NEWANGLE" */
    NUMBER(0),
    ORDERS(DLOAD, PDDL), /* "LEAVING PD SET AT 12D FOR NEWANGLE" */
    ADDRESS_PLUS_ONE(AZO),
    ADDRESS_PLUS_ONE(WEARTH),
    ORDERS(PUSH, CALL),
    ADDRESS(NEWANGLE),
    ORDERS(SETPD, PUSH), /* "18-19D=504AZ" */
    ADDRESS_PLUS_ONE_D(18),
    ORDERS(COS, PDDL),
    ADDRESS_PLUS_ONE(504AZ),
    ORDERS(SIN, PDDL),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(PDDL, SIN),
    ADDRESS_PLUS_ONE(504AZ),
    ORDERS(DCOMP, PDDL),
    ADDRESS_PLUS_ONE(504AZ),
    ORDERS(COS, PDVL),
    ADDRESS_PLUS_ONE(HI6ZEROS),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(HIDPHALF),
    ORDER(GOTO),
    ADDRESS(EARTHMXX),
};

static const word earthl[] = {
    /* EARTHL -- p. 1147 (CM p. 1250).
     * "SUBROUTINE TO COMPUTE L VECTOR FOR EARTH" from "AXO,AYO SET AT
     * LAUNCH TIME WITH AYO IMMEDIATELY FOLLOWING AXO IN CORE": L = (-AXO,
     * -AYO, 0) "RADIANS B-0", built at 504LPL (14D) and loaded into MPAC
     * (the LM's zeros are LO6ZEROS, the CM's HI6ZEROS). */
    /* EARTHL */ ORDERS(DLOAD, DCOMP),
    ADDRESS_PLUS_ONE(AXO),
    STODL(504LPL),
    ADDRESS_PLUS_ONE(MAYO), /* -AYO */
    STODL_AT(504LPL, 2),
#if LUMINARY099
    ADDRESS_PLUS_ONE(LO6ZEROS),
#else
    ADDRESS_PLUS_ONE(HI6ZEROS),
#endif
    STOVL_AT(504LPL, 4),
    ADDRESS_PLUS_ONE(504LPL),
    ORDER(RVQ),
};

#if COMANCHE055
/* "CONSTANTS AND ERASABLE ASSIGNMENTS" -- CM p. 1251: the moon's and the
 * earth's, each a 2DEC (the LM's are in its CONTROLLED_CONSTANTS). */
static const word constants[] = {
    /* COSI */ NUMBER(017775), /* 2DEC .99964173 B-1: "COS(5521.5 SEC) B-1" */
    NUMBER(002052),
    /* SINI */ NUMBER(000333), /* 2DEC .02676579 B-1: "SIN(5521.5 SEC) B-1" */
    NUMBER(010374),
    /* NODDOT */ NUMBER(077665), /* 2DEC -.457335121 E-2: "REVS/CSEC B+28" */
    NUMBER(042175),
    /* FDOT */ NUMBER(022211), /* 2DEC .570863327: "REVS/CSEC B+27" */
    NUMBER(000625),
    /* BDOT */ NUMBER(077777), /* 2DEC -3.07500686 E-8: "REVS/CSEC B+28" */
    NUMBER(077767),
    /* NODIO */ NUMBER(037436), /* 2DEC .986209434: "REVS B-0" */
    NUMBER(001613),
    /* FSUBO */ NUMBER(032417), /* 2DEC .829090536: "REVS B-0" */
    NUMBER(032160),
    /* BSUBO */ NUMBER(002052), /* 2DEC .0651201393 */
    NUMBER(035552),
    /* WEARTH */ NUMBER(037116), /* 2DEC .973561595: "REVS/CSEC B+23" */
    NUMBER(032523),
};
#endif

/* In a bank, the address of a fixed word's name (bank * 02000 plus its
 * address in the bank, as names.h has it) as the interpreter's LOC holds
 * it: 02000-03777. */
#define IN_BANK(fixed) (02000u + ((fixed) & 01777u))

const struct interpretive_code PLANETARY_CODE[PLANETARY_PARTS] = {
#if LUMINARY099
    {026u, K_RP_TO_R, COUNT(rp_to_r), rp_to_r}, {024u, K_R_TO_RP, COUNT(r_to_rp), r_to_rp},
    {025u, K_MOONMXA, COUNT(moonmxa), moonmxa}, {026u, K_EARTHMX, COUNT(earthmx), earthmx},
    {06u, K_EARTHL, COUNT(earthl), earthl},
#else
    /* All in bank 26, one part after another. */
    {026u, K_RP_TO_R, COUNT(rp_to_r), rp_to_r},
    {026u, K_R_TO_RP, COUNT(r_to_rp), r_to_rp},
    {026u, (word)(K_R_TO_RP + COUNT(r_to_rp)), COUNT(moonmxa), moonmxa},
    {026u, K_EARTHMX, COUNT(earthmx), earthmx},
    {026u, K_EARTHL, COUNT(earthl), earthl},
    {026u, IN_BANK(I_COSI), COUNT(constants), constants},
#endif
};
