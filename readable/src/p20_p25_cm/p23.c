/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 15 (Comanche055 pp. 619-622 and 627): the start of
 * P23, cislunar midcourse navigation (bank 31): "DO MIDCOURSE NAVIGATION
 * BY INCORPORATION OF" "STAR/EARTH AND STAR/MOON OPTICAL MEASUREMENTS."
 *
 * P23 leaves rendezvous navigation (RNDVZFLG), sets up restart group 2
 * (2PHSCHNG) and clears its flags and counters. With the IMU aligned
 * (REFSMFLG) it goes to P23.05; otherwise it calibrates the optics (R57),
 * aligns the IMU (R53) and goes to P23.60, the marks (part 16).
 *
 * P23.06 flashes verb 05 noun 70 for the star, landmark and horizon codes.
 * P23.15 checks them: one of the landmark and the horizon codes must be 0,
 * and the other's middle digit (BITS7-9) 1 or 2, the earth or the moon
 * (LUNAFLAG); the star code 0 to 37 (BESTI, 6 times it; the star's vector
 * from the catalog by LOWMEMRY into STARSAV2). With a horizon, its third
 * digit (BITS4-6) 1 or 2, the near or far horizon (NORFHOR); with a
 * landmark, its coordinates by P22SUBRB. A code out of range is an
 * operator error (R23.10: the lamp on, and the display again).
 *
 * P23.30: with no star, the planet's vector instead (LDPLANET); then,
 * unless SAVECFLG is set (on at P23.85, part 16), verb 50 noun 25 asks
 * the crew to maneuver (GOPERF1): terminate, proceed (verb 94's
 * maneuver, V94ENTER: the line of sight from POINTAXS, the vehicle's axis
 * JCAXIS, and R60CSM), or enter (the marks themselves, P23.56: R57 unless
 * done, then the star and horizon marks by R52 with V94FLAG set).
 *
 * Each call out of the file (BANKCALL, 2PHSCHNG, PHASCHNG, UPFLAG,
 * DOWNFLAG, FALTON) leaves Z at the routine and Q at the word after the
 * TC, where the routine comes back to (after their data words, for the
 * phase changes); GOTOPOOH doesn't come back. GOFLASH and GOPERF1 come
 * back to the word after the CADR on terminate, the next on proceed, and
 * the next on enter. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void p23_152(void);
static void p23_16(void);
static void r23_10(void);

/* P23 -- p. 619. */
void p23(void)
{
    tc_to(P23, DOWNFLAG); /* ADRES RNDVZFLG */
}

/* P23 +2: "LEAVE GROUP 4", "ENTER GROUP 2" (OCT 00004, OCT 00012). */
void p23_after_downflag(void)
{
    tc_to(P23 + 2u, TWO_PHSCHNG);
}

/* P23 +5: group 2's priority, PRIO13, in PHSPRDT2 (a CAF leaves no
 * overflow for the TS to skip on), and TC INTPRET, */
void p23_after_2phschng(void)
{
    set_A(PRIO13);
    if (ts(ADDRESS_OF(PHSPRDT2))) not_modelled();
    tc_at(P23 + 7u);
    intpret();
}

/* P23 +010: MARKINDX 1; TARG2FLG ("TARGET FLAG USED R52 AND R53") and
 * TARG1FLG cleared; STARIND and BESTI 0; R57FLAG ("SET = DO NOT REPERFORM
 * R57") and V94FLAG ("SET = ALLOW V94") cleared. */
static const word p23_words[] = {
    ORDERS(SSP, CLEAR),
    ADDRESS_PLUS_ONE(MARKINDX),
    NUMBER(1),
    /* "TARGET FLAG USED R52 AND R53" */
    SWITCH(CLEAR, TARG2FLG),
    ORDERS(CLEAR, SSP),
    SWITCH(CLEAR, TARG1FLG),
    ADDRESS_PLUS_ONE(STARIND),
    NUMBER(0),
    ORDERS(SSP, CLEAR),
    ADDRESS_PLUS_ONE(BESTI),
    NUMBER(0),
    /* "SET = DO NOT REPERFORM R57" */
    SWITCH(CLEAR, R57FLAG),
    ORDERS(CLEAR, EXIT),
    /* "SET = ALLOW V94" */
    SWITCH(CLEAR, V94FLAG),
};

/* P23.00 -- after the EXIT: TC INTPRET, */
void p23_00(void)
{
    tc_at(P23_00);
    intpret();
}

/* P23.00 +1 -- p. 620: REFSMFLG ("SET NOW AS INPUT, NORMALLY EXTERNAL
 * CONT"): "WHEN ALIGNED, PERFORM MEASUREMENT" (P23.05). Else "DO OPTICS
 * CALIBRATION IF IMU NOT ALIGNED." (R57), the IMU aligned by R53, and on
 * at P23.60. P23.05: SAVECFLG ("USED TO SAVE SPACE IN P23.65") cleared,
 * and out of the interpreter to P23.06. */
static const word p23_00_words[] = {
    ORDERS(BON, CALL),
    /* "SET NOW AS INPUT, NORMALLY EXTERNAL CONT" */
    SWITCH(BON, REFSMFLG),
    /* "WHEN ALIGNED, PERFORM MEASUREMENT" */
    ADDRESS(P23_05),
    /* "DO OPTICS CALIBRATION IF IMU NOT ALIGNED." */
    ADDRESS(R57),
    ORDER(CALL),
    ADDRESS(R53),
    ORDER(GOTO),
    ADDRESS(P23_60),
    /* P23.05 */
    ORDERS(CLEAR, EXIT),
    /* "USED TO SAVE SPACE IN P23.65" */
    SWITCH(CLEAR, SAVECFLG),
};

/* P23.06 -- after the EXIT: verb 05 noun 70 flashed by GOFLASH
 * ("IDENTIFICATION:  STAR, HOR  IDENT."). */
void p23_06(void)
{
    set_A(V05N70);
    tc_to(P23_06 + 1u, BANKCALL); /* CADR GOFLASH */
}

/* P23.06 +3: "TERMINATE". */
void p23_06_terminate(void)
{
    tc_to(P23_06 + 3u, GOTOPOOH);
}

/* P23.06 +4: proceed, TC P23.15. */
void p23_06_proceed(void)
{
    tc_at(P23_06 + 4u);
    p23_15();
}

/* P23.06 +5: "REDISPLAY", TC -5. */
void p23_06_redisplay(void)
{
    tc_at(P23_06 + 5u);
    p23_06();
}

/* P23.15: "IF C=2, LUNAFLAG=1.  IF C=1, LUNAFLAG=0": no landmark code,
 * P23.151; a landmark and a horizon code both, R23.10 ("OPERATOR DSKY
 * ERROR"); the landmark's alone, its digit C at P23.152. */
void p23_15(void)
{
    ca(ADDRESS_OF(LANDMARK));
    if (A_is_zero()) { /* EXTEND; BZF P23.151 */
        p23_151();
        return;
    }
    ca(ADDRESS_OF(HORIZON));
    if (A_is_zero()) { /* EXTEND; BZF +2 */
        p23_15_landmark();
        return;
    }
    tc_at(P23_15 + 6u); /* "OPERATOR DSKY ERROR" */
    r23_10();
}

/* P23.15 +7: the landmark's code, TC P23.152. */
void p23_15_landmark(void)
{
    ca(ADDRESS_OF(LANDMARK));
    tc_at(P23_15 + 010u);
    p23_152();
}

/* P23.151: neither code, R23.10; the horizon's, P23.152. */
void p23_151(void)
{
    ca(ADDRESS_OF(HORIZON));
    if (A_is_zero()) { /* EXTEND; BZF R23.10 */
        r23_10();
        return;
    }
    p23_152();
}

/* P23.152: "IS C EQUAL TO 1 OR 2": 1 (0100 less NEG100's 0100), P23.16,
 * LUNAFLAG reset; 2, LUNAFLAG set; anything else, R23.10. */
static void p23_152(void)
{
    mask(BITS7_9);
    ad(NEG100);
    if (A_is_zero()) { /* EXTEND; BZF P23.16 */
        p23_16();
        return;
    }
    ad(NEG100);
    if (A_is_zero()) { /* EXTEND; BZF +2 */
        p23_152_moon();
        return;
    }
    tc_at(P23_152 + 7u);
    r23_10();
}

/* P23.152 +010: the moon. */
void p23_152_moon(void)
{
    tc_to(P23_152 + 010u, UPFLAG); /* ADRES LUNAFLAG */
}

/* P23.152 +012: TCF +3, the star code. */
void p23_152_after_upflag(void)
{
    p23_starcode();
}

/* P23.16: the earth. */
static void p23_16(void)
{
    tc_to(P23_16, DOWNFLAG); /* ADRES LUNAFLAG */
}

/* P23.16 +2: "IS STARCODE GREATER THAN OR" "EQUAL TO 0 AND LESS THAN 37":
 * 0, no star, P23.176; negative, or 37 or more, R23.10; else TC INTPRET. */
void p23_starcode(void)
{
    ca(ADDRESS_OF(STARCODE));
    if (A_is_zero()) { /* EXTEND; BZF P23.176 */
        p23_176();
        return;
    }
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF R23.10 */
        r23_10();
        return;
    }
    ad(NEG37);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        p23_17();
        return;
    }
    tc_at(P23_17 - 2u);
    r23_10();
}

/* P23.17 -1 -- p. 621: TC INTPRET, */
void p23_17(void)
{
    tc_at(P23_17 - 1u);
    intpret();
}

/* P23.17: with a star (STARCODE not 0), BESTI is 6 times its number
 * (SPSIX), and LOWMEMRY ("NEEDED TO RETRIEVE STAR VECTOR FROM LOW") its
 * vector, in STARSAV2 ("STORE FOR R53,P23.  US(IN P23)=STARSAV2").
 * P23.175: out of the interpreter to P23.176. */
static const word p23_17_words[] = {
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(STARCODE),
    ADDRESS(P23_175),
    ORDER(PUSH),
    ORDERS(SLOAD, DMP),
    ADDRESS_PLUS_ONE(SPSIX),
    ORDERS(LXA_1, SXA_1),
    ADDRESS_AT(MPAC, 1),
    /* "BESTI = 6 X STAR NUMBER" */
    ADDRESS(BESTI),
    ORDER(CALL),
    /* "NEEDED TO RETRIEVE STAR VECTOR FROM LOW" */
    ADDRESS(LOWMEMRY),
    /* "STORE FOR R53,P23.  US(IN P23)=STARSAV2" */
    STORE(STARSAV2),
    /* P23.175 */
    ORDER(EXIT),
};

/* P23.176 -- after the EXIT: no horizon code, the landmark (P23.20). The
 * horizon's third digit (BITS4-6): 1 (010 less 010), P23.18, NORFHOR
 * reset; 2, NORFHOR set; anything else, R23.10. */
void p23_176(void)
{
    ca(ADDRESS_OF(HORIZON));
    if (A_is_zero()) { /* EXTEND; BZF P23.20 */
        p23_20();
        return;
    }
    mask(BITS4_TO_6);
    ad(MINUS_OCT10);
    if (A_is_zero()) { /* EXTEND; BZF P23.18 */
        p23_18();
        return;
    }
    ad(MINUS_OCT10);
    if (A_is_zero()) { /* EXTEND; BZF +2 */
        p23_176_far();
        return;
    }
    tc_at(P23_176 + 012u);
    r23_10();
}

/* P23.176 +013: the far horizon. */
void p23_176_far(void)
{
    tc_to(P23_176 + 013u, UPFLAG); /* ADRES NORFHOR */
}

/* P23.176 +015: TC P23.30. */
void p23_176_after_upflag(void)
{
    tc_at(P23_176 + 015u);
    p23_30();
}

/* P23.18: the near horizon. */
void p23_18(void)
{
    tc_to(P23_18, DOWNFLAG); /* ADRES NORFHOR */
}

/* P23.18 +2: TC P23.30. */
void p23_18_after_downflag(void)
{
    tc_at(P23_18 + 2u);
    p23_30();
}

/* P23.20: a landmark: TC INTPRET, */
void p23_20(void)
{
    tc_at(P23_20);
    intpret();
}

/* P23.20 +1: its coordinates (P22SUBRB), and out of the interpreter. */
static const word p23_20_words[] = {
    ORDER(CALL),
    ADDRESS(P22SUBRB),
    ORDER(EXIT),
};

/* P23.30 -- after the EXIT: TC INTPRET, */
void p23_30(void)
{
    tc_at(P23_30);
    intpret();
}

/* P23.30 +1: no star, the planet's vector (LDPLANET). P23.31: SAVECFLG
 * set, P23.85; else out of the interpreter. */
static const word p23_30_words[] = {
    ORDERS(SLOAD, BZE),
    ADDRESS_PLUS_ONE(STARCODE),
    ADDRESS(LDPLANET),
    /* P23.31 */
    ORDERS(BON, EXIT),
    SWITCH(BON, SAVECFLG),
    ADDRESS(P23_85),
};

/* P23.31 +3 -- after the EXIT: verb 50 noun 25 (V50N25P) by GOPERF1
 * ("GOPERF1 BLANKS OUT R2 AND R3"). */
void p23_31_after_exit(void)
{
    set_A(V50N25P);
    tc_to(P23_30 + 010u, BANKCALL); /* CADR GOPERF1 */
}

/* P23.30 +012: terminate. */
void p23_30_terminate(void)
{
    tc_to(P23_30 + 012u, GOTOPOOH);
}

/* P23.30 +013: "PROCEED.  AUTOCONTROL CMC": TC V94ENTER. */
void p23_30_proceed(void)
{
    tc_at(P23_30 + 013u);
    v94enter();
}

/* P23.55: enter: TC INTPRET, */
void p23_55(void)
{
    tc_at(P23_55);
    intpret();
}

/* P23.55 +1: the marks, P23.56. */
static const word p23_55_words[] = {
    ORDER(GOTO),
    ADDRESS(P23_56),
};

/* V94ENTER -- p. 622, "VERB 94 BEGINS HERE": TC INTPRET, */
void v94enter(void)
{
    tc_at(V94ENTER);
    intpret();
}

/* V94ENTER +1: "READ CLOCK" (LOADTIME) into MARKTIME, and POINTAXS
 * ("RETURN LOS IN RCLL AND MPAC"); the line of sight in stable member
 * axes (REFSMMAT, UNIT) in POINTVSM, and the axis to point, JCAXIS, in
 * SCAXIS. */
static const word v94enter_words[] = {
    ORDER(RTB),
    /* "READ CLOCK" */
    ADDRESS(LOADTIME),
    STCALL(MARKTIME),
    /* "RETURN LOS IN RCLL AND MPAC" */
    ADDRESS(POINTAXS),
    ORDERS(MXV, UNIT),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(POINTVSM),
    ADDRESS_PLUS_ONE(JCAXIS),
    STORE(SCAXIS),
    ORDER(EXIT),
};

/* V94ENTER +013 -- after the EXIT: "CLEAR AND GO TO VECPOINT IN R60". */
void v94enter_after_exit(void)
{
    tc_to(V94ENTER + 013u, DOWNFLAG); /* ADRES 3AXISFLG ("BIT 6 FLAG 5") */
}

/* V94ENTER +015: R60ADRS (R60CALL +3) in TEMPFLSH, and a restart point
 * (OCT 00012). (A CAF leaves no overflow for the TS to skip on.) */
void v94enter_after_downflag(void)
{
    set_A(R60ADRS);
    if (ts(ADDRESS_OF(TEMPFLSH))) not_modelled();
    tc_to(V94ENTER + 017u, PHASCHNG);
}

/* R60CALL: the maneuver, R60CSM. */
void r60call(void)
{
    tc_to(R60CALL, BANKCALL); /* CADR R60CSM */
}

/* R60CALL +2: a restart point (OCT 04022). */
void r60call_after_r60csm(void)
{
    tc_to(R60CALL + 2u, PHASCHNG);
}

/* R60CALL +4: TC INTPRET, */
void r60call_after_phaschng(void)
{
    tc_at(R60CALL + 4u);
    intpret();
}

/* R60CALL +5: R57FLAG set, "DO NOT REPERFORM R57" (P23.57). P23.56: R57.
 * P23.57: V94FLAG and R57FLAG set, the marks (R52), both cleared, and
 * P23.60: out of the interpreter (part 16). */
static const word r60call_words[] = {
    ORDER(BON),
    SWITCH(BON, R57FLAG),
    /* "DO NOT REPERFORM R57" */
    ADDRESS(P23_57),
    /* P23.56 */
    ORDER(CALL),
    ADDRESS(R57),
    /* P23.57 */
    ORDERS(SET, SET),
    SWITCH(SET, V94FLAG),
    SWITCH(SET, R57FLAG),
    ORDER(CALL),
    ADDRESS(R52),
    ORDERS(CLEAR, CLEAR),
    SWITCH(CLEAR, V94FLAG),
    SWITCH(CLEAR, R57FLAG),
    /* P23.60 */
    ORDER(EXIT),
};

/* R23.10 -- p. 627: the operator error lamp (FALTON), */
static void r23_10(void)
{
    tc_to(R23_10, FALTON);
}

/* R23.10 +1: and TC P23.06, the display again. */
void r23_10_after_falton(void)
{
    tc_at(R23_10 + 1u);
    p23_06();
}

/* ---- The interpretive code ---- */

const struct interpretive_code P23_CODE = {031u, P23 + 010u, COUNT(p23_words), p23_words};
const struct interpretive_code P23_00_CODE = {031u, P23_00 + 1u, COUNT(p23_00_words), p23_00_words};
const struct interpretive_code P23_17_CODE = {031u, P23_17, COUNT(p23_17_words), p23_17_words};
const struct interpretive_code P23_20_CODE = {031u, P23_20 + 1u, COUNT(p23_20_words), p23_20_words};
const struct interpretive_code P23_30_CODE = {031u, P23_30 + 1u, COUNT(p23_30_words), p23_30_words};
const struct interpretive_code P23_55_CODE = {031u, P23_55 + 1u, COUNT(p23_55_words), p23_55_words};
const struct interpretive_code V94ENTER_CODE = {031u, V94ENTER + 1u, COUNT(v94enter_words),
                                                v94enter_words};
const struct interpretive_code R60CALL_CODE = {031u, R60CALL + 5u, COUNT(r60call_words),
                                               r60call_words};

#endif
