/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 2 (Comanche055 pp. 565-569): P22, the CSM's
 * "ORBITAL NAVIGATION PROGRAM 22" (bank 30).
 *
 * The crew sights a landmark through the optics, and S22 (later in the
 * file) uses the marks to update the CSM's state vector. PROG22 checks the
 * IMU (R02BOTH) and shows +MGA, the angle between the stable member's Y
 * axis and the orbit's normal, with verb 06 noun 45; on proceed, PROG22A
 * clears the landmark's offset and sets the flags for an orbit of the moon
 * or the earth, and asks for the landmark (verb 05 noun 70, S22N7071 and
 * S22DSPPA): its code ABCDE, A known or unknown, DE the landing site or an
 * advanced orbit. A known landmark's latitude, longitude and altitude come
 * from the crew (verb 06 noun 89, P22SUBRB) or from the landing site
 * (S22LSITE); an advanced orbit runs ADVORB. Then R52 points the optics
 * (CALLR52), and after the marks (verb 05 noun 71) S22.1 works out the
 * orbit; P22OVER starts again at PROG22A, "POINT A ON GSOP".
 *
 * Each call out of the file (BANKCALL, 2PHSCHNG, PHASCHNG, UPFLAG,
 * DOWNFLAG, FALTON, BLANKET) leaves Z at the routine and Q at the word
 * after the TC, where the routine comes back to; ENDOFJOB and GOTOPOOH
 * don't come back. A display through GOFLASHR or GOFLASH comes back to the
 * word after its CADR on terminate, the next on proceed, the next on enter
 * (recycle) and, for GOFLASHR, the next on its immediate return. */
#include "p20_p25_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void prog22_display(void);
static void prog22a(void);
static void p22subrb_display(void);
static void s22dsppa_display(void);
static void s22dsppa_check(void);
static void s22dspp(void);
static void de_gr_50(void);

/* ---- PROG22 ---- */

/* PROG22 -- p. 565. "RESET RNDVZFLG". */
void prog22(void)
{
    tc_to(PROG22, DOWNFLAG); /* ADRES RNDVZFLG */
}

/* PROG22 +2: "IMU STATUS CHECK". */
void prog22_after_rndvzflg(void)
{
    tc_to(PROG22 + 2u, BANKCALL); /* CADR R02BOTH */
}

/* PROG22 +4: "COMPUTE ANGLE BETWEEN Y AND VXR SM". */
void prog22_after_r02both(void)
{
    tc_at(PROG22 + 4u);
    intpret();
}

/* PROG22 +5: the CSM's state vector "INTEGRATE TO PRESENT TIME" by its
 * conic (CSMCONIC, to TDEC1 the time now), then the unit "CROSS PRODUCT
 * BETWEEN V AND R" dotted with the stable member's Y axis (REFSMMAT +6):
 * +MGA is the angle between them, ARCCOS of twice its magnitude (SL1).
 * RENDWFLG cleared: the W matrix is not valid. */
static const word prog22_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STCALL(TDEC1),
    ADDRESS(CSMCONIC),  /* "INTEGRATE TO PRESENT TIME" */
    ORDERS(VLOAD, VXV), /* "CROSS PRODUCT BETWEEN V AND R" */
    ADDRESS_PLUS_ONE(VATT),
    ADDRESS_PLUS_ONE(RATT),
    ORDERS(UNIT, DOT),
    ADDRESS_PLUS_ONE_AT(REFSMMAT, 6),
    ORDER(ABS),
    ORDERS(SL1, ARCCOS),
    STORE(PMGA), /* +MGA */
    ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, RENDWFLG),
};

/* PROG22 +023 -- after the EXIT: verb 06 noun 45 (V06N45B) flashed by
 * GOFLASHR, +MGA in R3. */
void prog22_after_exit(void)
{
    prog22_display();
}

static void prog22_display(void)
{
    set_A(V06N45B);
    tc_to(PROG22 + 024u, BANKCALL); /* CADR GOFLASHR */
}

/* PROG22 +026: "TERM P22". */
void prog22_terminate(void)
{
    tc_to(PROG22 + 026u, GOTOPOOH);
}

/* PROG22 +027: "PROC". */
void prog22_proceed(void)
{
    tc_at(PROG22 + 027u);
    prog22a();
}

/* PROG22 +030: "ENTER": TC -5, the display again. */
void prog22_enter(void)
{
    tc_at(PROG22 + 030u);
    prog22_display();
}

/* PROG22 +031: the display's immediate return: "BLANK OUT R1 + R2"
 * (BLANKET's bits 1 and 2), */
void prog22_immediate_return(void)
{
    set_A(THREE);
    tc_to(PROG22 + 032u, BLANKET);
}

/* PROG22 +033: and the job ends. */
void prog22_after_blanket(void)
{
    tc_to(PROG22 + 033u, ENDOFJOB);
}

/* PROG22A -- p. 565. LANDMARK's offset bits (7-9) cleared: "RESULT=70777
 * SET OFFSET NO.=0". (A MASK leaves no overflow for the TS to skip on.) */
static void prog22a(void)
{
    set_A(negate(PRIO7)); /* CS PRIO7 */
    mask_at(ADDRESS_OF(LANDMARK));
    if (ts(ADDRESS_OF(LANDMARK))) not_modelled();
    tc_at(PROG22A + 3u);
    intpret();
}

/* PROG22A +4: P22MKFLG cleared; ERADFLAG set; with CMOONFLG (the CSM's
 * state vector about the moon) LUNAFLAG set, and the request for the
 * landmark: S22N7071 with MPAC verb 05 noun 70 (V05N7022, "MPAC+1=NONZERO
 * FOR N70"). About the earth (PROG22B), LUNAFLAG and KNOWNFLG cleared and
 * set, and P22SUBRB to "GET LAT/LONG/ALT FROM ASTRO". Then CALLR52. */
static const word prog22a_words[] = {
    ORDER(CLEAR),
    SWITCH(CLEAR, P22MKFLG),
    ORDERS(SET, BOFF),
    SWITCH(SET, ERADFLAG),
    SWITCH(BOF, CMOONFLG),
    ADDRESS(PROG22B), /* "EARTH" */
    ORDER(SET),       /* "MOON" */
    SWITCH(SET, LUNAFLAG),
    ORDERS(DLOAD, CALL), /* "MPAC=V05N70,MPAC+1=NONZERO FOR N70" */
    ADDRESS_PLUS_ONE(V05N7022),
    ADDRESS(S22N7071),
    ORDER(GOTO),
    ADDRESS(CALLR52),
    /* PROG22B */ ORDERS(CLEAR, SET), /* "EARTH ORBIT" */
    SWITCH(CLEAR, LUNAFLAG),
    SWITCH(SET, KNOWNFLG),
    ORDER(CALL), /* "GET LAT/LONG/ALT FROM ASTRO" */
    ADDRESS(P22SUBRB),
    /* CALLR52 */ ORDER(EXIT),
};

/* CALLR52 +1 -- p. 566, after the EXIT: 2PHSCHNG. OCT 00004 makes restart
 * group 4 inactive, and OCT 05022 restarts group 2 at CALLR52 +5, the
 * word after the phase words, as a job of priority 13 (OCT 13000). */
void callr52_after_exit(void)
{
    tc_to(CALLR52 + 1u, TWO_PHSCHNG);
}

/* CALLR52 +5: "SET MARK INDEX=5 FOR R52"; "SET FOR SIGHTING LMK". (CAF
 * FIVE leaves no overflow for the TS to skip on.) */
void callr52_after_2phschng(void)
{
    set_A(FIVE);
    if (ts(ADDRESS_OF(MARKINDX))) not_modelled();
    tc_to(CALLR52 + 7u, UPFLAG); /* ADRES TARG2FLG */
}

/* CALLR52 +011: "CLEAR FOR NON-LEM". */
void callr52_after_targ2flg(void)
{
    tc_to(CALLR52 + 011u, DOWNFLAG); /* ADRES TARG1FLG */
}

/* CALLR52 +013: R52 and the marks, in interpretive code. */
void callr52_after_targ1flg(void)
{
    tc_at(CALLR52 + 013u);
    intpret();
}

/* CALLR52 +014: R52, the automatic optics positioning; then DOV5N71's
 * request for the marks, S22N7071 with verb 05 noun 71 (V05N7122,
 * "MPAC=V05N71,MPAC+1=0 FOR N71"). PROG22C: X2 the complement of MARKSTAT
 * (the mark data's address), and SLOAD* QPRET,2 loads the word at the
 * mark data's address plus QPRET's; S22.1, "ESTABLISH LANDMARK -- COMPUTE
 * ORBITAL" from the marks, with 8NN the word loaded. */
static const word callr52_words[] = {
    ORDER(CALL),
    ADDRESS(R52),
    /* DOV5N71 */ ORDERS(SLOAD, CALL), /* "MPAC=V05N71,MPAC+1=0 FOR N71" */
    ADDRESS_PLUS_ONE(V05N7122),
    ADDRESS(S22N7071),
    /* PROG22C */ ORDERS(LXC_2, SLOAD_STAR),
    ADDRESS(MARKSTAT),
    ADDRESS_PLUS_ONE_AT_X2(QPRET, 0),
    STCALL(8NN),
    ADDRESS(S22_1), /* "ESTABLISH LANDMARK -- COMPUTE ORBITAL" */
    /* P22OVER */ ORDER(EXIT),
};

/* P22OVER +1 -- after the EXIT: PHASCHNG, OCT 04022 (group 2's restart
 * at P22OVER +3, the word after the phase word), */
void p22over_after_exit(void)
{
    tc_to(P22OVER + 1u, PHASCHNG);
}

/* P22OVER +3: and PROG22A again, "POINT A ON GSOP". */
void p22over_after_phaschng(void)
{
    tc_at(P22OVER + 3u);
    prog22a();
}

/* ---- The landmark's latitude, longitude and altitude ---- */

/* S22LSITE -- p. 566. "CONVERT RLS FROM MOON-FIXED TO BASIC REF": the
 * landing site RLS ("0-5D= LANDING SITE VECTOR") at the time now ("6-7D=
 * TIME") by RP-TO-R, with MPAC nonzero ("ANY NON-ZERO FOR MOON"); scaled
 * "B-29 FOR LAT-LONG" to ALPHAV, and LAT-LONG at the time now gives its
 * latitude, longitude and altitude; back through S22TOFF. OBTAINLL:
 * P22SUBRB, then back through S22TOFF. P22SUBRB (p. 567) keeps its return
 * in S22TOFF +1 and leaves the interpreter for the display. */
static const word s22lsite_words[] = {
    ORDER(RTB),
    ADDRESS(LOADTIME),
    STOVL_D(6),
    ADDRESS_PLUS_ONE(RLS),
    STODL_D(0),
    ADDRESS_PLUS_ONE(HIDPHALF), /* "MPAC= ANY NON-ZERO FOR MOON" */
    ORDER(CALL),
    ADDRESS(RP_TO_R), /* "RLS IN BASIC REF B-27 IN MPAC" */
    ORDER(VSR2),
    STORE(ALPHAV), /* "SCALE RLS B-29 FOR LAT-LONG" */
    ORDER(RTB),
    ADDRESS(LOADTIME), /* "SET PRESENT TIME IN MPAC FOR LAT-LONG" */
    ORDER(CALL),
    ADDRESS(LAT_LONG),
    ORDER(GOTO),
    ADDRESS(S22TOFF),           /* "EXIT" */
    /* OBTAINLL */ ORDER(CALL), /* "GET LAT/LONG/ALT FROM ASTRO" */
    ADDRESS(P22SUBRB),
    ORDER(GOTO),
    ADDRESS(S22TOFF),                 /* "EXIT" */
    /* P22SUBRB */ ORDERS(STQ, EXIT), /* "GET LAT/LONG/ALT FROM ASTRO" */
    ADDRESS_AT(S22TOFF, 1),
};

/* P22SUBRB +2 -- after the EXIT: verb 06 noun 89 (V06N89B) flashed by
 * GOFLASH: the landmark's latitude, longitude and altitude. */
void p22subrb_after_exit(void)
{
    p22subrb_display();
}

static void p22subrb_display(void)
{
    set_A(V06N89B);
    tc_to(P22SUBRB + 3u, BANKCALL); /* CADR GOFLASH */
}

/* P22SUBRB +5: "TERMINATE". */
void p22subrb_terminate(void)
{
    tc_to(P22SUBRB + 5u, GOTOPOOH);
}

/* P22SUBRB +6: "PROCEED": TC +2, the check. */
void p22subrb_proceed(void)
{
    tc_at(P22SUBRB + 6u);
    p22subrb_check();
}

/* P22SUBRB +7: "ENTER OR RECYCLE": TC -5, the display again. */
void p22subrb_enter(void)
{
    tc_at(P22SUBRB + 7u);
    p22subrb_display();
}

/* P22SUBRB +010: the check, in interpretive code. */
void p22subrb_check(void)
{
    tc_at(P22SUBRB + 010u);
    intpret();
}

/* P22SUBRB +011: "TEST LAT/LONG GREATER THAN 90": "1/4 REV - LAT"
 * (DP1/4TH less LANDLAT's magnitude) negative goes to N89ERRX, as does
 * LANDLONG's; within 90 degrees (BPL +4) it goes on at N89ERRX +3. */
static const word p22subrb_check_words[] = {
    /* "TEST LAT/LONG GREATER THAN 90" */
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(LANDLAT),
    /* "1/4 REV - LAT" */
    ORDERS(BDSU, BMN),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS(N89ERRX),
    ORDERS(DLOAD, ABS),
    ADDRESS_PLUS_ONE(LANDLONG),
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE(DP1_4TH),
    ADDRESS_AT(N89ERRX, 3), /* +4 */
    /* N89ERRX */ ORDER(EXIT),
};

/* N89ERRX +3: both within 90 degrees, LLASRDA, and back through S22TOFF
 * +1. S22N7071 (p. 567): MPAC to 8KK ("8KK=V05N71,V05N70 8KK+1=0 N71, NOT
 * 0 N70"), the return to S22TOFF, and out of the interpreter to the display
 * (S22DSPPA). */
static const word n89errx_3_words[] = {
    ORDER(CALL),
    ADDRESS(LLASRDA),
    ORDER(GOTO),
    ADDRESS_AT(S22TOFF, 1), /* "EXIT" */
    /* S22N7071 */ STORE(8KK),
    ORDERS(STQ, EXIT),
    ADDRESS(S22TOFF),
};

/* N89ERRX +1 -- after the EXIT: the operator error lamp (FALTON), */
void n89errx_after_exit(void)
{
    tc_to(N89ERRX + 1u, FALTON);
}

/* N89ERRX +2: and "LONG GR. THAN 90  REDISPLAY". */
void n89errx_after_falton(void)
{
    tc_at(N89ERRX + 2u);
    p22subrb_display();
}

/* ---- The landmark's code ---- */

/* S22DSPPA -- p. 567. "V05N70 OR V05N71" (8KK) flashed by GOFLASHR: the
 * landmark's code ABCDE in R1. */
void s22dsppa(void)
{
    s22dsppa_display();
}

static void s22dsppa_display(void)
{
    ca(ADDRESS_OF(8KK));
    tc_to(S22DSPPA + 1u, BANKCALL); /* CADR GOFLASHR */
}

/* S22DSPPA +3: "V34E TERMINATE". */
void s22dsppa_terminate(void)
{
    tc_to(S22DSPPA + 3u, GOTOPOOH);
}

/* S22DSPPA +4: "V33E PROCEED": TC +5, the code's check. */
void s22dsppa_proceed(void)
{
    tc_at(S22DSPPA + 4u);
    s22dsppa_check();
}

/* S22DSPPA +5: "V32E RECYCLE": TC -5, the display again. */
void s22dsppa_recycle(void)
{
    tc_at(S22DSPPA + 5u);
    s22dsppa_display();
}

/* S22DSPPA +6: "IMMEDIATE RETURN BLANK OUT R1,R3" (BLANKET's bits 1 and
 * 3), */
void s22dsppa_immediate_return(void)
{
    set_A(FIVE);
    tc_to(S22DSPPA + 7u, BLANKET);
}

/* S22DSPPA +010 -- p. 568: and the job ends. */
void s22dsppa_after_blanket(void)
{
    tc_to(S22DSPPA + 010u, ENDOFJOB);
}

/* S22DSPPA +011: the offset (LANDMARK's bits 7-9, "08000 = OFFSET
 * INDICATOR") to CXOFF: "OFF=0 THRU 5 OK"; "OFF=6.7 ILLEGAL REDISPLAY".
 * (A MASK leaves no overflow for the TS to skip on.) */
static void s22dsppa_check(void)
{
    ca(ADDRESS_OF(LANDMARK));
    mask(PRIO7); /* "07000" */
    if (ts(ADDRESS_OF(CXOFF))) not_modelled();
    set_A(negate(PRIO5)); /* CS PRIO5 */
    ad_at(ADDRESS_OF(CXOFF));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2: "OFF=0 THRU 5 OK" */
        s22dsppa_known();
        return;
    }
    tc_at(S22DSPPA + 020u); /* TC S22DSPP: "OFF=6.7 ILLEGAL REDISPLAY" */
    s22dspp();
}

/* S22DSPPA +021: "CLEAR KNOWNFLG". */
void s22dsppa_known(void)
{
    tc_to(S22DSPPA + 021u, DOWNFLAG); /* ADRES KNOWNFLG */
}

/* S22DSPPA +023: A of ABCDE (LANDMARK's bits 13-15): "A=0,4,5,6,7
 * ILLEGAL REDISPLAY". Then CCS on A less 2 (22SUBSCL less BIT14): "+ A=3
 * ILLEGAL REDISPLAY", "- A=1 KNOWN LMK  CHECK DE", "-0 A=2 UNKNOWN LMK,
 * DE MEANINGLESS". (A MASK leaves no overflow for the TS to skip on, nor
 * +0 for the CCS.) */
void s22dsppa_after_knownflg(void)
{
    ca(ADDRESS_OF(LANDMARK)); /* "MASK A FROM ABCDE" */
    mask(BITS13_14_15);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF S22DSPP */
        s22dspp();
        return;
    }
    if (ts(ADDRESS_OF(22SUBSCL))) not_modelled(); /* "TEMP" */
    set_A(negate(BIT14));                         /* CS BIT14 */
    ad_at(ADDRESS_OF(22SUBSCL));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: s22dspp(); break; /* "+ A=3 ILLEGAL REDISPLAY" */
    case IS_PLUS_ZERO:               /* (never: A=2 less BIT14 is -0, not +0) */
        tc_at(S22DSPP + 1u);         /* TC S22DSPPA */
        s22dsppa_display();
        break;
    case NEGATIVE: /* "- A=1 KNOWN LMK  CHECK DE": TC +2 */
        tc_at(S22DSPP + 2u);
        s22dspp_known_landmark();
        break;
    case IS_MINUS_ZERO: /* "-0 A=2 UNKNOWN LMK, DE MEANINGLESS" */
        tc_at(S22DSPP + 3u);
        s22abde();
        break;
    }
}

/* S22DSPP -- p. 568. The operator error lamp, and the display again:
 * "COMMON ERROR EXIT BACK TO DISPLAY". */
static void s22dspp(void)
{
    tc_to(S22DSPP, FALTON);
}

/* S22DSPP +1: "COMMON ERROR EXIT BACK TO DISPLAY". */
void s22dspp_after_falton(void)
{
    tc_at(S22DSPP + 1u);
    s22dsppa_display();
}

/* S22DSPP +2: the CCS's negative way, TC +2. */
void s22dspp_plus2(void)
{
    tc_at(S22DSPP + 2u);
    s22dspp_known_landmark();
}

/* S22DSPP +3: the CCS's -0 way, TC S22ABDE. */
void s22dspp_plus3(void)
{
    tc_at(S22DSPP + 3u);
    s22abde();
}

/* S22DSPP +4: "SET KNOWNFLG". */
void s22dspp_known_landmark(void)
{
    tc_to(S22DSPP + 4u, UPFLAG); /* ADRES KNOWNFLG */
}

/* S22DSPP +6: DE of ABCDE ("RESULT= 00077" masks it out of LANDMARK, to
 * 22SUBSCL): "DE=0,1 OK FOR BOTH N70,N71". Else a noun 71 request (8KK +1
 * zero) redisplays: "DE MUST= 0 OR 1"; for noun 70, "TEST DE= 50-57 OCTAL
 * FOR ADV. ORBIT" ("D=5 OR DE=50-57, OK FOR N70"), "DE NOT 50-57  ILLEGAL,
 * REDISPLAY". (A MASK leaves no overflow for the TS to skip on.) */
void s22dspp_after_knownflg(void)
{
    set_A(negate(HIGH9)); /* CS HIGH9: "RESULT= 00077" */
    mask_at(ADDRESS_OF(LANDMARK));
    if (ts(ADDRESS_OF(22SUBSCL))) not_modelled(); /* "000DE" */
    set_A(negate(BIT1));                          /* CS BIT1 */
    ad_at(ADDRESS_OF(22SUBSCL));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF S22ABDE: "DE=0,1 OK FOR BOTH N70,N71" */
        s22abde();
        return;
    }
    ca(ADDRESS_OF(8KK) + 1u); /* "=0 FOR N71, NOT =0 FOR N70" */
    if (A_is_zero()) {        /* EXTEND; BZF S22DSPP: "N71 REDISPLAY  DE MUST= 0 OR 1" */
        s22dspp();
        return;
    }
    set_A(BIT5);                   /* "N70 TEST DE= 50-57 OCTAL FOR ADV. ORBIT" */
    ad(OCT50);                     /* "SUM=00070" */
    mask_at(ADDRESS_OF(22SUBSCL)); /* "00D0" */
    com();                         /* CS A */
    ad(OCT50);
    if (A_is_zero()) { /* EXTEND; BZF DE-GR-50: "D=5 OR DE=50-57, OK FOR N70" */
        de_gr_50();
        return;
    }
    tc_at(S22DSPP + 027u); /* TC S22DSPP: "DE NOT 50-57  ILLEGAL, REDISPLAY" */
    s22dspp();
}

/* S22ABDE: on in interpretive code. */
void s22abde(void)
{
    tc_at(S22ABDE);
    intpret();
}

/* S22ABDE +1: an unknown landmark ("UNKNOWN LMK, DE MEANINGLESS") goes
 * back through S22TOFF. A known one: 22SUBSCL, DE, "=0 GET LLA FROM ASTRO,
 * NOT=0 USE RLS": OBTAINLL ("N89") or S22LSITE. */
static const word s22abde_words[] = {
    ORDERS(BOFF, SLOAD),
    /* "UNKNOWN LMK, DE MEANINGLESS" */
    SWITCH(BOF, KNOWNFLG),
    ADDRESS(S22TOFF),
    /* "=0 GET LLA FROM ASTRO, NOT=0 USE RLS" */
    ADDRESS_PLUS_ONE(22SUBSCL),
    ORDERS(BHIZ, GOTO),
    /* "GET LAT/LONG/ALT FROM ASTRO  N89" */
    ADDRESS(OBTAINLL),
    /* "GET LAT/LONG/ALT FROM RLS" */
    ADDRESS(S22LSITE),
};

/* DE-GR-50 -- p. 569. An advanced orbit: 2PHSCHNG as at CALLR52 (OCT
 * 00004, OCT 05022, OCT 13000: group 2 restarts at DE-GR-50 +4, a job of
 * priority 13), */
static void de_gr_50(void)
{
    tc_to(DE_GR_50, TWO_PHSCHNG);
}

/* DE-GR-50 +4: the mark index 5 and the landmark target flags, as at
 * CALLR52. (CA FIVE leaves no overflow for the TS to skip on.) */
void de_gr_50_after_2phschng(void)
{
    set_A(FIVE);
    if (ts(ADDRESS_OF(MARKINDX))) not_modelled();
    tc_to(DE_GR_50 + 6u, UPFLAG); /* ADRES TARG2FLG */
}

/* DE-GR-50 +010. */
void de_gr_50_after_targ2flg(void)
{
    tc_to(DE_GR_50 + 010u, DOWNFLAG); /* ADRES TARG1FLG */
}

/* DE-GR-50 +012: ADVORB, then the marks at DOV5N71. */
void de_gr_50_after_targ1flg(void)
{
    tc_at(DE_GR_50 + 012u);
    intpret();
}

static const word de_gr_50_words[] = {
    ORDER(CALL),
    ADDRESS(ADVORB),
    ORDER(GOTO),
    ADDRESS(DOV5N71),
};

/* ---- The interpretive code ---- */

const struct interpretive_code PROG22_CODE = {030u, PROG22 + 5u, COUNT(prog22_words), prog22_words};
const struct interpretive_code PROG22A_CODE = {030u, PROG22A + 4u, COUNT(prog22a_words),
                                               prog22a_words};
const struct interpretive_code CALLR52_CODE = {030u, CALLR52 + 014u, COUNT(callr52_words),
                                               callr52_words};
const struct interpretive_code S22LSITE_CODE = {030u, S22LSITE, COUNT(s22lsite_words),
                                                s22lsite_words};
const struct interpretive_code P22SUBRB_CHECK_CODE = {
    030u, P22SUBRB + 011u, COUNT(p22subrb_check_words), p22subrb_check_words};
const struct interpretive_code N89ERRX_3_CODE = {030u, N89ERRX + 3u, COUNT(n89errx_3_words),
                                                 n89errx_3_words};
const struct interpretive_code S22ABDE_CODE = {030u, S22ABDE + 1u, COUNT(s22abde_words),
                                               s22abde_words};
const struct interpretive_code DE_GR_50_CODE = {030u, DE_GR_50 + 013u, COUNT(de_gr_50_words),
                                                de_gr_50_words};

#endif
