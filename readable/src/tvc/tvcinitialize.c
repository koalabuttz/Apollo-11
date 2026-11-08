/* SPDX-License-Identifier: MIT */
/* TVCINITIALIZE.agc (Comanche055 pp. 937-944, bank 17): "TVC DAP
 * INITIALIZATION AND STARTUP CALL", the switch-over to the docked CSM/LM's
 * low bandwidth DAP, the filter coefficients and the DAP's gains. The
 * faithful C is c64-port/src/cm_tvc_init.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* DSALMOUT, channel 11. */
#define DSALMOUT_CHANNEL 011u

/* ---- Words read under INDEX ---- */

/* The words in fixed memory the file reads under INDEX: the filter
 * coefficients and their addresses (CSMN10 to HBCFADR, CSMCFADR +1),
 * TCORR and the word after it, and BIT3 and BIT2 (BIT2 -1 and BIT2). */
static bool a_constant(unsigned address)
{
    return (address >= ADDRESS_OF(CSMN10) && address <= ADDRESS_OF(CSMCFADR) + 1u) ||
           address == ADDRESS_OF(TCORR) || address == ADDRESS_OF(TCORR) + 1u ||
           address == ADDRESS_OF(BIT2) - 1u || address == ADDRESS_OF(BIT2);
}

/* INDEX, then an order whose address field is field: the field plus the
 * index, a 12-bit address. Not modelled (the faithful port refuses them
 * too): a sum outside 0-07777 or at Z, or a word of fixed memory other
 * than the constants above. */
static unsigned index_field(unsigned field, word index)
{
    int sum = (int)field + value_of(index);

    if (sum < 0 || sum > 07777 || sum == (int)Z_ADDRESS) not_modelled();
    if (sum >= 02000 && !a_constant((unsigned)sum)) not_modelled();
    return (unsigned)sum;
}

/* INDEX k; CA (or CAE, CAF) address. */
static void ca_indexed(unsigned address, word index)
{
    ca(index_field(address, index));
}

/* INDEX k; DCA address, whose address field is address +1: the double
 * word at address + C(k). A field of 0 names the low word at 0, A itself,
 * and the high word one before it, which wraps to 01777 of the erasable
 * bank (as the faithful port reads it). Not modelled, as above: in fixed
 * memory, either word not one of the constants. */
static void dca_indexed(unsigned address, word index)
{
    unsigned field = index_field(address + 1u, index);

    if (field == 0u) {
        set_L(overflow_corrected(A));
        A = sign_extend(fetch(01777u));
        return;
    }
    if (field >= 02000u && !a_constant(field - 1u)) not_modelled();
    dca(field - 1u);
}

/* The word at an erasable address, as INDEX reads it. */
static word index_at(unsigned address)
{
    return fetch(address);
}

/* TC (or TCR) to routine at fixed address at. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* ---- TVCDAPON and MRCLEAN -- pp. 937-938 ---- */

/* TVCDAPON, the "T5 RUPT ARRIVAL (CALL BY DOTVCON - P40)": BANKRUPT and
 * QRUPT kept ("SAVE Q REQUIRED IN RESTARTS (MRCLEAN AND" "TVCINIT4 ARE
 * ENTRIES)"), then MRCLEAN. */
void tvcdapon(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    mrclean();
}

/* MRCLEAN ("TVC RESTARTS ENTER HERE  (NEW BANK)"): the DAP's erasables
 * zeroed, OMEGAYC on, two words a pass. NZERO is the "NUMBER TO ZERO, LESS
 * ONE  (MUST BE ODD)": CCS A takes it one less, to the even CNTR of the
 * pass ("FIRST (LAST) TWO LOCATIONS" at 50), and CCS CNTR one less again
 * for the next; the pass at CNTR +0 is the last. Then "SET UP ANOTHER T5
 * RUPT TO CONTINUE" "INITIALIZATION AT TVCINIT1" (INITLOC2 to T5LOC, TIME5
 * POSMAX), and RESUME (ENDMRC). */
void mrclean(void)
{
    set_A(NZERO);
    do {
        ccs_register(A_ADDRESS); /* MRCLEAN +1 */
        ts(ADDRESS_OF(CNTR));
        set_A(ZERO);
        ts(L_ADDRESS);
        dxch(indexed(ADDRESS_OF(OMEGAYC), index_at(ADDRESS_OF(CNTR))));
    } while (ccs_at(ADDRESS_OF(CNTR)) == POSITIVE); /* TCF MRCLEAN +1 */
    dca(ADDRESS_OF(INITLOC2));
    dxch(ADDRESS_OF(T5LOC));
    set_A(POSMAX);
    ts(ADDRESS_OF(TIME5));
    continue_at(RESUME); /* ENDMRC */
}

/* ---- TVCINIT1 to TVCINIT4 -- pp. 938-941 ---- */

/* TVCINIT1, the next T5 interrupt: BANKRUPT and QRUPT kept; MASSPROP
 * (IBNKCALL) to "UPDATE IXX, IAVG/TLX FOR DAP GAINS". */
void tvcinit1(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT));
    tc_to(TVCINIT1 + 3u, IBNKCALL); /* CADR MASSPROP */
}

/* TVCINIT1 +5, after MASSPROP: TENMDOT, the "10-SEC MASS LOSS B+16 KG"
 * (EMDOT, the "SPS FLOW RATE, SCALED B+3 KG/CS", times ONETHOU), and
 * MASSTMP the CSM's mass less it, the "DECREMENT FOR FIRST 10 SEC OF
 * BURN" (an overflow there skips the CAE DAPDATR1, leaving +1 or -1 in A
 * for the MASK). CNTR from DAPDATR1's bit 14 ("CHECK LEM-ON/OFF"): BIT1
 * for "LEM-ON (BIT1)", +0 for "LEM-OFF (ZERO)". Then LOADCOEF, from the
 * coefficients at CSMCFADR or HBCFADR (CSMCFADR +1) as CNTR picks them;
 * the faithful port's run stops at LOADCOEF, a key of its own. */
void tvcinit1_after_massprop(void)
{
    ca(ADDRESS_OF(EMDOT));
    mp(ONETHOU);
    ts(ADDRESS_OF(TENMDOT));
    com();
    ad_at(ADDRESS_OF(CSMMASS));
    if (!ts(ADDRESS_OF(MASSTMP))) ca(ADDRESS_OF(DAPDATR1));
    mask(BIT14);
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(BIT1); /* else +0 */
    ts(ADDRESS_OF(CNTR));
    ca_indexed(ADDRESS_OF(CSMCFADR), index_at(ADDRESS_OF(CNTR)));
    ts(ADDRESS_OF(COEFFADR));
    tc_to(TVCINIT1 + 024u, LOADCOEF); /* "LOAD THE FILTER COEFFICIENTS" */
}

/* TVCINIT1 +025, after LOADCOEF: KTLX/I, the "LM-OFF,-ON KTLX/I" CNTR
 * picks from EKTLX/I on ("SCALED AT 1/(8 ASCREV) OF ACTUAL VALUE"), and
 * TCR S40.15 to "COMPUTE 1/CONACC , VARK"; the run stops at S40.15, a key
 * of its own. */
void tvcinit1_after_loadcoef(void)
{
    ca_indexed(ADDRESS_OF(EKTLX_I), index_at(ADDRESS_OF(CNTR)));
    ts(ADDRESS_OF(KTLX_I));
    tc_to(TVCINIT1 + 030u, S40_15);
}

/* TVCINIT2: KPRIMEDT, BIT2 (CNTR +0) or BIT3 (CNTR 1), the filter period
 * ("BIT2 FOR CSM ONLY 40MS FILTER", "BIT3 FOR CSM/LM 80MS FILTER");
 * T5TVCDT POSMAX +1 less it, the T5 time to "PREPARE"; the switch-over
 * flag, bit 15 of FLAGWRD9, cleared ("RESET SWTOVER FLAG"; an overflow at
 * the TS T5TVCDT would skip the CS BIT15, leaving +1 or -1 in A for the
 * MASK). Then KPRIMEDT the "LEM-OFF,-ON KPRIME" CNTR picks from EKPRIME on
 * ("SCALED (100 PI)/16"), times the period ("(TVCDT/2, SC.AT B+14CS)"):
 * the product's low word (LXCH A) and the period's next word (KPRIMEDT
 * +1, by DXCH). The run stops at TVCINIT2 +021, a key of its own. */
void tvcinit2(void)
{
    cs(ADDRESS_OF(CNTR));
    ca_indexed(ADDRESS_OF(BIT2), a_word()); /* INDEX A; CAF BIT2 */
    ts(ADDRESS_OF(KPRIMEDT));
    com();
    ad(POSMAX);
    ad(BIT1);
    if (!ts(ADDRESS_OF(T5TVCDT))) set_A(negate(BIT15)); /* CS BIT15 */
    mask_at(ADDRESS_OF(FLAGWRD9));
    ts(ADDRESS_OF(FLAGWRD9));
    ca_indexed(ADDRESS_OF(EKPRIME), index_at(ADDRESS_OF(CNTR)));
    mp_at(ADDRESS_OF(KPRIMEDT));
    lxch(A_ADDRESS); /* "SC.AT PI/8" */
    dxch(ADDRESS_OF(KPRIMEDT));
    continue_at(TVCINIT2 + 021u);
}

/* ATTINIT, the "ATTITUDE-ERROR INITIALIZATION LOGIC", for the CSM alone
 * (DAPDATR1's bit 13 set; "BYPASS INITIALIZATION FOR CSM/LM"): the RCS
 * attitude errors in ERRBTMP, yaw (ERRBTMP +1) then pitch, each limited
 * to ATTLIM, 1.5 degrees, with its sign (".007325(ERROR) = 0 IF ERROR <
 * 1.5 DEG": a product of +0 or -0 keeps it); then PERRB and YERRB. */
static void attinit(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT13);
    if (A_is_zero()) return; /* EXTEND; BZF NEEDLEIN */
    set_A(BIT1);             /* "SET UP TEMPORARY COUNTER" */
    do {
        unsigned error;

        ts(ADDRESS_OF(TTMP1)); /* ATTINIT +5 */
        error = indexed(ADDRESS_OF(ERRBTMP), index_at(ADDRESS_OF(TTMP1)));
        ca(error);
        mp(ONE_OVER_ATTLIM);
        if (!A_is_zero()) { /* EXTEND; BZF +8D: "|ERROR| LESS THAN 1.5 DEG" */
            set_A(A_is_zero_or_minus() ? negate(ATTLIM) : ATTLIM); /* BZMF +3 */
            ts(error);
        }
    } while (ccs_at(ADDRESS_OF(TTMP1)) == POSITIVE); /* +8: "BACK TO REPEAT FOR PITCH ERROR" */
    ca(ADDRESS_OF(ERRBTMP)); /* "ERRORS ESTABLISHED AND LIMITED" */
    ts(ADDRESS_OF(PERRB));
    ca(ADDRESS_OF(ERRBTMP) + 1u);
    ts(ADDRESS_OF(YERRB));
}

/* TVCINIT2 +021: REPFRAC, the "LEM-OFF,-ON REPFRAC" CNTR picks from
 * EREPFRAC on; CNTR the "ONE-SHOT CORRECTION TIME" from TCORR on (5 for
 * the CSM, +0 for the CSM/LM); STRKTIME -1 to "PREVENT STROKE TEST UNTIL
 * CALLED"; VCNTR and V97VCNTR 19, the "VARIABLE-GAIN UPDATES IN" "10
 * SECONDS (TVCEXEC 1/2 SEC RATE)". TVCINIT3: the "TRIM VALUES TO
 * TRIM-TRACKERS, OUTPUT" "TRACKERS, OFFSET-UPDATES, AND" "OFFSET-TRACKER
 * FILTERS", PACTOFF and YACTOFF. ATTINIT; then NEEDLEIN: bit 3 of
 * RCSFLAGS set "FOR INITIALIZATION PASS AND GO" "TO NEEDLER" (IBNKCALL). */
void tvcinit2_repfrac(void)
{
    ca_indexed(ADDRESS_OF(EREPFRAC), index_at(ADDRESS_OF(CNTR)));
    ts(ADDRESS_OF(REPFRAC));
    ca_indexed(ADDRESS_OF(TCORR), index_at(ADDRESS_OF(CNTR)));
    ts(ADDRESS_OF(CNTR));
    set_A(NEGONE);
    ts(ADDRESS_OF(STRKTIME));
    set_A(NINETEEN);
    ts(ADDRESS_OF(VCNTR));
    ts(ADDRESS_OF(V97VCNTR)); /* "FOR ENGFAIL (R41) LOGIC" */
    /* TVCINIT3 */ ca(ADDRESS_OF(PACTOFF));
    ts(ADDRESS_OF(PDELOFF));
    ts(ADDRESS_OF(PCMD));
    ts(ADDRESS_OF(DELPBAR));
    ca(ADDRESS_OF(YACTOFF));
    ts(ADDRESS_OF(YDELOFF));
    ts(ADDRESS_OF(YCMD));
    ts(ADDRESS_OF(DELYBAR));
    attinit();
    /* NEEDLEIN */ cs(ADDRESS_OF(RCSFLAGS));
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    tc_to(NEEDLEIN + 3u, IBNKCALL); /* CADR NEEDLER */
}

/* TVCINIT4 ("POSSIBLE TVC-RESTART ENTRY"): TVCPHASE +0, "TVCDAPON-THRU-"
 * "NEEDLEIN INITIALIZATION FINISHED."; OGANOW CDUX, to "PREPARE ROLL
 * DAP". With the engine on (bit 13 of DSALMOUT) the "COPYCYCLE", MASSTMP
 * to CSMMASS; "IF ENGINE IS ALREADY OFF, ENGINOFF HAS" "ALREADY
 * ESTABLISHED THE POST-BURN" "CSMMASS". Then TVCEXEC in half a second
 * (WAITLIST, 2CADR TVCEXEC): "CALL TVCEXECUTIVE (ROLLDAP CALL, ETC)". */
void tvcinit4(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(TVCPHASE));
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(OGANOW));
    set_A(BIT13);
    rand_channel(DSALMOUT_CHANNEL);
    if (!A_is_zero()) { /* EXTEND; BZF +3 */
        ca(ADDRESS_OF(MASSTMP));
        ts(ADDRESS_OF(CSMMASS));
    }
    set_A(HALF_SEC); /* +3 */
    tc_to(TVCINIT4 + 014u, WAITLIST);
}

/* TVCINIT4 +017, after WAITLIST: "CALL FOR DAPINIT", its 2CADR
 * (DAPINIT5) in A,L. The run stops at TVCINIT4 +021, a key of its own. */
void tvcinit4_after_waitlist(void)
{
    dca(ADDRESS_OF(DAPINIT5));
    continue_at(TVCINIT4 + 021u);
}

/* TVCINIT4 +021: the 2CADR to T5LOC, and TIME5 T5TVCDT ("ALLOW TIME FOR
 * RESTART COMPUTATIONS"); RESUME (ENDTVCIN). */
void tvcinit4_t5(void)
{
    dxch(ADDRESS_OF(T5LOC));
    ca(ADDRESS_OF(T5TVCDT));
    ts(ADDRESS_OF(TIME5));
    continue_at(RESUME);
}

/* ---- PRESWTCH and SWICHOVR -- pp. 941-942 ---- */

/* SWICHOVR +5 ("RESTART ENTRY POINT," "TVCPHASE AND PHASETMP ALREADY
 * SET"): the return kept in RTRNLOC ("SAVE Q FOR RETURN"); the "ZEROING
 * LOOP FOR FILTER STORAGE LOCS", PTMP1 -1 on, two words a pass from
 * CNTRTMP NZEROJR (23, odd: MCLEANJR's two CCS's take it two less a pass,
 * to 1, the last); the switch-over flag set "FOR DOWNLINK" (bit 15 of
 * FLAGWRD9); KTLX/I the "LOW BANDWIDTH GAINS" (EKTLX/I +2), and
 * TCR S40.15 +7 for VARK. The run stops at S40.15 +7, a key of its own. */
void swichovr_restart(void)
{
    qxch(ADDRESS_OF(RTRNLOC));
    set_A(NZEROJR);
    do {
        ts(ADDRESS_OF(CNTRTMP)); /* SWICHOVR +8D */
        /* MCLEANJR */ set_A(ZERO);
        ts(L_ADDRESS);
        dxch(indexed(ADDRESS_OF(PTMP1) - 1u, index_at(ADDRESS_OF(CNTRTMP))));
    } while (ccs_at(ADDRESS_OF(CNTRTMP)) == POSITIVE && ccs_register(A_ADDRESS) == POSITIVE);
    cs(ADDRESS_OF(FLAGWRD9));
    mask(BIT15);
    ads(ADDRESS_OF(FLAGWRD9));
    ca(ADDRESS_OF(EKTLX_I) + 2u);
    ts(ADDRESS_OF(KTLX_I));
    tc_to(SWICHOVR + 025u, S40_15 + 7u);
}

/* SWICHOVR, by TCR: INHINT; TVCPHASE kept in PHASETMP ("SAVE
 * TVCPHASE"), and set to -2 ("INDICATES SWITCH-OVER" "TO RESTART
 * LOGIC"); then SWICHOVR +5. */
static void swichovr(void)
{
    inhint();
    ca(ADDRESS_OF(TVCPHASE));
    ts(ADDRESS_OF(PHASETMP));
    set_A(negate(BIT2)); /* CS BIT2 */
    ts(ADDRESS_OF(TVCPHASE));
    swichovr_restart();
}

/* PRESWTCH, the "ENTRY FROM V46": TCR SWICHOVR. */
void preswtch(void)
{
    tc_at(PRESWTCH);
    swichovr();
}

/* PRESWTCH +1, SWICHOVR's return: POSTJUMP to PINBRNCH, "AN EXIT FROM
 * SWITCH-OVER" "(PINBRNCH DOES A RELINT)". */
void preswtch_after_swichovr(void)
{
    tc_to(PRESWTCH + 1u, POSTJUMP); /* CADR PINBRNCH */
}

/* SWICHOVR +026, after S40.15 +7: the CSM/LM's low bandwidth KPRIMEDT
 * (FKPRIMDT, "- STEERING") and REPFRAC (FREPFRAC, "- TMC LOOP"); "UPDATE
 * TRIM ESTIMATES", DELPBAR to PDELOFF and DELYBAR to YDELOFF; and
 * LOADCOEF from LBN10 (LBCFADR). The run goes on into LOADCOEF, and stops
 * at LOADCOEF +014, a key of its own. */
void swichovr_after_s40_15(void)
{
    set_A(FKPRIMDT);
    ts(ADDRESS_OF(KPRIMEDT));
    set_A(FREPFRAC);
    ts(ADDRESS_OF(REPFRAC));
    dca(ADDRESS_OF(DELPBAR));
    dxch(ADDRESS_OF(PDELOFF));
    dca(ADDRESS_OF(DELYBAR));
    dxch(ADDRESS_OF(YDELOFF));
    set_A(LBCFADR);
    ts(ADDRESS_OF(COEFFADR));
    tc_at(SWICHOVR + 042u); /* TC LOADCOEF */
    loadcoef();
}

/* SWICHOVR +043, after LOADCOEF: "RESTORE TVCPHASE"; TC RTRNLOC, "BACK TO
 * PRESWTCH OR TVCRESTARTS": to the erasable word, which the program runs
 * as TC <return> (rtrnloc_return). */
void swichovr_after_loadcoef(void)
{
    ca(ADDRESS_OF(PHASETMP));
    ts(ADDRESS_OF(TVCPHASE));
    tc_to(SWICHOVR + 045u, ADDRESS_OF(RTRNLOC));
}

/* RTRNLOC run as an instruction: TC <the return SWICHOVR kept> (PRESWTCH
 * +1, or TVCRESTARTS's). */
void rtrnloc_return(void)
{
    tc_through(ADDRESS_OF(RTRNLOC));
}

/* ---- LOADCOEF -- p. 942 ---- */

/* LOADCOEF: "LOAD DAP FILTER COEFFICIENTS" from COEFFADR on, into N10
 * on, two words at a time: "FROM: ERASABLE FOR CSM/LM HB" "FIXED    FOR
 * CSM/LM LB" "FIXED    FOR CSM". The first three pairs; the run stops at
 * LOADCOEF +014, a key of its own. */
void loadcoef(void)
{
    dca_indexed(0u, index_at(ADDRESS_OF(COEFFADR)));
    dxch(ADDRESS_OF(N10));
    dca_indexed(2u, index_at(ADDRESS_OF(COEFFADR)));
    dxch(ADDRESS_OF(N10) + 2u);
    dca_indexed(4u, index_at(ADDRESS_OF(COEFFADR)));
    dxch(ADDRESS_OF(N10) + 4u);
    continue_at(LOADCOEF + 014u);
}

/* LOADCOEF +023: the fifth pair stored (N10 +8D), the sixth and seventh,
 * and the fifteenth word (14D); TC Q. */
void loadcoef_n10_8(void)
{
    dxch(ADDRESS_OF(N10) + 8u);
    dca_indexed(10u, index_at(ADDRESS_OF(COEFFADR)));
    dxch(ADDRESS_OF(N10) + 10u);
    dca_indexed(12u, index_at(ADDRESS_OF(COEFFADR)));
    dxch(ADDRESS_OF(N10) + 12u);
    ca_indexed(14u, index_at(ADDRESS_OF(COEFFADR)));
    ts(ADDRESS_OF(N10) + 14u);
    tc_q();
}

/* LOADCOEF +017: the fourth pair stored (N10 +6), and on from the fifth. */
void loadcoef_n10_6(void)
{
    dxch(ADDRESS_OF(N10) + 6u);
    dca_indexed(8u, index_at(ADDRESS_OF(COEFFADR)));
    loadcoef_n10_8();
}

/* LOADCOEF +014: the fourth pair, and on. */
void loadcoef_dca6(void)
{
    dca_indexed(6u, index_at(ADDRESS_OF(COEFFADR)));
    loadcoef_n10_6();
}

/* ---- S40.15 -- p. 943 ---- */

/* S40.15 +8: VARK, KTLX/I (in A) times IAVG/TLX, scaled "(B+3 ASCREV)";
 * TC Q. Not modelled: an overflow at the TS VARK, whose skip lands on
 * CSMN10's coefficients, data (the faithful port refuses it too). */
static void vark(void)
{
    mp_at(ADDRESS_OF(IAVG_TLX));
    ddoubl();
    ddoubl();
    if (ts(ADDRESS_OF(VARK))) not_modelled();
    tc_q();
}

/* S40.15, the "GAIN COMPUTATIONS (1/CONACC, VARK)" "ENTERED FROM
 * TVCINITIALIZE AND TVCEXEC": 1/CONACC, IXX times 2PI/M ("SCALED B+9
 * SEC-SQ/REV"); then VARK (an overflow at the TS 1/CONACC skips the CAE
 * KTLX/I, leaving +1 or -1 in A). */
void s40_15(void)
{
    ca(ADDRESS_OF(IXX));
    mp(TWO_PI_OVER_M);
    ddoubl();
    ddoubl();
    ddoubl();
    if (!ts(ADDRESS_OF(1_CONACC))) ca(ADDRESS_OF(KTLX_I));
    vark();
}

/* S40.15 +7, the "ENTRY FROM CSM/LM V46 SWITCH-OVER": VARK alone. */
void s40_15_switchover(void)
{
    ca(ADDRESS_OF(KTLX_I));
    vark();
}

#endif
