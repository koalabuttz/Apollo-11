/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 11 (Comanche055 pp. 603-606): S22.1's next mark, and
 * the mark's incorporation into the CSM's state vector (bank 30).
 *
 * S22SAVET keeps the mark's time, and S22I=N counts the marks: past the
 * last one, S22.1 goes on at S22F244X (the flowchart's figure 2.4-4, a
 * later part); otherwise 8KK, the mark's number, and S22LOC, its address,
 * step on to the next mark (seven words on), under a restart point, and
 * the CSM's state vector is integrated to its time again (S22FLGS's flags
 * choose the W matrix's integration). The offset point's first mark keeps
 * its time and UM apart (S22OFF=I) and starts over at S22I=N.
 *
 * With D=9 the landmark's position is carried to the mark's time (R-TO-RP
 * then RP-TO-R, through the planetary inertial orientation). S22BOX32:
 * RCL, the line from the CSM to the landmark, and USTAR, the unit normal
 * to it and UM; S22BOX12: the measurement's variance, the sextant's and
 * the IMU's, scaled by the distance squared; BVECTORS and INCORP1 give the
 * change to the state vector. When 22DSPFLG asks, the change in position
 * and velocity is shown (verb 06 noun 49) for the crew to take, refuse or
 * recycle. The next part goes on at S22BOX42.
 *
 * Each call out of the file (BANKCALL, PHASCHNG, BLANKET) leaves Z at the
 * routine and Q at the word after the TC, where the routine comes back to;
 * ENDOFJOB and GOTOPOOH don't come back. GOFLASHR comes back to the word
 * after its CADR on terminate, the next on proceed, the next on enter
 * (recycle) and the next on its immediate return. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void incorporate_changes(void);

/* S22SAVET -- p. 603: the mark's time (GETTF): "SAVE PRESENT TIME FOR
 * PIOS". S22I=N: "TEST I=N", in basic code. */
static const word s22savet_words[] = {
    ORDER(CALL),
    ADDRESS(GETTF),
    STORE(S22TPRIM), /* "SAVE PRESENT TIME FOR PIOS" */
    ORDER(EXIT),     /* "TEST I=N" */
};

/* S22I=N +1 -- after the EXIT: restart group 2 at S22I=N +3. */
void s22i_eq_n_after_exit(void)
{
    tc_to(S22I_EQ_N + 1u, PHASCHNG); /* OCT 04022 */
}

/* S22I=N +3: 8NN less 8KK, the marks left; none (+0 or less), "EXIT TO
 * FIGURE 2.4-4" (S22F244X's TC INTPRET). Otherwise "I=I+1" into TEMPOR1,
 * and S22LOC plus seven into TEMPOR1 +1: "ADD 7 TO LOC TO GET ADDR. OF
 * NEXT MARK", and restart group 2 at S22I=N +017. (8KK is below 8NN, so
 * I+1 leaves no overflow for the TS to skip on.) */
void s22i_eq_n_after_phaschng(void)
{
    cs(ADDRESS_OF(8KK));
    ad_at(ADDRESS_OF(8NN));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF S22F244X */
        tc_at(S22F244X);
        intpret();
        return;
    }
    ca(ADDRESS_OF(8KK));
    ad(ONE);
    ts(ADDRESS_OF(TEMPOR1));
    ca(ADDRESS_OF(S22LOC));
    ad(SEVEN);
    if (ts(ADDRESS_OF(TEMPOR1) + 1u)) not_modelled();
    tc_to(S22I_EQ_N + 015u, PHASCHNG); /* OCT 04022 */
}

/* S22I=N +017: the next mark's number and address into 8KK and S22LOC,
 * then interpretive code. (A CA leaves no overflow for a TS to skip on.) */
void s22i_eq_n_next_mark(void)
{
    ca(ADDRESS_OF(TEMPOR1));
    ts(ADDRESS_OF(8KK));
    ca(ADDRESS_OF(TEMPOR1) + 1u);
    ts(ADDRESS_OF(S22LOC));
    tc_at(S22I_EQ_N + 023u);
    intpret();
}

/* S22I=N +024 -- pp. 604-606. S2INTS1: the integration set up again ("FOR
 * ALL INTEGRATIONS OTHER THAN FIRST": INTSTALL, and S22FLGS's flags), and
 * on to S22NXTIN, "RETURN ALWAYS EXCEPT OFFSET POINT MARK 1" (DMENFLG set,
 * DIM0FLAG cleared); with ORBWFLAG clear (DIM0FLAG set), "OFFSET POINT MARK
 * 1, NO W INTEGRATION"; else "OFFSET POINT MARK 1, INTEGRATE W 6X6"
 * (D6OR9FLG cleared).
 *
 * S22OFF=I: the offset point's "TIME SUB OFF" (S22TOFF) and "U SUB OFF"
 * (S22UOFF, UM), and S22I=N again.
 *
 * S22D=9, the "D=9 PATH": the landmark's position X789 at S22TPRIM, "CALL
 * PIOS TWICE TO TRANSFORM RL TO TIME", "T(SUB F) FROM TIME T PRIME" (the
 * mark's), through R-TO-RP and RP-TO-R; S22EORM, "0=EARTH, NON-ZERO=MOON".
 *
 * S22BOX32: INCORFLG set ("FLAG=1"), BOV "CLEAR OVERFLOW"; "RCL=RL-RC",
 * the line from the CSM's position CSMPOS to the landmark's, in RCLP;
 * "USTAR=UNIT(UNIT(RCL)XUM)", or on overflow S22SAVET again: "COMPUTATION
 * OVERFLOW, SAVE TF".
 *
 * S22BOX12: DMENFLG "=1 FOR 9X9 W" and VEHUPFLG "=1 FOR CSM"; the
 * sextant's and the IMU's variances (SCTVAR, IMUVARR, "B+18") in VARIANCE;
 * times RCL's length squared, normalized (the shift in X1), and X1 plus
 * twice the normalization and twice S22EORM ("DOUBLE NORM SHIFT SINCE RCLP
 * WAS SQUARED", "DOUBLE EARTH OR MOON SHIFT, SAME REASON"), the shift
 * that will "SCALE VARIANCE B-40 FOR BOTH EARTH, MOON"; "CHANGE MODE TO
 * TRIPLE" (TLOAD MPAC), into VARIANCE. BVECTORS: "CALC B0,B1,DELTAQ, NEW
 * USTAR"; "B2=-B0" into BVECTOR +12D; INCORP1, the state vector's change.
 * GRP2PC, a restart point.
 *
 * With 22DSPFLG ("=1 DISPLAY DELTA R,V") clear, on at S22BOX42; set, it is
 * cleared, a restart point, and N49DISP gets the changes' lengths:
 * DELTAX's ("DELTA R") shifted by S22EORM, "SCALE DELTA R ALWAYS METERS
 * B-29", and DELTAX +6's ("DELTA V") by the same, "DELTA V=METERS/CSEC B-7
 * ALWAYS". The display is in basic code. */
static const word s22next_words[] = {
    /* S2INTS1: "FOR ALL INTEGRATIONS OTHER THAN FIRST" */
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDER(CALL),
    ADDRESS(S22FLGS),
    ORDERS(BON, CLEAR),
    SWITCH(BON, DMENFLG),
    /* "RETURN ALWAYS EXCEPT OFFSET POINT MARK 1" */
    ADDRESS(S22NXTIN),
    SWITCH(CLEAR, DIM0FLAG),
    ORDERS(BOFF, SET),
    SWITCH(BOF, ORBWFLAG),
    /* "OFFSET POINT MARK 1, NO W INTEGRATION" */
    ADDRESS(S22NXTIN),
    SWITCH(SET, DIM0FLAG),
    ORDER(CLRGO),
    SWITCH(CLRGO, D6OR9FLG),
    /* "OFFSET POINT MARK 1, INTEGRATE W 6X6" */
    ADDRESS(S22NXTIN),
    /* S22OFF=I */
    ORDER(CALL),
    ADDRESS(GETTF),
    /* "TIME SUB OFF" */
    STOVL(S22TOFF),
    ADDRESS_PLUS_ONE(UM),
    /* "U SUB OFF" */
    STCALL(S22UOFF),
    /* "TEST I=N" */
    ADDRESS(S22I_EQ_N),
    /* S22D=9: "D=9 PATH" */
    ORDER(VLOAD),
    ADDRESS_PLUS_ONE(X789),
    /* "CALL PIOS TWICE TO TRANSFORM RL TO TIME" */
    STODL_D(0),
    /* "T(SUB F) FROM TIME T PRIME" */
    ADDRESS_PLUS_ONE(S22TPRIM),
    STORE_D(6),
    ORDERS(SLOAD, CALL),
    /* "0=EARTH, NON-ZERO=MOON" */
    ADDRESS_PLUS_ONE(S22EORM),
    /* S2RTRP */
    ADDRESS(R_TO_RP),
    /* "R-TO-RP LEAVES PUSHLOC AT 0" */
    ORDERS(PUSH, CALL),
    ADDRESS(GETTF),
    STORE_D(6),
    ORDERS(SLOAD, CALL),
    ADDRESS_PLUS_ONE(S22EORM),
    /* S2RPTR */
    ADDRESS(RP_TO_R),
    /* S22BOX32 */
    STORE(X789),
    ORDERS(SET, BOV),
    /* "FLAG=1" */
    SWITCH(SET, INCORFLG),
    /* "CLEAR OVERFLOW" */
    ADDRESS_AT(S22BOX32, 4), /* +1 */
    ORDER(VSU),
    ADDRESS_PLUS_ONE(CSMPOS),
    /* "RCL=RL-RC" */
    STORE(RCLP),
    /* "USTAR=UNIT(UNIT(RCL)XUM)" */
    ORDERS(UNIT, VXV),
    ADDRESS_PLUS_ONE(UM),
    ORDERS(UNIT, BOV),
    /* "COMPUTATION OVERFLOW, SAVE TF" */
    ADDRESS(S22SAVET),
    STORE(USTAR),
    /* S22BOX12 */
    ORDERS(SET, SET),
    /* "=1 FOR 9X9 W" */
    SWITCH(SET, DMENFLG),
    /* "=1 FOR CSM" */
    SWITCH(SET, VEHUPFLG),
    ORDERS(DLOAD, DAD),
    /* "B+18" */
    ADDRESS_PLUS_ONE(SCTVAR),
    /* "B+18" */
    ADDRESS_PLUS_ONE(IMUVARR),
    STOVL(VARIANCE),
    /* "B-29 OR B-27" */
    ADDRESS_PLUS_ONE(RCLP),
    ORDERS(ABVAL, NORM),
    ADDRESS_PLUS_ONE(X1),
    ORDERS(DSQ, DMP),
    ADDRESS_PLUS_ONE(VARIANCE),
    ORDERS(XAD_1, XAD_1),
    /* "DOUBLE NORM SHIFT SINCE RCLP WAS SQUARED" */
    ADDRESS(X1),
    /* "DOUBLE EARTH OR MOON SHIFT, SAME REASON" */
    ADDRESS(S22EORM),
    ORDERS(XAD_1, SR_STAR),
    ADDRESS(S22EORM),
    /* "SCALE VARIANCE B-40 FOR BOTH EARTH, MOON" */
    SR_X1(0),
    /* "CHANGE MODE TO TRIPLE" */
    ORDER(TLOAD),
    ADDRESS_PLUS_ONE(MPAC),
    /* "CALC B0,B1,DELTAQ, NEW USTAR" */
    STCALL(VARIANCE),
    /* S2BVTRS */
    ADDRESS(BVECTORS),
    ORDERS(VLOAD, VCOMP),
    ADDRESS_PLUS_ONE(BVECTOR),
    /* "B2=-B0" */
    STCALL_AT(BVECTOR, 12),
    /* S2INCP1 */
    ADDRESS(INCORP1),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(BOFF, CLEAR),
    /* "=1 DISPLAY DELTA R,V" */
    SWITCH(BOF, 22DSPFLG),
    ADDRESS(S22BOX42),
    SWITCH(CLEAR, 22DSPFLG),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(VLOAD, ABVAL),
    /* "DELTA R" */
    ADDRESS_PLUS_ONE(DELTAX),
    ORDERS(LXA_1, SR_STAR),
    /* "SCALE DELTA R ALWAYS METERS B-29" */
    ADDRESS(S22EORM),
    SR_X1(0),
    STOVL(N49DISP),
    /* "DELTA V" */
    ADDRESS_PLUS_ONE_AT(DELTAX, 6),
    /* "DELTA V=METERS/CSEC B-7 ALWAYS" */
    ORDERS(ABVAL, SR_STAR),
    SR_X1(0),
    STORE_AT(N49DISP, 2),
    ORDER(EXIT),
};

/* S22.1 +0560 -- after the EXIT: verb 06 noun 49 (V06N49EE), the changes
 * in position and velocity, flashed by GOFLASHR. */
void s22_display_after_exit(void)
{
    set_A(V06N49EE);
    tc_to(S22_1 + 0561u, BANKCALL); /* CADR GOFLASHR */
}

/* S22.1 +0563: "V34E TERMINATE". */
void s22_display_terminate(void)
{
    tc_to(S22_1 + 0563u, GOTOPOOH);
}

/* S22.1 +0564: proceed, TC +5: "INCORPORATE CHANGES". */
void s22_display_proceed(void)
{
    tc_at(S22_1 + 0564u);
    incorporate_changes();
}

/* S22.1 +0565: enter, "V32E RECYCLE" (S22EXEX, part 12). */
void s22_display_enter(void)
{
    tc_at(S22_1 + 0565u);
    s22exex();
}

/* S22.1 +0566: the display's immediate return: R3 blanked (BLANKET's bit
 * 3), */
void s22_display_immediate_return(void)
{
    set_A(BIT3);
    tc_to(S22_1 + 0567u, BLANKET);
}

/* S22.1 +0570: and the job ends. */
void s22_display_after_blanket(void)
{
    tc_to(S22_1 + 0570u, ENDOFJOB);
}

/* S22.1 +0571: TC INTPRET, on in interpretive code at S22BOX42 (the next
 * part). */
static void incorporate_changes(void)
{
    tc_at(S22_1 + 0571u);
    intpret();
}

/* ---- The interpretive code ---- */

const struct interpretive_code S22SAVET_CODE = {030u, S22_1 + 0375u, COUNT(s22savet_words),
                                                s22savet_words};
const struct interpretive_code S22NEXT_CODE = {030u, S22_1 + 0424u, COUNT(s22next_words),
                                               s22next_words};

#endif
