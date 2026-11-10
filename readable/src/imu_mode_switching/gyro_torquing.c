/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc, part 2: torquing the gyros (LM pages
 * 1319-1326, CM pages 1430-1437).
 *
 * "THE FOLLOWING ROUTINE TORQUES THE IRIGS ACCORDING TO DOUBLE PRECISION
 * INPUTS IN THE SIX REGISTERS BEGINNING AT THE ECADR ARRIVING IN A." The
 * gyros go one at a time, Y, Z then X, each a WAITLIST task (STRTGYRO);
 * LGYRO holds the ECADR and, in bits 13-14, how many are done. A command
 * of more than 16383 pulses goes out in trains of 8192 (8192AUG). "THE
 * UNSENT PORTION OF THE COMMAND IS LEFT INTACT IN THE INPUT COMMAND
 * REGISTERS."
 *
 * The commands are read as 1400 indexed, under the EBANK the ECADR
 * sets: the listing's "DCA 1400". */
#include "subroutines.h"

/* The switched-erasable address the commands are indexed from. */
#define E1400 01400u

/* GWAKE2 -- LM p. 1319, CM p. 1430: STRTGYRO in A centiseconds. "(IF A
 * JOB WAS PUT TO SLEEP, THE POWER SUPPLY IS LEFT ON BY THE WAKING JOB)." */
static void gwake2(void)
{
    tc_at(GWAKE2); /* TC WAITLIST; 2CADR STRTGYRO */
    continue_at(WAITLIST);
}

/* REGSLEEP -- LM p. 1320, CM p. 1431: the job sleeps until GWAKE. */
static void regsleep(void)
{
    set_A(LGWAKE); /* CAF LGWAKE: CADR GWAKE */
    continue_at(JOBSLEEP);
}

/* IMUPULSE, GYROBUSY -- LM pp. 1319-1320, CM pp. 1430-1431.
 * "SAVE ARRIVING ECADR." "DON'T PROCEED IF IMU BEING CAGED." "SEE IF
 * GYROS BUSY.": if so GYROBUSY, "SAVE RETURN 2FCADR." (BUF2 into MPAC)
 * and sleep; if not "ENABLE THE POWER SUPPLY." and STRTGYRO in 40 ms. */
void imupulse(void)
{
    if (!ts(ADDRESS_OF(MPAC) + 5u)) { /* an overflow skips the TC */
        tc_at((word)(IMUPULSE + 1u)); /* TC CAGETSTJ */
        if (!cagetstj()) return;
    }
    switch (ccs_at(ADDRESS_OF(LGYRO))) {
    case POSITIVE:
        tc_at((word)(IMUPULSE + 3u)); /* TC GYROBUSY */
        dca(ADDRESS_OF(BUF2));
        dxch(ADDRESS_OF(MPAC));
        regsleep();
        return;
    case IS_PLUS_ZERO: ts(ADDRESS_OF(MPAC) + 2u); /* fall through */
    case NEGATIVE: set_A(BIT6); break;
    case IS_MINUS_ZERO: break; /* to the EXTEND before WOR */
    }
    wor_channel(CHAN14);
    set_A(FOUR);
    gwake2();
}

/* GYROAGRE -- LM p. 1319, CM p. 1430.
 * "FORCE SIGN AGREEMENT ON INPUTS.": for each gyro (MPAC +3 counting 2,
 * 1, 0) its command into MPAC, and TPAGREE. */
static void gyroagre(void)
{
    ts(ADDRESS_OF(MPAC) + 3u);
    double_A();
    ad_at(ADDRESS_OF(ITEMP1));
    ts(ADDRESS_OF(MPAC) + 4u);
    dca(indexed(E1400, a_word())); /* EXTEND; INDEX A; DCA 1400 */
    dxch(ADDRESS_OF(MPAC));
    tc_at((word)(GYROAGRE + 010u)); /* TC TPAGREE */
    continue_at(TPAGREE);
}

/* GWAKE2 +3, where WAITLIST returns.
 * "SET UP EBANK, SAVING CALLER'S EBANK FOR RESTORATION ON RETURN." The
 * ECADR to LGYRO ("RESERVES GYROS."), its low eight bits to ITEMP1. */
void gwake2_after_waitlist(void)
{
    ca(ADDRESS_OF(MPAC) + 5u);
    xch(EBANK_ADDRESS);
    xch(ADDRESS_OF(MPAC) + 5u);
    ts(ADDRESS_OF(LGYRO));
    mask(LOW8);
    ts(ADDRESS_OF(ITEMP1));
    set_A(TWO);
    gyroagre();
}

/* GYROAGRE +011, where TPAGREE returns: the command back, the next gyro,
 * and at the end "RESTORE CALLER'S EBANK." and MODEEXIT. */
void gyroagre_after_tpagree(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(indexed(E1400, MPAC(4))); /* INDEX MPAC +4; DXCH 1400 */
    switch (ccs_at(ADDRESS_OF(MPAC) + 3u)) {
    case POSITIVE: gyroagre(); return;            /* TCF GYROAGRE */
    case IS_PLUS_ZERO: ca(ADDRESS_OF(MPAC) + 5u); /* fall through */
    case NEGATIVE: ts(EBANK_ADDRESS); break;
    case IS_MINUS_ZERO: break; /* to TCF MODEEXIT */
    }
    modeexit();
}

/* GWAKE -- LM p. 1320, CM p. 1431: JOBWAKE's job.
 * "WHEN AWAKENED, SEE IF GYROS STILL BUSY." "IF SO, SLEEP SOME MORE."
 * Otherwise "RESTORE SWRETURN INFO." (MPAC into BUF2), and STRTGYRO in
 * 10 ms. */
void gwake(void)
{
    switch (ccs_at(ADDRESS_OF(LGYRO))) {
    case POSITIVE: regsleep(); return;
    case IS_PLUS_ZERO:
        ts(ADDRESS_OF(MPAC) + 2u);
        dca(ADDRESS_OF(MPAC));
        break;
    case NEGATIVE: dca(ADDRESS_OF(MPAC)); break;
    case IS_MINUS_ZERO: ca(ADDRESS_OF(MPAC) + 1u); break; /* the DCA word, unextended */
    }
    dxch(ADDRESS_OF(BUF2));
    set_A(ONE);
    gwake2();
}

/* ======================================================================
 * The WAITLIST tasks
 * ====================================================================== */

/* LASTSEG +1 -- LM p. 1324, CM p. 1435: the "WAITLIST DT" for GYROCMD's
 * pulses, plus three ("TRUNCATION AND PHASE UNCERTAINTIES."), and
 * STRTGYRO then. */
static void lastseg_plus1(void)
{
    mp(BIT10);
    ad(THREE);
    tc_at((word)(LASTSEG + 4u)); /* TC WAITLIST; 2CADR STRTGYRO */
    continue_at(WAITLIST);
}

/* AUG3 -- LM p. 1324, CM p. 1435: "GET WAITLIST DT TO TIME WHEN TRAIN IS
 * ALMOST OUT.", and 8192AUG then. */
static void aug3(void)
{
    mp(BIT10);
    ad(NEG3);
    tc_at((word)(AUG3 + 3u)); /* TC WAITLIST; 2CADR 8192AUG */
    continue_at(WAITLIST);
}

/* LONGGYRO -- LM p. 1324, CM p. 1435: "INITIAL COMMAND OUT PLUS N
 * AUGMENTS OF 8192.  INITIAL COMMAND IS AT LEAST 8192." */
static void longgyro(void)
{
    dxch(indexed(E1400, ITEMP1)); /* INDEX ITEMP1; DXCH 1400 */
    set_A(BIT14);
    ad_at(ADDRESS_OF(ITEMP2));
    ts(ADDRESS_OF(GYROCMD));
    aug3();
}

/* MAJ+ -- LM p. 1323, CM p. 1434: "DEFINITE POSITIVE OUTPUT." The
 * command plus GYROFRAC (rounding), "SELECT POSITIVE TORQUING FOR THIS
 * GYRO.", and "LEAVE NUMBER OF POSSIBLE 8192 AUGMENTS TO INITIAL COMMAND
 * IN MAJOR PART OF LONG TERM STORAGE AND TRUNCATED FRACTION IN MINOR
 * PART." */
static void maj_plus(void)
{
    dca(GYROFRAC);
    das(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP4));
    wor_channel(CHAN14);
    set_A(LOW7);
    mask_at(ADDRESS_OF(RUPTREG2));
    xch(ADDRESS_OF(RUPTREG2));
}

/* MAJ- -- LM p. 1326, CM p. 1437: "DEFINITE NEGATIVE OUTPUT." The same
 * for a negative command, with bit 9 of channel 14 for the sign: "ALL
 * NUMBERS PUT INTO GYROCMD ARE POSITIVE". */
static void maj_minus(void)
{
    dcs(GYROFRAC);
    das(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(ITEMP4));
    ad(BIT9);
    wor_channel(CHAN14);
    cs(ADDRESS_OF(RUPTREG1)); /* "SET UP RUPTREGS TO FALL INTO GMERGE." */
    ts(ADDRESS_OF(RUPTREG1));
    cs(ADDRESS_OF(RUPTREG2));
    mask(LOW7);
    com();
    xch(ADDRESS_OF(RUPTREG2));
    com();
}

/* GSELECT, MIN+, MIN-, GMERGE -- LM pp. 1322-1326, CM pp. 1433-1437.
 * "SELECT GYRO." from the word after the caller's TC: "PACKED WORD
 * CONTAINS GYRO SELECT BITS AND INCREMENT TO LGYRO." Then "MOVE DP
 * COMMAND TO RUPTREGS FOR TESTING.": a command under 16 pulses (MIN+,
 * MIN-: "SEE IF AT LEAST 16 GYRO PULSES.") is skipped; one more than
 * 16383 pulses ("MORE THAN ONE PULSE TRAIN NEEDED") goes by LONGGYRO.
 * True if the next gyro is to be done (TCF STRTGYR2). */
static bool gselect(void)
{
    enum sign major;
    enum sign minor;

    set_A(read_word(Q)); /* INDEX Q; CAF 0 */
    ts(ADDRESS_OF(ITEMP4));
    mask(SEVEN);
    ad(BIT13);
    ads(ADDRESS_OF(LGYRO));
    if (!ts(EBANK_ADDRESS)) mask(LOW8);
    ts(ADDRESS_OF(ITEMP1));
    set_A(negate(SEVEN)); /* CS SEVEN */
    mask_at(ADDRESS_OF(ITEMP4));
    ts(ADDRESS_OF(ITEMP4));
    dca(indexed(E1400, ITEMP1)); /* EXTEND; INDEX ITEMP1; DCA 1400 */
    dxch(ADDRESS_OF(RUPTREG1));
    major = ccs_at(ADDRESS_OF(RUPTREG1));
    if (major == IS_PLUS_ZERO || major == IS_MINUS_ZERO) {
        minor = ccs_at(ADDRESS_OF(RUPTREG2));
        if (minor == IS_PLUS_ZERO || minor == IS_MINUS_ZERO) return true;
        ad(MINUS_GYROMIN);                     /* MIN+ or MIN- */
        if (A_is_zero_or_minus()) return true; /* EXTEND; BZMF STRTGYR2 */
        major = minor;
    }
    if (major == POSITIVE)
        maj_plus();
    else
        maj_minus();

    /* GMERGE: "THE MAJOR PART WILL BE COUNTED DOWN TO ZERO IN THE COURSE
     * OF PUTTING OUT THE ENTIRE COMMAND." */
    mp(BIT8);
    ts(ADDRESS_OF(ITEMP2));
    ca(ADDRESS_OF(RUPTREG1));
    mp(BIT9);
    ts(ADDRESS_OF(RUPTREG1));
    ca(L_ADDRESS);
    mp(BIT14);
    ads(ADDRESS_OF(ITEMP2)); /* "INITIAL COMMAND." */
    dca(ADDRESS_OF(RUPTREG1));
    ad(MINUS1);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: longgyro(); return false;
    case IS_PLUS_ZERO:
        /* The word -GYROMIN, run as an instruction: MASK 07601. (A +0
         * cannot come here: RUPTREG1 less one is +0 for no RUPTREG1.) */
        mask(fetch_fixed(ADDRESS_FIELD(MINUS_GYROMIN)));
        break;
    case NEGATIVE: break; /* TCF +4 */
    case IS_MINUS_ZERO:
        set_A(BIT14);
        ads(ADDRESS_OF(ITEMP2));
        set_A(ZERO);
        break;
    }
    dxch(indexed(E1400, ITEMP1)); /* +4: INDEX ITEMP1; DXCH 1400 */
    ca(ADDRESS_OF(ITEMP2));       /* "ENTIRE COMMAND." */
    ts(ADDRESS_OF(GYROCMD));      /* LASTSEG */
    lastseg_plus1();
    return false;
}

/* STRTGYRO, STRTGYR2 -- LM pp. 1321-1322, CM pp. 1432-1433.
 * The task: "DE-SELECT LAST GYRO.", and unless caged "JUMP ON PHASE
 * COUNTER IN BITS 13-14." of LGYRO (MP BIT4 leaves it doubled in A):
 * "DO Y GYRO.", "DO Z GYRO.", "DO X GYRO." ("SPECIAL ENTRY TO REGRESS
 * LGYRO FOR X."), or done: LGYRO +0, and "WAKE A POSSIBLE SLEEPING
 * JOB." An odd count lands on the packed word after a TC, which runs as
 * a TC into erasable. */
void strtgyro(void)
{
    word at;

    set_A(negate(GDESELCT)); /* CS GDESELCT */
    wand_channel(CHAN14);
    tc_at((word)(STRTGYRO + 3u)); /* TC CAGETEST */
    if (!cagetest()) return;
    for (;;) {
        ca(ADDRESS_OF(LGYRO)); /* STRTGYR2 */
        mp(BIT4);
        at = (word)(STRTGYR2 + 5u + a_word()); /* INDEX A; TCF +1 */
        switch (a_word()) {
        case 0: tc_at(at); break; /* TC GSELECT: "=0.  DO Y GYRO." */
        case 2: tc_at(at); break; /* TC GSELECT: "=1.  DO Z GYRO." */
        case 4:                   /* TC GSELECT -2: "=2.  DO X GYRO." */
            tc_at(at);
            set_A(negate(FOUR)); /* CS FOUR; ADS LGYRO */
            ads(ADDRESS_OF(LGYRO));
            break;
        case 1:
        case 3:
        case 5:
            tc_at(at);
            continue_at(fetch_fixed(at)); /* OCT 00202, 00302 or 00100 */
            return;
        default:
            if (a_word() == 6u) set_A(ZERO); /* "=3.  DONE" */
            ts(ADDRESS_OF(LGYRO));
            set_A(LGWAKE);
            tc_at((word)(STRTGYR2 + 016u)); /* TC JOBWAKE */
            continue_at(JOBWAKE);
            return;
        }
        if (!gselect()) return;
    }
}

/* NORESET -- LM p. 1321, CM p. 1432: where JOBWAKE returns. "DO NOT
 * RESET POWER SUPPLY": TCF IMUFINED. */
void noreset(void)
{
    imufined();
}

/* GYROEXIT -- LM p. 1324, CM p. 1435: where LASTSEG's WAITLIST returns.
 * Channel 14 bit 10, and TASKOVER. */
void gyroexit(void)
{
    set_A(BIT10);
    wor_channel(CHAN14);
    continue_at(TASKOVER);
}

/* AUG3 +6, where WAITLIST returns: TCF GYROEXIT. */
void aug3_after_waitlist(void)
{
    gyroexit();
}

/* 8192AUG, AUG2 -- LM pp. 1324-1325, CM pp. 1435-1436.
 * The task, unless caged or in coarse align (channel 12 bit 4: IMUBAD):
 * "ADD 8192 PULSES TO GYROCMD" and "SEE IF THIS IS THE LAST AUG.": the
 * command's major part counts the trains left. "MORE TO COME." is AUG2
 * (one less, and AUG3 to "COMPUTE DT."); the last goes by LASTSEG +1. */
void eighty_one_ninety_two_aug(void)
{
    tc_at(EIGHT_ONE_NINETY_TWO_AUG); /* TC CAGETEST */
    if (!cagetest()) return;
    set_A(BIT4);
    rand_channel(CHAN12);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        imubad();
        return;
    }
    ca(ADDRESS_OF(LGYRO));
    ts(EBANK_ADDRESS);
    mask(LOW8);
    ts(ADDRESS_OF(ITEMP1));
    switch (ccs_at(indexed(E1400, ITEMP1))) { /* INDEX ITEMP1; CCS 1400 */
    case POSITIVE:
        ts(indexed(E1400, ITEMP1)); /* AUG2 */
        set_A(BIT14);
        ads(ADDRESS_OF(GYROCMD));
        aug3();
        return;
    case IS_PLUS_ZERO: set_A(BIT14); /* fall through */
    case NEGATIVE: ads(ADDRESS_OF(GYROCMD)); break;
    case IS_MINUS_ZERO: break; /* to TCF LASTSEG +1 */
    }
    lastseg_plus1();
}
