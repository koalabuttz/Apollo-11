/* SPDX-License-Identifier: MIT */
/* CM_ENTRY_DIGITAL_AUTOPILOT.agc, part F (Comanche055 pp. 1085-1089, bank
 * 15): the 2 s roll system's jets, after CM/RCS (part E) has its two
 * burns, TON1 and TON2, and their jets' directions, JNDX and JNDX1.
 *
 * GETOFF finds the coasting time between them, TOFF, and scales the three
 * times to centiseconds. TIMETST then makes them a list: "TIMETST
 * ESTABLISHES 6 LOCS CONTAINING JET CODES AND CORRESPONDING TIME
 * INTERVALS.  THUS:  TON1, T1BITS," "TOFF, TBITS, TON2, T2BITS." It
 * "ASSURES THAT WTLST CALLS ARE MADE ONLY" "(1) FOR POS INTERVALS GREATER
 * THAN A SPECIFIED MINIMUM (HERE CHOSEN AS 2 CS) AND" "(2) FOR THE
 * INTERVALS THAT WILL BE EXECUTED WITHIN THE TIME REMAINING IN THE SAMPLE
 * INTERVAL T (2 SEC)."
 *
 * JETCALL works down the list: "A POSITIVE NZ NUMBER IN A TIME REGISTER
 * INDICATES THAT A WTLST CALL IS TO BE MADE, AND ITS JET BITS" "EXECUTED.
 * A +0 INDICATES THAT THE TIME INTERVAL DOES NOT APPLY, BUT THE
 * CORRESPONDING JET BITS ARE TO BE" "EXECUTED.  A NEG NUMBER INDICATES
 * THAT THE TIME INTERVAL HAS BEEN PROCESSED." Each WAITLIST task comes
 * back at JETCALL for the next interval.
 *
 * DZCALL is the dead zone's way out: no roll jets, and the roll to hold. */
#include "entry_dap.h"
#include "../agc/names.h"

#if COMANCHE055

static void getoff2(void);
static void timscal(void);
static void timetst(void);
static void timetst1(void);
static void tofftest(void);
static void ton2_ts(void);
static void ton2_interval(void);
static void jetactn_write(void);
static void jetcall_outtag(void);

/* ---- GETOFF -- pp. 1085-1086 ---- */

/* GETOFF: "TON2 / 4T" complemented, */
void getoff(void)
{
    cs(ADDRESS_OF(TON2));
    getoff_mp();
}

/* GETOFF +1: times "VDT/180, OR VT/180, OR VMT/180.", */
void getoff_mp(void)
{
    mp_at(ADDRESS_OF(VDT_180)); /* EXTEND; MP VDT/180 */
    getoff_ts();
}

/* GETOFF +3: to XD/360, "USE AS TEMP" (an overflow skips the CS
 * VDT/180: A +1 or -1); "OMIT THE DIVIDE IF DEN = 0." (BZF TOFFOVFL);
 * else -VT/180 added, */
void getoff_ts(void)
{
    if (!ts(ADDRESS_OF(XD_360))) cs(ADDRESS_OF(VDT_180));
    if (A_is_zero()) { /* EXTEND; BZF TOFFOVFL */
        toffovfl();
        return;
    }
    ad_at(ADDRESS_OF(MVT_180));
    getoff_mp_ton1();
}

/* GETOFF +010: times "TON1 /4T", plus the temporary, "TEMP = -VDT/180 / 2
 * TON2", and LCX/360: the numerator, to L (ZL; XCH L). "TEST THE DIVIDE"
 * (DV VDT/180 of +0 and it): a quotient of +0, "DIVIDE OK" (GETOFF2);
 * else TOFFOVFL. */
void getoff_mp_ton1(void)
{
    mp_at(ADDRESS_OF(TON1)); /* EXTEND; MP TON1 */
    ad_at(ADDRESS_OF(XD_360));
    ad_at(ADDRESS_OF(LCX_360));
    zl();
    xch(L_ADDRESS);
    dv_at(ADDRESS_OF(VDT_180)); /* EXTEND; DV VDT/180 */
    if (A_is_zero()) {          /* EXTEND; BZF GETOFF2 */
        getoff2();
        return;
    }
    toffovfl();
}

/* TOFFOVFL: "OVFL, USE  2T  FOR CONVENIENCE." (2JETT, 4SECS); on at
 * TIMSCAL. */
void toffovfl(void)
{
    set_A(TWO_JETT); /* CA 2JETT */
    timscal();
}

/* GETOFF2: "GET NUMERATOR." (XCH L), over VDT/180, "C(A) = TOFF / 2T";
 * times 2JETT, */
static void getoff2(void)
{
    xch(L_ADDRESS);
    dv_at(ADDRESS_OF(VDT_180)); /* EXTEND; DV VDT/180 */
    mp(TWO_JETT);               /* EXTEND; MP 2JETT */
    timscal();
}

/* TIMSCAL: TOFF, "IN CS"; and 4JETT, */
static void timscal(void)
{
    ts(ADDRESS_OF(TOFF));
    set_A(FOUR_JETT); /* CAF 4JETT */
    timscal_ton1();
}

/* TIMSCAL +2: times "C(TON1) = TON1 / 4T", TON1 "IN CS", and the same for
 * TON2. "CANNOT REDO AFTER TIMETST.  TUSED GONE": JETAG +0, "SET +0 TO
 * SHOW ROLL DAP CALLED."; SW/NDX 1: "CAUSE THE TM OF BODY RATES VIA UPBUFF
 * TO BE" "INITIALIZED. ALSO CAUSE NEEDLES TO BE DONE ON EXIT" "AND ON
 * ALTERNATE PASSES THROUGH CM/DUMPR." On at TIMETST. */
void timscal_ton1(void)
{
    mp_at(ADDRESS_OF(TON1)); /* EXTEND; MP TON1 */
    ts(ADDRESS_OF(TON1));
    set_A(FOUR_JETT);        /* CAF 4JETT */
    mp_at(ADDRESS_OF(TON2)); /* EXTEND; MP TON2 */
    ts(ADDRESS_OF(TON2));
    set_A(ZERO); /* CA ZERO */
    ts(ADDRESS_OF(JETAG));
    set_A(ONE); /* CA ONE */
    ts(ADDRESS_OF(SW_NDX));
    timetst();
}

/* ---- TIMETST -- pp. 1087-1088 ---- */

/* TIMETST: TIME1, "CORRECT FOR POSSIBLE TIME1 OVFL." (POS1/2 twice, "OVFL
 * GUARANTEED."), added to TUSED, "B(TUSED) =-TUSED =-OLTIME1"; then -T-3,
 * "=-T +2 -5 (SEE SETJTAG)": "TUSED = TIME(K)-TIME(K-1)-T+2". TON1 less
 * 2, "USE 2 SINCE TIME3 UNCERTAIN TO 1": not positive, TIMETST1. Else its
 * jets (INDEX JNDX; CAF P/RJCODE) to T1BITS, and TON1 added to TUSED: not
 * positive (it ends in time), TOFFTEST; else TON1 +0 and the others -1
 * (on at TIMETST3, a key of another of the faithful port's parts). */
static void timetst(void)
{
    ca(ADDRESS_OF(TIME1));
    ad(POS1_2);
    ad(POS1_2);
    ads(ADDRESS_OF(TUSED));
    set_A(MINUS_T_3); /* CA -T-3 */
    ads(ADDRESS_OF(TUSED));
    set_A(negate(TWO)); /* CS TWO */
    ad_at(ADDRESS_OF(TON1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TIMETST1 */
        timetst1();
        return;
    }
    index_ca(ADDRESS_OF(JNDX), P_RJCODE); /* INDEX JNDX; CAF P/RJCODE */
    ts(ADDRESS_OF(T1BITS));
    ca(ADDRESS_OF(TON1));
    ads(ADDRESS_OF(TUSED));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TOFFTEST */
        tofftest();
        return;
    }
    set_A(ZERO); /* CA ZERO */
    continue_at(TIMETST3);
}

/* TIMETST1: TON1 -1, done; */
static void timetst1(void)
{
    set_A(negate(ONE)); /* CS ONE */
    ts(ADDRESS_OF(TON1));
    tofftest();
}

/* TOFFTEST: the same for TOFF (TIMETST2: -1, done): TOFF added to TUSED,
 * not positive, on at TON2TEST; else TOFF +0 and TON2 -1 (TIMETST4). Each
 * way on at a key of another of the faithful port's parts. */
static void tofftest(void)
{
    set_A(negate(TWO)); /* CS TWO */
    ad_at(ADDRESS_OF(TOFF));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TIMETST2 */
        set_A(negate(ONE));     /* TIMETST2: CS ONE */
        ts(ADDRESS_OF(TOFF));
        continue_at(TON2TEST);
        return;
    }
    ca(ADDRESS_OF(TOFF));
    ads(ADDRESS_OF(TUSED));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TON2TEST */
        continue_at(TON2TEST);
        return;
    }
    set_A(ZERO); /* CA ZERO */
    continue_at(TIMETST4);
}

/* TON2TEST: and for TON2: not past 2 cs, TIMETST5 (-1); */
void ton2test(void)
{
    set_A(negate(TWO)); /* CS TWO */
    ad_at(ADDRESS_OF(TON2));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TIMETST5 */
        timetst5();
        return;
    }
    ton2test_t2bits();
}

/* TON2TEST +4: else its jets (INDEX JNDX1; CAF P/RJCODE) to T2BITS, and
 * TON2 added to TUSED: not positive, JETCALL1; */
void ton2test_t2bits(void)
{
    index_ca(ADDRESS_OF(JNDX1), P_RJCODE); /* INDEX JNDX1; CAF P/RJCODE */
    ts(ADDRESS_OF(T2BITS));
    ca(ADDRESS_OF(TON2));
    ads(ADDRESS_OF(TUSED));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF JETCALL1 */
        jetcall1();
        return;
    }
    ton2test_zero();
}

/* TON2TEST +013: else TON2 +0 (CA ZERO; TCF TIMETST5 +1). */
void ton2test_zero(void)
{
    set_A(ZERO);
    ton2_ts();
}

/* TIMETST3: TON1 (an overflow skips the CS ONE: A +1 or -1); TOFF -1, */
void timetst3(void)
{
    if (!ts(ADDRESS_OF(TON1))) set_A(negate(ONE)); /* CS ONE */
    timetst4();
}

/* TIMETST4: TOFF (an overflow skips TIMETST5's CS ONE); TON2 -1, */
void timetst4(void)
{
    if (!ts(ADDRESS_OF(TOFF))) {
        timetst5();
        return;
    }
    ton2_ts();
}

/* TIMETST5: TON2 -1, */
void timetst5(void)
{
    set_A(negate(ONE)); /* CS ONE */
    ton2_ts();
}

/* TIMETST5 +1: to TON2; on at JETCALL1. */
static void ton2_ts(void)
{
    ts(ADDRESS_OF(TON2));
    jetcall1();
}

/* ---- JETCALL -- pp. 1088-1089 ---- */

/* JETCALL1: OUTTAG, NUJET and TBITS +0; TON1 and T1BITS to A and L (DXCH),
 * and CCS A: positive, "CALL WTLST" (JETCALL2); +0, its bits taken
 * (JETCALL3); -1 or less, done (JETCALL3 +1); -0, JETCALL3 +2. */
void jetcall1(void)
{
    set_A(ZERO); /* CA ZERO */
    ts(ADDRESS_OF(OUTTAG));
    ts(ADDRESS_OF(NUJET));
    ts(ADDRESS_OF(TBITS));
    dxch(ADDRESS_OF(TON1));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: jetcall2(); break; /* TCF JETCALL2 */
    case IS_PLUS_ZERO: jetcall3(); break;
    case NEGATIVE: jetcall3_cs(); break;
    case IS_MINUS_ZERO: jetcall3_toff(); break;
    }
}

/* JETCALL3: "WTLST ENTRIES COME HERE FROM JETCALL": the bits to NUJET
 * (LXCH), */
void jetcall3(void)
{
    lxch(ADDRESS_OF(NUJET));
    jetcall3_cs();
}

/* JETCALL3 +1: -1 (CS ONE) and, */
void jetcall3_cs(void)
{
    set_A(negate(ONE));
    jetcall3_toff();
}

/* JETCALL3 +2: exchanged with TOFF and TBITS, which -1 marks done; CCS A
 * on TOFF the same way: positive, "CALL WTLST"; +0, its bits to NUJET;
 * -0 skips the CS ONE; then TON2. */
void jetcall3_toff(void)
{
    dxch(ADDRESS_OF(TOFF));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: jetcall2(); return; /* TCF JETCALL2 */
    case IS_PLUS_ZERO:
        lxch(ADDRESS_OF(NUJET));
        set_A(negate(ONE)); /* CS ONE */
        break;
    case NEGATIVE: set_A(negate(ONE)); break;
    case IS_MINUS_ZERO: break;
    }
    ton2_interval();
}

/* JETCALL3 +7: TON2 and T2BITS, CCS A: positive or -0, "CALL WTLST"; +0,
 * its bits to NUJET; then (and for -1 or less) "C(A) = +0" (TC JETACTN,
 * the list done). */
static void ton2_interval(void)
{
    dxch(ADDRESS_OF(TON2));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: jetcall2(); return; /* TCF JETCALL2 */
    case IS_PLUS_ZERO: lxch(ADDRESS_OF(NUJET)); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: jetcall2(); return;
    }
    tc_at(JETCALL3 + 013u);
    jetactn();
}

/* JETCALL2: "SAVE JET BITS FOR AFTER WTLST CALL" (added to NUJET); the
 * time, less 1 from the CCS, "RESTORE FOR CCS" (AD ONE), to WAITLIST:
 * JETCALL in that many centiseconds (2CADR JETCALL); back at JETACTN. */
void jetcall2(void)
{
    xch(L_ADDRESS);
    ads(ADDRESS_OF(NUJET));
    xch(L_ADDRESS);
    ad(ONE);
    tc_to(JETCALL2 + 4u, WAITLIST);
}

/* JETACTN: "COME HERE WHEN DESIRED JET CODE IS KNOWN" (NUJET), */
void jetactn(void)
{
    ca(ADDRESS_OF(NUJET));
    jetactn_write();
}

/* JETACTN +1: "NO NEED TO SAVE OLD CODES", "SET RCHAN TO NEW BIT CONFIG."
 * (WRITE ROLLJETS). CCS OUTTAG: positive (a WAITLIST task's), the task
 * ends (TASKOVER); +0, ROLLDUMP; negative, on at JETCALL; -0, JETCALL +1
 * with A +0. */
static void jetactn_write(void)
{
    write_channel(6); /* EXTEND; WRITE ROLLJETS */
    switch (ccs_at(ADDRESS_OF(OUTTAG))) {
    case POSITIVE: tc_to(JETACTN + 4u, TASKOVER); break;
    case IS_PLUS_ZERO: rolldump(); break;
    case NEGATIVE: jetcall(); break;
    case IS_MINUS_ZERO: jetcall_outtag(); break;
    }
}

/* ROLLDUMP: TC CM/FDAIR (a key of another of the faithful port's parts). */
void rolldump(void)
{
    tc_to(ROLLDUMP, CM_FDAIR);
}

/* JETCALL: "WAITLIST ENTRIES COME HERE.": BIT2, "CM/DSTBY =103D BIT2", */
void jetcall(void)
{
    set_A(BIT2); /* CAF BIT2 */
    jetcall_outtag();
}

/* JETCALL +1: to OUTTAG, "SIGNIFY WTLST ENTRY"; "IS SYSTEM DISABLED  $"
 * (MASK CM/FLAGS): "YES, QUENCH ROLL JETS, IF ON AND EXIT." (BZF JETACTN
 * +1, A +0); "NO, CONTINUE." (ZL), "C(A) POS, C(L) = +0": JETCALL3. */
static void jetcall_outtag(void)
{
    ts(ADDRESS_OF(OUTTAG));
    mask_at(ADDRESS_OF(CM_FLAGS));
    if (A_is_zero()) { /* EXTEND; BZF JETACTN +1 */
        jetactn_write();
        return;
    }
    zl();
    jetcall3();
}

/* ---- DZCALL -- p. 1089 ---- */

/* DZCALL: "DEAD ZONE ENTRIES COME HERE.": "POSSIBLE VALUES OF CMDAPMOD: -1,
 * +0, -0." Its complement's bit 1 to L: "C(L)=0 FOR -0: C(L)=1 FOR -1 OR
 * +0."; on at DZCALL +3 (a key of another of the faithful port's parts). */
void dzcall(void)
{
    cs(ADDRESS_OF(CMDAPMOD));
    mask(BIT1);
    ts(L_ADDRESS);
    continue_at(DZCALL + 3u);
}

#endif
