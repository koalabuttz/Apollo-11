/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 13 (Luminary099 pp. 582-585): LSR22.4, INITWMX6,
 * INTGRCAL and WLINIT, the lunar surface's measurement incorporation and
 * the W-matrix's start.
 *
 * "LSR22.4 IS THE ENTRY TO PERFORM LUNAR SURFACE NAVIGATION FOR THE LM
 * COMPUTER ONLY". LSR22.3 comes here when SURFFLAG says the LM is on the
 * surface: only the CSM's state vector is corrected, with a 6 by 6
 * W-matrix (DMENFLG clear). The LM's state vector goes to the mark time
 * first (STATEFLG set: its permanent one), then the CSM's: with the
 * W-matrix, or on the first mark (MARKCTR zero) after INITWMX6 has set the
 * W-matrix up (WLINIT), without it. Then the range and the range rate
 * (RANGEBQ, LSR22.3's), but not the angles.
 *
 * INTGRCAL brings a state vector to the mark time (MKTIME) by INTEGRV, the
 * way INTSTALL's flags say. WLINIT "INITIALIZES THE W-MATRIX BY ZEROING
 * ALL W THEN SETTING DIAGONAL ELEMENTS TO INITIAL STORED VALUES": the
 * position's (WRENDPOS, or WSURFPOS on the surface), the velocity's
 * (WRENDVEL or WSURFVEL) and the radar angles' (WSHAFT, WTRUN). RENDWFLG
 * then says the W-matrix is valid, and MARKCTR counts the marks from zero.
 *
 * As in the other parts, a branch or CALL to a place the faithful port
 * keys a routine at ends the pass there. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- LSR22.4, INITWMX6 and INTGRCAL ---- */

/* LSR22.4 -- p. 583: the LM's state vector to the mark time ("CALL TO GET
 * LM POS + VEL IN REF COORD."). "SET MATRIX SIZE TO 6X6 FOR INCORP"
 * (DMENFLG clear); "IS THIS FIRST TIME THROUGH" (MARKCTR zero): "YES,
 * INITIALIZE 6X6 W-MATRIX" (INITWMX6). Else the CSM's state vector with
 * the W-matrix (DIM0FLAG), 6 by 6 (D6OR9FLG clear); RANGEBQ.
 *
 * INITWMX6: "INITIALIZE W-MATRIX" (WLINIT), and the CSM's state vector
 * without it (SETIFLGS); RANGEBQ.
 *
 * INTGRCAL -- p. 584: "THIS ROUTINE CLEARS RFINAL (DP) AND CALLS INTEGRV";
 * this code does not clear RFINAL: it keeps its return in IGRET, puts the
 * mark time (MKTIME) in TDEC1, and calls INTEGRV. */
static const word lsr22_4_words[] = {
    /* LSR22.4 */ ORDER(CALL),
    ADDRESS(INTSTALL),
    ORDERS(SET, CLEAR),
    SWITCH(SET, STATEFLG),
    SWITCH(CLEAR, VINTFLAG), /* "CALL TO GET LM POS + VEL IN REF COORD." */
    ORDER(CALL),
    ADDRESS(INTGRCAL),
    ORDER(CALL),
    ADDRESS(GRP2PC),
    ORDERS(CLEAR, CALL),
    SWITCH(CLEAR, DMENFLG), /* "SET MATRIX SIZE TO 6X6 FOR INCORP" */
    ADDRESS(INTSTALL),
    ORDERS(DLOAD, BHIZ), /* "IS THIS FIRST TIME THROUGH" */
    ADDRESS_PLUS_ONE(MARKCTR),
    ADDRESS(INITWMX6), /* "YES, INITIALIZE 6X6 W-MATRIX" */
    ORDERS(CLEAR, SET),
    SWITCH(CLEAR, D6OR9FLG),
    SWITCH(SET, DIM0FLAG),
    ORDERS(SET, CLEAR),
    SWITCH(SET, VINTFLAG),
    SWITCH(CLEAR, INTYPFLG),
    ORDER(CALL),
    ADDRESS(INTGRCAL),
    ORDER(GOTO),
    ADDRESS(RANGEBQ),
    /* INITWMX6 */ ORDER(CALL),
    ADDRESS(WLINIT), /* "INITIALIZE W-MATRIX" */
    ORDERS(SET, CALL),
    SWITCH(SET, VINTFLAG),
    ADDRESS(SETIFLGS),
    ORDER(CALL),
    ADDRESS(INTGRCAL),
    ORDER(GOTO),
    ADDRESS(RANGEBQ),
    /* INTGRCAL */ ORDERS(STQ, DLOAD),
    ADDRESS(IGRET),
    ADDRESS_PLUS_ONE(MKTIME),
    STCALL(TDEC1),
    ADDRESS(INTEGRV),
    ORDER(GOTO),
    ADDRESS(IGRET),
    /* WLINIT */ ORDER(EXIT),
};

/* ---- WLINIT ---- */

/* WLINIT +1 -- p. 584: E bank 5, W's (WBANK), and every word of W zeroed,
 * from W +WSIZE (161) down to W: W.IND counts down, and CCS W.IND goes
 * round again (TC -5) while it is positive. At +0 the pass ends at WLINIT
 * +012, where the faithful port keys a routine. (A, a constant or the
 * count, does not overflow: no TS skips.) */
void wlinit_zero_w(void)
{
    set_A(WBANK);
    ts(BBANK_ADDRESS);
    set_A(WSIZE);
    for (;;) {
        ts(ADDRESS_OF(W_IND));
        set_A(ZERO);
        ts(indexed(ADDRESS_OF(W), fetch(ADDRESS_OF(W_IND)))); /* INDEX W.IND; TS W */
        if (ccs_at(ADDRESS_OF(W_IND)) != POSITIVE) break;
        tc_at(WLINIT + 011u); /* TC -5 */
    }
    continue_at(WLINIT + 012u);
}

/* WLINIT +012: "RESTORE EBANK 7" (AIGBANK), and the interpreter. */
void wlinit_restore_ebank(void)
{
    set_A(AIGBANK);
    ts(BBANK_ADDRESS);
    tc_at(WLINIT + 014u);
    intpret();
}

/* WLINIT +015 -- pp. 584-585: "IF ON LUNAR SURFACE, INITIALIZE WITH
 * WSURFPOS AND WSURFVEL INSTEAD OF WRENDPOS AND WRENDVEL". The position's
 * (single precision) "SHIFT TO B-19 SCALE" to W, W +8D and W +16D; the
 * velocity's to W +72D, W +80D and W +88D; WSHAFT to W +144D and WTRUN to
 * W +152D. "SET RENDWFLG -- W-MATRIX VALID", "SET MARK COUNTER EQUAL
 * ZERO", and back (RVQ). */
static const word wlinit_words[] = {
    ORDERS(BON, SLOAD), /* "IF ON LUNAR SURFACE, INITIALIZE WITH" */
    SWITCH(BON, SURFFLAG),
    ADDRESS(WLSRFPOS),
    ADDRESS_PLUS_ONE(WRENDPOS),
    ORDER(GOTO),
    ADDRESS(WPOSTORE),
    /* WLSRFPOS */ ORDER(SLOAD),
    ADDRESS_PLUS_ONE(WSURFPOS),
    /* WPOSTORE */ ORDER(SR), /* "SHIFT TO B-19 SCALE" */
    SR_COUNT(5),
    STORE(W),
    STORE_AT(W, 8),
    STORE_AT(W, 16),
    ORDERS(BON, SLOAD),
    SWITCH(BON, SURFFLAG),
    ADDRESS(WLSRFVEL),
    ADDRESS_PLUS_ONE(WRENDVEL),
    ORDER(GOTO),
    ADDRESS(WVELSTOR),
    /* WLSRFVEL */ ORDER(SLOAD),
    ADDRESS_PLUS_ONE(WSURFVEL),
    /* WVELSTOR */ STORE_AT(W, 72),
    STORE_AT(W, 80),
    STORE_AT(W, 88),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(WSHAFT),
    STORE_AT(W, 144),
    ORDER(SLOAD),
    ADDRESS_PLUS_ONE(WTRUN),
    STORE_AT(W, 152),
    ORDERS(SET, SSP), /* "SET RENDWFLG -- W-MATRIX VALID" */
    SWITCH(SET, RENDWFLG),
    ADDRESS_PLUS_ONE(MARKCTR), /* "SET MARK COUNTER EQUAL ZERO" */
    NUMBER(0),
    ORDER(RVQ),
};

/* ---- The interpretive code ---- */

const struct interpretive_code LSR22_4_CODE = {026u, LSR22_4, COUNT(lsr22_4_words), lsr22_4_words};
const struct interpretive_code WLINIT_CODE = {026u, WLINIT + 015u, COUNT(wlinit_words),
                                              wlinit_words};

#endif
