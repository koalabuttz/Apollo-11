/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 4 (Comanche055 pp. 576-578): BVECTORS and GETUM, the
 * measurement geometry of the CSM's rendezvous navigation (bank 23), both
 * interpretive subroutines that R22 calls.
 *
 * BVECTORS takes UM, the "1/2 UNIT VECTOR ALONG THE CSM-LM LINE-OF-SIGHT",
 * and RCLP, the "RELATIVE CSM TO LM POSITION VECTOR" the state vectors
 * give, and works out the measurement's B vector (USTAR made at right
 * angles to RCLP) and DELTAQ, the "MEASURED DEVIATION": the angle between
 * the line of sight measured and the one estimated, times RCLP's length.
 *
 * GETUM gives UM itself, in MPAC: the mark's shaft and trunnion angles
 * (SXTNB, to navigation base axes) and its IMU gimbal angles (NBSM, to the
 * stable member) and REFSMMAT, to the basic reference system. It copies
 * the mark data to MARKDOWN for the downlink too. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* BVECTORS -- p. 576. The return in EGRESS. RCLP's unit vector to 26D and
 * its length ("RCLP ABS VALUE") from 36D to TEMPOR1, a "SAFE LOCATION";
 * "USTAR = UNIT(US X UCL)" is the B vector, kept in BVECTOR and USTAR (a
 * restart phase change between, GRP2PC). Then the angle between USTAR and
 * UM, less a quarter turn: ARCCOS of twice "USTAR DOT UM" (SL1, half unit
 * vectors) less DP1/4TH, normalized (NORM, its shift count in X1), times
 * pi/4 (PI/4.0) and TEMPOR1, and shifted back by the count ("ADJUST
 * SCALING"): DELTAQ. The 9 dimensional BVECTOR's other two vectors +0
 * (ZEROVECS), and back through EGRESS. */
static const word bvectors_words[] = {
    ORDER(STQ),
    ADDRESS(EGRESS),
    ORDERS(VLOAD, UNIT),
    ADDRESS_PLUS_ONE(RCLP), /* "RELATIVE POSITION VECTOR" */
    STODL_D(26),            /* "RCLP UNIT VEC" */
    ADDRESS_PLUS_ONE_D(36), /* "RCLP ABS VALUE" */
    STOVL(TEMPOR1),         /* "MOVE TO SAFE LOCATION" */
    ADDRESS_PLUS_ONE(USTAR),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE_D(26), /* "USTAR = UNIT(US X UCL)" */
    STCALL(BVECTOR),
    ADDRESS(GRP2PC), /* "PHASE CHANGE" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(BVECTOR),
    STORE(USTAR),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE(UM), /* "USTAR DOT UM" */
    ORDERS(ACOS, DSU),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ORDERS(NORM, DMP),
    ADDRESS_PLUS_ONE(X1),
    ADDRESS_PLUS_ONE(PI_4_0),
    ORDERS(DMP, SRR_STAR),
    ADDRESS_PLUS_ONE(TEMPOR1), /* "RCLP ABS VALUE" */
    SRR_X1(-3),                /* "ADJUST SCALING" */
    STOVL(DELTAQ),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE_AT(BVECTOR, 6),
    STORE_AT(BVECTOR, 12),
    ORDER(GOTO),
    ADDRESS(EGRESS),
};

/* GETUM -- p. 578. The return in EGRESS, the push-down pointer 0. MARKDATA
 * "CONTAINS ADDRESS OF MARK DATA"; X1 is its complement (LXC,1), so that
 * n,1 is the mark data's word n. Its seven words go to MARKDOWN: words 1
 * to 6 as a vector (VLOAD* and STODL* MARKDOWN +1) and words 0 and 1 as a
 * double (0,1 and STORE MARKDOWN). X2 is 2 less X1 (AXT,2 and XSU,2), the
 * mark data's address +2, and S1 keeps it (SXA,2): the angles, for SXTNB
 * and NBSM. Then by REFSMMAT to the basic reference system ("MPAC = (UM)
 * LINE OF SIGHT VECTOR", VSL1 to keep it a half unit vector), and back
 * through EGRESS. */
static const word getum_words[] = {
    ORDERS(STQ, SETPD),
    ADDRESS(EGRESS),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(LXC_1, VLOAD_STAR),
    ADDRESS(MARKDATA), /* "CONTAINS ADDRESS OF MARK DATA" */
    ADDRESS_PLUS_ONE_X1(1),
    STODL_STAR_AT(MARKDOWN, 1), /* "TRANSFER DATA FROM WORKING STORAGE" */
    ADDRESS_PLUS_ONE_X1(0),     /* "TO MARKDOWN ARRAY FOR DOWNLINK" */
    STORE(MARKDOWN),
    ORDER(AXT_2),
    NUMBER(2),
    ORDERS(XSU_2, SXA_2),
    ADDRESS(X1), /* "X1 = MARKDATA" */
    ADDRESS(S1), /* "S1 = MARKDATA(ADR) +2" */
    ORDER(CALL),
    ADDRESS(SXTNB), /* "SEXT. ANGULAR READINGS TO NAV. BASE COOR." */
    ORDER(CALL),
    ADDRESS(NBSM), /* "TRANSFORM FROM NAV. BASE TO STABLE MEM." */
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(GOTO),     /* "MPAC = (UM) LINE OF SIGHT VECTOR" */
    ADDRESS(EGRESS), /* "EXIT" */
};

/* ---- The interpretive code ---- */

const struct interpretive_code BVECTORS_CODE = {023u, BVECTORS, COUNT(bvectors_words),
                                                bvectors_words};
const struct interpretive_code GETUM_CODE = {023u, GETUM, COUNT(getum_words), getum_words};

#endif
