/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, the transfers between the integration's
 * state vector and the permanent ones, in the listing's notation
 * (src/interpreter/notation.h): ATOPCSM and PTOACSM (LM pp. 1210-1211, CM
 * pp. 1314-1315), ATOPLEM and PTOALEM (LM pp. 1211-1212, CM pp.
 * 1315-1316), and USEPIOS (LM pp. 1212-1213, CM pp. 1316-1317). Each runs
 * a move (moves.c) by RTB.
 *
 * Each permanent vector keeps the sphere it is in with it (CMOONFLG,
 * LMOONFLG): ATOP... copies MOONFLAG there, PTOA... back, with PBODY, the
 * origin's index (0 the earth, 2 the moon). */
#include "integration_init.h"
#include "../agc/names.h"

static const word atopcsm_words[] = {
    /* ATOPCSM -- LM p. 1210 (CM p. 1314): "ATOPCSM TRANSFERS RRECT TO
     * RRECT +41 TO RRECTCSM TO RRECTCSM +41" (MOVEACSM), the return kept
     * in S2; then the CSM's vector for the downlink (SVDWN1), and
     * CMOONFLG as MOONFLAG. */
    /* ATOPCSM */ ORDERS(STQ, RTB),
    ADDRESS(S2),
    ADDRESS(MOVEACSM),
    ORDERS(SET, CALL),
    SWITCH(SET, CMOONFLG),
    ADDRESS(SVDWN1),
    ORDERS(BON, CLRGO),
    SWITCH(BON, MOONFLAG),
    ADDRESS(S2),
    SWITCH(CLRGO, CMOONFLG),
    ADDRESS(S2),
};

const struct interpretive_code ATOPCSM_CODE = {CODE_BANK(I_ATOPCSM), CODE_Z(I_ATOPCSM),
                                               COUNT(atopcsm_words), atopcsm_words};

static const word ptoacsm_words[] = {
    /* PTOACSM -- LM p. 1211 (CM p. 1315): "PTOACSM TRANSFERS RRECTCSM TO
     * RRECTCSM +41 TO RRECT TO RRECT +41" (MOVEPCSM), and MOONFLAG and
     * PBODY as CMOONFLG has them. */
    /* PTOACSM */ ORDERS(RTB, BON),
    ADDRESS(MOVEPCSM),
    SWITCH(BON, CMOONFLG),
    ADDRESS(SETMOON),
    /* CLRMOON: the earth's sphere. */
    /* CLRMOON */ ORDERS(CLEAR, SSP),
    SWITCH(CLEAR, MOONFLAG),
    ADDRESS_PLUS_ONE(PBODY),
    NUMBER(0),
    ORDER(RVQ),
    /* SETMOON: the moon's. */
    /* SETMOON */ ORDERS(SET, SSP),
    SWITCH(SET, MOONFLAG),
    ADDRESS_PLUS_ONE(PBODY),
    NUMBER(2),
    ORDER(RVQ),
};

const struct interpretive_code PTOACSM_CODE = {CODE_BANK(I_PTOACSM), CODE_Z(I_PTOACSM),
                                               COUNT(ptoacsm_words), ptoacsm_words};

static const word atoplem_words[] = {
    /* ATOPLEM -- LM p. 1211 (CM p. 1315): "ATOPLEM TRANSFERS RRECT TO
     * RRECT +41 TO RRECTLEM TO RRECTLEM +41" (MOVEALEM), as ATOPCSM: the
     * LM's vector for the downlink (SVDWN2), and LMOONFLG. */
    /* ATOPLEM */ ORDERS(STQ, RTB),
    ADDRESS(S2),
    ADDRESS(MOVEALEM),
    ORDERS(SET, CALL),
    SWITCH(SET, LMOONFLG),
    ADDRESS(SVDWN2),
    ORDERS(BON, CLRGO),
    SWITCH(BON, MOONFLAG),
    ADDRESS(S2),
    SWITCH(CLRGO, LMOONFLG),
    ADDRESS(S2),
};

const struct interpretive_code ATOPLEM_CODE = {CODE_BANK(I_ATOPLEM), CODE_Z(I_ATOPLEM),
                                               COUNT(atoplem_words), atoplem_words};

static const word ptoalem_words[] = {
    /* PTOALEM -- LM p. 1212 (CM p. 1316): "PTOALEM TRANSFERS RRECTLEM TO
     * RRECTLEM +41 TO RRECT TO RRECT +41" (MOVEPLEM), and MOONFLAG as
     * LMOONFLG has it; but with the LM on the moon (SURFFLAG), USEPIOS. */
    /* PTOALEM */ ORDERS(BON, RTB),
    SWITCH(BON, SURFFLAG),
    ADDRESS(USEPIOS),
    ADDRESS(MOVEPLEM),
    ORDERS(BON, GOTO),
    SWITCH(BON, LMOONFLG),
    ADDRESS(SETMOON),
    ADDRESS(CLRMOON),
};

const struct interpretive_code PTOALEM_CODE = {CODE_BANK(I_PTOALEM), CODE_Z(I_PTOALEM),
                                               COUNT(ptoalem_words), ptoalem_words};

static const word usepios_words[] = {
    /* USEPIOS -- LM p. 1212 (CM p. 1316): the LM on the moon's surface,
     * where "THE PLANETARY INERTIAL ORIENTATION SUBROUTINES" (LM p. 1205,
     * CM p. 1309) give its state. The landing site (RLS, in the moon's
     * own frame) at TDEC1, the time of the vector (TET), and RP-TO-R to
     * the reference frame: the position (RCV). Then the moon's pole
     * (ZUNIT) the same way, MOONFLAG set for X1; the velocity (VCV) the
     * pole's rotation (OMEGMOON) of the position. No deviations (TDELTAV,
     * TNUV), the moon the origin (PBODY 2), and on at A-PCHK. */
    /* USEPIOS */ ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RLS),
    ORDERS(PDDL, PUSH),
    ADDRESS_PLUS_ONE(TDEC1),
    STODL(TET),
    ADDRESS_PLUS_ONE(5_8),
    ORDER(CALL),
    ADDRESS(RP_TO_R),
    STOVL(RCV),
    ADDRESS_PLUS_ONE(ZUNIT),
    STODL_D(0),
    ADDRESS_PLUS_ONE(TET),
    STODL_D(6),
    ADDRESS_PLUS_ONE(5_8),
    ORDERS(SET, CALL), /* "NEEDED FOR SETTING X1 ON EXIT" */
    SWITCH(SET, MOONFLAG),
    ADDRESS(RP_TO_R),
    ORDERS(VXV, VXSC),
    ADDRESS_PLUS_ONE(RCV),
    ADDRESS_PLUS_ONE(OMEGMOON),
    STOVL(VCV),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(TDELTAV),
    ORDERS(AXT_2, SXA_2),
    NUMBER(2),
    ADDRESS(PBODY),
    STCALL(TNUV),
    ADDRESS(A_PCHK),
#if COMANCHE055
    /* OMEGMOON: the moon's rate of rotation. */
    /* OMEGMOON */ NUMBER(007112), /* 2DEC* 2.66169947 E-8 B+23* */
    NUMBER(006620),
#endif
};

const struct interpretive_code USEPIOS_CODE = {CODE_BANK(I_USEPIOS), CODE_Z(I_USEPIOS),
                                               COUNT(usepios_words), usepios_words};
