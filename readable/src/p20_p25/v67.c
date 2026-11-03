/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 19 (Luminary099 pp. 608-611): the "W-MATRIX
 * MONITOR", verb 67 (V67CALL, bank 31, and ADDPOS, bank 30).
 *
 * V67 shows the astronaut, in noun 99, how big the W-matrix's errors are:
 * the root sum square of its words in three groups, position, velocity
 * and bias (radar angle) (V67WW). The astronaut can load new values.
 * When any of them changed ("THE SUM OF ALL DIFFERENCES MUST BE ZERO."
 * otherwise), V67FLAG is set, and the new values go in as the W-matrix's
 * initial values (WRENDPOS and WRENDVEL, or WSURFPOS and WSURFVEL on the
 * surface, and WTRUN and WSHAFT). RENDWFLG is cleared, so that the next
 * mark starts the W-matrix again from them.
 *
 * V67WW runs as a CALL from V67CALL. It first waits its turn for the
 * integration (INTSTALL), since marks change the W-matrix; at the end
 * INTWAKE wakes any job waiting for the integration and returns to
 * V67CALL. Its loop goes down W, 6 words (a vector) at a time, 9 times:
 * X1 runs 54, 48, ... 6 (TIX,1, step S1 6), and W +54D,1 is W +54D - X1.
 * The squares of the position group (W to W +53D) are summed into WWPOS,
 * those of the velocity group (W +54D to W +107D) into WWVEL, and those
 * of the bias group (W +108D to W +161D) into WWBIAS. Then come the
 * square roots. On an overflow, all three show DPPOSMAX. The position
 * shows at most FT99999 (30479 meters, 99,996 feet).
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- V67CALL and the display ---- */

/* V67CALL -- p. 608: V67WW, by the interpreter. */
void v67call(void)
{
    tc_at(V67CALL);
    intpret();
}

static const word v67call_words[] = {
    ORDER(CALL),
    ADDRESS(V67WW),
    ORDER(EXIT),
};

/* V06N99DS: V06N99 on the display (GOXDSPF), waiting for the astronaut. */
static void v06n99ds(void)
{
    set_A(V06N99);
    tc_to(V06N99DS + 1u, BANKCALL); /* CADR GOXDSPF */
}

/* V67CALL +4: "SAVE THE PRESENT N99 VALUES FOR" "COMPARISON AFTER THE
 * DISPLAY": WWPOS, WWVEL and WWBIAS to WWBIAS +2 to +7. Then V06N99DS. */
void v67call_after_exit(void)
{
    dca(ADDRESS_OF(WWPOS)); /* EXTEND; DCA WWPOS */
    dxch(ADDRESS_OF(WWBIAS) + 2u);
    dca(ADDRESS_OF(WWVEL)); /* EXTEND; DCA WWVEL */
    dxch(ADDRESS_OF(WWBIAS) + 4u);
    dca(ADDRESS_OF(WWBIAS)); /* EXTEND; DCA WWBIAS */
    dxch(ADDRESS_OF(WWBIAS) + 6u);
    v06n99ds();
}

/* V06N99DS +3: GOXDSPF's first return, terminate: ENDEXT. */
void v06n99ds_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* V06N99DS +5: GOXDSPF's third return (data loaded): display again. */
void v06n99ds_loaded(void)
{
    v06n99ds(); /* TCF V06N99DS */
}

/* V6N99PRO -- p. 608, GOXDSPF's second return, proceed: L sums WWPOS +Q
 * subtracted from its saved copy (WWPOS +6 +Q) for Q 5 down to 0. "THE SUM
 * OF ALL DIFFERENCES MUST BE ZERO." (V06N9933); else V67FLAG is set. (A
 * TS of 5 ... 0 does not skip; Q is never negative, and CCS Q takes Q
 * less one back to TS Q.) */
void v06n99ds_proceed(void)
{
    zl();
    set_A(FIVE);
    for (;;) { /* N99LOOP */
        if (ts(Q_ADDRESS)) not_modelled();
        cs(indexed(ADDRESS_OF(WWPOS), fetch(Q_ADDRESS)));         /* INDEX Q; CS WWPOS */
        ad_at(indexed(ADDRESS_OF(WWPOS) + 6u, fetch(Q_ADDRESS))); /* INDEX Q; AD WWPOS +6 */
        ads(L_ADDRESS);
        enum sign q = ccs_at(Q_ADDRESS); /* "THE SUM OF ALL DIFFERENCES MUST BE ZERO." */
        if (q == POSITIVE) continue;     /* TCF N99LOOP */
        if (q != IS_PLUS_ZERO) not_modelled();
        break;
    }
    lxch(A_ADDRESS);
    if (A_is_zero()) { /* EXTEND; BZF V06N9933 */
        continue_at(V06N9933);
        return;
    }
    tc_to(V6N99PRO + 015u, UPFLAG); /* ADRES V67FLAG */
}

/* V06N9933 -- pp. 608-609: if V67FLAG is clear, nothing changed: EXIT. */
void v06n9933(void)
{
    tc_at(V06N9933);
    intpret();
}

static const word v06n9933_words[] = {
    ORDERS(BON, EXIT),
    SWITCH(BON, V67FLAG),
    ADDRESS_AT(V06N9933, 5), /* +2 */
};

/* V06N9933 +4: nothing changed: ENDEXT. */
void v06n9933_after_exit(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* V06N9933 +5 -- p. 609: the new values (the high words), as the
 * W-matrix's initial ones. The position's error times 32 to 0D, the
 * velocity's to 2D, and the bias's "SHIFT FROM NOUN SCALING (B-5) TO"
 * "INTERNAL SCALING (B+5)" to 4D. On the surface (SURFFLAG), the position
 * and velocity go to WSURFPOS and WSURFVEL (V67SURF); else to WRENDPOS and
 * WRENDVEL. The bias goes to WTRUN and WSHAFT (V67CLRF). RENDWFLG is
 * cleared: the W-matrix starts again. */
static const word v67_new_w_words[] = {
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(WWPOS),
    ORDERS(SL4, SL1),
    STODL_D(0),
    ADDRESS_PLUS_ONE(WWVEL),
    STODL_D(2),
    ADDRESS_PLUS_ONE(WWBIAS),
    ORDER(SL),    /* "SHIFT FROM NOUN SCALING (B-5) TO" */
    SL_COUNT(10), /* "INTERNAL SCALING (B+5)" */
    STORE_D(4),
    ORDERS(BON, LXA_1),
    SWITCH(BON, SURFFLAG),
    ADDRESS(V67SURF),
    ADDRESS_D(0),
    ORDERS(SXA_1, LXA_1),
    ADDRESS(WRENDPOS),
    ADDRESS_D(2),
    ORDERS(SXA_1, GOTO),
    ADDRESS(WRENDVEL),
    ADDRESS(V67CLRF),
    /* V67SURF */ ORDERS(LXA_1, SXA_1),
    ADDRESS_D(0),
    ADDRESS(WSURFPOS),
    ORDERS(LXA_1, SXA_1),
    ADDRESS_D(2),
    ADDRESS(WSURFVEL),
    /* V67CLRF */ ORDERS(LXA_1, SXA_1),
    ADDRESS_D(4),
    ADDRESS(WTRUN),
    ORDER(SXA_1),
    ADDRESS(WSHAFT),
    ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, RENDWFLG),
};

/* V67CLRF +7: ENDEXT. */
void v67clrf_after_exit(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* ---- V67WW and ADDPOS ---- */

/* V67WW -- pp. 609-610: the return to S2 (STQ), the overflow indicator
 * cleared (BOV +1), V67FLAG cleared, and its turn for the integration
 * (INTSTALL). The step S1 6, WWPOS, WWVEL and WWBIAS zeroed, X1 54, and
 * the loop (NXPOSVEL): a vector of W's position group squared (VSQ), on
 * to ADDPOS. */
static const word v67ww_words[] = {
    ORDERS(STQ, BOV),
    ADDRESS(S2),
    ADDRESS_AT(V67WW, 3), /* +1 */
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, V67FLAG),
    ADDRESS(INTSTALL),
    ORDERS(SSP, DLOAD),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE(WWPOS),
    STORE(WWVEL),
    STORE(WWBIAS),
    ORDER(AXT_1),
    NUMBER(54),
    /* NXPOSVEL */ ORDERS(VLOAD_STAR, VSQ),
    ADDRESS_PLUS_ONE_AT_X1(W, 54),
    ORDER(GOTO),
    ADDRESS(ADDPOS),
};

/* ADDPOS -- p. 610: the square added to WWPOS; the velocity group's
 * (W +108D,1) to WWVEL and the bias group's (W +162D,1) to WWBIAS; round
 * again (TIX,1 NXPOSVEL). Then the square roots: WWBIAS's "SHIFT FROM
 * INTERNAL SCALING (B+5) TO" "NOUN SCALING (B-5)", WWVEL's and WWPOS's.
 * On an overflow, DPPOSMAX in all three. V67XXX: WWPOS at most FT99999.
 * The return from S2 back to QPRET, and EXIT. */
static const word addpos_words[] = {
    ORDER(DAD),
    ADDRESS_PLUS_ONE(WWPOS),
    STORE(WWPOS),
    ORDERS(VLOAD_STAR, VSQ),
    ADDRESS_PLUS_ONE_AT_X1(W, 108),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(WWVEL),
    STORE(WWVEL),
    ORDERS(VLOAD_STAR, VSQ),
    ADDRESS_PLUS_ONE_AT_X1(W, 162),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(WWBIAS),
    STORE(WWBIAS),
    ORDERS(TIX_1, SQRT),
    ADDRESS(NXPOSVEL),
    ORDER(SR),    /* "SHIFT FROM INTERNAL SCALING (B+5) TO" */
    SR_COUNT(10), /* "NOUN SCALING (B-5)" */
    STODL(WWBIAS),
    ADDRESS_PLUS_ONE(WWVEL),
    ORDER(SQRT),
    STODL(WWVEL),
    ADDRESS_PLUS_ONE(WWPOS),
    ORDER(SQRT),
    STORE(WWPOS),
    ORDERS(BOV, GOTO),
    ADDRESS_AT(ADDPOS, 033), /* +2 */
    ADDRESS(V67XXX),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STORE(WWPOS),
    STORE(WWVEL),
    STORE(WWBIAS),
    /* V67XXX */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(WWPOS),
    ADDRESS_PLUS_ONE(FT99999),
    ORDERS(BMN, DLOAD),
    ADDRESS_AT(ADDPOS, 047), /* +3 */
    ADDRESS_PLUS_ONE(FT99999),
    STORE(WWPOS),
    ORDERS(LXA_1, SXA_1),
    ADDRESS(S2),
    ADDRESS(QPRET),
    ORDER(EXIT),
};

/* V67XXX +013 -- p. 611: INTWAKE wakes any job waiting for the
 * integration, releases it, and returns by QPRET (RVQ) to V67CALL's EXIT. */
void v67xxx_after_exit(void)
{
    tc_to(ADDPOS + 053u, POSTJUMP); /* CADR INTWAKE */
}

/* ---- The interpretive code ---- */

const struct interpretive_code V67CALL_CODE = {031u, V67CALL + 1u, COUNT(v67call_words),
                                               v67call_words};
const struct interpretive_code V06N9933_CODE = {031u, V06N9933 + 1u, COUNT(v06n9933_words),
                                                v06n9933_words};
const struct interpretive_code V67_NEW_W_CODE = {031u, V06N9933 + 5u, COUNT(v67_new_w_words),
                                                 v67_new_w_words};
const struct interpretive_code V67WW_CODE = {031u, V67WW, COUNT(v67ww_words), v67ww_words};
const struct interpretive_code ADDPOS_CODE = {030u, ADDPOS, COUNT(addpos_words), addpos_words};

#endif
