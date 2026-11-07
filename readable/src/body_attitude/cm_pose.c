/* SPDX-License-Identifier: MIT */
/* CM_BODY_ATTITUDE.agc, pp. 883-886 (Comanche055, bank 37): CM/POSE and
 * CM/TRIO, the attitude of the CM to the air stream from the state and
 * the gimbal angles. The faithful C is c64-port/src/cm_body_attitude.c. */
#include "body_attitude.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- CM/POSE -- p. 883 ---- */

/* CM/POSE: "COME HERE VIA AVEGEXIT.", into the interpreter. */
void cm_pose(void)
{
    tc_at(CM_POSE);
    intpret();
}

/* The velocity relative to the air, which turns with the earth: "VREL =
 * V - WE*R", VN scaled by -KVSCALE ("KVSCALE = (12800/ .3048) /2VS") less
 * UNITW ("FULL UNIT VECTOR") cross UNITR times KWE: -VREL, "SAVE FOR
 * ENTRY GUIDANCE." Its unit vector is UXA/2, "-UVREL", its length to X1
 * ("ABVAL( -VREL) TO X1"); UYA/2 the unit of UXA/2 cross UNITR.
 *
 * "THE FOLLOWING IS TO PROVIDE A STABLE" "UN FOR THE END OF THE TERMINAL
 * PHASE.": SSP sets S1 to SPVQUIT, the word after it ("1000/ 2 VS"), and
 * TIX,1 goes on to CM/POSE2 while the speed is above it ("IF V-VQUIT POS,
 * BRANCH."). Below it, "OTHERWISE CONTINUE TO USE OLDUYA.": "RESTORE, OR
 * SAVE AS CASE MAY BE." UZA/2 is UYA/2 cross UXA/2: "FINISH OBTAINING
 * TRAJECTORY TRIAD."
 *
 * Then the gimbal angles "CORRESPONDING" "TO PIPUP TIME" (AOG/PIP, AIG/PIP,
 * AMG/PIP: the outer, inner and middle) into 24D to 26D, and at CM/TRIO
 * each angle through CDULOGIC to its sine and cosine (CI, SI, CM, SM, CO,
 * SO in the comments): the body axes in platform coordinates, built a
 * component at a time on the push-down list,
 *
 *   "XB = ( CMCI , SM , -CMSI )"
 *   "YB = (-COSMCI + SOSI , COCM , COSMSI + SOCI )"
 *   "ZB = ( SOSMCI + COSI , -SOCM , -SOSMSI + COCI)", as XB cross YB,
 *
 * each through REFSMMAT to reference coordinates: UBX/2, UBY/2, UBZ/2. */
static const word cm_pose_words[] = {
    /* CM/POSE +1 */ ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(VN), /* "KVSCALE = (12800/ .3048) /2VS" */
    ORDERS(VXSC, PDVL),
    ADDRESS_PLUS_ONE(MKVSCALE), /* "KVSCALE = .81491944" */
    ADDRESS_PLUS_ONE(UNITW),    /* "FULL UNIT VECTOR" */
    ORDERS(VXV, VXSC),          /* "VREL = V - WE*R" */
    ADDRESS_PLUS_ONE(UNITR),
    ADDRESS_PLUS_ONE(KWE),
    ORDERS(VAD, STADR),
    AFTER_STADR(STORE(MVREL)), /* "SAVE FOR ENTRY GUIDANCE." */
    ORDERS(UNIT, LXA_1),
    ADDRESS_D(36), /* "ABVAL( -VREL) TO X1" */
    STORE(UXA_2),  /* "-UVREL" */
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE(UNITR),    /* ".5 UNIT" */
    ORDERS(UNIT, SSP),          /* "THE FOLLOWING IS TO PROVIDE A STABLE" */
    ADDRESS_PLUS_ONE(S1),       /* "UN FOR THE END OF THE TERMINAL PHASE." */
    /* SPVQUIT */ NUMBER(0476), /* DEC .019405, "1000/ 2 VS" */
    ORDERS(TIX_1, VLOAD),       /* "IF V-VQUIT POS, BRANCH." */
    ADDRESS(CM_POSE2),          /* "SAVE UYA IN OLDUYA" */
    ADDRESS_PLUS_ONE(OLDUYA),   /* "OTHERWISE CONTINUE TO USE OLDUYA." */
    /* CM/POSE2 */ STORE(UYA_2),
    STORE(OLDUYA), /* "RESTORE, OR SAVE AS CASE MAY BE." */
    ORDERS(VXV, VCOMP),
    ADDRESS_PLUS_ONE(UXA_2), /* "FINISH OBTAINING TRAJECTORY TRIAD." */
    ORDER(VSL1),
    STORE(UZA_2),
    ORDER(TLOAD),              /* "PICK UP CDUX, CDUY, CDUZ CORRESPONDING" */
    ADDRESS_PLUS_ONE(AOG_PIP), /* "TO PIPUP TIME IN 2S.C AND SAVE." */
    /* CM/TRIO */ STODL_D(24),
    ADDRESS_PLUS_ONE_D(25), /* "AIG/PIP" */
    ORDERS(RTB, PUSH),      /* "TO PDL0" */
    ADDRESS(CDULOGIC),
    ORDER(COS),
    STODL(UBX_2), /* "CI /2"; "AIG/PIP FROM PDL 0" */
    ORDERS(SIN, DCOMP),
    STODL_AT(UBX_2, 4),     /* "-SI /2" */
    ADDRESS_PLUS_ONE_D(26), /* "AMG/PIP" */
    ORDERS(RTB, PUSH),      /* "TO PDL 0" */
    ADDRESS(CDULOGIC),
    ORDERS(SIN, PDDL),     /* "XCH PDL 0.  SAVE SM /2" */
    ORDERS(COS, PDDL),     /* "CM /2 TO PDL 2" */
    ADDRESS_PLUS_ONE_D(0), /* "SM /2" */
    ORDERS(DCOMP, VXSC),
    ADDRESS_PLUS_ONE(UBX_2),
    ORDER(VSL1),            /* "NOISE WONT OVFL." */
    STODL(UBY_2),           /* "=(-SMCI, NOISE, SMSI)/2" */
    ADDRESS_PLUS_ONE_D(2),  /* "CM /2 REPLACES NOISE" */
    STODL_AT(UBY_2, 2),     /* "UBY/2=(-SMCI, CM, SMSI)/2" */
    ADDRESS_PLUS_ONE_D(24), /* "AOG/PIP" */
    ORDERS(RTB, PUSH),      /* "TO PDL 4" */
    ADDRESS(CDULOGIC),
    ORDERS(SIN, PDDL), /* "XCH PDL 4.  SAVE SO /2" */
    ORDERS(COS, VXSC), /* "CO /2" */
    ADDRESS_PLUS_ONE(UBY_2),
    STODL(UBY_2),          /* "UBY/2=(-COSMCI, COCM, COSMSI)/4" */
    ADDRESS_PLUS_ONE_D(4), /* "SO /2" */
    ORDERS(DMP, DCOMP),
    ADDRESS_PLUS_ONE_AT(UBX_2, 4), /* "-SI /2" */
    ORDER(DAD),
    ADDRESS_PLUS_ONE(UBY_2), /* "INCREMENT BY (SOSI /4)" */
    STODL(UBY_2),            /* "SO /2 FROM PDL 4" */
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(UBX_2), /* "CI /2" */
    ADDRESS_PLUS_ONE_AT(UBY_2, 4),
    STOVL_AT(UBY_2, 4), /* "YB/4" */
    ADDRESS_PLUS_ONE(UBY_2),
    ORDERS(VXM, VSL2),
    ADDRESS_PLUS_ONE(REFSMMAT), /* ".5 UNIT" */
    STODL(UBY_2),               /* "YB/2 DONE"; "CM /2 FROM PDL 2" */
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UBX_2),
    STODL(UBX_2),                    /* "=( CMCI, NOISE, -CMSI)/2" */
    ORDER(STADR),                    /* "SM /2 FROM PDL 0" */
    AFTER_STADR(STOVL_AT(UBX_2, 2)), /* "SM /2 REPLACES NOISE" */
    ADDRESS_PLUS_ONE(UBX_2),         /* "XB/2" */
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT), /* ".5 UNIT" */
    STORE(UBX_2),               /* "XB/2 DONE" */
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(UBY_2),
    STOVL(UBZ_2),            /* "ZB/2 DONE" */
    ADDRESS_PLUS_ONE(UXA_2), /* "-UVREL/2 = -UVA/2" */

    /* ---- The angles -- pp. 885-886 ----
     *
     * UL/2, the unit of -UVREL cross UBY/2 ("GET UNIT(-UVREL*UBY)/2 =
     * UL/2"), on the push-down list; its dots with UZA/2 and UYA/2 are
     * "COS(ROLL)/4" and "-SIN(ROLL)/4", ARCTRIG the angle from both:
     * "-(ROLL/180) /2" to 6D. "-UVA.UBY = -SIN(BETA)", through ARCSIN to
     * 7D; "UL.UBX = -SIN(ALFA)" and UL.UBZ, through ARCTRIG to 8D; and
     * UNITR dot UZA/2 ("MORE ACCURATE AT LARGE ARG.") through ARCCOS:
     * "(-GAMA/180)/2" to 10D. "ANGLES IN MPAC IN THE ORDER" "-( (ROLL,
     * BETA, ALFA) /180)/2" ("THESE VALUES CORRECT AT PIPUP TIME."), and
     * out of the interpreter into CM/ATUP (cm_atup.c). */
    ORDERS(VXV, UNIT),       /* "GET UNIT(-UVREL*UBY)/2 = UL/2" */
    ADDRESS_PLUS_ONE(UBY_2), /* "YB/2" */
    ORDERS(PUSH, DOT),       /* "UL/2 TO PDL 0,5" */
    ADDRESS_PLUS_ONE(UZA_2), /* "UNA/2" */
    STOVL(COSTH),            /* "COS(ROLL)/4" */
    ADDRESS_PLUS_ONE_D(0),   /* "UL/2" */
    ORDER(DOT),
    ADDRESS_PLUS_ONE(UYA_2),
    STCALL(SINTH), /* "-SIN(ROLL)/4" */
    ADDRESS(ARCTRIG),
    STOVL_D(6), /* "-(ROLL/180) /2" */
    ADDRESS_PLUS_ONE(UBY_2),
    ORDERS(DOT, SL1),        /* "-UVA.UBY = -SIN(BETA)" */
    ADDRESS_PLUS_ONE(UXA_2), /* "-UVREL/2" */
    ORDER(ARCSIN),
    STOVL_D(7),              /* "-(BETA/180) /2" */
    ADDRESS_PLUS_ONE(UBX_2), /* "XB/2" */
    ORDER(DOT),              /* "UL.UBX = -SIN(ALFA)" */
    ADDRESS_PLUS_ONE_D(0),   /* "UL/2" */
    STOVL(SINTH),            /* "-SIN(ALFA)/4" */
    ORDER(DOT),              /* "UL/2 FROM PDL 0" */
    ADDRESS_PLUS_ONE(UBZ_2),
    STCALL(COSTH), /* "COS(ALFA)/2" */
    ADDRESS(ARCTRIG),
    STOVL_D(8),              /* "-(ALFA/180) /2" */
    ADDRESS_PLUS_ONE(UNITR), /* "UR/2" */
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UZA_2), /* "MORE ACCURATE AT LARGE ARG." */
    ORDER(ARCCOS),
    STORE_D(10),           /* "(-GAMA/180)/2" */
    ORDERS(TLOAD, EXIT),   /* "ANGLES IN MPAC IN THE ORDER" */
    ADDRESS_PLUS_ONE_D(6), /* "THESE VALUES CORRECT AT PIPUP TIME." */
};

const struct interpretive_code CM_POSE_CODE = {037u, CM_POSE + 1u, COUNT(cm_pose_words),
                                               cm_pose_words};

#endif
