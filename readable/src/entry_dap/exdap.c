/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part D (Comanche055 pp. 1077-1079, bank
 * 15): "EXTRA ATMOSPHERIC DIGITAL AUTOPILOT", from ATTRATES while .05GSW
 * is clear (the BZF EXDAP), every 0.1 s.
 *
 * Its mode, CMDAPMOD, comes from the angle of attack, by CALFA:
 *   "1.	IF ABS(CALF) -C(45) POS, USE": the yaw jets hold beta,
 *     "YAW ERROR = SGN(CALF) (BETACOM -BETA)", the rate BETADOT, "IF CALFA
 *     POS, CMDAPMOD= +0", or RREL, "IF CALFA NEG, CMDAPMOD= -0"; the roll
 *     error, "SGN(CALF) (ROLLC - ROLL)", is the 2 s roll system's
 *     (CM/RCS, part E). EXDAP3 and EXDAP1.
 *   "2.	IF C(45) GEQ CALFA GEQ -C(45), USE		CMDAPMOD = +1": the
 *     roll jets hold beta, "ROLL ERROR = SGN(-SALF) (BETACOM -BETA)" with
 *     "RATE = BETADOT", and the yaw jets damp the roll, "RATE DAMP ONLY.",
 *     against ROLLHOLD. EXDAP2.
 *   "3.	FOR ALL CASES, USE": the pitch jets hold alpha, "PITCH ERROR =
 *     (ALFACOM - ALFA)", "RATE = QREL".
 * Each error and rate gets its tag from BIASEDZ (part C), and the tags
 * their jets from the tables YJETCODE and P/RJCODE; then on at EXDAPIN,
 * with the DAPs in the atmosphere.
 *
 * The first pass with CALFA past 45 degrees and P63FLAG +1 (P62's) calls
 * WAKEP62 in NSEC, "CALL TO TERMINATE P62 IN N SEC.": "65 DEG/ 3DEG/SEC =
 * 21 SEC NOMINAL" "TRANSIT TIME FROM ALFA=45 TO ALFA TRIM." */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void exdap3(void);
static void exdap2(void);

/* ---- EXDAP -- p. 1077 ---- */

/* EXDAP: "+0 FOR NOW" (CMDAPMOD); then C45LIM, "=1.0-COS(45)", added to
 * the magnitude of CALFA, less 1 (CCS; +0 for either zero, which take no
 * AD). An overflow at TS CMDAPMOD, from the caller's A, skips the CCS:
 * the AD C45LIM, with A +1 or -1. */
void exdap(void)
{
    enum sign calfa = POSITIVE;

    if (!ts(ADDRESS_OF(CMDAPMOD))) calfa = ccs_at(ADDRESS_OF(CALFA));
    if (calfa == POSITIVE || calfa == NEGATIVE) ad(C45LIM);
    exdap_ts_a();
}

/* EXDAP +3: TCF +2, to the TS A. */
void exdap_tcf(void)
{
    exdap_ts_a();
}

/* EXDAP +4: C45LIM added (CALFA negative), */
void exdap_ad_c45lim(void)
{
    ad(C45LIM);
    exdap_ts_a();
}

/* EXDAP +5: TS A, which skips on an overflow: without one, "HERE IF
 * ABS(CALFA) L COS(45)" (EXDAP2); with one, "|CALFA| > 0.707": "CONTINUE
 * IF POS; GO TO EXDAP4 IF NEG." (TC EXDAP4; -0 takes the TC +2 below).
 * Then P63FLAG, "VALID VALUES ARE:  -1, +1, +0.": +1 (positive after CCS
 * +0) goes on to the phase change and WAKEP62's call; the others, and a
 * positive one, to EXDAP4. */
void exdap_ts_a(void)
{
    if (!ts(A_ADDRESS)) {
        exdap2();
        return;
    }
    switch (ccs_at(ADDRESS_OF(CALFA))) {
    case POSITIVE:
    case IS_PLUS_ZERO: break; /* TCF +1 */
    case NEGATIVE:
        tc_at(EXDAP + 012u);
        exdap4();
        return;
    case IS_MINUS_ZERO: exdap_p63_plus0(); return;
    }
    switch (ccs_at(ADDRESS_OF(P63FLAG))) {
    case POSITIVE:
        tc_at(EXDAP + 012u);
        exdap4();
        break;
    case IS_PLUS_ZERO: exdap_p63_plus0(); break;
    case NEGATIVE: exdap_p63_negative(); break;
    case IS_MINUS_ZERO: exdap_phaschng(); break;
    }
}

/* EXDAP +013: TC +2, to the phase change. */
void exdap_p63_plus0(void)
{
    tc_at(EXDAP + 013u);
    exdap_phaschng();
}

/* EXDAP +014: TC EXDAP4. */
void exdap_p63_negative(void)
{
    tc_at(EXDAP + 014u);
    exdap4();
}

/* EXDAP +015: "SINGLE PASS THROUGH HERE." (PHASCHNG, OCT 40334), */
void exdap_phaschng(void)
{
    tc_to(EXDAP + 015u, PHASCHNG);
}

/* EXDAP +017: -1 to P63FLAG, "SET FLAG TO ASSURE SINGLE PASS.", and
 * WAKEP62 in NSEC (WAITLIST, 2CADR WAKEP62); back at EXDAP4. */
void exdap_after_phaschng(void)
{
    set_A(negate(ONE)); /* CS ONE */
    ts(ADDRESS_OF(P63FLAG));
    set_A(NSEC); /* CA */
    tc_to(EXDAP + 022u, WAITLIST);
}

/* ---- EXDAP4, EXDAP3 and EXDAP1 -- p. 1078 ---- */

/* EXDAP4: "ROLLJET INTERFACE TEST BETWEEN .1 SEC" "DAP AND THE 2 SEC
 * CM/RCS DAP": a negative JETAG (-1, the roll jets run at 0.1 s), or -0,
 * "TURN OFF ROLL JETS IF ON AND WAIT" "UNTIL START OF 2 SEC CM/RCS
 * CYCLE" (+0 to channel 6 and JETAG, "RESTORE PROPER VALUE +0"). "ROLL
 * FDAI WILL BE IN ERROR UNTIL NEXT CM/RCS CALL." */
void exdap4(void)
{
    enum sign jetag = ccs_at(ADDRESS_OF(JETAG));

    if (jetag == NEGATIVE) set_A(ZERO); /* CA ZERO (the -0 leaves +0 too) */
    if (jetag == NEGATIVE || jetag == IS_MINUS_ZERO) {
        write_channel(6); /* EXTEND; WRITE ROLLJETS */
        ts(ADDRESS_OF(JETAG));
    }
    exdap3();
}

/* EXDAP3: "HERE IF ABS(CALFA) GEQ COS(45)": the yaw error, RAXERR,
 * "BETACOM - BETA/180", as it is for CALFA positive (+0 for CALFA +0);
 * for CALFA negative CMDAPMOD -0 ("FOR CM/RCS"; +0 for CALFA -0) and
 * "COMPLEMENT OF YAW ERROR.". */
static void exdap3(void)
{
    switch (ccs_at(ADDRESS_OF(CALFA))) {
    case POSITIVE: ca(ADDRESS_OF(RAXERR)); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
        set_A(negate(ZERO)); /* CS ZERO */
        ts(ADDRESS_OF(CMDAPMOD));
        cs(ADDRESS_OF(RAXERR));
        break;
    case IS_MINUS_ZERO:
        ts(ADDRESS_OF(CMDAPMOD));
        cs(ADDRESS_OF(RAXERR));
        break;
    }
    exdap1();
}

/* EXDAP1: the yaw error to RAXERR, "FOR YAW FDAI", AK2, "WANT RAXERR FOR
 * TM.", and L (an overflow skips the TS AK2, A +1 or -1); then the rate,
 * "COORDINATE BETA CONTROL.": "C(CMDAPMOD) CAN BE +1, +0, OR -0." "USE
 * BETADOT TO COORD IN MODE +0" "OTHERWISE USE RREL." (+1, by TC +3, and
 * -0; a negative CMDAPMOD indexes from RREL). "GO TEST DZ. +0 IF IN DZ,
 * +-1 OTHERWISE" "IF GEQ 4D/S, SET ERROR BIT IN L=0" (BIASEDZ, back at
 * EXDAP1 +011). */
void exdap1(void)
{
    if (!ts(ADDRESS_OF(RAXERR))) ts(ADDRESS_OF(AK2));
    ts(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(CMDAPMOD))) {
    case POSITIVE:
        tc_at(EXDAP1 + 4u); /* TC +3 */
        ca(ADDRESS_OF(RREL));
        break;
    case IS_PLUS_ZERO: exdap1_mode0(); return;
    case NEGATIVE: index_a_ca(ADDRESS_OF(RREL)); break; /* INDEX A; CA RREL */
    case IS_MINUS_ZERO: ca(ADDRESS_OF(RREL)); break;
    }
    tc_to(EXDAP1 + 010u, BIASEDZ);
}

/* EXDAP1 +5: mode +0: CA ONE; INDEX A; CA RREL, the word after it,
 * BETADOT. */
void exdap1_mode0(void)
{
    set_A(ONE);
    index_a_ca(ADDRESS_OF(RREL));
    tc_to(EXDAP1 + 010u, BIASEDZ);
}

/* EXDAP1 +011: the tag with the error bit, "L HAS BETA BIT" (ROR LCHAN),
 * gives the yaw jets (YJETCODE) in JETEM. Then "ALFA ERROR." (QAXERR) in
 * L and, "FOR ALPHADOT USE QREL", its tag (BIASEDZ, back at EXDAP1 +022). */
void exdap1_after_biasedz(void)
{
    ror_lchan();        /* EXTEND; ROR LCHAN */
    jet_code(YJETCODE); /* INDEX A; CAF YJETCODE */
    ts(ADDRESS_OF(JETEM));
    ca(ADDRESS_OF(QAXERR));
    ts(L_ADDRESS);
    ca(ADDRESS_OF(QREL));
    tc_to(EXDAP1 + 021u, BIASEDZ);
}

/* EXDAP1 +022: the pitch tag with its error bit, and "CONTINUE ON IN DAP"
 * (TCF EXDAPIN), where the pitch jets are added. */
void exdap1_after_alfa_dz(void)
{
    ror_lchan(); /* EXTEND; ROR LCHAN */
    continue_at(EXDAPIN);
}

/* ---- EXDAP2 -- pp. 1078-1079 ---- */

/* EXDAP2: "SET CMDAPMOD TO +1" (INCR, from the +0 EXDAP left); -1 to
 * JETAG, "INDICATE CHANGE FROM .1 SEC UPDATE TO" "2 SEC FOR ROLL JETS.
 * (IF CMDAPMOD" "=0 AND JETAG =-1, QUENCHES JETS IF ON)"; on at EXDAP2
 * +3 (the faithful port's run stops there, at a key of another of its
 * parts). */
static void exdap2(void)
{
    incr(ADDRESS_OF(CMDAPMOD));
    set_A(negate(ONE)); /* CS ONE */
    ts(ADDRESS_OF(JETAG));
    continue_at(EXDAP2 + 3u);
}

/* EXDAP2 +3: P63FLAG, "IF FLAG WAS +1, SET =0." (and a larger one less
 * 1; then NOOP). "BETA CONTROL WITH P JETS": the error "BETACOM -
 * BETA/180" from RAXERR, against SALFA's sign ("SGN(-SALF)"; +0 for
 * either zero), "TEMP SAVE.  ERROR/180" in PAXERR1. (A -0 P63FLAG skips
 * the CCS SALFA: the CS RAXERR.) */
void exdap2_p63flag(void)
{
    enum sign salfa = POSITIVE;

    switch (ccs_at(ADDRESS_OF(P63FLAG))) {
    case POSITIVE:
        ts(ADDRESS_OF(P63FLAG));
        salfa = ccs_at(ADDRESS_OF(SALFA));
        break;
    case IS_PLUS_ZERO:
    case NEGATIVE: salfa = ccs_at(ADDRESS_OF(SALFA)); break;
    case IS_MINUS_ZERO: break;
    }
    if (salfa == POSITIVE) cs(ADDRESS_OF(RAXERR));
    if (salfa == NEGATIVE) ca(ADDRESS_OF(RAXERR));
    ts(ADDRESS_OF(PAXERR1));
    exdap2_paxerr1();
}

/* EXDAP2 +013: halved, "CM/FDAI EXPECTS ERROR/360.", into PAXERR1,
 * "ERROR/360 FOR FDAI, GET ERROR/180." to L; and the rate, "USE BETADOT
 * TO COORD IN MODE +1", against SALFA's sign (+0 for either zero), to
 * BIASEDZ (back at EXDAP2 +024). */
void exdap2_paxerr1(void)
{
    mp(HALF); /* EXTEND; MP HALF */
    xch(ADDRESS_OF(PAXERR1));
    ts(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(SALFA))) {
    case POSITIVE:
        cs(ADDRESS_OF(BETADOT));
        tc_at(EXDAP2 + 021u); /* TC +2 */
        break;
    case IS_PLUS_ZERO: tc_at(EXDAP2 + 021u); break;
    case NEGATIVE: exdap2_betadot(); return;
    case IS_MINUS_ZERO: break;
    }
    tc_to(EXDAP2 + 023u, BIASEDZ);
}

/* EXDAP2 +022: CA BETADOT (SALFA negative), to BIASEDZ. */
void exdap2_betadot(void)
{
    ca(ADDRESS_OF(BETADOT));
    tc_to(EXDAP2 + 023u, BIASEDZ);
}

/* EXDAP2 +024: the tag with its error bit, "GET ROLL CODE" (P/RJCODE):
 * "ROLL CONTROL WITH YAW JETS." "WE,LL SKIP REGULAR ROLL SYST" (channel
 * 6). Then the roll's drift, "ROLL/180 AT CM/DAPON TIME." (ROLLHOLD) less
 * ROLL/180 ("1,S COMPL, BUT SO WHATS A BIT.?"), to L, "FORCE A LIMIT
 * CYCLE IN YAW RATE." "TO REMOVE ITS BIASING EFFECT ON M DOT.": against
 * SALFA's sign (+0 for either zero), the yaw error for EXDAP1. (The
 * faithful port's run stops at EXDAP1, a key of another of its parts.) */
void exdap2_after_biasedz(void)
{
    ror_lchan();        /* EXTEND; ROR LCHAN */
    jet_code(P_RJCODE); /* INDEX A; CAF P/RJCODE */
    write_channel(6);   /* EXTEND; WRITE ROLLJETS */
    ca(ADDRESS_OF(ROLLHOLD));
    msu_at(ADDRESS_OF(ROLL_180)); /* EXTEND; MSU ROLL/180 */
    ts(L_ADDRESS);
    switch (ccs_at(ADDRESS_OF(SALFA))) {
    case POSITIVE:
        ca(L_ADDRESS);
        tc_to(EXDAP2 + 040u, EXDAP1);
        break;
    case IS_PLUS_ZERO: tc_to(EXDAP2 + 040u, EXDAP1); break;
    case NEGATIVE: exdap2_cs_l(); break;
    case IS_MINUS_ZERO: tc_to(EXDAP2 + 042u, EXDAP1); break;
    }
}

/* EXDAP2 +041: CS L (SALFA negative), to EXDAP1. */
void exdap2_cs_l(void)
{
    cs(L_ADDRESS);
    tc_to(EXDAP2 + 042u, EXDAP1);
}

/* EXDAP2 +043: NSEC, a constant (DEC 2100), which a TC Q to EXDAP2 +043
 * would run: the faithful port does not run it. */
void nsec_constant(void)
{
    not_modelled();
}

#endif
