/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 19 (Comanche055 pp. 632-634): the W-matrix monitor,
 * verb 67 (V67CALL, bank 40).
 *
 * V67 shows the astronaut, in noun 99, how big the W-matrix's errors are:
 * the root sum square of its position part and of its velocity part
 * (V67WW), and an option, R3 (WWOPT), which V67WW sets to zero. The
 * astronaut can load new values. With R3 zero, nothing changes ("IF R3 OF
 * V67 = 0 EXIT"). Otherwise the loaded values (their high words) go in as
 * the initial values of one of the W-matrices: R3 1 the rendezvous one
 * (WRENDPOS and WRENDVEL), 2 the orbital navigation one (WORBPOS and
 * WORBVEL), 3 or more the midcourse one (WMIDPOS and WMIDVEL). ORBWFLAG
 * and RENDWFLG are cleared, so that the next mark starts the W-matrix
 * again from them.
 *
 * V67WW runs as a CALL from V67CALL. It first waits its turn for the
 * integration (INTSTALL), since marks change the W-matrix; at the end
 * INTWAKE wakes any job waiting for the integration and returns to
 * V67CALL. Its loop goes down W, a vector (6 words) at a time, 6 times:
 * X1 runs 36, 30, ... 6 (TIX,1, step S1 6), and W +36D,1 is W +36D - X1.
 * The squares of W to W +35D are summed into WWPOS, those of W +54D to W
 * +89D into WWVEL. Then come the square roots. On an overflow, both show
 * DPPOSMAX. The position shows at most FT99999 (30479 meters, 99,996
 * feet).
 *
 * GOXDSPF comes back to the word after its CADR on terminate, the next on
 * proceed, and the next once data is loaded. POSTJUMP doesn't come back.
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- V67CALL and the display ---- */

/* V67CALL: V67WW, by the interpreter. */
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

/* V06N99DS -- after the EXIT: verb 06 noun 99 on the display (GOXDSPF),
 * waiting for the astronaut. */
void v06n99ds(void)
{
    set_A(V06N99A);
    tc_to(V06N99DS + 1u, BANKCALL); /* CADR GOXDSPF */
}

/* V06N99DS +3: terminate: ENDEXT. */
void v06n99ds_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* V06N9933: TC INTPRET. */
void v06n9933(void)
{
    tc_at(V06N9933);
    intpret();
}

/* V06N99DS +4: proceed: TC V06N9933. */
void v06n99ds_proceed(void)
{
    tc_at(V06N99DS + 4u);
    v06n9933();
}

/* V06N99DS +5: data loaded: TC V06N99DS, the display again. */
void v06n99ds_loaded(void)
{
    tc_at(V06N99DS + 5u);
    v06n99ds();
}

/* V06N9933 +1: "IF R3 OF V67 = 0 EXIT". */
static const word v06n9933_words[] = {
    ORDERS(SLOAD, BHIZ),
    ADDRESS_PLUS_ONE(WWOPT), /* R3 */
    ADDRESS_AT(V06N9933, 6), /* +3 */
    ORDER(GOTO),
    ADDRESS(V6N99INP),
    ORDER(EXIT),
};

/* V06N9933 +7 -- after the EXIT: ENDEXT. */
void v06n9933_after_exit(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* V6N99INP: the loaded values' high words in X1 and X2, and by R3 (less
 * 2: zero, positive or negative) to one W-matrix's initial values. */
static const word v6n99inp_words[] = {
    ORDERS(LXA_1, LXA_2),
    ADDRESS(WWPOS),
    ADDRESS(WWVEL),
    ORDERS(SLOAD, DSU),
    ADDRESS_PLUS_ONE(WWOPT),
    ADDRESS_PLUS_ONE(V67DEC2),
    ORDERS(BHIZ, BPL),
    ADDRESS(V67WORB),
    ADDRESS(V67WMID),
    ORDERS(SXA_1, SXA_2),
    ADDRESS(WRENDPOS),
    ADDRESS(WRENDVEL),
    ORDER(GOTO),
    ADDRESS(V67EXITX),
    /* V67WORB */ ORDERS(SXA_1, SXA_2),
    ADDRESS(WORBPOS),
    ADDRESS(WORBVEL),
    ORDER(GOTO),
    ADDRESS(V67EXITX),
    /* V67WMID */ ORDERS(SXA_1, SXA_2),
    ADDRESS(WMIDPOS),
    ADDRESS(WMIDVEL),
    /* V67EXITX */ ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, ORBWFLAG),
    SWITCH(CLEAR, RENDWFLG),
    ORDER(EXIT),
};

/* V67EXITX +4 -- after the EXIT: ENDEXT. */
void v67exitx_after_exit(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* ---- V67WW ---- */

/* V67WW -- pp. 633-634: the return to S2 (STQ), the overflow indicator
 * cleared (BOV +1), and its turn for the integration (INTSTALL). The step
 * S1 6, WWPOS, WWVEL and WWOPT zeroed, X1 36, and the loop (NXPOSVEL);
 * the square roots; DPPOSMAX on an overflow; V67XXX: WWPOS at most
 * FT99999. The return from S2 back to QPRET, and EXIT. */
static const word v67ww_words[] = {
    ORDERS(STQ, BOV),
    ADDRESS(S2),
    ADDRESS_AT(V67WW, 3), /* +1 */
    ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(SSP, DLOAD),
    ADDRESS_PLUS_ONE(S1),
    NUMBER(6),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STORE(WWPOS),
    STORE(WWVEL),
    STORE(WWOPT),
    ORDER(AXT_1),
    NUMBER(36),
    /* NXPOSVEL */ ORDERS(VLOAD_STAR, VSQ),
    ADDRESS_PLUS_ONE_AT_X1(W, 36),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(WWPOS),
    STORE(WWPOS),
    ORDERS(VLOAD_STAR, VSQ),
    ADDRESS_PLUS_ONE_AT_X1(W, 90),
    ORDER(DAD),
    ADDRESS_PLUS_ONE(WWVEL),
    STORE(WWVEL),
    ORDERS(TIX_1, SQRT),
    ADDRESS(NXPOSVEL),
    STODL(WWVEL),
    ADDRESS_PLUS_ONE(WWPOS),
    ORDER(SQRT),
    STORE(WWPOS),
    ORDERS(BOV, GOTO),
    ADDRESS_AT(V67WW, 041), /* +2 */
    ADDRESS(V67XXX),
    ORDER(DLOAD),
    ADDRESS_PLUS_ONE(DPPOSMAX),
    STORE(WWPOS),
    STORE(WWVEL),
    /* V67XXX */ ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(WWPOS),
    ADDRESS_PLUS_ONE(FT99999),
    ORDERS(BMN, DLOAD),
    ADDRESS_AT(V67XXX, 7), /* +3 */
    ADDRESS_PLUS_ONE(FT99999),
    STORE(WWPOS),
    ORDERS(LXA_1, SXA_1),
    ADDRESS(S2),
    ADDRESS(QPRET),
    ORDER(EXIT),
};

/* V67XXX +013 -- after the EXIT: INTWAKE wakes any job waiting for the
 * integration, releases it, and returns by QPRET (RVQ) to V67CALL's EXIT. */
void v67xxx_after_exit(void)
{
    tc_to(V67XXX + 013u, POSTJUMP); /* CADR INTWAKE */
}

/* ---- The interpretive code ---- */

const struct interpretive_code V67CALL_CODE = {040u, V67CALL + 1u, COUNT(v67call_words),
                                               v67call_words};
const struct interpretive_code V06N9933_CODE = {040u, V06N9933 + 1u, COUNT(v06n9933_words),
                                                v06n9933_words};
const struct interpretive_code V6N99INP_CODE = {040u, V06N9933 + 010u, COUNT(v6n99inp_words),
                                                v6n99inp_words};
const struct interpretive_code V67WW_CODE = {040u, V67WW, COUNT(v67ww_words), v67ww_words};

#endif
