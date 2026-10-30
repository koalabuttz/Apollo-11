/* SPDX-License-Identifier: MIT */
/* Miscellaneous vector operations, the orders that mostly call
 * subroutines, and SIGN and COMP: INTERPRETER.agc LM pp. 1038-1046, CM pp.
 * 1143-1151. The faithful C is c64-port/src/lm_interp_core.c (lmi_dot,
 * vxmmxv, lmi_vxsc, lmi_vproj, lmi_vxv, mpacvbuf, alsignag, lmi_dmpr,
 * lmi_ddv, lmi_setpd, lmi_tslc, lmi_gshift, lmi_vsc, lmi_sign, comp).
 *
 * "MISCELLANEOUS VECTOR OPERATIONS. INCLUDED HERE ARE THE FOLLOWING.
 *	1. DOT	DP VECTOR DOT PRODUCT.
 *	2. VXV	DP VECTOR CROSS PRODUCT.
 *	3. VXSC	DP VECTOR TIMES SCALAR.
 *	4. V/SC	DP VECTOR DIVIDED BY SCALAR.
 *	5. VPROJ	DP VECTOR PROJECTION. ( (MPAC.X)MPAC ).
 *	6. VXM	DP VECTOR POST-MULTIPLIED BY MATRIX.
 *	7. MXV	DP VECTOR PRE-MULTIPLIED BY MATRIX."
 *
 * A vector in MPAC is three double-precision components: MPAC, MPAC +3 and
 * MPAC +5 (MPAC +2 is the third word of a triple-precision scalar). The
 * operand X is at the address in ADDRWD; a matrix is three such vectors,
 * by rows. */
#include "interpreter.h"
#include "../agc/names.h"

static void vxm_mxv(void);
static void vvxsc(void);
static void vvxsc_plus1(void);
static void dvxsc(void);
static void uppos(void);
static void upneg(void);
static void bddv_plus4(void);
static void vv_sc(void);
static void vv_sc_plus1(void);
static void v_sc1(void);
static void dv_sc(void);
static void ccsl(void);

/* ======================================================================
 * DOT, MXV, VXM
 * ====================================================================== */

/* DOT, DMODE -- LM p. 1038, CM p. 1143.
 * "DO THE DOT PRODUCT AND EXIT, CHANGING THE MODE TO DP SCALAR." */
void dot(void)
{
    tc_at(DOT);
    predot();
    dmode();
}

void dmode(void)
{
    set_A(ZERO);
    newmode();
}

/* MXV, VXM -- LM p. 1038, CM p. 1143.
 * Each component of the answer is a dot product of the vector in MPAC with
 * a vector of the matrix. MATINC steps ADDRWD from one to the next and
 * DOTINC from one component to the next within it: for MXV the rows ("SET
 * UP MATINC AND DOTINC FOR ROW VECTORS": DOTINC 2, MATINC 2 more), for VXM
 * the columns ("REFER TO MATRIX AS THREE COLUMN VECTORS": DOTINC 6, MATINC
 * back to the next column, -10). */
void mxv(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(MATINC));
    vxm_mxv(); /* "GO TO COMMON PORTION." (A, still 2, is DOTINC) */
}

void vxm(void)
{
    set_A(negate(TEN));
    ts(ADDRESS_OF(MATINC));
    set_A(SIX);
    vxm_mxv();
}

/* VXM/MXV -- LM p. 1039, CM p. 1144. "COMMON PORTION OF MXV AND VXM."
 * The given vector is kept in VBUF; the answer builds in BUF2 (x), VBUF +2
 * (y) and MPAC (z), and is moved into MPAC at the end. */
static void vxm_mxv(void)
{
    ts(ADDRESS_OF(DOTINC));
    tc_at(VXM_MXV + 1u);
    mpacvbuf(); /* "SAVE VECTOR IN MPAC FOR FURTHER USE." */

    tc_at(VXM_MXV + 2u);
    dotsub();          /* "GO DOT TO GET X COMPONENT OF ANSWER." */
    dca(VBUF_AT(0u));  /* "MOVE MPAC VECTOR BACK INTO MPAC, SAVING */
    dxch(MPAC_AT(0u)); /* NEW X COMPONENT IN BUF2." */
    dxch(ADDRESS_OF(BUF2));
    dca(VBUF_AT(2u));
    dxch(MPAC_AT(3u));
    dca(VBUF_AT(4u));
    dxch(MPAC_AT(5u));
    ca(ADDRESS_OF(MATINC)); /* "INITIALIZE ADDRWD FOR NEXT DOT PRODUCT." */
    ads(ADDRESS_OF(ADDRWD));

    tc_at(VXM_MXV + 15u);
    dotsub();
    dxch(VBUF_AT(0u)); /* "MORE GIVEN VECTOR BACK TO MPAC, SAVING Y */
    dxch(MPAC_AT(0u)); /* COMPONENT OF ANSWER IN VBUF +2." */
    dxch(VBUF_AT(2u));
    dxch(MPAC_AT(3u));
    dxch(VBUF_AT(4u));
    dxch(MPAC_AT(5u));
    ca(ADDRESS_OF(MATINC)); /* "FORM ADDRESS OF LAST COLUMN OR ROW." */
    ads(ADDRESS_OF(ADDRWD));

    tc_at(VXM_MXV + 24u);
    dotsub();
    dxch(ADDRESS_OF(BUF2)); /* "ANSWER NOW COMPLETE. PUT COMPONENTS INTO */
    dxch(MPAC_AT(0u));      /* PROPER MPAC REGISTERS." */
    dxch(MPAC_AT(5u));
    dxch(VBUF_AT(2u));
    dxch(MPAC_AT(3u));
    continue_at(DANZIG); /* "EXIT." */
}

/* ======================================================================
 * VXSC, VPROJ
 * ====================================================================== */

/* VXSC -- LM p. 1040, CM p. 1145. "VECTOR TIMES SCALAR."
 * "TEST PRESENT MODE": a vector in MPAC times the scalar at X (VVXSC), or
 * the scalar in MPAC times the vector at X ("SEPARATE ROUTINE WHEN SCALAR
 * IS IN MPAC", DVXSC). (MODE is never -0; if it were, CCS would go one
 * word further and skip VVXSC's first TC.) */
void vxsc(void)
{
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: dvxsc(); break;
    case NEGATIVE: vvxsc(); break;
    case IS_MINUS_ZERO: vvxsc_plus1(); break;
    }
}

/* VVXSC: each component of MPAC in turn is rotated into MPAC, multiplied
 * and rounded. */
static void vvxsc(void)
{
    tc_at(VVXSC);
    dmpsub(); /* "COMPUTE X COMPONENT" */
    vvxsc_plus1();
}

static void vvxsc_plus1(void)
{
    tc_at(VVXSC + 1u);
    vround();          /* "AND ROUND IT." */
    dxch(MPAC_AT(3u)); /* "PUT Y COMPONENT INTO MPAC SAVING MPAC IN */
    dxch(MPAC_AT(0u)); /* MPAC +3." */
    dxch(MPAC_AT(3u));

    tc_at(VVXSC + 5u);
    dmpsub(); /* "DO SAME FOR Y AND Z COMPONENTS." */
    tc_at(VVXSC + 6u);
    vround();
    dxch(MPAC_AT(5u));
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(5u));

    tc_at(VVXSC + 10u);
    dmpsub();
    tc_at(VVXSC + 11u);
    vround();
    vrotatex();
}

/* VROTATEX -- LM p. 1040, CM p. 1145.
 * "EXIT USED TO RESTORE MPAC AFTER THIS TYPE OF ROTATION. CALLED BY VECTOR
 * SHIFT RIGHT, V/SC, ETC." The components went round x, y, z through MPAC
 * and are put back in order. */
void vrotatex(void)
{
    dxch(MPAC_AT(0u));
    dxch(MPAC_AT(5u));
    dxch(MPAC_AT(3u));
    dxch(MPAC_AT(0u));
    continue_at(DANZIG);
}

/* VPROJ -- LM p. 1041, CM p. 1146. "DP VECTOR PROJECTION ROUTINE."
 * "(MPAC.X)MPAC IS COMPUTED AND LEFT IN MPAC. DO DOT AND FALL INTO DVXSC."
 * (The dot product left ADDRWD at X's z component, four words on.) */
void vproj(void)
{
    tc_at(VPROJ);
    predot();
    set_A(negate(FOUR));
    ads(ADDRESS_OF(ADDRWD));
    dvxsc();
}

/* DVXSC -- LM p. 1041, CM p. 1146.
 * "VXSC WHEN SCALAR ARRIVES IN MPAC AND VECTOR IS AT X." The scalar is
 * kept in MPAC +3 and each component of X multiplied by it. */
static void dvxsc(void)
{
    dca(MPAC_AT(0u));  /* "SAVE SCALAR IN MPAC +3 AND GET X */
    dxch(MPAC_AT(3u)); /* COMPONENT OF ANSWER." */
    tc_at(DVXSC + 3u);
    dmpsub();
    tc_at(DVXSC + 4u);
    vround();

    set_A(TWO); /* "ADVANCE ADDRWD TO Y COMPONENT OF X." */
    ads(ADDRESS_OF(ADDRWD));
    dca(MPAC_AT(3u));  /* "PUT SCALAR BACK INTO MPAC AND SAVE */
    dxch(MPAC_AT(0u)); /* X RESULT IN MPAC +5." */
    dxch(MPAC_AT(5u));
    tc_at(DVXSC + 11u);
    dmpsub();
    tc_at(DVXSC + 12u);
    vround();

    set_A(TWO);
    ads(ADDRESS_OF(ADDRWD)); /* "TO Z COMPONENT." */
    dxch(MPAC_AT(3u));       /* "BRING SCALAR BACK, PUTTING Y RESULT IN */
    dxch(MPAC_AT(0u));       /* THE PROPER PLACE." */
    dxch(MPAC_AT(3u));
    tc_at(DVXSC + 18u);
    dmpsub();
    tc_at(DVXSC + 19u);
    vround();

    dxch(MPAC_AT(0u)); /* "PUT Z COMPONENT IN PROPER PLACE, ALSO */
    dxch(MPAC_AT(5u)); /* POSITIONING X." */
    dxch(MPAC_AT(0u));
    vmode(); /* "MODE HAS CHANGED TO VECTOR." */
}

/* ======================================================================
 * VXV
 * ====================================================================== */

/* VXV -- LM p. 1042, CM p. 1147.
 * "VECTOR CROSS PRODUCT ROUTINE CALCULATES (X3 M2 - X2 M3, X1 M3 - X3 M1,
 * X2 M1 - X1 M2) WHERE M IS THE VECTOR IN MPAC AND X THE VECTOR AT THE
 * GIVEN ADDRESS." Six products, each of an M component (or its negative)
 * moved into MPAC by a component of X; the partial answers wait in VBUF
 * and MPAC +3, +5. */
void vxv(void)
{
    dca(MPAC_AT(5u)); /* "FORM UP M3X1, LEAVING M1 IN VBUF." */
    dxch(MPAC_AT(0u));
    dxch(VBUF_AT(0u));
    tc_at(VXV + 4u);
    dmpsub(); /* "BY X1." */

    dcs(MPAC_AT(3u)); /* "CALCULATE -X1M2, SAVING X1M3 IN VBUF +2." */
    dxch(MPAC_AT(0u));
    dxch(VBUF_AT(2u));
    tc_at(VXV + 9u);
    dmpsub();

    set_A(TWO); /* "ADVANCE ADDRWD TO X2." */
    ads(ADDRESS_OF(ADDRWD));
    dcs(MPAC_AT(5u));  /* "PREPARE TO GET -X2M3, SAVING -X1M2 IN */
    dxch(MPAC_AT(0u)); /* MPAC +5." */
    dxch(MPAC_AT(5u));
    tc_at(VXV + 16u);
    dmpsub();

    dca(VBUF_AT(0u)); /* "GET X2M1, SAVING -X2M3 IN VBUF +4." */
    dxch(MPAC_AT(0u));
    dxch(VBUF_AT(4u));
    tc_at(VXV + 21u);
    dmpsub();

    set_A(TWO); /* "ADVANCE ADDRWD TO X3." */
    ads(ADDRESS_OF(ADDRWD));
    dcs(VBUF_AT(0u));  /* "GET -X3M1, ADDING X2M1 TO MPAC +5 TO */
    dxch(MPAC_AT(0u)); /* COMPLETE THE Z COMPONENT OF THE ANSWER." */
    das(MPAC_AT(5u));
    if (!A_is_zero()) {
        tc_at(VXV + 30u);
        overflwz();
    }

    tc_at(VXV + 31u);
    dmpsub();
    dxch(VBUF_AT(2u)); /* "MOVE X1M3 TO MPAC +3 SETTING UP FOR X3M2 */
    dxch(MPAC_AT(3u)); /* AND ADD -X3M1 TO MPAC +3 TO COMPLETE THE */
    dxch(MPAC_AT(0u)); /* Y COMPONENT OF THE RESULT." */
    das(MPAC_AT(3u));
    if (!A_is_zero()) {
        tc_at(VXV + 38u);
        overflwy();
    }

    tc_at(VXV + 39u);
    dmpsub();
    dxch(VBUF_AT(4u)); /* "GO ADD -X2M3 TO X3M2 TO COMPLETE THE X */
    endvxv();          /* COMPONENT (TAIL END OF DAD)." */
}

/* ======================================================================
 * MPACVBUF, ALSIGNAG
 * ====================================================================== */

/* MPACVBUF -- LM p. 1043, CM p. 1148.
 * "SAVES THE VECTOR IN MPAC IN VBUF WITHOUT CLOBBERING MPAC." "CALLED BY
 * MXV, VXM, AND UNIT." */
void mpacvbuf(void)
{
    dca(MPAC_AT(0u));
    dxch(VBUF_AT(0u));
    dca(MPAC_AT(3u));
    dxch(VBUF_AT(2u));
    dca(MPAC_AT(5u));
    dxch(VBUF_AT(4u));
    tc_through(Q_ADDRESS); /* "RETURN TO CALLER." */
}

/* ALSIGNAG -- LM p. 1043, CM p. 1148.
 * "DOUBLE PRECISION SIGN AGREE ROUTINE. ARRIVE WITH INPUT IN A+L. OUTPUT
 * IS IN A + L." When the two words' signs differ, one unit is borrowed
 * from the upper word into the lower (an upper word of zero needs
 * nothing). */
void alsignag(void)
{
    switch (ccs_at(A_ADDRESS)) { /* "TEST UPPER PART." */
    case POSITIVE: uppos(); break;
    case NEGATIVE: upneg(); break;
    default: tc_through(Q_ADDRESS); break; /* "ZERO" */
    }
}

/* UPPOS: the lower word plus one (HALF + HALF) overflows unless it was
 * negative; then the upper keeps its unit back. */
static void uppos(void)
{
    xch(L_ADDRESS); /* "SAVE DECREMENTED UPPER PART." */
    ad(HALF);
    ad(HALF);
    if (ts(A_ADDRESS)) incr(L_ADDRESS); /* "RESTORE UPPER TO ORIGINAL VALUE" */
    xch(L_ADDRESS);                     /* "SWAP A + L BANCK." */
    tc_through(Q_ADDRESS);
}

static void upneg(void)
{
    xch(L_ADDRESS); /* "SAVE COMPLEMENTED + DECREMENTED UPPER PT" */
    ad(NEGMAX);
    ad(NEGONE);
    if (ts(A_ADDRESS)) incr(L_ADDRESS); /* (no increment if no overflow) */
    xch(L_ADDRESS);
    com(); /* "MAKE NEGATIVE AGAIN." */
    tc_through(Q_ADDRESS);
}

/* ======================================================================
 * Orders that mostly call subroutines
 * ====================================================================== */

/* DMP1, DMPR -- LM p. 1044, CM p. 1149. "INTERPRETIVE INSTRUCTIONS WHOSE
 * EXECUTION CONSISTS OF PRINCIPALLY CALLING SUBROUTINES." DMP and DMPR
 * (rounded: "C(A) = +0" after DMPSUB, the mode to give ROUNDSUB +1). */
void dmp1(void)
{
    tc_at(DMP1);
    dmpsub();
    continue_at(DANZIG);
}

void dmpr(void)
{
    tc_at(DMPR);
    dmpsub();
    tc_at(DMPR + 1u);
    roundsub_plus1();
    continue_at(DANZIG);
}

/* DDV, BDDV -- LM p. 1044, CM p. 1149.
 * Divide MPAC by X, or X by MPAC: the dividend goes into BUF and the
 * divisor into MPAC, and the divide routines in bank 0 do the rest. */
void ddv(void)
{
    dca(indexed(0u, ADDRWD)); /* "MOVE DIVIDEND INTO BUF." */
    bddv_plus4();
}

void bddv(void)
{
    dca(indexed(0u, ADDRWD)); /* "MOVE DIVISOR INTO MPAC SAVING MPAC, THE */
    dxch(MPAC_AT(0u));        /* DIVIDEND, IN BUF." */
    bddv_plus4();
}

static void bddv_plus4(void)
{
    dxch(ADDRESS_OF(BUF));
    set_A(ZERO); /* "DIVIDE ROUTINES IN BANK 0." */
    ts(FBANK_ADDRESS);
    ddv_bddv();
}

/* SETPD -- LM p. 1044, CM p. 1149.
 * Set the push-down pointer. "MUST SET TO WORK AREA, OR EBANK TROUBLE."
 * The order is in the bank already: "NO FBANK SWITCH REQUIRED." */
void setpd(void)
{
    ca(ADDRESS_OF(ADDRWD));
    ts(ADDRESS_OF(PUSHLOC));
    continue_at(NOIBNKSW);
}

/* TSLC, GSHIFT -- LM p. 1044, CM p. 1149.
 * "SHIFTING ROUTINES LOCATED IN BANK 00." GSHIFT's LOW7 sets bank 0 too
 * (FBANK keeps only the bank bits) and is "USED AS MASK AT GENSHIFT. THIS
 * PROCESSES ANY SHIFT INSTRUCTION (EXCEPT TSLC) WITH AN ADDRESS". */
void tslc(void)
{
    set_A(ZERO);
    ts(FBANK_ADDRESS);
    tslc2();
}

void gshift(void)
{
    set_A(LOW7);
    ts(FBANK_ADDRESS);
    genshift();
}

/* ======================================================================
 * V/SC
 * ====================================================================== */

/* V/SC -- LM p. 1045, CM p. 1150.
 * "THE FOLLOWING IS THE PROLOGUE TO V/SC. IF THE PRESENT MODE IS VECTOR,
 * IT SAVES THE SCALAR AT X IN BUF AND CALLS THE V/SC ROUTINE IN BANK 0.
 * IF THE PRESENT MODE IS SCALAR, IT MOVES THE VECTOR AT X INTO MPAC,
 * SAVING THE SCALAR IN MPAC IN BUF BEFORE CALLING THE V/SC ROUTINE IN BANK
 * 0." (A MODE of -0 would skip VV/SC's EXTEND, and the DCA 0 would run as
 * CA 1: the word after the scalar.) */
void v_sc(void)
{
    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: dv_sc(); break;
    case NEGATIVE: vv_sc(); break;
    case IS_MINUS_ZERO: vv_sc_plus1(); break;
    }
}

static void vv_sc(void)
{
    dca(indexed(0u, ADDRWD));
    v_sc1();
}

static void vv_sc_plus1(void)
{
    ca(indexed(1u, ADDRWD));
    v_sc1();
}

/* V/SC1: "IN BOTH CASES, VECTOR IS NOW IN MPAC AND SCALAR IN BUF." */
static void v_sc1(void)
{
    dxch(ADDRESS_OF(BUF));
    set_A(ZERO);
    ts(FBANK_ADDRESS);
    v_sc2();
}

/* DV/SC: the vector at X into MPAC, the scalar there into BUF. */
static void dv_sc(void)
{
    dca(indexed(2u, ADDRWD));
    dxch(MPAC_AT(3u));
    dca(indexed(4u, ADDRWD));
    dxch(MPAC_AT(5u));

    set_A(negate(ONE)); /* "CHANGE MODE TO VECTOR." */
    ts(ADDRESS_OF(MODE));

    dca(indexed(0u, ADDRWD));
    dxch(MPAC_AT(0u));
    v_sc1(); /* "FINISH PROLOGUE AT COMMON SECTION." */
}

/* ======================================================================
 * SIGN, COMP
 * ====================================================================== */

/* SIGN -- LM p. 1046, CM p. 1151. "SIGN AND COMPLEMENT INSTRUCTIONS."
 * "CALL COMP INSTRUCTION IF WORD AT X IS NEGATIVE NON-ZERO": the sign of
 * the double-precision X is that of its major word, or of its minor word
 * when the major is zero (CCSL). */
void sign(void)
{
    switch (ccs_at(indexed(0u, ADDRWD))) {
    case POSITIVE: continue_at(DANZIG); break;
    case NEGATIVE: comp(); break; /* "DO THE COMPLEMENT." */
    default: ccsl(); break;
    }
}

static void ccsl(void)
{
    if (ccs_at(indexed(1u, ADDRWD)) == NEGATIVE)
        comp();
    else
        continue_at(DANZIG);
}

/* COMP -- LM p. 1046, CM p. 1151.
 * "COMPLEMENT DP MPAC IN EVERY CASE", then "EITHER COMPLEMENT MPAC +3 OR
 * THE REST OF THE VECTOR ACCUMULATOR." (A MODE of -0 would skip the
 * vector's first EXTEND: its DCS MPAC +3 would run as CS MPAC +4.) */
void comp(void)
{
    dcs(MPAC_AT(0u));
    dxch(MPAC_AT(0u));

    switch (ccs_at(ADDRESS_OF(MODE))) {
    case POSITIVE:
    case IS_PLUS_ZERO: dcomp(); return;
    case NEGATIVE: dcs(MPAC_AT(3u)); break; /* "VECTOR COMPLEMENT." */
    case IS_MINUS_ZERO: cs(MPAC_AT(4u)); break;
    }
    dxch(MPAC_AT(3u));
    dcs(MPAC_AT(5u));
    dxch(MPAC_AT(5u));
    continue_at(DANZIG);
}

/* DCOMP: a scalar's third word. */
void dcomp(void)
{
    cs(MPAC_AT(2u));
    ts(MPAC_AT(2u));
    continue_at(DANZIG);
}
