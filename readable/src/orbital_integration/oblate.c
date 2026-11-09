/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, OBLATE to COSPHIE (LM pp. 1234-1237, CM pp.
 * 1341-1344), in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs. The LM's QUALITY1 and QUALITY2,
 * which COMTERM calls, are in quality.c; the CM's are in RT8_OP_CODES.
 *
 * "THE OBLATE ROUTINE COMPUTES THE ACCELERATION DUE TO OBLATENESS." The
 * central body is not a sphere, and near it (ALPHAM, the distance, less
 * than RDE for the earth or RDM for the moon) its pull has more terms:
 * ALPHAV, the position's unit, and COSPHI/2, its component along the
 * body's pole (UZ), give the Legendre polynomials P2 to P5 (in 0D, 2D,
 * 4D), and with the body's J2, J3 and J4 constants (J2REQSQ, 2J3RE/J2,
 * J4REQ/J3, picked by X2, PBODY) the acceleration, added to FV. For the
 * earth the pole is the reference frame's Z axis (COSPHIE); for the moon
 * the position is first turned into the moon's own frame (R-TO-RP at TET,
 * into URPV) and the pole is the moon's (ZUNIT and the libration vector
 * 504LM, through MMATRIX), and QUALITY1 and QUALITY3 add the term of the
 * moon's equator not being round (J22, E3J22R2M and E32C31RM), turned
 * back (RP-TO-R). On an overflow, GOBAQUE.
 *
 * NBRANCH then goes on to the step's stage of the integration: DIFEQCNT
 * (0, -12 or -24) times -1/12 picks DIFEQ+0, DIFEQ+1 or DIFEQ+2 from
 * DIFEQTAB (CGOTO). */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word oblate_words[] = {
    /* OBLATE -- LM p. 1234 (CM p. 1341): X2 the central body; far from it
     * (ALPHAM at least RDE,2), NBRANCH; for the earth, COSPHIE. */
    /* OBLATE */ ORDERS(LXA_2, DLOAD),
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(SETPD, DSU_STAR),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_AT_X2(RDE, 0),
    ORDERS(BPL, BOF), /* "GET URPV" */
    ADDRESS(NBRANCH),
    SWITCH(BOF, MOONFLAG),
    ADDRESS(COSPHIE),
    /* The moon: the position in the moon's frame (R-TO-RP, at TET) into
     * URPV, and UZ the moon's pole. */
    ORDERS(VLOAD, PDDL),
    ADDRESS_PLUS_ONE(ALPHAV),
    ADDRESS_PLUS_ONE(TET),
    ORDERS(PDDL, CALL),
    ADDRESS_PLUS_ONE(3_5),
    ADDRESS(R_TO_RP),
    STORE(URPV),
    ORDERS(VLOAD, VXV),
    ADDRESS_PLUS_ONE(504LM),
    ADDRESS_PLUS_ONE(ZUNIT),
    ORDERS(VAD, VXM),
    ADDRESS_PLUS_ONE(ZUNIT),
    ADDRESS_PLUS_ONE(MMATRIX),
    ORDER(UNIT), /* "POSSIBLY UNNECESSARY" */
                 /* COMTERM: the Legendre polynomials of COSPHI/2. */
    /* COMTERM */ STORE(UZ),
    ORDERS(DLOAD, DMPR),
    ADDRESS_PLUS_ONE(COSPHI_2),
    ADDRESS_PLUS_ONE(3_32),
    ORDERS(PDDL, DSQ), /* "P2/64 TO PD0" */
    ADDRESS_PLUS_ONE(COSPHI_2),
    ORDERS(DMPR, DSU),
    ADDRESS_PLUS_ONE(15_16),
    ADDRESS_PLUS_ONE(3_64),
    ORDERS(PUSH, DMPR), /* "P3/32 TO PD2" */
    ADDRESS_PLUS_ONE(COSPHI_2),
    ORDERS(DMP, SL1R),
    ADDRESS_PLUS_ONE(7_12),
    ORDERS(PDDL, DMPR),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(2_3),
    ORDERS(BDSU, PUSH), /* "P4/128 TO PD4" */
    ORDERS(DMPR, DMPR),
    ADDRESS_PLUS_ONE(COSPHI_2), /* "BEGIN COMPUTING P5/1024" */
    ADDRESS_PLUS_ONE(9_16),
    ORDERS(PDDL, DMPR),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE(5_128),
    /* LM p. 1235 (CM p. 1342): the polynomials, over powers of the
     * distance, with the body's constants: the part along the position
     * (TVEC) and the part along the pole (UZ). */
    ORDER(BDSU),
    ORDER(DMP_STAR),
    ADDRESS_PLUS_ONE_AT_X2(J4REQ_J3, 0),
    ORDERS(DDV, DAD),
    ADDRESS_PLUS_ONE(ALPHAM),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(DMPR_STAR, DDV),
    ADDRESS_PLUS_ONE_AT_X2(2J3RE_J2, 0),
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(DAD, VXSC),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS_PLUS_ONE(ALPHAV),
    STODL(TVEC),
    ORDERS(DMP_STAR, SR1),
    ADDRESS_PLUS_ONE_AT_X2(J4REQ_J3, 0),
    ORDERS(DDV, DAD),
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(DMPR_STAR, SR3),
    ADDRESS_PLUS_ONE_AT_X2(2J3RE_J2, 0),
    ORDERS(DDV, DAD),
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(VXSC, VSL1),
    ADDRESS_PLUS_ONE(UZ),
    ORDER(BVSU),
    ADDRESS_PLUS_ONE(TVEC),
    STODL(TVEC),
    /* Over the distance to the fourth, normalized (the shifts in X1 and
     * S1), times J2REQSQ, shifted back and added to FV; then QUALITY1. */
    ADDRESS_PLUS_ONE(ALPHAM),
    ORDERS(NORM, DSQ),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DSQ, NORM),
    ADDRESS_PLUS_ONE(S1),
    ORDERS(PUSH, BDDV_STAR), /* "NORMED R  TO 0D" */
    ADDRESS_PLUS_ONE_AT_X2(J2REQSQ, 0),
    ORDERS(VXSC, BOV),
    ADDRESS_PLUS_ONE(TVEC),
    ADDRESS_AT(COMTERM, 073), /* (+1) "(RESET OVERFLOW INDICATOR)" */
    ORDERS(XAD_1, XAD_1),
    ADDRESS(X1),
    ADDRESS(X1),
    ORDERS(XAD_1, VSL_STAR),
    ADDRESS(S1),
    SL_X1(-22),
    ORDERS(VAD, BOV),
    ADDRESS_PLUS_ONE(FV),
    ADDRESS(GOBAQUE),
    STCALL(FV),
    ADDRESS(QUALITY1),
    /* QUALITY3 -- LM p. 1235 (CM p. 1342): back from QUALITY2 with the
     * J22 term's part in 2D; the rest of it, in the moon's frame, turned
     * back (RP-TO-R) and added to FV. */
    /* QUALITY3 */ ORDER(DSQ), /* "J22 TERM X R**4 IN 2D" */
    ORDERS(PUSH, DMP),         /* "STORE COSPHI**2 SCALED B2 IN 8D" */
    ADDRESS_PLUS_ONE(5_8),     /* "5 SCALED B3" */
    ORDERS(PDDL, SR2),
    ORDERS(DAD, BDSU), /* "END UP WITH (1-7 COSPHI**2), B5" */
    ADDRESS_PLUS_ONE_D(8),
    ADDRESS_PLUS_ONE(D1_32), /* "1 SCALED B5" */
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE(URPV), /* "X COMPONENT" */
    ADDRESS_PLUS_ONE(5_8),
    ORDERS(VXSC, VSL5), /* "AFTER SHIFT, SCALED B5" */
    ADDRESS_PLUS_ONE(URPV),
    ORDER(PDDL),
    ORDERS(DSU, DAD),
    ADDRESS_PLUS_ONE(D1_32),
    ADDRESS_PLUS_ONE_D(8),
    STODL_D(8),
    ADDRESS_PLUS_ONE(URPV),
    ORDERS(DMP, DMP),
    ADDRESS_PLUS_ONE_AT(URPV, 4), /* "Z COMPONENT" */
    ADDRESS_PLUS_ONE(5_8),
    ORDERS(SL1, DAD),
    ORDERS(PDDL, NORM),
    ADDRESS_PLUS_ONE(ALPHAM), /* "SCALED B27 FOR MOON" */
    ADDRESS_PLUS_ONE(X2),
    ORDERS(PUSH, SLOAD),
    ADDRESS_PLUS_ONE(E32C31RM),
    ORDERS(DDV, VXSC),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-3),
    ORDERS(VSL_STAR, V_SC),
    SL_X1(-27),
    ORDERS(PDDL, PDDL),
    ADDRESS_PLUS_ONE(TET),
    ADDRESS_PLUS_ONE(5_8), /* "ANY NON-ZERO CONSTANT" */
    ORDERS(LXA_2, CALL),
    ADDRESS(PBODY),
    ADDRESS(RP_TO_R),
    ORDERS(VAD, BOV),
    ADDRESS_PLUS_ONE(FV),
    ADDRESS(GOBAQUE),
    /* LM p. 1237 (CM p. 1344) */
    STORE(FV),
    /* NBRANCH: X1 DIFEQCNT, and the stage it picks. */
    /* NBRANCH */ ORDERS(SLOAD, LXA_1),
    ADDRESS_PLUS_ONE(DIFEQCNT),
    ADDRESS(MPAC),
    ORDERS(DMP, CGOTO),
    ADDRESS_PLUS_ONE(M1_12),
    ADDRESS_PLUS_ONE(MPAC),
    ADDRESS(DIFEQTAB),
    /* COSPHIE: the earth's pole is ZUNIT: COSPHI/2 the position's Z. */
    /* COSPHIE */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE_AT(ALPHAV, 4),
    STOVL(COSPHI_2),
    ADDRESS_PLUS_ONE(ZUNIT),
    ORDER(GOTO),
    ADDRESS(COMTERM),
};

const struct interpretive_code OBLATE_CODE = {CODE_BANK(I_OBLATE), CODE_Z(I_OBLATE),
                                              COUNT(oblate_words), oblate_words};
