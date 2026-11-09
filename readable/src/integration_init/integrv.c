/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, the entries to orbital integration, in
 * the listing's notation (src/interpreter/notation.h): CSMPREC, LEMPREC,
 * CSMCONIC and LEMCONIC (LM pp. 1213-1215, CM pp. 1317-1319), INTEGRVS
 * (LM p. 1215, CM p. 1319), and INTEGRV with ALOADED (LM p. 1216, CM p.
 * 1320).
 *
 * "CSMPREC AND LEMPREC PERFORM ORBIT INTEGRATION BY THE ENCKE METHOD TO
 * THE TIME INDICATED IN TDEC1"; CSMCONIC and LEMCONIC by Kepler's method,
 * a conic. Each first takes the integration (INTSTALL), keeps its caller's
 * X1 as the return (IRETURN), and sets the flags of the table in the
 * listing: VINTFLAG for the CSM, INTYPFLG for a conic, DIM0FLAG clear (no
 * W-matrix). "THE PERMANENT STATE VECTOR IS NOT UPDATED."
 *
 * In the CM, ALOADED is in bank 11 (INTINIT1), apart from the rest. */
#include "integration_init.h"
#include "../agc/names.h"

static const word integrv_words[] = {
    /* CSMPREC -- LM p. 1214 (CM p. 1318): INTSTALL, the return in
     * IRETURN, VINTFLAG set (the CSM). IFLAGP: PRECIFLG set ("WAS THIS
     * CALL VIA CSM(LEM)PREC"), DIM0FLAG and INTYPFLG cleared (a precision
     * integration), and INTEGRV1. */
    /* CSMPREC */ ORDERS(STQ, CALL),
    ADDRESS(X1),
    ADDRESS(INTSTALL),
    ORDERS(SXA_1, SET),
    ADDRESS(IRETURN),
    SWITCH(SET, VINTFLAG),
    /* IFLAGP */ ORDERS(SET, CLEAR),
    SWITCH(SET, PRECIFLG),
    SWITCH(CLEAR, DIM0FLAG),
    ORDER(CLRGO),
    SWITCH(CLRGO, INTYPFLG),
    ADDRESS(INTEGRV1),
    /* LEMPREC: the same with VINTFLAG cleared (the LM). */
    /* LEMPREC */ ORDERS(STQ, CALL),
    ADDRESS(X1),
    ADDRESS(INTSTALL),
    ORDERS(SXA_1, CLRGO),
    ADDRESS(IRETURN),
    SWITCH(CLRGO, VINTFLAG),
    ADDRESS(IFLAGP),
    /* CSMCONIC: the CSM's conic. IFLAGC: DIM0FLAG cleared, INTYPFLG set,
     * and INTEGRV1. */
    /* CSMCONIC */ ORDERS(STQ, CALL),
    ADDRESS(X1),
    ADDRESS(INTSTALL),
    ORDERS(SXA_1, SET),
    ADDRESS(IRETURN),
    SWITCH(SET, VINTFLAG),
    /* IFLAGC */ ORDERS(CLEAR, SETGO),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(SETGO, INTYPFLG),
    ADDRESS(INTEGRV1),
    /* LEMCONIC: the LM's. */
    /* LEMCONIC */ ORDERS(STQ, CALL),
    ADDRESS(X1),
    ADDRESS(INTSTALL),
    ORDERS(SXA_1, CLRGO),
    ADDRESS(IRETURN),
    SWITCH(CLRGO, VINTFLAG),
    ADDRESS(IFLAGC),
    /* INTEGRVS -- LM p. 1215 (CM p. 1319): the integration of the state
     * vector in RCV and VCV as it stands. PRECIFLG set, the origin (PBODY)
     * the earth, or the moon with MOONFLAG set; the return in IRETURN; no
     * deviations (TDELTAV, TNUV) and RECTIFY. Then DIM0FLAG cleared,
     * NEWIFLG and RPQFLAG set, and on at ALOADED. */
    /* INTEGRVS */ ORDERS(SET, SSP),
    SWITCH(SET, PRECIFLG),
    ADDRESS_PLUS_ONE(PBODY),
    NUMBER(0),
    ORDERS(BOF, SSP),
    SWITCH(BOF, MOONFLAG),
    ADDRESS_AT(INTEGRVS, 011), /* (+3) */
    ADDRESS_PLUS_ONE(PBODY),
    NUMBER(2),
    ORDERS(STQ, VLOAD),
    ADDRESS(IRETURN),
    ADDRESS_PLUS_ONE(ZEROVEC),
    STORE(TDELTAV),
    STCALL(TNUV),
    ADDRESS(RECTIFY),
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, DIM0FLAG),
    SWITCH(SET, NEWIFLG),
    ORDER(SETGO),
    SWITCH(SETGO, RPQFLAG),
    ADDRESS(ALOADED),
    /* INTEGRV -- LM p. 1216 (CM p. 1320): "AN ENTRY TO ORBIT INTEGRATION
     * WHICH PERMITS THE CALLER" to set the flags, the return in IRETURN.
     * INTEGRV1: RPQFLAG and NEWIFLG set. INTEGRV2: QPRET gets ALOADED, the
     * return of the transfer of the permanent vector (PTOACSM for the CSM,
     * VINTFLAG set; PTOALEM for the LM). */
    /* INTEGRV */ ORDER(STQ),
    ADDRESS(IRETURN),
    /* INTEGRV1 */ ORDERS(SET, SET),
    SWITCH(SET, RPQFLAG),
    SWITCH(SET, NEWIFLG),
    /* INTEGRV2 */ ORDER(SSP),
    ADDRESS_PLUS_ONE(QPRET),
    ADDRESS(ALOADED),
    ORDERS(BON, GOTO),
    SWITCH(BON, VINTFLAG),
    ADDRESS(PTOACSM),
    ADDRESS(PTOALEM),
};

const struct interpretive_code INTEGRV_CODE = {CODE_BANK(I_CSMPREC), CODE_Z(I_CSMPREC),
                                               COUNT(integrv_words), integrv_words};

static const word aloaded_words[] = {
    /* ALOADED -- LM p. 1216 (CM p. 1320): the time to integrate to, TDEC1,
     * in TDEC; a precision integration (INTYPFLG clear) goes on at
     * TESTLOOP, a conic at RVCON. */
    /* ALOADED */ ORDER(DLOAD),
    ADDRESS_PLUS_ONE(TDEC1),
    STORE(TDEC),
    ORDERS(BOFF, GOTO),
    SWITCH(BOFF, INTYPFLG),
    ADDRESS(TESTLOOP),
    ADDRESS(RVCON),
};

const struct interpretive_code ALOADED_CODE = {CODE_BANK(I_ALOADED), CODE_Z(I_ALOADED),
                                               COUNT(aloaded_words), aloaded_words};
