/* SPDX-License-Identifier: MIT */
/* AGS_INITIALIZATION.agc, Luminary099 only (listing pages 206-210): R47,
 * AGS initialization. The faithful C is c64-port/src/lm_ags_init.c.
 *
 * "(1) TO PROVIDE THE AGS ABORT ELECTRONICS ASSEMBLY (AEA) WITH THE LEM
 * AND CSM STATE VECTORS (POSITION,VELOCITY,TIME) IN LEM IMU COORDINATES
 * BY MEANS OF THE LGC DIGITAL DOWNLINK. (2) TO ZERO THE ICDU, LGC, AND AEA
 * GIMBAL ANGLE COUNTER SIMULTANEOUSLY IN ORDER TO ESTABLISH A COMMON ZERO
 * REFERENCE FOR THE MEASUREMENT OF GIMBAL (EULER) ANGLES WHICH DEFINE LEM
 * ATTITUDE (3) TO ESTABLISH THE GROUND ELAPSED TIME OF AEA CLOCK ZERO."
 *
 * The AGS (the abort guidance system, the LM's second computer) counts
 * its own clock from a zero the LGC keeps in AGSK. Verb 47 starts this
 * routine as an extended verb; it ends at ENDEXT.
 *
 * Its basic code runs in bank 32, where the binding
 * (src/binding/ags_initialization_routines.c) runs it; each call out of
 * the file (BANKCALL, ALARM, DOWNFLAG, ENDEXT, INTPRET) ends a function
 * with Q and Z set, and the word the call returns to is an entry of its
 * own. */
#include "ags_initialization.h"
#include "../agc/names.h"

#if LUMINARY099

static void newags(void);
static void redsptem_minus1(void);
static void redsptem(void);
static void agsvcalc(void);
static void ckstall(void);

/* AGSINIT -- LM p. 207.
 * "CHECK REFSMFLG": with REFSMMAT good, display the AGS clock's zero
 * (REDSPTEM); else alarm 220 ("REFSMMAT IS BAD") and end. CCS of one bit
 * of FLAGWRD3 is positive or +0, never negative. */
void agsinit(void)
{
    set_A(REFSMBIT); /* CAF REFSMBIT */
    mask_at(ADDRESS_OF(FLAGWRD3));
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* CCS A */
        tc_at((word)(AGSINIT + 3u));     /* TC REDSPTEM: "REFSMMAT IS OK" */
        redsptem();
        return;
    }
    tc_at((word)(AGSINIT + 4u)); /* TC ALARM; OCT 220 */
    continue_at(ALARM);
}

/* AGSINIT +6, where ALARM returns: TC ENDEXT. */
void agsinit_after_alarm(void)
{
    tc_at((word)(AGSINIT + 6u));
    continue_at(ENDEXT);
}

/* NEWAGS -- LM p. 207.
 * "TIME OF THE :ENTER: KEYSTROKE BECOMES NEW AEA CLOCK :ZERO:", and on
 * into REDSPTEM. */
static void newags(void)
{
    dca(ADDRESS_OF(SAMPTIME)); /* EXTEND; DCA SAMPTIME */
    redsptem_minus1();
}

/* REDSPTEM -1 (NEWAGS's DXCH AGSK): A,L become AGSK. The enter return
 * comes here with the time the crew keyed in. */
static void redsptem_minus1(void)
{
    dxch(ADDRESS_OF(AGSK));
    redsptem();
}

/* REDSPTEM, AGSDISPK -- LM p. 207.
 * Display AGSK (V06N16: "R1 = 00XXX. HRS., R2 = 000XX MIN., R3 = 0XX.XX
 * SEC.") from DSPTEMX, through GOMARKF, which waits for the crew. */
static void redsptem(void)
{
    dca(ADDRESS_OF(AGSK)); /* EXTEND; DCA AGSK */
    dxch(ADDRESS_OF(DSPTEMX));
    set_A(V06N16);                /* AGSDISPK CAF V06N16 */
    tc_at((word)(AGSDISPK + 1u)); /* TC BANKCALL; CADR GOMARKF */
    continue_at(BANKCALL);
}

/* AGSDISPK +3 -- LM p. 207. GOMARKF's terminate return: TC ENDEXT. */
void agsdispk_after_terminate(void)
{
    tc_at((word)(AGSDISPK + 3u));
    continue_at(ENDEXT);
}

/* AGSDISPK +4 -- LM p. 207. The proceed return: TC AGSVCALC. */
void agsdispk_after_proceed(void)
{
    tc_at((word)(AGSDISPK + 4u));
    agsvcalc();
}

/* AGSDISPK +5 -- LM p. 207. The enter return. "IS ENTER VIA A V32"
 * (MPAC holds BIT6, the verb): "YES, USE KEYSTROKE TIME FOR NEW AGSK".
 * "NO, NEW AGSK LOADED VIA V25": "LOADED INTO DSPTEMX BY KEYING V25E
 * FOLLOWED BY HRS.,MINS.,SECS." Either way "DISPLAY THE NEW K". */
void agsdispk_after_enter(void)
{
    set_A(negate(BIT6)); /* CS BIT6 */
    ad_at(ADDRESS_OF(MPAC));
    if (A_is_zero()) { /* EXTEND; BZF NEWAGS */
        newags();
        return;
    }
    dca(ADDRESS_OF(DSPTEMX));       /* EXTEND; DCA DSPTEMX */
    tc_at((word)(AGSDISPK + 013u)); /* TC REDSPTEM -1 */
    redsptem_minus1();
}

/* AGSVCALC -- LM p. 207: TC INTPRET, into AGSVCALC_CODE. */
static void agsvcalc(void)
{
    tc_at(AGSVCALC);
    continue_at(INTPRET);
}

/* AGSVCALC +5 -- LM p. 207, where AGSVCALC_CODE's EXIT leaves: display
 * the time again (V06N16) through EXDSPRET, which does not wait. */
void agsvcalc_after_exit(void)
{
    set_A(V06N16);                /* CAF V06N16 */
    tc_at((word)(AGSVCALC + 6u)); /* TC BANKCALL; CADR EXDSPRET */
    continue_at(BANKCALL);
}

/* AGSVCALC +010 -- LM p. 207, where EXDSPRET returns: TC INTPRET, into
 * AGSVCALC_CODE_2. */
void agsvcalc_after_exdspret(void)
{
    tc_at((word)(AGSVCALC + 010u));
    continue_at(INTPRET);
}

/* AGSVCALC +034 -- LM p. 208, where AGSVCALC_CODE_2's EXIT leaves: send
 * the AGS downlist (LAGSLIST) "FOR 20 SEC WHILE THE AGS DOWNLIST IS
 * TRANSMITTED" (DELAYJOB). */
void agsvcalc_after_second_exit(void)
{
    set_A(LAGSLIST); /* CAF LAGSLIST */
    ts(ADDRESS_OF(DNLSTCOD));
    set_A(TWENTY_SEC);              /* CAF 20SEC */
    tc_at((word)(AGSVCALC + 037u)); /* TC BANKCALL; CADR DELAYJOB */
    continue_at(BANKCALL);
}

/* AGSVCALC +041 -- LM p. 208, where DELAYJOB returns: "RETURN TO THE OLD
 * DOWNLIST". "CHECK IMUSE FLAG": "IMU IS BEING USED -- DO NOT ZERO"
 * (AGSEND); else CKSTALL. (The TS never overflows: AGSWORD is a word.
 * CCS of one bit is positive or +0.) */
void agsvcalc_after_delayjob(void)
{
    ca(ADDRESS_OF(AGSWORD));
    ts(ADDRESS_OF(DNLSTCOD));
    set_A(IMUSEBIT); /* CAF IMUSEBIT */
    mask_at(ADDRESS_OF(FLAGWRD0));
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* CCS A */
        tc_at((word)(AGSVCALC + 046u));  /* TC AGSEND */
        agsend();
        return;
    }
    ckstall();
}

/* CKSTALL -- LM p. 208.
 * "CHECK FOR IMU USAGE WHICH AVOIDS THE IMUSE BIT: I.E., IMU
 * COMPENSATION": IMUCADR +0 is "FREE. GO AHEAD WITH THE IMU ZERO" (+6:
 * IMUZERO, "SET IMU ZERO DISCRETE FOR 320 MSECS."). Anything else: "WAIT
 * .1 SEC AND TRY AGAIN" (+3, DELAYJOB). */
static void ckstall(void)
{
    if (ccs_at(ADDRESS_OF(IMUCADR)) == IS_PLUS_ZERO) { /* CCS IMUCADR; TCF +6 */
        tc_at((word)(CKSTALL + 010u));                 /* TC BANKCALL; CADR IMUZERO */
        continue_at(BANKCALL);
        return;
    }
    /* TCF +3 (positive), TCF +1 (negative, on to +3), or -0, at +3 */
    set_A(TEN);                  /* +3 CAF TEN */
    tc_at((word)(CKSTALL + 5u)); /* TC BANKCALL; CADR DELAYJOB */
    continue_at(BANKCALL);
}

/* CKSTALL +7, where DELAYJOB returns: TCF CKSTALL. */
void ckstall_after_delayjob(void)
{
    ckstall();
}

/* CKSTALL +012, where IMUZERO returns: "WAIT 3 SEC FOR COUNTERS TO
 * INCREMENT" (IMUSTALL). */
void ckstall_after_imuzero(void)
{
    tc_at((word)(CKSTALL + 012u)); /* TC BANKCALL; CADR IMUSTALL */
    continue_at(BANKCALL);
}

/* CKSTALL +014, where IMUSTALL returns: TC AGSEND. */
void ckstall_after_imustall(void)
{
    tc_at((word)(CKSTALL + 014u));
    agsend();
}

/* AGSEND -- LM p. 208: "ALLOW V37" (DOWNFLAG NODOFLAG). */
void agsend(void)
{
    tc_at(AGSEND); /* TC DOWNFLAG; ADRES NODOFLAG */
    continue_at(DOWNFLAG);
}

/* AGSEND +2, where DOWNFLAG returns: display V50N16 through GOMARK3. */
void agsend_after_downflag(void)
{
    set_A(V50N16);              /* CAF V50N16 */
    tc_at((word)(AGSEND + 3u)); /* TC BANKCALL; CADR GOMARK3 */
    continue_at(BANKCALL);
}

/* AGSEND +5, +6 and +7, GOMARK3's terminate, proceed and enter returns:
 * each ends the extended verb. */
void agsend_after_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

void agsend_after_proceed(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

void agsend_after_enter(void)
{
    tc_at((word)(AGSEND + 7u)); /* TC ENDEXT */
    continue_at(ENDEXT);
}

/* ---- The interpretive code ---- */

/* AGSVCALC -- LM p. 207. "DON'T ALLOW V37" (NODOFLAG) and XDSPFLAG (the
 * display EXDSPRET puts up is an extended verb's), and EXIT to the basic
 * code after it. */
static const word agsvcalc_words[] = {
    ORDER(SET), /* AGSVCALC +1 */
    SWITCH(SET, NODOFLAG),
    ORDERS(SET, EXIT),
    SWITCH(SET, XDSPFLAG),
};

/* AGSVCALC +011 -- LM pp. 207-208.
 * "EXTRAPOLATE LEM AND CSM STATE VECTORS TO THE PRESENT TIME" (LOADTIME:
 * "LOAD MPAC WITH TIME2,TIME1"): "CALCULATE LEM STATE VECTOR" (LEMPREC),
 * then SCALEVEC turns it "TO SM COORDS AND PROVIDE[S] PROPER SCALING" into
 * AGSBUFF. "LEMPREC AND CSMPREC LEAVE TDEC1 IN TAT", "TIME TO WHICH RATT1
 * AND VATT1 ARE COMPUTED (CSEC SINCE CLOCK START B-28)": the CSM's for the
 * same time to AGSBUFF +6, and "CALCULATE AND STORE THE TIME" since the
 * AGS clock's zero, in seconds (TSCALE: "CSEC TO SEC SCALE FACTOR"), at
 * AGSBUFF +12D. EXIT. */
static const word agsvcalc_words_2[] = {
    ORDER(RTB), /* AGSVCALC +011 */
    ADDRESS(LOADTIME),
    STCALL(TDEC1),
    ADDRESS(LEMPREC),
    ORDER(CALL),
    ADDRESS(SCALEVEC),
    STODL(AGSBUFF),
    ADDRESS_PLUS_ONE(TAT),
    STCALL(TDEC1),
    ADDRESS(CSMPREC),
    ORDER(CALL),
    ADDRESS(SCALEVEC),
    STODL_AT(AGSBUFF, 6u),
    ADDRESS_PLUS_ONE(TAT),
    ORDERS(DSU, DDV),
    ADDRESS_PLUS_ONE(AGSK),
    ADDRESS_PLUS_ONE(TSCALE),
    STORE_AT(AGSBUFF, 12u), /* AGSBUFF +12D */
    ORDER(EXIT),
};

/* SCALEVEC -- LM pp. 208-209.
 * VATT1 and RATT1, the state vector, into stable-member axes (REFSMMAT)
 * and the AGS's units: velocity "METERS/CS TO FEET/SEC" (VSCALE),
 * position "METERS TO FEET" (RSCALE). "THIS SECTION ROUNDS THE VECTOR, AND
 * CORRECTS FOR THE FACT THAT THE AGS IS A 2 S COMPLEMENT MACHINE WHILE THE
 * LGC IS A 1 S COMPLEMENT MACHINE" (VECSGNAG). "AGAIN THIS SECTION ROUNDS.
 * TWO VECTORS ARE ADDED TO DEFEAT ALSIGNAG IN THE CASE OF A HIGH-ORDER
 * ZERO COUPLED WITH A LOW ORDER NEGATIVE PART." Then the velocity's high
 * words go with the position's (each component's low word, MPAC +1, +4
 * and +6, takes VATT1's high word through index register 1), and RVQ. */
static const word scalevec_words[] = {
    ORDERS(VLOAD, MXV), /* SCALEVEC */
    ADDRESS_PLUS_ONE(VATT1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDERS(VXSC, VSL2),
    ADDRESS_PLUS_ONE(VSCALE),
    ORDERS(VAD, VAD),
    ADDRESS_PLUS_ONE(AGSRND1),
    ADDRESS_PLUS_ONE(AGSRND2),
    ORDER(RTB),
    ADDRESS(VECSGNAG),
    STOVL(VATT1),
    ADDRESS_PLUS_ONE(RATT1),
    ORDERS(MXV, VXSC),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ADDRESS_PLUS_ONE(RSCALE),
    ORDERS(VSL8, VAD),
    ADDRESS_PLUS_ONE(AGSRND1),
    ORDERS(VAD, RTB),
    ADDRESS_PLUS_ONE(AGSRND2),
    ADDRESS(VECSGNAG),
    ORDER(LXA_1), /* LXA,1 */
    ADDRESS(VATT1),
    ORDERS(SXA_1, LXA_1), /* SXA,1  LXA,1 */
    ADDRESS_AT(MPAC, 1u),
    ADDRESS_AT(VATT1, 2u),
    ORDERS(SXA_1, LXA_1),
    ADDRESS_AT(MPAC, 4u),
    ADDRESS_AT(VATT1, 4u),
    ORDERS(SXA_1, RVQ),
    ADDRESS_AT(MPAC, 6u),
};

#define WORDS(array) (sizeof(array) / sizeof((array)[0]))

const struct interpretive_code AGSVCALC_CODE = {032u, AGSVCALC + 1u, WORDS(agsvcalc_words),
                                                agsvcalc_words};
const struct interpretive_code AGSVCALC_CODE_2 = {032u, AGSVCALC + 011u, WORDS(agsvcalc_words_2),
                                                  agsvcalc_words_2};
const struct interpretive_code SCALEVEC_CODE = {032u, SCALEVEC, WORDS(scalevec_words),
                                                scalevec_words};

#endif
