/* SPDX-License-Identifier: MIT */
/* INTEGRATION_INITIALIZATION.agc, AVETOMID (LM pp. 1221-1222, CM pp.
 * 1327-1328), in the listing's notation (src/interpreter/notation.h),
 * which the readable interpreter runs.
 *
 * "THIS ROUTINE PERFORMS THE TRANSITION FROM A THRUSTING PHASE TO THE
 * COAST PHASE": at the end of a burn, the state vector AVERAGEG has kept
 * (RN, VN, PIPTIME) becomes this vehicle's permanent one. Before that the
 * W-matrix, when it is valid (RENDWFLG or ORBWFLAG), is integrated through
 * the burn with the state vector from before it (INT/W). The other
 * vehicle's state vector is brought to PIPTIME too: the LM's code does it
 * first (OTHERS, the CSM's), the CM's last (the LM's, unless it is on the
 * moon's surface). The CM's AVETOMID is in another bank (INTINIT2). */
#include "integration_init.h"
#include "../agc/names.h"

static const word avetomid_words[] = {
    /* AVETOMID -- LM p. 1221 (CM p. 1327): the return kept in EGRESS;
     * INT/W first when "W-MATRIX VALID". */
    /* AVETOMID */ ORDERS(STQ, BON),
    ADDRESS(EGRESS),
    SWITCH(BON, RENDWFLG),
    ADDRESS(INT_W), /* "W-MATRIX VALID" */
    ORDER(BON),
    SWITCH(BON, ORBWFLAG),
    ADDRESS(INT_W),
#if LUMINARY099
    /* OTHERS: the CSM's state vector ("CM") integrated, permanent and
     * without the W-matrix (SETIFLGS), to PIPTIME ("DESIRED TIME"). */
    /* OTHERS */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(PIPTIME), /* "DESIRED TIME" */
    ADDRESS(INTSTALL),
    ORDERS(SET, CALL),
    SWITCH(SET, VINTFLAG), /* "CM" */
    ADDRESS(SETIFLGS),
    STCALL(TDEC1),
    ADDRESS(INTEGRV),
#endif
    /* "NOW MOVE PROPERLY SCALE" RN, VN and PIPTIME "TO INTEGRATION
     * ERASABLES": X2 2 in the moon's sphere (the LM's MOONTHIS, the CM's
     * AMOONFLG), else 0, the shift for its scale; RN into RRECT and RCV,
     * PIPTIME into TET, and VN to MINIRECT, which "FINISH SETTING UP
     * STATE VECTOR". MOVATHIS (by RTB) makes it this vehicle's permanent
     * one, and TRKMKCNT is zeroed. */
    /* SETCOAST (the CM's) */ ORDERS(AXT_2, CALL),
    NUMBER(2),
    ADDRESS(INTSTALL),
    ORDERS(BON, AXT_2),
#if LUMINARY099
    SWITCH(BON, MOONTHIS),
    ADDRESS_AT(OTHERS, 017), /* (+2) */
#else
    SWITCH(BON, AMOONFLG),
    ADDRESS_AT(SETCOAST, 7), /* (+2) */
#endif
    NUMBER(0),
    ORDERS(VLOAD, VSR_STAR),
    ADDRESS_PLUS_ONE(RN),
    SR_X2(0),
    STORE(RRECT),
    STODL(RCV),
    ADDRESS_PLUS_ONE(PIPTIME),
    STOVL(TET),
    ADDRESS_PLUS_ONE(VN),
    ORDERS(VSR_STAR, CALL),
    SR_X2(0),
    ADDRESS(MINIRECT), /* "FINISH SETTING UP STATE VECTOR" */
    ORDERS(RTB, SSP),
    ADDRESS(MOVATHIS), /* "PUT TEMP STATE VECTOR INTO PERMANENT" */
    ADDRESS_PLUS_ONE(TRKMKCNT),
    NUMBER(0),
#if LUMINARY099
    ORDER(GOTO),
    ADDRESS(FAZAB5),
#else
    /* The CSM's sphere (CMOONFLG) as AMOONFLG; then the LM's state
     * vector, unless it is on the surface ("NO COASTING LM", SURFFLAG),
     * to PIPTIME, permanent, and back to the caller (EGRESS). */
    ORDERS(SET, BON),
    SWITCH(SET, CMOONFLG),
    SWITCH(BON, AMOONFLG),
    ADDRESS_AT(SETCOAST, 034), /* (+3) */
    ORDER(CLEAR),
    SWITCH(CLEAR, CMOONFLG),
    ORDERS(BON, DLOAD), /* "NOW DO LM" */
    SWITCH(BON, SURFFLAG),
    ADDRESS(FAZAB5), /* "NO COASTING LM" */
    ADDRESS_PLUS_ONE(PIPTIME),
    STCALL(TDEC1),
    ADDRESS(SETIFLGS),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, VINTFLAG),
    ADDRESS(INTEGRV),
    ORDER(GOTO),
    ADDRESS(EGRESS),
#endif
    /* INT/W -- LM p. 1222 (CM p. 1328): "INTEGRATE W THRU BURN" to
     * PIPTIME, this vehicle's state vector (VINTFLAG) and the W-matrix
     * (DIM0FLAG; D6OR9FLG set, "9X9 FOR LM", in the LM's code, clear in
     * the CM's), not touching RN, VN and PIPTIME (AVEMIDSW: "SO WON'T
     * CLOBBER" them); then the rest of AVETOMID. */
    /* INT/W */ ORDERS(DLOAD, CALL),
    ADDRESS_PLUS_ONE(PIPTIME), /* "INTEGRATE W THRU BURN" */
    ADDRESS(INTSTALL),
    ORDERS(SET, SET),
    SWITCH(SET, DIM0FLAG), /* "DO W-MATRIX" */
    SWITCH(SET, AVEMIDSW),
#if LUMINARY099
    ORDERS(SET, CLEAR),
    SWITCH(SET, D6OR9FLG),   /* "9X9 FOR LM" */
    SWITCH(CLEAR, VINTFLAG), /* "LM" */
#else
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, D6OR9FLG),
    SWITCH(SET, VINTFLAG),
#endif
    STCALL(TDEC1),
    ADDRESS(INTEGRV),
    ORDER(GOTO),
#if LUMINARY099
    ADDRESS(OTHERS), /* "NOW GO DO THE OTHER VEHICLE" */
#else
    ADDRESS(SETCOAST),
#endif
};

const struct interpretive_code AVETOMID_CODE = {CODE_BANK(I_AVETOMID), CODE_Z(I_AVETOMID),
                                                COUNT(avetomid_words), avetomid_words};
