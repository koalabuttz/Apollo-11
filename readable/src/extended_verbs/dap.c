/* SPDX-License-Identifier: MIT */
/* EXTENDED_VERBS.agc, part 5: the DAP data load, verb 48 -- LM pp.
 * 292-295, CM pp. 253-254. The faithful C is c64-port/src/lanec_ev_dap.c.
 *
 * Verb 48 shows the digital autopilot's configuration and the vehicle's
 * masses, takes the crew's changes and sets the autopilot up from them.
 * Part 2's DAPDISP starts the LM's (routine R03): with the display claimed
 * and the job at priority 7, POSTJUMP to DAPDATA1 in bank 20.
 *   - DAPDATA1: noun 46, the configuration: R1 is DAPDATR1, built from
 *     DAPBOOLS's bits with bit 14 the stage (APSFLAG's complement), R2
 *     and R3 blank. Proceed sets APSFLAG, DAPBOOLS, MASS, the X-axis jets
 *     (NJETSFLAG) and the maneuver rate (RATEINDX) from it.
 *   - STIKLOAD, in bank 01: the hand controller's sensitivity and rate
 *     deadband, a tenth as much with the CSM docked.
 *   - DAPDATA2: noun 47, the LM's and the CSM's masses. Each must be more
 *     than the empty vehicle's; then MASS and RESTORDB (the deadband and
 *     the moments of inertia).
 *   - DAPDATA3: noun 48, the descent engine's trim angles, before staging
 *     only. Proceed trims the gimbal (TRIMGIMB, a task), which comes back
 *     at TRIMDONE: V50 N48, please terminate.
 * The CM's DAPDISP (bank 43) refuses while the TVC autopilot runs, then
 * BANKCALLs DAPDISP1 in bank 42: at priority 10, noun 46 (the
 * configuration; S41.2 sets the RCS autopilot up from it), noun 47 (the
 * masses; MASSPROP and S40.14 for configurations 1, 2 and 6) and noun 48
 * (the trims).
 *
 * Each display is a flashing verb and noun (GOXDSPF, or the LM's
 * GOXDSPFR and GOMARK3R) that returns on the crew's terminate (V34),
 * proceed (V33) or enter, and for the LM's also at once, with the display
 * up (the immediate return). Each such call, and each call out of the
 * file, ends a C function with Q and Z set (tc() below, or continue_at()),
 * and the word it returns to is an entry of its own; the binding goes on
 * from one entry of this part to the next while Z is one in the same bank.
 * A routine that falls into the next label calls it. */
#include "extended_verbs.h"
#include "../agc/names.h"

/* TC at fixed address at to routine: Q gets the return, at +1. */
static void tc(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* CCS A: the masked word is positive (not +0). */
static bool masked_is_set(void)
{
    return ccs_register(A_ADDRESS) == POSITIVE;
}

/* A flashing display from the word at: CAF <verb-noun>, TC BANKCALL, CADR
 * GOXDSPF (or GOXDSPFR, GOMARK3R). */
static void show(word at, word verb_noun)
{
    set_A(verb_noun);
    tc(at + 1u, BANKCALL);
}

#if LUMINARY099
/* ---- Noun 46: the configuration (bank 20) ---- */

/* DAPDATA1 -- LM p. 292.
 * "SET DISPLAY ACCORDING TO DAPBOOLS BITS", then "SET BIT 14 TO BE
 * COMPLEMENT OF APSFLAG": 1 before staging. */
void dapdata1(void)
{
    set_A(BOOLSMSK);
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPDATR1));
    cs(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (masked_is_set()) set_A(BIT14);
    ads(ADDRESS_OF(DAPDATR1));
    chkdata1();
}

/* CHKDATA1 -- LM p. 292, where a load comes back.
 * "IF BITS 13 AND 14 ARE BOTH ZERO, FORCE A ONE INTO BIT 13." */
void chkdata1(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT13_14);
    if (A_is_zero()) { /* EXTEND; BZF FORCEONE */
        continue_at(FORCEONE);
        return;
    }
    ca(ADDRESS_OF(DAPDATR1));
    mskdatr1();
}

/* MSKDATR1 -- LM p. 292.
 * "ENSURE THAT NO ILLEGAL BITS SET BY CREW", then V01 N46, flashing. */
void mskdatr1(void)
{
    mask(DSPLYMSK);
    ts(ADDRESS_OF(DAPDATR1));
    show(MSKDATR1 + 2u, V01N46); /* CADR GOXDSPFR */
}

/* MSKDATR1 +5 -- LM p. 292: V34, terminate. */
void mskdatr1_after_terminate(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT */
}

/* MSKDATR1 +6 -- LM p. 292: V33, proceed. */
void mskdatr1_after_proceed(void)
{
    continue_at(DPDAT1); /* TCF DPDAT1 */
}

/* MSKDATR1 +7 -- LM p. 292: "NEW DATA", "CHECK AND REDISPLAY". */
void mskdatr1_after_enter(void)
{
    continue_at(CHKDATA1); /* TCF CHKDATA1 */
}

/* MSKDATR1 +8 -- LM p. 292: GOXDSPFR's immediate return, with the display
 * up: "BLANKS R2 & R3" (REVCNT, bits 2 and 3) and end the job. */
void mskdatr1_immediate_return(void)
{
    set_A(REVCNT);
    tc(MSKDATR1 + 9u, BLANKET);
}

/* MSKDATR1 +10 -- LM p. 292. */
void mskdatr1_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* FORCEONE -- LM p. 292: bit 13. */
void forceone(void)
{
    set_A(BIT13);
    ads(ADDRESS_OF(DAPDATR1));
    continue_at(MSKDATR1); /* TCF MSKDATR1 */
}

/* DPDAT1 -- LM pp. 292-293.
 * "INHINT FOR SETTING OF FLAG BITS AND MASS ON BASIS OF DISPLAYED
 * DAPDATR1" (STIKLOAD allows them again):
 *   - "SET APSFLAG TO BE COMPLEMENT OF BIT 14";
 *   - "SET BITS OF DAPBOOLS ON BASIS OF DISPLAY", "MASK OUT CSMDOCKD (BIT
 *     13) UNLESS BOTH 13 AND 14 ARE SET";
 *   - "LOAD MASS IN ACCORDANCE WITH CSMDOCKD" (the LM's mass, and the
 *     CSM's when docked), "MASS IS USUALLY OKAY, SO DO NOT TOUCH ITS
 *     LOW-ORDER PART";
 *   - NJETSFLAG (FLAGWRD1's bit 15) 0 for 4 jets of X translation, 1 for
 *     2 (ACC4OR2X 1 for 4);
 *   - "SELECT DESIRED KALCMANU AUTOMATIC MANEUVER RATE": RATEINDX twice
 *     DAPBOOLS's low two bits, "0,2,4,6 SINCE RATES ARE DP".
 * Then POSTJUMP to STIKLOAD in bank 01. */
void dpdat1(void)
{
    inhint();
    set_A(negate(APSFLBIT)); /* CS APSFLBIT */
    mask_at(ADDRESS_OF(FLGWRD10));
    ts(L_ADDRESS);
    cs(ADDRESS_OF(DAPDATR1));
    mask(BIT14);
    if (masked_is_set()) set_A(APSFLBIT);
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(FLGWRD10));

    cs(ADDRESS_OF(DAPDATR1));
    mask(BIT13_14);
    if (masked_is_set()) set_A(negate(CSMDOCKD)); /* CS CSMDOCKD */
    ad(BOOLSMSK);
    mask_at(ADDRESS_OF(DAPDATR1));
    ts(L_ADDRESS);
    set_A(negate(BOOLSMSK)); /* CS BOOLSMSK */
    mask_at(ADDRESS_OF(DAPBOOLS));
    ad_at(L_ADDRESS);
    ts(ADDRESS_OF(DAPBOOLS));

    mask(CSMDOCKD);
    if (masked_is_set()) ca(ADDRESS_OF(CSMMASS));
    ad_at(ADDRESS_OF(LEMMASS));
    if (!ts(ADDRESS_OF(MASS))) /* an overflow skips the CAE */
        ca(ADDRESS_OF(DAPBOOLS));
    mask(ACC4OR2X);
    if (A_is_zero()) { /* EXTEND; BZF +5: "SET NJTSFLAG TO 1 FOR 2 JETS" */
        cs(ADDRESS_OF(FLAGWRD1));
        mask(BIT15);
        ads(ADDRESS_OF(FLAGWRD1));
    } else {                  /* "CLEAR NJTSFLAG TO 0 FOR 4 JETS" */
        set_A(negate(BIT15)); /* CS BIT15 */
        mask_at(ADDRESS_OF(FLAGWRD1));
        ts(ADDRESS_OF(FLAGWRD1));
    }

    ca(ADDRESS_OF(DAPBOOLS));
    mask(THREE);
    double_A();
    ts(ADDRESS_OF(RATEINDX));
    tc(DPDAT1 + 051u, POSTJUMP); /* CADR STIKLOAD */
}

/* ---- The hand controller and noun 47, the masses (bank 01) ---- */

/* STIKLOAD -- LM pp. 293-294, in E-bank 6.
 * "SET STICK SENSITIVITY TO CORRESPOND TO A MAXIMUM COMMANDED RATE (AT 42
 * COUNTS) OF 20 D/S (NORMAL) OR 4 D/S (FINE), SCALED AT 45 D/S": FINE,
 * and NORMAL with it where DAPBOOLS's RHCSCALE bit is set. The
 * "LM-ONLY BREAKOUT LEVEL IS .6 D/S". "IF CSM-DOCKED, DIVIDE STICK
 * SENSITIVITY BY 10" and the breakout level is .3 D/S. Then "PROCEED TO
 * NOUN 47, MASS LOAD". */
void stikload(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    set_A(RHCSCALE);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (masked_is_set()) set_A(NORMAL);
    ad(FINE);
    ts(ADDRESS_OF(STIKSENS));
    set_A(MINUS_0_6_DEG_SEC);
    ts(ADDRESS_OF(MINUS_RATEDB));
    set_A(CSMDOCKD);
    mask_at(ADDRESS_OF(DAPBOOLS));
    if (!A_is_zero()) { /* EXTEND; BZF +7: "BRANCH IF CSM IS NOT DOCKED" */
        ca(ADDRESS_OF(STIKSENS));
        mp(ONE_TENTH);
        ts(ADDRESS_OF(STIKSENS));
        set_A(MINUS_0_3_DEG_SEC);
        ts(ADDRESS_OF(MINUS_RATEDB));
    }
    relint();
    dapdata2();
}

/* DAPDATA2 -- LM p. 294: V06 N47, flashing: the LM's and the CSM's
 * masses. */
void dapdata2(void)
{
    show(DAPDATA2, V0647); /* CADR GOXDSPFR */
}

/* DAPDATA2 +3 -- LM p. 294: V34, terminate: "FIRST SET DB. DO 1/ACCS". */
void dapdata2_after_terminate(void)
{
    continue_at(ENDR03); /* TCF ENDR03 */
}

/* DAPDATA2 +4 -- LM p. 294: V33, proceed. */
void dapdata2_after_proceed(void)
{
    continue_at(DAPDAT2); /* TCF DAPDAT2 */
}

/* DAPDATA2 +5 -- LM p. 294: "LOAD NEW DATA AND RECYCLE". */
void dapdata2_after_enter(void)
{
    continue_at(DAPDATA2); /* TCF DAPDATA2 */
}

/* DAPDATA2 +6 -- LM p. 294: the immediate return: "BLANKS R3". */
void dapdata2_immediate_return(void)
{
    set_A(BIT3);
    tc(DAPDATA2 + 7u, BLANKET);
}

/* DAPDATA2 +8 -- LM p. 294. */
void dapdata2_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* ENDR03 -- LM p. 294: with interrupts held, RESTORDB (the deadband and
 * the moments of inertia). */
void endr03(void)
{
    inhint();
    tc(ENDR03 + 1u, IBNKCALL); /* CADR RESTORDB */
}

/* ENDR03 +3 -- LM p. 294. */
void endr03_after_ibnkcall(void)
{
    continue_at(ENDEXT); /* TCF ENDEXT: "DOES RELINT" */
}

/* DAPDAT2 -- LM pp. 294-295.
 * "DETERMINE STAGE FROM APSFLAG": "LEMMASS MUST BE GREATER THAN EMPTY
 * LEM", both stages' least masses before staging, the ascent stage's
 * after (MINLMD and MINMINLM are their negatives). Docked, "CSMMASS MUST
 * BE GREATER THAN EMPTY CSM" too. Else "ASK FOR NEW MASSES". */
void dapdat2(void)
{
    cs(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (masked_is_set()) set_A(MINLMD);
    ad(MINMINLM);
    ad_at(ADDRESS_OF(LEMMASS));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DAPDATA2 */
        continue_at(DAPDATA2);
        return;
    }
    ca(ADDRESS_OF(DAPBOOLS));
    mask(CSMDOCKD);
    if (A_is_zero()) { /* EXTEND; BZF LEMALONE: "SKIP TEST ON CSMMASS IF NOT DOCKED" */
        continue_at(LEMALONE);
        return;
    }
    set_A(negate(MINCSM)); /* CS MINCSM: "TEST CSM MASS" */
    ad_at(ADDRESS_OF(CSMMASS));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DAPDATA2 */
        continue_at(DAPDATA2);
        return;
    }
    ca(ADDRESS_OF(CSMMASS)); /* "DOCKED: MASS = CSMMASS + LEMMASS" */
    lemalone();
}

/* LEMALONE -- LM p. 295.
 * "LEM ALONE: MASS = LEMMASS": the mass, low part zero, into MASS (DXCH),
 * then with interrupts held RESTORDB: "SET DEADBANK AND COMPUTE MOMENTS
 * OF INERTIA". */
void lemalone(void)
{
    ad_at(ADDRESS_OF(LEMMASS));
    zl();
    dxch(ADDRESS_OF(MASS));
    inhint();
    tc(LEMALONE + 4u, IBNKCALL); /* CADR RESTORDB */
}

/* LEMALONE +6 -- LM p. 295: "PROCEED TO NOUN 48 (OR END)". */
void lemalone_after_ibnkcall(void)
{
    relint();
    dapdata3();
}

/* ---- Noun 48: the trims (bank 01) ---- */

/* DAPDATA3 -- LM p. 295: "END ROUTINE IF LEM HAS STAGED". */
void dapdata3(void)
{
    cs(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    if (A_is_zero()) { /* EXTEND; BZF ENDEXT */
        continue_at(ENDEXT);
        return;
    }
    dapdata3_plus4();
}

/* DAPDATA3 +4 -- LM p. 295: "DISPLAY TRIM ANGLES AND REQUEST RESPONSE",
 * V06 N48, flashing. */
void dapdata3_plus4(void)
{
    show(DAPDATA3 + 4u, V06N48); /* CADR GOXDSPFR */
}

/* DAPDATA3 +7 -- LM p. 295: V34, terminate. */
void dapdata3_after_terminate(void)
{
    tc(DAPDATA3 + 7u, ENDEXT);
}

/* DAPDATA3 +8 -- LM p. 295: V33, proceed: "GO DO TRIM (WAITLIST TO
 * TRIMGIMB)". */
void dapdata3_after_proceed(void)
{
    continue_at(DPDAT3); /* TCF DPDAT3 */
}

/* DAPDATA3 +9 -- LM p. 295: "LOAD NEW DATA AND RECYCLE". */
void dapdata3_after_enter(void)
{
    continue_at(DAPDATA3 + 4u); /* TCF -5 */
}

/* DAPDATA3 +10 -- LM p. 295: the immediate return: "BLANK R3". */
void dapdata3_immediate_return(void)
{
    set_A(BIT3);
    tc(DAPDATA3 + 11u, BLANKET);
}

/* DAPDATA3 +12 -- LM p. 295. */
void dapdata3_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* DPDAT3 -- LM p. 295: "GO TO TRIMGIMB VIA WAITLIST SO IT CAN USE
 * FIXDELAY AND VARDELAY", in 10 ms (BIT1). */
void dpdat3(void)
{
    set_A(BIT1);
    inhint();
    tc(DPDAT3 + 2u, WAITLIST); /* 2CADR TRIMGIMB */
}

/* DPDAT3 +5 -- LM p. 295. */
void dpdat3_after_waitlist(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB: "DOES A RELINT" */
}

/* TRIMDONE -- LM p. 295: "TRIM IS FINISHED; PLEASE TERMINATE R03", V50
 * N48, flashing. */
void trimdone(void)
{
    show(TRIMDONE, V50N48); /* CADR GOMARK3R */
}

/* TRIMDONE +3, +4 and +5 -- LM p. 295: V34 terminate, and the others
 * end it too. */
void trimdone_after_terminate(void)
{
    tc(TRIMDONE + 3u, ENDEXT);
}

void trimdone_after_proceed(void)
{
    tc(TRIMDONE + 4u, ENDEXT);
}

void trimdone_after_enter(void)
{
    tc(TRIMDONE + 5u, ENDEXT);
}

/* TRIMDONE +6 -- LM p. 295: GOMARK3R's immediate return: OCT24, "BIT5 TO
 * CHANGE TO PERFORM, 3 TO BLANK 43". */
void trimdone_immediate_return(void)
{
    set_A(OCT24);
    tc(TRIMDONE + 7u, BLANKET);
}

/* TRIMDONE +8 -- LM p. 295. */
void trimdone_after_blanket(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}
#else
/* ---- DAPDISP (bank 43) ---- */

/* DAPDISP -- CM p. 253.
 * Verb 48, load the autopilot's data (R03): "CHECKFAIL AND RETURN IF
 * TVC". FLAGWRD6's bits 15 and 14 are 10 while the TVC autopilot runs:
 * "CS YIELDS 01", positive, and ALM/END. */
void dapdisp(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        continue_at(DAPDISP + 5u);
        return;
    }
    tc(DAPDISP + 4u, ALM_END); /* "RETURN IF TVC" */
}

/* DAPDISP +5 -- CM p. 253: "REQUIRE EXT VERB DISPLAY AVAILABLE AND SET
 * BUSY FLAG". */
void dapdisp_plus5(void)
{
    tc(DAPDISP + 5u, TESTXACT);
}

/* DAPDISP +6 -- CM p. 253. */
void dapdisp_after_testxact(void)
{
    tc(DAPDISP + 6u, BANKCALL); /* CADR DAPDISP1 */
}

/* ---- DAPDISP1 (bank 42) ---- */

/* DAPDISP1 -- CM p. 253: E-bank 6, and "LOWER PRIORITY TO 10". */
void dapdisp1(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    set_A(PRIO10);
    tc(DAPDISP1 + 3u, PRIOCHNG);
}

/* DONOUN46 -- CM p. 253, where PRIOCHNG returns.
 * "DISPLAY FLASHING V04,N46 (DISPLAY AUTOPILOT CONFIGURATION)": R1 and R2
 * are DAPDATR1 and DAPDATR2. */
void donoun46(void)
{
    show(DONOUN46, V04N46); /* CADR GOXDSPF */
}

/* DONOUN46 +3 -- CM p. 253: V34, terminate. "EXT. VBS GO TO ENDEXT, NOT
 * ENDOFJOB." */
void donoun46_after_terminate(void)
{
    tc(DONOUN46 + 3u, ENDEXT);
}

/* DONOUN46 +4 -- CM p. 253: V33, proceed. */
void donoun46_after_proceed(void)
{
    tc(DONOUN46 + 4u, DONOUN46 + 6u); /* TC +2 */
}

/* DONOUN46 +5 -- CM p. 253: enter: again. */
void donoun46_after_enter(void)
{
    tc(DONOUN46 + 5u, DONOUN46);
}

/* DONOUN46 +6 -- CM p. 253: MAXDBFLG up with DAPDATR1's bit 4 (MAXIN),
 * else down. */
void donoun46_plus6(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT4);
    if (masked_is_set()) {
        continue_at(MAXIN); /* TCF MAXIN */
        return;
    }
    tc(DONOUN46 + 10u, DOWNFLAG); /* ADRES MAXDBFLG */
}

/* MAXOUT -- CM p. 253: "UPON PROCEED RESPONSE, EXECUTE S41.2". */
void maxout(void)
{
    tc(MAXOUT, BANKCALL); /* CADR S41.2 */
}

/* DONOUN47 -- CM pp. 253-254: "DISPLAY FLASHING V06,N47 (DISPLAY CSM WGT..
 * LEM WGT.)". */
void donoun47(void)
{
    show(DONOUN47, V06N47); /* CADR GOXDSPF */
}

/* DONOUN47 +3 -- CM p. 254: V34, terminate. */
void donoun47_after_terminate(void)
{
    tc(DONOUN47 + 3u, ENDEXT);
}

/* DONOUN47 +4 -- CM p. 254: V33, proceed. */
void donoun47_after_proceed(void)
{
    tc(DONOUN47 + 4u, DONOUN47 + 6u); /* TC +2 */
}

/* DONOUN47 +5 -- CM p. 254: enter: again. */
void donoun47_after_enter(void)
{
    tc(DONOUN47 + 5u, DONOUN47);
}

/* DONOUN47 +6 -- CM p. 254.
 * "DO MASS PROPERTIES CALCULATION ONLY IF CONFIG = 1(CSM), 2(CSM/LM),
 * 6(CSM/LMA)": DAPDATR1's bits 14 and 13 (PRIO30) neither both clear
 * ("SKIP IF 0, 4") nor both set ("SKIP IF 3, 7"). Then with interrupts
 * held MASSPROP: "UPDATE IXX, IAVG, IAVG/TLX". */
void donoun47_plus6(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(PRIO30);
    if (A_is_zero()) { /* EXTEND; BZF DONOUN48 */
        continue_at(DONOUN48);
        return;
    }
    com();
    mask(PRIO30);
    if (A_is_zero()) { /* EXTEND; BZF DONOUN48 */
        continue_at(DONOUN48);
        return;
    }
    inhint();
    tc(DONOUN47 + 15u, IBNKCALL); /* CADR MASSPROP */
}

/* DONOUN47 +17 -- CM p. 254: "UPON PROCEED RESPONSE EXECUTE S40.14":
 * "COMPUTE RCS DAP STUFF". */
void donoun47_after_ibnkcall(void)
{
    relint();
    tc(DONOUN47 + 18u, BANKCALL); /* CADR S40.14 */
}

/* DONOUN48 -- CM p. 254: "DISPLAY FLASHING V06,N48 (DISPLAY PITCH TRIM,
 * YAW TRIM)". */
void donoun48(void)
{
    show(DONOUN48, V0648); /* CADR GOXDSPF */
}

/* DONOUN48 +3 and +4 -- CM p. 254: V34 terminate, or V33 proceed: "UPON
 * PROCEED RESPONSE, RELEASE EXTENDED VERB DESPLAY SYSTEM". */
void donoun48_after_terminate(void)
{
    tc(DONOUN48 + 3u, ENDEXT);
}

void donoun48_after_proceed(void)
{
    tc(DONOUN48 + 4u, ENDEXT);
}

/* DONOUN48 +5 -- CM p. 254: enter: again. */
void donoun48_after_enter(void)
{
    tc(DONOUN48 + 5u, DONOUN48);
}

/* MAXIN -- CM p. 254. */
void maxin(void)
{
    tc(MAXIN, UPFLAG); /* ADRES MAXDBFLG */
}

/* MAXIN +2 -- CM p. 254. */
void maxin_after_upflag(void)
{
    tc(MAXIN + 2u, MAXOUT);
}
#endif
