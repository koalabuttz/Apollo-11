/* SPDX-License-Identifier: MIT */
/* LATITUDE_LONGITUDE_SUBROUTINES.agc (Luminary099 pages 1133-1139,
 * Comanche055 pages 1236-1242): its interpretive code in the listing's
 * notation (src/interpreter/notation.h), which the readable interpreter
 * runs (src/binding/lat_long_routines.c). The faithful C is c64-port/src/
 * lm_lat_long.c and cm_lat_long.c. */
#include "lat_long.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static const word lat_long[] = {
    /* LAT-LONG -- p. 1133 (CM p. 1236).
     * "SUBROUTINE TO CONVERT RAD VECTOR AT GIVEN TIME TO LAT,LONG AND
     * ALT". In: ALPHAV, "POSITION VECTOR METERS B-29", and "MPAC-- TIME
     * (CSECS B-28)"; LUNAFLAG "0 FOR EARTH,1 FOR MOON", ERADFLAG "1, TO
     * COMPUTE EARTH RADIUS, =0 FOR FIXED EARTH RADIUS". Out: "LATITUDE IN
     * LAT", "LONGITUDE IN LONG" (revolutions) and "ALTITUDE IN ALT".
     *
     * The return to INCORPEX (STQ), the time to 6D and the vector to 0D,
     * as R-TO-RP takes them, and its length to ALPHAM. R-TO-RP (in
     * PLANETARY_INERTIAL_ORIENTATION) turns it into the planet's own
     * axes, the earth's if MPAC is zero, the moon's if not (COS of zero
     * makes it not). Its unit vector to ALPHAV; SETGAMMA and SETRE. Then
     * the latitude from its sine, the vector's Z, and its cosine, GAMMA
     * times the length of its X and Y; the longitude from Y and X; and
     * the altitude, the length less the radius. Out through INCORPEX. */
    /* LAT-LONG */ ORDERS(STQ, SETPD),
    ADDRESS(INCORPEX),
    ADDRESS_PLUS_ONE_D(0),
    STOVL_D(6), /* "SAVE TIME IN 6-7D FOR R-TO-RP" */
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(PUSH, ABVAL), /* "0-5D= R FOR R-TO-RP" */
    STODL(ALPHAM),       /* "ABS. VALUE OF R FOR ALT FORMULA BELOW" */
    ADDRESS_PLUS_ONE(ZEROVEC),
    ORDERS(BOFF, COS), /* "USE COS(0) TO GET NON-ZERO IN MPAC" */
    SWITCH(BOFF, LUNAFLAG),
    ADDRESS(CALLRTRP),
    /* CALLRTRP */ ORDER(CALL),
    ADDRESS(R_TO_RP), /* "RP VECTOR CONVERTED FROM R B-29" */
    ORDER(UNIT),      /* "UNIT RP B-1" */
    STCALL(ALPHAV),   /* "U2= 1/2 SINL FOR SETRE SUBR BELOW" */
    ADDRESS(SETGAMMA),
    ORDER(CALL),
    ADDRESS(SETRE), /* "CALC RE METERS B-29" */
    ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDERS(PDDL, DSQ),
    ADDRESS_PLUS_ONE_AT(ALPHAV, 2),
    ORDERS(DAD, SQRT),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(GAMRP),
    STODL(COSTH), /* "COS(LAT) B-1" */
    ADDRESS_PLUS_ONE_AT(ALPHAV, 4),
    STCALL(SINTH), /* "SIN(LAT) B-1" */
    ADDRESS(ARCTAN),
    STODL(LAT), /* "LAT B0" */
    ADDRESS_PLUS_ONE(ALPHAV),
    STODL(COSTH), /* "COS(LONG) B-1" */
    ADDRESS_PLUS_ONE_AT(ALPHAV, 2),
    STCALL(SINTH), /* "SIN(LONG) B-1" */
    ADDRESS(ARCTAN),
    STODL(LONG), /* "LONG. REVS B-0 IN RANGE -1/2 TO 1/2" */
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDER(DSU), /* "ALT= R-RE METERS B-29" */
    ADDRESS_PLUS_ONE(ERADM),
    STCALL(ALT), /* "EXIT WITH ALT METERS B-29" */
    ADDRESS(INCORPEX),

    /* LALOTORV -- p. 1135 (CM p. 1237).
     * "SUBROUTINE TO CONVERT LAT,LONG,ALT AT GIVEN TIME TO RADIUS VECTOR":
     * LAT, LONG, ALT and the time in MPAC to "R-VECTOR IN ALPHAV (METERS
     * B-29)". The return to INCORPEX and the time to 6D; SETGAMMA. The
     * planet's axes' unit vector from the angles, built on the push-down
     * list: (COS(LONG)COS(LAT), SIN(LONG)COS(LAT), GAMMA*SIN(LAT)), made a
     * unit vector, to 0D for RP-TO-R and to ALPHAV for SETRE. RP-TO-R
     * turns it back into the reference axes (MPAC zero or not, as in
     * LAT-LONG), and the radius plus the altitude times it is the vector,
     * to ALPHAV. Out through INCORPEX. */
    /* LALOTORV */ ORDERS(STQ, SETPD), /* "LAT,LONG,ALT TO R VECTOR" */
    ADDRESS(INCORPEX),
    ADDRESS_PLUS_ONE_D(0),
    STCALL_D(6), /* "6-7D= TIME FOR RP-TO-R" */
    ADDRESS(SETGAMMA),
    ORDERS(DLOAD, SIN),
    ADDRESS_PLUS_ONE(LAT),
    ORDERS(DMPR, PDDL), /* "PD 2" */
    ADDRESS_PLUS_ONE(GAMRP),
    ADDRESS_PLUS_ONE(LAT),
    ORDERS(COS, PDDL), /* "2-3D=COS(LAT) B-1 TEMPORARILY" */
    ADDRESS_PLUS_ONE(LONG),
    ORDERS(SIN, DMPR), /* "PD 2" */
    ORDERS(PDDL, COS), /* "2-3D=SIN(LONG)COS(LAT) B-2" */
    ADDRESS_PLUS_ONE(LAT),
    ORDERS(PDDL, COS), /* "4-5D=COS(LAT) B-1 TEMPORARILY" */
    ADDRESS_PLUS_ONE(LONG),
    ORDERS(DMPR, VDEF),  /* "COS(LONG)COS(LAT) B-2" */
    ORDERS(UNIT, PUSH),  /* "0-5D= UNIT RP FOR RP-TO-R SUBR." */
    STCALL(ALPHAV),      /* "ALPHAV +4= SINL FOR SETRE SUBR." */
    ADDRESS(SETRE),      /* "RE METERS B-29" */
    ORDERS(DLOAD, BOFF), /* "SET MPAC=0 FOR EARTH,NON-ZERO FOR MOON" */
    ADDRESS_PLUS_ONE(ZEROVEC),
    SWITCH(BOFF, LUNAFLAG),
    ADDRESS(CALLRPRT),
    ORDER(COS), /* "USE COS(0) TO GET NON-ZERO IN MPAC" */
    /* CALLRPRT */ ORDER(CALL),
    ADDRESS(RP_TO_R), /* "EXIT WITH UNIT R VECTOR IN MPAC" */
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(ERADM),
    ORDERS(DAD, VXSC), /* "(RE + ALT)(UNIT R) METERS B-30" */
    ADDRESS_PLUS_ONE(ALT),
    ADDRESS_PLUS_ONE(ALPHAV),
    ORDER(VSL1),    /* "R METERS B-29" */
    STCALL(ALPHAV), /* "EXIT WITH R IN METERS B-29" */
    ADDRESS(INCORPEX),

    /* GETERAD -- p. 1136 (CM p. 1238).
     * "SUBROUTINE TO COMPUTE EARTH RADIUS" at a latitude, from "1/2 SIN
     * LAT IN ALPHAV +4", to "EARTH RADIUS IN ERADM AND MPAC": the radius
     * of the ellipse with half axes A and B at that latitude, B over the
     * root of 1 less EE COS**2(L):
     *   RE = SQRT(B2XSC / ((1 - EE COS**2(L)) / 2)), shifted right 4. */
    /* GETERAD */ ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE_AT(ALPHAV, 4), /* "SIN**2(L)" */
    ORDERS(SL1, BDSU),
    ADDRESS_PLUS_ONE(DP1_2), /* "COS**2(L)" */
    ORDERS(DMPR, BDSU),
    ADDRESS_PLUS_ONE(EE),
    ADDRESS_PLUS_ONE(DP1_2),
    ORDERS(BDDV, SQRT),
    ADDRESS_PLUS_ONE(B2XSC),
    ORDER(SR4R),
    STORE(ERADM),
    ORDER(RVQ),

    /* "THE FOLLOWING CONSTANTS WERE COMPUTED WITH A=6378166,B=6356784
     * METERS" (DP1/2 = XUNIT). */
    /* B2XSC */ NUMBER(000446), /* 2DEC .0179450689: "B**2 SCALED B-51" */
    NUMBER(000305),
    /* B2/A2 */ NUMBER(017711), /* 2DEC .9933064884 B-1: "GAMMA= B**2/A**2 B-1" */
    NUMBER(005254),
    /* EE */ NUMBER(000155), /* 2DEC 6.6935116 E-3: "(1-B**2/A**2) B-0" */
    NUMBER(025250),
#if COMANCHE055
    /* ERAD */ NUMBER(000302), /* 2DEC 6373338 B-29: "PAD RADIUS" */
    NUMBER(017755),
#endif

    /* ARCTAN -- p. 1137 (CM p. 1239).
     * "SIN THETA IN SINTH B-1", "COS THETA IN COSTH B-1"; out, "ARCTAN
     * THETA IN MPAC AND THETA B-0 IN RANGE -1/2 TO +1/2" (revolutions).
     * The overflow indicator cleared (BOV, to the next line either way).
     * The sine over the root of the sum of the squares: on overflow, its
     * size one, the angle is a quarter turn with SINTH's sign (ATAN=90);
     * else its ARCSIN, THETA, the angle were COSTH positive. With COSTH
     * negative (NEGCOS) it is a half turn less THETA, 1/2 - THETA, or with
     * THETA not positive, -1/2 - THETA (NEGOUT). Both zero, "ATAN=0/0  SET
     * THETA=0". */
    /* ARCTAN */ ORDER(BOV),
    ADDRESS(CLROVFLW),
    /* CLROVFLW */ ORDERS(DLOAD, DSQ),
    ADDRESS_PLUS_ONE(SINTH),
    ORDERS(PDDL, DSQ),
    ADDRESS_PLUS_ONE(COSTH),
    ORDER(DAD),
    ORDERS(BZE, SQRT),
    ADDRESS(ARCTANXX), /* "ATAN=0/0  SET THETA=0" */
    ORDERS(BDDV, BOV),
    ADDRESS_PLUS_ONE(SINTH),
    ADDRESS(ATAN_EQ_90),
    ORDERS(SR1, ASIN),
    STORE(THETA),
    ORDERS(PDDL, BMN),
    ADDRESS_PLUS_ONE(COSTH),
    ADDRESS(NEGCOS),
    ORDERS(DLOAD, RVQ),
    /* NEGCOS */ ORDERS(DLOAD, DCOMP),
    ORDERS(BPL, DAD),
    ADDRESS(NEGOUT),
    ADDRESS_PLUS_ONE(DP1_2),
    /* ARCTANXX */ STORE(THETA),
    ORDER(RVQ),
    /* NEGOUT */ ORDERS(DSU, GOTO),
    ADDRESS_PLUS_ONE(DP1_2),
    ADDRESS(ARCTANXX),
    /* ATAN=90 */ ORDERS(DLOAD, SIGN),
    ADDRESS_PLUS_ONE(LODP1_4),
    ADDRESS_PLUS_ONE(SINTH),
    STORE(THETA),
    ORDER(RVQ),
    /* 2DZERO = DPZERO */

    /* SETGAMMA -- p. 1138 (CM p. 1240).
     * "SUBROUTINE TO SET GAMMA FOR THE LAT-LONG AND LALOTORV SUBROUTINES":
     * "GAMMA = B**2/A**2 FOR EARTH (B-1)", "GAMMA = 1 FOR MOON (B-1)", to
     * GAMRP (8D). */
    /* SETGAMMA */ ORDERS(DLOAD, BOFF), /* "BRANCH FOR EARTH" */
    ADDRESS_PLUS_ONE(B2_A2),            /* "EARTH GAMMA" */
    SWITCH(BOFF, LUNAFLAG),
    ADDRESS(SETGMEX),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(1B1), /* "MOON GAMMA" */
    /* SETGMEX */ STORE(GAMRP),
    ORDER(RVQ),

    /* SETRE -- p. 1139 (CM p. 1241).
     * "SUBROUTINE TO SET RE (EARTH OR MOON RADIUS)", to ERADM, the return
     * kept in SETREX (S2): the moon's, 504RM, or with ERADFLAG clear, the
     * landing site's distance from its centre (RLS, "SCALE FROM B-27 TO
     * B-29"); the earth's, ERAD, or with ERADFLAG set, GETERAD's at the
     * latitude. */
    /* SETRE */ ORDERS(STQ, DLOAD),
    ADDRESS(SETREX),
    ADDRESS_PLUS_ONE(504RM),
    ORDERS(BON, DLOAD), /* "BRANCH FOR MOON" */
    SWITCH(BON, LUNAFLAG),
    ADDRESS(TSTRLSRM),
    ADDRESS_PLUS_ONE(ERAD),
    ORDERS(BOFF, CALL), /* "ERADFLAG=0 FOR FIXED RE,1 FOR COMPUTED" */
    SWITCH(BOFF, ERADFLAG),
    ADDRESS(SETRXX),
    ADDRESS(GETERAD),
    /* SETRXX */ STCALL(ERADM), /* "EXIT WITH RE OR RM METERS B-29" */
    ADDRESS(SETREX),
    /* TSTRLSRM */ ORDERS(BON, VLOAD), /* "ERADFLAG=0,SET R0=RLS" */
    SWITCH(BON, ERADFLAG),
    ADDRESS(SETRXX),
    ADDRESS_PLUS_ONE(RLS),
    ORDERS(ABVAL, SR2R), /* "SCALE FROM B-27 TO B-29" */
    ORDER(GOTO),
    ADDRESS(SETRXX),
/* SETREX = S2 */
#if COMANCHE055
    /* 504RM */ NUMBER(000065), /* 2DEC 1738090 B-29: "METERS B-29 (MOON RADIUS)" */
    NUMBER(001265),
#endif
};

const struct interpretive_code LAT_LONG_CODE = {013u, K_LAT_LONG, COUNT(lat_long), lat_long};
