/* SPDX-License-Identifier: MIT */
/* ORBITAL_INTEGRATION.agc, DIFEQTAB to DIFEQ0 (LM pp. 1237-1238, CM pp.
 * 1344-1345), with the sphere of influence checks (the LM's EARSPH and
 * LUNSPH, the CM's CHKSWTCH, RPQOK and EARSPH), in the listing's notation
 * (src/interpreter/notation.h), which the readable interpreter runs.
 *
 * TIMESTEP starts each time step of the precision integration. In
 * midcourse (MIDFLAG) it first asks whether the vehicle has crossed the
 * moon's sphere of influence (RSPHERE from the moon): if so, DOSWITCH
 * moves the state vector to the other center. The LM asks only while the
 * vehicle moves away from the center ((R.V) times the step positive):
 * in the earth's sphere, with the moon's position (LSPOS into RPQV, or
 * RPQV itself when RPQFLAG says it is current), whether the vehicle is
 * nearer the moon than RSPHERE (INLUNCHK); in the moon's sphere (LUNSPH),
 * whether it is farther (10D, the distance, shifted to the earth's
 * scale), and then the earth's position from the moon (LUNPOS, VCOMP).
 * The CM's CHKSWTCH gives the same test as a number, the distance outside
 * the sphere (negative when the vehicle should change), with the moon's
 * position kept in RPQV.
 *
 * RECTEST: "RECTIFY IF" the deviation from the conic has grown (TDELTAV
 * or TNUV at least 3/4, or TDELTAV at least RECRATIO of the distance),
 * CALLRECT (RECTIFY). INTGRATE sets the step's Nystrom variables (YV, ZV
 * from TDELTAV, TNUV; JSWITCH clear: the state vector, not the W-matrix)
 * and DIFEQ0 its first stage (DIFEQCNT 0, H zero, ALPHAV the deviation),
 * on to ACCOMP (or DOW.. for the W-matrix). */
#include "orbital_integration.h"
#include "../agc/names.h"

static const word timestep_words[] = {
    /* DIFEQTAB -- LM p. 1237 (CM p. 1344): NBRANCH's three stages. */
    /* DIFEQTAB */ ADDRESS(DIFEQP0),
    ADDRESS(DIFEQP1),
    ADDRESS(DIFEQP2),
#if LUMINARY099
    /* TIMESTEP: out of midcourse, or moving in, RECTEST; then by the
     * sphere the vehicle is in. */
    /* TIMESTEP */ ORDERS(BOF, VLOAD),
    SWITCH(BOF, MIDFLAG),
    ADDRESS(RECTEST),
    ADDRESS_PLUS_ONE(RCV),
    ORDERS(DOT, DMP),
    ADDRESS_PLUS_ONE(VCV),
    ADDRESS_PLUS_ONE(DT_2), /* "(R.V) X (DELTA T)" */
    ORDER(BMN),
    ADDRESS(RECTEST),
    ORDERS(BON, BOF),
    SWITCH(BON, MOONFLAG),
    ADDRESS(LUNSPH),
    SWITCH(BOF, RPQFLAG),
    ADDRESS(EARSPH),
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TET),
    ADDRESS(LSPOS), /* "RPQV IN MPAC" */
    STORE(RPQV),
    ORDER(LXA_2),
    ADDRESS(PBODY),
    /* INLUNCHK: the distance from the moon, less RSPHERE; inside,
     * DOSWITCH. */
    /* INLUNCHK */ ORDERS(BVSU, ABVAL),
    ADDRESS_PLUS_ONE(RCV),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(RSPHERE),
    ADDRESS(DOSWITCH),
#else
    /* TIMESTEP: out of midcourse, RECTEST; otherwise CHKSWTCH, and
     * DOSWITCH when it says so. */
    /* TIMESTEP */ ORDERS(BOF, CALL),
    SWITCH(BOF, MIDFLAG),
    ADDRESS(RECTEST), /* "SKIP ORIGIN CHANGE LOGIC" */
    ADDRESS(CHKSWTCH),
    ORDER(BMN),
    ADDRESS(DOSWITCH),
#endif
    /* RECTEST -- LM p. 1237 (CM p. 1344): RECTIFY on an overflow or
     * past either limit; otherwise INTGRATE. */
    /* RECTEST */ ORDERS(VLOAD, ABVAL), /* "RECTIFY IF" */
    ADDRESS_PLUS_ONE(TDELTAV),
    ORDER(BOV),
    ADDRESS(CALLRECT),
    ORDERS(DSU, BPL), /* "EITHER TDELTAV OR TNUV EQUALS OR" */
    ADDRESS_PLUS_ONE(3_4),
    ADDRESS(CALLRECT),
    ORDERS(DAD, SL_STAR),
    ADDRESS_PLUS_ONE(3_4),
    SL_X2(-7),
    ORDERS(DDV, DSU),
    ADDRESS_PLUS_ONE_D(10),
    ADDRESS_PLUS_ONE(RECRATIO),
    ORDERS(BPL, VLOAD),
    ADDRESS(CALLRECT),
    ADDRESS_PLUS_ONE(TNUV),
    ORDERS(ABVAL, DSU),
    ADDRESS_PLUS_ONE(3_4),
    ORDER(BOV),
    ADDRESS(CALLRECT),
    ORDER(BMN),
    ADDRESS(INTGRATE),
    /* CALLRECT */ ORDER(CALL),
    ADDRESS(RECTIFY),
    /* INTGRATE: ZV and YV the deviations, JSWITCH clear. */
    /* INTGRATE */ ORDER(VLOAD),
    ADDRESS_PLUS_ONE(TNUV),
    STOVL(ZV),
    ADDRESS_PLUS_ONE(TDELTAV),
    STORE(YV),
    ORDER(CLEAR),
    SWITCH(CLEAR, JSWITCH),
    /* DIFEQ0: the first stage; the W-matrix's (DOW..) or the state
     * vector's (ACCOMP). */
    /* DIFEQ0 */ ORDERS(VLOAD, SSP),
    ADDRESS_PLUS_ONE(YV),
    ADDRESS_PLUS_ONE(DIFEQCNT),
    NUMBER(0),
    STODL(ALPHAV),
    ADDRESS_PLUS_ONE(DPZERO),
    STORE(H), /* "START H AT ZERO.  GOES 0(DELT/2)DELT." */
    ORDERS(BON, GOTO),
    SWITCH(BON, JSWITCH),
    ADDRESS(DOW__),
    ADDRESS(ACCOMP),
#if LUMINARY099
    /* EARSPH: the moon's position current (RPQFLAG), INLUNCHK. */
    /* EARSPH */ ORDERS(VLOAD, GOTO),
    ADDRESS_PLUS_ONE(RPQV),
    ADDRESS(INLUNCHK),
    /* LUNSPH: inside the moon's sphere, RECTEST; outside, the earth's
     * position from the moon (unless RPQFLAG says RPQV is current), and
     * DOSWITCH. */
    /* LUNSPH */ ORDERS(DLOAD, SR2),
    ADDRESS_PLUS_ONE_D(10),
    ORDERS(DSU, BMN),
    ADDRESS_PLUS_ONE(RSPHERE),
    ADDRESS(RECTEST),
    ORDERS(BOF, DLOAD),
    SWITCH(BOF, RPQFLAG),
    ADDRESS(DOSWITCH),
    ADDRESS_PLUS_ONE(TET),
    ORDER(CALL),
    ADDRESS(LUNPOS),
    ORDER(VCOMP),
    STORE(RPQV),
#else
    /* CHKSWTCH: the return kept in ORIGEX; the moon's position at TET
     * (from the moon, the earth's: VCOMP) into RPQV, unless RPQFLAG says
     * it is current. */
    /* CHKSWTCH */ ORDERS(STQ, BOF),
    ADDRESS(ORIGEX),
    SWITCH(BOF, RPQFLAG),
    ADDRESS(RPQOK), /* "MOON POSITION IS AVAILABLE" */
    ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(TET),
    ADDRESS(LUNPOS), /* "GET MOON POSITION" */
    ORDERS(BOF, VCOMP),
    SWITCH(BOF, MOONFLAG),
    ADDRESS_AT(CHKSWTCH, 012), /* (+1) */
    STORE(RPQV),
    /* RPQOK: the position, X2 the central body again; in the moon's
     * sphere RSPHERE less the distance, outside it (EARSPH) the distance
     * from the moon less RSPHERE; back by ORIGEX. */
    /* RPQOK */ ORDERS(LXA_2, VLOAD), /* "RESTORE X2 AFTER USING LUNPOS" */
    ADDRESS(PBODY),
    ADDRESS_PLUS_ONE(TDELTAV),
    ORDERS(VSL_STAR, VAD),
    VSL_X2(-7),
    ADDRESS_PLUS_ONE(RCV), /* "R = RDEVIATION + RCONIC" */
    ORDERS(BOF, ABVAL),
    SWITCH(BOF, MOONFLAG),
    ADDRESS(EARSPH),
    ORDERS(SR2, BDSU), /* "INSIDE" */
    ADDRESS_PLUS_ONE(RSPHERE),
    ORDER(GOTO),
    ADDRESS(ORIGEX),
    /* EARSPH */ ORDERS(VSU, ABVAL), /* "OUTSIDE" */
    ADDRESS_PLUS_ONE(RPQV),
    ORDERS(DSU, GOTO),
    ADDRESS_PLUS_ONE(RSPHERE),
    ADDRESS(ORIGEX),
#endif
};

const struct interpretive_code TIMESTEP_CODE = {CODE_BANK(I_DIFEQTAB), CODE_Z(I_DIFEQTAB),
                                                COUNT(timestep_words), timestep_words};
