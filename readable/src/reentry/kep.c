/* SPDX-License-Identifier: MIT */
/* REENTRY_CONTROL.agc, pp. 869-871 (Comanche055, bank 25): "START
 * BALLISTIC PHASE" (KEP, KEP2, MM 66) and "START FINAL PHASE" (PREFINAL,
 * PREDICT3, MM 67), the steering to the target along the reference
 * trajectory of the VREFER table. The faithful C is
 * c64-port/src/cm_reentry.c. */
#include "reentry.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- KEP, KEP2, PREFINAL, PREDICT3 -- pp. 869-870 ---- */

/* KEP, the "UPCONTRL ENTRY INTO KEP2.": P66, "DISPLAY TRIM GIMBAL ANGLE
 * VALUES.", and "SET GOTOADDR TO KEPLER PHASE." KEP2, above the
 * atmosphere: "IF Q7F+KDMIN -D NEG, GO TO FINAL PHASE."; else the roll
 * held, "IF D > .05G, KEEP PRESENT ROLL COMMAND.", "IF D < .05G, SET ROLL
 * COMMAND = 0.", and P62.3 for the trim attitude: "CALC DESIRED GIMBAL
 * ANGLES AT PRESENT" "RN, VN TO YIELD TRIM ATTITUDE."
 *
 * PREFINAL: "RESTART PROTECT: RESET GOTOADDR IF CAME" "FROM HUNTEST." and
 * P67; EGSW set, GOTOADDR PREDICT3. PREDICT3: "IF V-VQUIT NEG, STOP
 * STEERING" (STEEROFF); else out of the interpreter, ... */
static const word kep_words[] = {
    /* KEP */ ORDERS(RTB, SSP),
    ADDRESS(P66),               /* "DISPLAY TRIM GIMBAL ANGLE VALUES." */
    ADDRESS_PLUS_ONE(GOTOADDR), /* "SET GOTOADDR TO KEPLER PHASE." */
    ADDRESS(KEP2),
    /* KEP2 */ ORDERS(DLOAD, DSU), /* "IF Q7F+KDMIN -D NEG, GO TO FINAL PHASE." */
    ADDRESS_PLUS_ONE(Q7FKDMIN),    /* "(Q7F + KDMIN)/805" */
    ADDRESS_PLUS_ONE(D),
    ORDERS(BMN, TLOAD),
    ADDRESS(PREFINAL),
    ADDRESS_PLUS_ONE(ROLLC),  /* "= +1 EVER ENTERED." */
    ORDERS(BON, TLOAD),       /* "IF D > .05G, KEEP PRESENT ROLL COMMAND." */
    SWITCH(BON, _05GSW),      /* "IF D < .05G, SET ROLL COMMAND = 0." */
    ADDRESS_AT(KEP2, 10),     /* "+2" */
    ADDRESS_PLUS_ONE(3ZEROS), /* "SET ROLLC & ROLLHOLD =0." */
    /* +2 */ STCALL(ROLLC),   /* "(SP ROLLHOLD FOLLOWS DP ROLLC)" */
    ADDRESS(P62_3),           /* "CALC DESIRED GIMBAL ANGLES AT PRESENT" */
    /* PREFINAL */ ORDERS(SSP, RTB),
    ADDRESS_PLUS_ONE(GOTOADDR), /* "RESTART PROTECT: RESET GOTOADDR IF CAME" */
    ADDRESS(PREFINAL),          /* "FROM HUNTEST." */
    ADDRESS(P67),               /* "DISABLES GRP4.  FINE IF FROM HUNTEST.BUT" */
    ORDERS(SET, SSP),
    SWITCH(SET, EGSW),
    ADDRESS_PLUS_ONE(GOTOADDR),
    ADDRESS(PREDICT3),
    /* PREDICT3 */ ORDERS(DLOAD, DSU), /* "IF V-VQUIT NEG, STOP STEERING" */
    ADDRESS_PLUS_ONE(V),
    ADDRESS_PLUS_ONE(VQUIT),
    ORDERS(BMN, EXIT),
    ADDRESS(STEEROFF),
};

const struct interpretive_code KEP_CODE = {025u, KEP, COUNT(kep_words), kep_words};

/* ---- PREDICT3's basic code -- pp. 870-871 ---- */

/* BACK, down the VREFER table from its end ("HIGHEST VREF AT END OF
 * TABLE."): JJ from 12, one less each time ("DECREMENT JJ, JJ CANNOT BE
 * ZERO"), while "IF VREF-V POS LOOP BACK". The entry found, V less its
 * VREF in TEM1B, "V-VREF IN TEM1B (MUST BE POSITIVE NUM)"; a V of a VREF
 * exactly makes VREF - V -0, and TEM1B +0. The cases a CCS never meets
 * (VREF - V +0, JJ run out) go on as the AGC's would. */
static void back(void)
{
    for (;;) {
        ts(ADDRESS_OF(JJ));
        cs(ADDRESS_OF(V));
        /* "VREF - V, HIGHEST VREF AT END OF TABLE." */
        ad_at(indexed(ADDRESS_OF(VREFER), fetch(ADDRESS_OF(JJ))));
        switch (ccs_register(A_ADDRESS)) { /* "IF VREF-V POS LOOP BACK" */
        case POSITIVE:
            switch (ccs_at(ADDRESS_OF(JJ))) { /* "DECREMENT JJ, JJ CANNOT BE ZERO" */
            case POSITIVE: continue;          /* TCF BACK */
            case IS_PLUS_ZERO:
                ad(ONE);
                ts(ADDRESS_OF(TEM1B));
                return;
            case NEGATIVE: ts(ADDRESS_OF(TEM1B)); return;
            case IS_MINUS_ZERO: return;
            }
            return;
        case IS_PLUS_ZERO: continue; /* TCF BACK, with JJ +0 */
        case NEGATIVE:
            ad(ONE);
            ts(ADDRESS_OF(TEM1B)); /* "V-VREF IN TEM1B (MUST BE POSITIVE NUM)" */
            return;
        case IS_MINUS_ZERO: ts(ADDRESS_OF(TEM1B)); return;
        }
    }
}

/* BACK2: the six other columns of the table at the same place, each
 * "FX = AK + GRAD (AK+1 - AK)", MM from 5 to 0 (FX +5 to FX), JJ 13 on
 * each time. Its CCS MM: on (TCF BACK2) while positive; at +0 on to the
 * end, "ZERO FX +1 AND GET DREFR". (A TS FX that overflows skips the CCS,
 * back to BACK2 with A +1 or -1; a negative MM skips the XCH, a -0 the AD
 * D too: none happens in flight.) The result: how the end goes on. */
static enum sign back2(void)
{
    enum sign mm;

    set_A(FIVE);
    for (;;) {
        ts(ADDRESS_OF(MM));
        set_A(THIRTEEN);
        ads(ADDRESS_OF(JJ));
        cs(indexed(ADDRESS_OF(VREFER), a_word()));
        ad_at(indexed(ADDRESS_OF(VREFER) + 1u, fetch(ADDRESS_OF(JJ)))); /* "X(K+1) - X(K)" */
        mp_at(ADDRESS_OF(GRAD));
        ad_at(indexed(ADDRESS_OF(VREFER), fetch(ADDRESS_OF(JJ))));
        if (ts(indexed(ADDRESS_OF(FX), fetch(ADDRESS_OF(MM)))))
            continue; /* "FX = AK + GRAD (AK+1 - AK)" */
        mm = ccs_at(ADDRESS_OF(MM));
        if (mm != POSITIVE) return mm;
    }
}

/* PREDICT3 +5, after the EXIT: "PRECAUTIONARY." EBANK; BACK, and GRAD,
 * "GRAD = (V-VREF)/(VK+1 - VK)"; BACK2 to the other columns. Then the
 * range the reference trajectory predicts from here: "MPAC = F1(D-DREF)",
 * plus F2 times RDOTREF - RDOT ("SCALE UP BY 8 FOR THIS PHASE."), plus
 * RTOGO: PREDANG. "L/D = LOD + (THETA- PREDANG)/ Y", back in the
 * interpreter. */
void predict3_after_exit(void)
{
    enum sign mm;

    set_A(EBENTRY); /* "PRECAUTIONARY." */
    ts(EBANK_ADDRESS);
    set_A(TWELVE);
    back();
    cs(indexed(ADDRESS_OF(VREFER), fetch(ADDRESS_OF(JJ))));
    ad_at(indexed(ADDRESS_OF(VREFER) + 1u, fetch(ADDRESS_OF(JJ)))); /* "V(K+1) - V(K)" */
    xch(ADDRESS_OF(TEM1B));
    zl();
    dv_at(ADDRESS_OF(TEM1B));
    ts(ADDRESS_OF(GRAD)); /* "GRAD = (V-VREF)/(VK+1 - VK)" */
    mm = back2();
    if (mm == IS_PLUS_ZERO) xch(ADDRESS_OF(FX) + 1u); /* "ZERO FX +1 AND GET DREFR" */
    if (mm != IS_MINUS_ZERO) ad_at(ADDRESS_OF(D));
    mp_at(ADDRESS_OF(FX) + 5u); /* "F1" */
    dxch(ADDRESS_OF(MPAC));     /* "MPAC = F1(D-DREF)" */
    dcs(ADDRESS_OF(RDOT));      /* "FORM RDOTREF - RDOT" */
    ddoubl();
    ddoubl();
    ddoubl();                   /* "SCALE UP BY 8 FOR THIS PHASE." */
    ad_at(ADDRESS_OF(FX) + 3u); /* "RDOTREF" */
    mp_at(ADDRESS_OF(FX) + 4u); /* "F2" */
    ad_at(ADDRESS_OF(FX) + 2u); /* "RTOGO" */
    das(ADDRESS_OF(MPAC));      /* "ADD F2(DADV1-DADVR)" */
    ca(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(PREDANG));
    tc_at(PREDICT3 + 073u);
    intpret();
}

/* ---- PREDICT3, back in the interpreter -- p. 871 ---- */

/* PREDANG less THETAH, the range error; GONEPAST holds it, "SET GONEPAST
 * IF GONEBY SET & LATCH IN-PLACE", "DISPLAY = 9999.9 IF GONEBY" (MAXRNG);
 * on at GONEGLAD, or HAVDNRNG. */
static const word predict3_words[] = {
    /* PREDICT3 +074 */ ORDERS(SR3, DSU),
    ADDRESS_PLUS_ONE(THETAH),
    ORDERS(BON, BOFF),
    SWITCH(BON, GONEPAST),
    ADDRESS(GONEGLAD),
    SWITCH(BOFF, GONEBY),
    ADDRESS(HAVDNRNG),
    ORDERS(DLOAD, SET),       /* "SET GONEPAST IF GONEBY SET & LATCH IN-PLACE" */
    ADDRESS_PLUS_ONE(MAXRNG), /* "DISPLAY = 9999.9 IF GONEBY" */
    SWITCH(SET, GONEPAST),
    STCALL(DNRNGERR),
    ADDRESS(GONEGLAD),
};

const struct interpretive_code PREDICT3_CODE = {025u, PREDICT3 + 074u, COUNT(predict3_words),
                                                predict3_words};

#endif
