/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 2: the IMU monitor of T4RUPT_PROGRAM.agc, Luminary099 (LM,
 * listing pages 161-181) and Comanche055 (CM, pages 139-159). The
 * faithful C is c64-port/src/lm_t4rupt_imu.c and cm_t4rupt_imu.c.
 *
 * The missions run the same code but for a few words: where the LM keeps
 * the IMU-in-use flag (IMUSEBIT, FLAGWRD0 bit 8) the CM keeps it in STATE;
 * the LM's ISSUP also starts the free-fall IMU compensation (DRIFTFLG),
 * its ITURNON2 resets the radar's modes (RADMODES), and its IMUCAGE clears
 * ENGONFLG and turns the engine off its own way; the CM's LAMPTEST ends in
 * PINBALL's words.
 *
 * Channel 30 and IMODES30. Channel 30 holds the IMU's status inbits; a 0
 * is the signal present ("ISS TURN-ON REQUEST", "IMU OPERATE", ...).
 * IMODES30 keeps them as last sampled (bits 15-9) and the monitor's flags:
 * bit 7 "wait one sample", bit 8 "second sample", bit 2 "turn-on request
 * failed", and the warning inhibits in bits 4, 3 and 1.
 *
 * The bit scans. IMUMON exclusive-ors channel 30 with IMODES30 for the
 * bits that changed, and works through them from the top (NXTIFBIT):
 * DOUBLE shifts the changes left until the top one overflows, RUPTREG1
 * counting positions; the rest wait in RUPTREG2. Position n is bit 14 - n,
 * and IFAILJMP's n-th word calls its routine with the bit's new value in
 * A and Q at NXTIFAIL, which goes on with the rest. C33TEST does the same
 * for channel 33's three flip-flops (C33JMP, NXTFL33).
 *
 * The binding (src/binding/t4rupt_imu_routines.c) runs these in bank 06,
 * continuing a scan only where its RUPTREG1 and RUPTREG2 are as the scans
 * leave them. */
#include "imu_monitor.h"

static void nxtifbit(void);
static void nxtifail(void);
static void tnontest(void);
static void proctnon(void);
static void retnon(void);
static void endtnon2(void);
static void issup_after_setissw(void);
static void oponly(void);
static void isszero(void);
static void c33test(void);
static void nxtibt(void);
static void nxtibt_plus1(void);
static void nxtfl33(void);
static void glockmon(void);
static void glockchk(void);
static void setglock(void);
static void glinvert(void);
static void glamptst(void);
static void tlim(void);
static void tempok(void);
static void iturnon(void);
static void iturnon2(void);
static void imucage(void);
static void imuop(void);
static void imuop2(void);
static void pipfail(void);
static void pipfail_after_setissw(void);
static void dntmfast(void);
static void uptmfast(void);
static void isswon(void);
static void cagesub(void);
static void cagesub1(void);
static void cagesub2(void);
static bool lamptest(void);

/* DSPTAB +11D, the word of the DSKY's lamps (bit 6 GIMBAL LOCK, bit 4 NO
 * ATT), as T4RUPT sends it. */
#define LAMPS (ADDRESS_OF(DSPTAB) + 11u)

/* A word MASKed to one bit is + or +0, so a CCS of it either goes on
 * (the bit set) or skips one word (clear). */
static bool ccs_A_positive(void)
{
    return ccs_at(A_ADDRESS) == POSITIVE;
}

/* ======================================================================
 * Channel 30
 * ====================================================================== */

/* IMUMON -- LM p. 161, CM p. 139.
 * "SEE IF THERE HAS BEEN A CHANGE IN THE RELEVANT BITS OF CHAN 30". With
 * none, go on to the turn-on sequence (TNONTEST). Else save the changes
 * (RUPTREG1), update IMODES30, and scan them: a change of bit 15 (the
 * temperature, the sign) first, at TLIM. */
void imumon(void)
{
    ca(ADDRESS_OF(IMODES30));
    rxor_channel(CHAN30); /* EXTEND; RXOR CHAN30 */
    mask(K30RDMSK);
    if (A_is_zero()) { /* EXTEND; BZF TNONTEST: "NO CHANGE IN STATUS" */
        tnontest();
        return;
    }
    ts(ADDRESS_OF(RUPTREG1));   /* "SAVE BITS WHICH HAVE CHANGED" */
    lxch(ADDRESS_OF(IMODES30)); /* "UPDATE IMODES30" */
    rxor_lchan();               /* EXTEND; RXOR LCHAN */
    ts(ADDRESS_OF(IMODES30));
    set_A(negate(ONE)); /* CS ONE: RUPTREG1 -1, "BEGIN BIT SCAN" */
    xch(ADDRESS_OF(RUPTREG1));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TLIM: "CHANGE IN IMU TEMP" */
        tlim();
        return;
    }
    nxtifbit();
}

/* NXTIFBIT -- LM p. 162, CM p. 140.
 * "ADVANCE BIT POSITION NUMBER" and DOUBLE until a change overflows (TS A
 * skips); "SAVE OVERFLOW-CORRECTED DATA" (the changes left) in RUPTREG2.
 * "SELECT NEW VALUE OF THIS BIT" (INDEX RUPTREG1; CAF BIT14, from the
 * table BIT15, BIT14 ... BIT1) from IMODES30, and call the bit's routine:
 * INDEX RUPTREG1; TC IFAILJMP, which returns to NXTIFAIL. */
static void nxtifbit(void)
{
    do {
        incr(ADDRESS_OF(RUPTREG1));
        double_A();
    } while (!ts(A_ADDRESS)); /* TS A: "SKIP IF OVERFLOW", else TCF NXTIFBIT */
    xch(ADDRESS_OF(RUPTREG2));
    ca(indexed(ADDRESS_OF(BIT14), RUPTREG1));
    mask_at(ADDRESS_OF(IMODES30));
    tc_at((word)(NXTIFAIL - 1u)); /* INDEX RUPTREG1; TC IFAILJMP */
    switch (indexed(IFAILJMP, RUPTREG1) - IFAILJMP) {
    case 0: iturnon(); break; /* bit 14, ISS TURN-ON REQUEST */
    case 1:                   /* bit 13: IMUFAIL EQUALS SETISSW */
    case 2:                   /* bit 12: ICDUFAIL EQUALS SETISSW */
        if (setissw()) nxtifail();
        break;
    case 3: imucage(); break;       /* bit 11, IMU CAGE */
    case 5: imuop(); break;         /* bit 9, IMU OPERATE */
    default: not_modelled(); break; /* 30RDMSK: bit 10 is never scanned */
    }
}

/* NXTIFAIL -- LM p. 162, CM p. 140.
 * "PROCESS ANY ADDITIONAL CHANGES": with RUPTREG2 positive, scan on
 * (NXTIFBIT -1 adds back the one CCS took off); with none left, fall into
 * TNONTEST. */
static void nxtifail(void)
{
    switch (ccs_at(ADDRESS_OF(RUPTREG2))) {
    case POSITIVE:
        ad(ONE); /* TCF NXTIFBIT -1: AD ONE */
        nxtifbit();
        return;
    case IS_PLUS_ZERO: tnontest(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); return; /* the changes left are never negative */
    }
}

/* TNONTEST -- LM p. 163-164, CM p. 141-142.
 * "AFTER PROCESSING ALL CHANGES, SEE IF IT IS TIME TO ACT ON A TURN-ON
 * SEQUENCE": a turn-on request or IMU OPERATE sets IMODES30 bit 7, and
 * the sequence starts at the second sample after it (bit 8 marks the
 * first). Then channel 33 (C33TEST). */
static void tnontest(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT7);
    if (ccs_A_positive()) { /* "NO -- EXAMINE CHANNEL 33" */
        c33test();
        return;
    }
    set_A(BIT8); /* "SEE IF FIRST SAMPLE OR SECOND" */
    mask_at(ADDRESS_OF(IMODES30));
    if (ccs_A_positive()) { /* "REACT AFTER A SECOND SAMPLE" */
        proctnon();
        return;
    }
    set_A(BIT8); /* "IF FIRST SAMPLE, SET BIT TO REACT NEXT TIME" */
    ads(ADDRESS_OF(IMODES30));
    c33test();
}

/* PROCTNON -- LM p. 164, CM p. 142.
 * "PROCESS IMU TURN-ON REQUESTS AFTER WAITING 1 SAMPLE FOR ALL SIGNALS TO
 * ARRIVE". Clear bits 7 and 8. With no turn-on request, only IMU OPERATE
 * (OPONLY). Else, "IF TURN-ON REQUEST, WE SHOULD HAVE IMU OPERATE": alarm
 * 213 if not; then cage the ISS for 90 seconds. */
static void proctnon(void)
{
    set_A(negate(BITS7_8)); /* CS BITS7&8 */
    mask_at(ADDRESS_OF(IMODES30));
    ts(ADDRESS_OF(IMODES30));
    mask(BIT14);            /* "SEE IF TURN-ON REQUEST" */
    if (ccs_A_positive()) { /* "OPERATE ON ONLY" */
        oponly();
        return;
    }
    cs(ADDRESS_OF(IMODES30));
    mask(BIT9);
    if (ccs_A_positive()) { /* TCF +3 */
        proctnon_after_alarm();
        return;
    }
    tc_at((word)(PROCTNON + 012u)); /* TC ALARM; OCT 213: "ALARM IF NOT" */
    continue_at(ALARM);
}

/* PROCTNON +014 ("+3"), where ALARM returns: CAGESUB, and ENDTNON as a
 * task in 90 seconds. */
void proctnon_after_alarm(void)
{
    tc_at((word)(PROCTNON + 014u)); /* TC CAGESUB */
    cagesub();
    set_A(K90SECS);                 /* CAF 90SECS */
    tc_at((word)(PROCTNON + 016u)); /* TC WAITLIST; 2CADR ENDTNON */
    continue_at(WAITLIST);
}

/* PROCTNON +021, where WAITLIST returns: TCF C33TEST. */
void proctnon_after_waitlist(void)
{
    c33test();
}

/* RETNON -- LM p. 164, CM p. 142.
 * "RE-ENTER 90 SEC DELAY IN WL": wait 90 seconds more, then ENDTNON again
 * (VARDELAY's return). */
static void retnon(void)
{
    set_A(K90SECS);             /* CAF 90SECS */
    tc_at((word)(RETNON + 1u)); /* TC VARDELAY */
    continue_at(VARDELAY);
}

/* ENDTNON -- LM p. 164-165, CM p. 142.
 * The end of the 90-second cage. "RESET TURN-ON REQUEST FAIL BIT"; "IF IT
 * WAS OFF, SEND ISS DELAY COMPLETE" (ENDTNON2). If it was on (the request
 * went away during the wait) and the request is back, wait again
 * (RETNON). Else end the task, or, if a program was waiting for the IMU,
 * tell it the turn-on failed: POSTJUMP to IMUBAD. */
void endtnon(void)
{
    set_A(negate(BIT2)); /* CS BIT2 */
    mask_at(ADDRESS_OF(IMODES30));
    xch(ADDRESS_OF(IMODES30));
    mask(BIT2);
    if (A_is_zero()) { /* EXTEND; BZF ENDTNON2 */
        endtnon2();
        return;
    }
    set_A(BIT14); /* "IF IT WAS ON AND TURN-ON REQUEST NOW PRESENT" */
    mask_at(ADDRESS_OF(IMODES30));
    if (A_is_zero()) { /* EXTEND; BZF RETNON */
        retnon();
        return;
    }
    /* "IF IT IS NOT ON NOW, SEE IF A PROG WAS WAITING" */
#if LUMINARY099
    cs(ADDRESS_OF(FLAGWRD0));
    mask(IMUSEBIT);
#else
    cs(ADDRESS_OF(STATE));
    mask(IMUSEFLG);
#endif
    if (ccs_A_positive()) {
        continue_at(TASKOVER); /* TCF TASKOVER */
        return;
    }
    tc_at((word)(ENDTNON2 - 2u)); /* TC POSTJUMP; CADR IMUBAD: "UNSUCCESSFUL TURN-ON" */
    continue_at(POSTJUMP);
}

/* ENDTNON2 -- LM p. 165, CM p. 143.
 * "SEND ISS DELAY COMPLETE" (channel 12 bit 15) and "TURN OFF NO ATT
 * LAMP" (IBNKCALL NOATTOFF, which goes on to zero the ICDUs: ISSZERO +2). */
static void endtnon2(void)
{
    set_A(BIT15);
    wor_channel(CHAN12);          /* EXTEND; WOR CHAN12 */
    tc_at((word)(ENDTNON2 + 3u)); /* TC IBNKCALL; CADR NOATTOFF */
    continue_at(IBNKCALL);
}

/* UNZ2 -- LM p. 165, CM p. 143.
 * A task, 300 ms after the ICDUs' zero began: zero the counters
 * (ZEROICDU), and come back to UNZ2 +1. */
void unz2(void)
{
    tc_at(UNZ2); /* TC ZEROICDU */
    continue_at(ZEROICDU);
}

/* UNZ2 +1: "REMOVE ZERO AND COARSE", and "WAIT 10 SECS FOR CTRS TO FIND
 * GIMBALS" (VARDELAY, then ISSUP). */
void unz2_after_zeroicdu(void)
{
    set_A(negate(BITS4_5)); /* CS BITS4&5 */
    wand_channel(CHAN12);
    set_A(BIT11);
    tc_at((word)(UNZ2 + 5u)); /* TC VARDELAY */
    continue_at(VARDELAY);
}

/* ISSUP -- LM p. 165-166, CM p. 143.
 * "REMOVE CAGING, IMU FAIL INHIBIT BIT, AND ICDUFAIL INHIBIT FLAGS", and
 * "ENABLE DAP". The LM then sets DRIFTFLG, if off, "AND 1/PIPADT TO GET
 * FREEFALL IMUCOMP GOING". Then SETISSW ("ISS WARNING MIGHT HAVE BEEN
 * INHIBITED"). */
void issup(void)
{
    set_A(negate(OCT54)); /* CS OCT54 */
    mask_at(ADDRESS_OF(IMODES30));
    ts(ADDRESS_OF(IMODES30));
    set_A(negate(BIT6)); /* CS BIT6 */
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
#if LUMINARY099
    cs(ADDRESS_OF(FLAGWRD2)); /* "TEST DRIFTFLG: IF ON DO NOTHING" */
    mask(DRFTBIT);
    if (!A_is_zero()) { /* EXTEND; BZF +4 */
        ads(ADDRESS_OF(FLAGWRD2));
        ca(ADDRESS_OF(TIME1));
        xch(ADDRESS_OF(1_PIPADT)); /* XCH 1/PIPADT */
    }
#endif
    tc_at(ISSUP_TC_SETISSW); /* TC SETISSW */
    if (setissw()) issup_after_setissw();
}

/* After ISSUP's SETISSW: "REMOVE IMU DELAY COMPLETE DISCRETE", and "DON'T
 * ENABLE PROG ALARM ON PIP FAIL FOR ANOTHER 4 SECS" (WAITLIST, PFAILOK). */
static void issup_after_setissw(void)
{
    set_A(negate(BIT15)); /* CS BIT15 */
    wand_channel(CHAN12);
    set_A(K4SECS);                        /* CAF 4SECS */
    tc_at((word)(ISSUP_TC_SETISSW + 5u)); /* TC WAITLIST; 2CADR PFAILOK */
    continue_at(WAITLIST);
}

/* Where WAITLIST returns: TCF TASKOVER. */
void issup_after_waitlist(void)
{
    continue_at(TASKOVER);
}

/* OPONLY -- LM p. 166, CM p. 143-144.
 * "IF OPERATE ON ONLY, AND WE ARE IN COARSE ALIGN, DON'T ZERO THE CDUS
 * BECAUSE WE MIGHT BE IN GIMBAL LOCK", nor "IF SOMEONE IS USING THE IMU".
 * Otherwise "SET TURNON FLAGS" (CAGESUB2) and zero them (ISSZERO). */
static void oponly(void)
{
    set_A(BIT4);
    rand_channel(CHAN12); /* EXTEND; RAND CHAN12 */
    if (ccs_A_positive()) {
        c33test();
        return;
    }
#if LUMINARY099
    set_A(IMUSEBIT);
    mask_at(ADDRESS_OF(FLAGWRD0));
#else
    set_A(IMUSEFLG);
    mask_at(ADDRESS_OF(STATE));
#endif
    if (ccs_A_positive()) {
        c33test();
        return;
    }
    tc_at((word)(ISSZERO - 1u)); /* TC CAGESUB2 */
    cagesub2();
    isszero();
}

/* ISSZERO -- LM p. 166, CM p. 144.
 * "TURN OFF NO ATT LAMP" (IBNKCALL NOATTOFF). Also IMUCAGE's "IMU CAGE OFF
 * ENTRY". */
static void isszero(void)
{
    tc_at(ISSZERO); /* TC IBNKCALL; CADR NOATTOFF */
    continue_at(IBNKCALL);
}

/* ISSZERO +2: "ISS CDU ZERO" (channel 12 bit 5), and zero the counters
 * (ZEROICDU). */
void isszero_after_noattoff(void)
{
    set_A(BIT5);
    wor_channel(CHAN12);
    tc_at((word)(ISSZERO + 5u)); /* TC ZEROICDU */
    continue_at(ZEROICDU);
}

/* ISSZERO +6: "WAIT 300 MS. FOR AGS TO RECEIVE SIGNAL", then UNZ2 as a
 * task. */
void isszero_after_zeroicdu(void)
{
    set_A(BIT6);
    tc_at((word)(ISSZERO + 7u)); /* TC WAITLIST; 2CADR UNZ2 */
    continue_at(WAITLIST);
}

/* ISSZERO +012, where WAITLIST returns: TCF C33TEST. */
void isszero_after_waitlist(void)
{
    c33test();
}

/* ======================================================================
 * Channel 33
 * ====================================================================== */

/* C33TEST -- LM p. 167, CM p. 145.
 * "SEE IF RELEVANT CHAN33 BITS HAVE CHANGED": PIPA FAIL (bit 13),
 * DOWNLINK TOO FAST (12) and UPLINK TOO FAST (11) are flip-flops, read
 * with WAND because "A `WRITE' PULSE IS REQUIRED TO RESET THE
 * FLIP-FLOPS". With no change, on to the gimbal lock monitor (GLOCKMON).
 * Else update IMODES33 and scan the changes as IMUMON does. */
static void c33test(void)
{
    ca(ADDRESS_OF(IMODES33));
    mask(K33RDMSK);
    ts(L_ADDRESS); /* TS L */
    set_A(K33RDMSK);
    wand_channel(CHAN33); /* EXTEND; WAND CHAN33: "RESETS FLIP-FLOP INPUTS" */
    rxor_lchan();         /* EXTEND; RXOR LCHAN */
    if (A_is_zero()) {    /* EXTEND; BZF GLOCKMON: "ON NO CHANGE" */
        glockmon();
        return;
    }
    ts(ADDRESS_OF(RUPTREG1)); /* "SAVE BITS WHICH HAVE CHANGED" */
    lxch(ADDRESS_OF(IMODES33));
    rxor_lchan();
    ts(ADDRESS_OF(IMODES33)); /* "UPDATED IMODES33" */
    set_A(ZERO);              /* CAF ZERO */
    xch(ADDRESS_OF(RUPTREG1));
    double_A();
    nxtibt_plus1(); /* TCF NXTIBT +1: "SCAN FOR BIT CHANGES" */
}

/* NXTIBT -- LM p. 168, CM p. 146.
 * "(CODING IDENTICAL TO CHAN 30)": the new value of the changed bit
 * (INDEX RUPTREG1; CAF BIT13) from IMODES33, to its routine through
 * C33JMP, which returns to NXTFL33. */
static void nxtibt(void)
{
    incr(ADDRESS_OF(RUPTREG1));
    nxtibt_plus1();
}

static void nxtibt_plus1(void)
{
    double_A();
    while (!ts(A_ADDRESS)) { /* TS A, else TCF NXTIBT */
        incr(ADDRESS_OF(RUPTREG1));
        double_A();
    }
    xch(ADDRESS_OF(RUPTREG2));
    ca(indexed(ADDRESS_OF(BIT13), RUPTREG1)); /* "GET NEW VALUE OF BIT WHICH CHANGED" */
    mask_at(ADDRESS_OF(IMODES33));
    tc_at((word)(NXTFL33 - 1u)); /* INDEX RUPTREG1; TC C33JMP */
    switch (indexed(C33JMP, RUPTREG1) - C33JMP) {
    case 0: pipfail(); break;       /* bit 13 */
    case 1: dntmfast(); break;      /* bit 12 */
    case 2: uptmfast(); break;      /* bit 11 */
    default: not_modelled(); break; /* past the table: channel 33 has three bits */
    }
}

/* NXTFL33 -- LM p. 168, CM p. 146.
 * "PROCESS POSSIBLE ADDITIONAL CHANGES", then fall into GLOCKMON. */
static void nxtfl33(void)
{
    switch (ccs_at(ADDRESS_OF(RUPTREG2))) {
    case POSITIVE:
        ad(ONE); /* TCF NXTIBT -1: AD ONE */
        nxtibt();
        return;
    case IS_PLUS_ZERO: glockmon(); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); return; /* the changes left are never negative */
    }
}

/* ======================================================================
 * Gimbal lock
 * ====================================================================== */

/* GLOCKMON -- LM p. 169, CM p. 147.
 * The middle gimbal angle (CDUZ): "SEE IF MAGNITUDE OF MGA IS GREATER
 * THAN 70 DEGREES" (GLOCKCHK); at +0 or -0, lamp off (SETGLOCK with A
 * +0). */
static void glockmon(void)
{
    switch (ccs_at(ADDRESS_OF(CDUZ))) {
    case POSITIVE:
    case NEGATIVE: glockchk(); return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: setglock(); return;
    }
}

/* GLOCKCHK -- LM p. 169, CM p. 147.
 * A is |MGA| less one bit. Up to 70 degrees, "NO LOCK" (SETGLOCK -1: lamp
 * off); up to 85, lamp on (NOGIMRUN). Beyond 85 the ISS "SHOULD BE IN
 * COARSE ALIGN TO PREVENT GIMBAL RUNAWAY": if it is not (channel 12 bit
 * 4), SETCOARS. */
static void glockchk(void)
{
    ad(MINUS_70DEGS);           /* AD -70DEGS */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF SETGLOCK -1 */
        set_A(ZERO);            /* CAF ZERO */
        setglock();
        return;
    }
    ad(MINUS_15DEGS);           /* AD -15DEGS: "SEE IF ABS(MGA) GREATER THAN 85 DEGREES" */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF NOGIMRUN */
        nogimrun();
        return;
    }
    set_A(BIT4);
    rand_channel(CHAN12); /* EXTEND; RAND CHAN12 */
    if (ccs_A_positive()) {
        nogimrun();
        return;
    }
    tc_at((word)(GLOCKCHK + 013u)); /* TC IBNKCALL; CADR SETCOARS */
    continue_at(IBNKCALL);
}

/* GLOCKCHK +015, where SETCOARS returns: "ENABLE ISS ERROR COUNTERS IN 60
 * MS" (WAITLIST, CA+ECE). */
void glockchk_after_setcoars(void)
{
    set_A(SIX);
    tc_at((word)(GLOCKCHK + 016u)); /* TC WAITLIST; 2CADR CA+ECE */
    continue_at(WAITLIST);
}

/* NOGIMRUN -- LM p. 170, CM p. 148: "TURN ON GIMBAL LOCK LAMP". */
void nogimrun(void)
{
    set_A(BIT6);
    setglock();
}

/* SETGLOCK -- LM p. 170, CM p. 148.
 * A is the lamp wanted (BIT6 on, +0 off). "SEE IF PRESENT STATE OF GIMBAL
 * LOCK LAMP AGREES WITH DESIRED STATE BY HALF ADDING THE TWO": if so, "OK
 * AS IS" (GLOCKOK EQUALS RESUME). If the lamp is on, "TURN OFF UNLESS
 * LAMP TEST IN PROGRESS" (GLAMPTST); if off, "DON'T TURN ON IF IMU BEING
 * CAGED" (IMODES30 bit 6). */
static void setglock(void)
{
    ad_at(LAMPS); /* AD DSPTAB +11D */
    mask(BIT6);
    if (A_is_zero()) { /* EXTEND; BZF GLOCKOK */
        continue_at(RESUME);
        return;
    }
    mask_at(LAMPS); /* MASK DSPTAB +11D */
    if (ccs_A_positive()) {
        glamptst();
        return;
    }
    set_A(BIT6);
    mask_at(ADDRESS_OF(IMODES30));
    if (ccs_A_positive()) { /* TCF GLOCKOK */
        continue_at(RESUME);
        return;
    }
    glinvert();
}

/* GLINVERT -- LM p. 170, CM p. 148.
 * "INVERT GIMBAL LOCK LAMP", with bit 15 set "TO INDICATE CHANGE IN DSPTAB
 * +11D" (T4RUPT sends it), and RESUME. */
static void glinvert(void)
{
    cs(LAMPS);
    mask(BIT6);
    ad(BIT15);
    xch(LAMPS);
    mask(OCT37737);
    ads(LAMPS);
    continue_at(RESUME); /* TCF GLOCKOK */
}

/* GLAMPTST -- LM p. 170, CM p. 148: LAMPTEST returns, or skips (no lamp
 * test) to TCF GLINVERT. */
static void glamptst(void)
{
    tc_at(GLAMPTST); /* TC LAMPTEST */
    if (lamptest())
        glinvert();
    else
        continue_at(RESUME); /* TCF GLOCKOK */
}

/* ======================================================================
 * The routines of the channel 30 bits
 * ====================================================================== */

/* TLIM -- LM p. 171, CM p. 149.
 * "REMOVE BIT FROM WORD OF CHANGES AND SET DSKY TEMP LAMP ACCORDINGLY":
 * IMODES30 negative (bit 15 set, the temperature out of limits): "TURN ON
 * LAMP" (DSALMOUT bit 4). At -0 the CCS lands on the EXTEND of the WOR,
 * with A +0. */
static void tlim(void)
{
    mask(POSMAX);
    ts(ADDRESS_OF(RUPTREG2));
    switch (ccs_at(ADDRESS_OF(IMODES30))) {
    case POSITIVE:
    case IS_PLUS_ZERO: tempok(); return; /* TCF TEMPOK, twice */
    case NEGATIVE: set_A(BIT4); break;
    case IS_MINUS_ZERO: break;
    }
    wor_channel(DSALMOUT); /* EXTEND; WOR DSALMOUT */
    nxtifail();
}

/* TEMPOK -- LM p. 171, CM p. 149.
 * "IF TEMP NOW OK, DON'T TURN OFF LAMP IF LAMP TEST IN PROGRESS". */
static void tempok(void)
{
    tc_at(TEMPOK); /* TC LAMPTEST */
    if (lamptest()) {
        set_A(negate(BIT4));    /* CS BIT4 */
        wand_channel(DSALMOUT); /* "TURN OFF LAMP" */
    }
    nxtifail();
}

/* ITURNON -- LM p. 172, CM p. 150.
 * Bit 14, ISS TURN-ON REQUEST. "IF DELAY REQUEST HAS GONE OFF
 * PREMATURELY, DO NOT PROCESS ANY CHANGES UNTIL THE CURRENT 90 SEC WAIT
 * EXPIRES" (IMODES30 bit 2). A request just on: ITURNON2. Just off: if
 * the ISS delay complete discrete (channel 12 bit 15) has been sent,
 * "ACTION COMPLETE"; if not, "SET BIT TO INDICATE REQUEST NOT PRESENT FOR
 * FULL DURATION" and alarm 207. */
static void iturnon(void)
{
    set_A(BIT2);
    mask_at(ADDRESS_OF(IMODES30));
    if (ccs_A_positive()) {
        nxtifail();
        return;
    }
    set_A(BIT14); /* "SEE IF JUST ON OR OFF" */
    mask_at(ADDRESS_OF(IMODES30));
    if (A_is_zero()) { /* EXTEND; BZF ITURNON2: "IF JUST ON" */
        iturnon2();
        return;
    }
    set_A(BIT15);
    rand_channel(CHAN12); /* EXTEND; RAND CHAN12 */
    if (!A_is_zero()) {   /* EXTEND; BZF +2 */
        nxtifail();       /* TCF NXTIFAIL */
        return;
    }
    set_A(BIT2);
    ads(ADDRESS_OF(IMODES30));
    tc_at((word)(ITURNON + 020u)); /* TC ALARM; OCT 207 */
    continue_at(ALARM);
}

/* ITURNON +022, where ALARM returns: TCF NXTIFAIL. */
void iturnon_after_alarm(void)
{
    nxtifail();
}

/* ITURNON2 -- LM p. 173, CM p. 151.
 * "SET BIT7 TO INDICATE WAIT OF 1 SAMPLE" (TNONTEST starts the sequence).
 * The LM also resets the radar's modes: CAF RRINIT; TS RADMODES. */
static void iturnon2(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT7);
    ads(ADDRESS_OF(IMODES30));
#if LUMINARY099
    set_A(RRINIT);
    ts(ADDRESS_OF(RADMODES));
#endif
    nxtifail();
}

/* IMUCAGE -- LM p. 174-175, CM p. 152-153.
 * Bit 11, IMU CAGE. Released (1): zero the ICDUs (ISSZERO). Pressed (0):
 * "TERMINATE ICDU, RCDU, GYRO PULSE TRAINS", knock down the ISS outbits
 * of channel 12, turn the engine off, light NO ATT and set the cage flags
 * (CAGESUB1), and "KNOCK DOWN TRACK, REFSMMAT, DRIFT FLAGS" (IBNKCALL
 * RNDREFDR). */
static void imucage(void)
{
    if (ccs_A_positive()) { /* CCS A; TCF ISSZERO */
        isszero();
        return;
    }
    set_A(negate(OCT77000)); /* CS OCT77000 */
    wand_channel(CHAN14);
    set_A(negate(OCT272)); /* CS OCT272 */
    wand_channel(CHAN12);
#if LUMINARY099
    set_A(negate(ENGONBIT)); /* "INSURE ENGONFLG IS CLEAR" */
    mask_at(ADDRESS_OF(FLAGWRD5));
    ts(ADDRESS_OF(FLAGWRD5));
    set_A(negate(PRIO30)); /* "TURN ENGINE OFF" */
    rand_channel(DSALMOUT);
    ad(BIT14);
    write_channel(DSALMOUT); /* "FORCE BIT14=1, BIT13=0" */
#else
    set_A(negate(BIT13)); /* "TURN ENGINE OFF" */
    wand_channel(DSALMOUT);
#endif
    tc_at(IMUCAGE_TC_CAGESUB1); /* TC CAGESUB1 */
    cagesub1();
    tc_at((word)(IMUCAGE_TC_CAGESUB1 + 1u)); /* TC IBNKCALL; CADR RNDREFDR */
    continue_at(IBNKCALL);
}

/* After IMUCAGE's RNDREFDR: zero the drive counters (CS ZERO: -0), and
 * "HAVING WAITED AT LEAST 27 MCT FROM GYRO PULSE TRAIN TERMINATION, WE CAN
 * DE-SELECT THE GYROS". */
void imucage_after_rndrefdr(void)
{
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(CDUXCMD));
    ts(ADDRESS_OF(CDUYCMD));
    ts(ADDRESS_OF(CDUZCMD));
    ts(ADDRESS_OF(GYROCMD));
    set_A(negate(OCT740)); /* CS OCT740 */
    wand_channel(CHAN14);
    nxtifail();
}

/* IMUOP -- LM p. 176, CM p. 154.
 * Bit 9, IMU OPERATE. On (0): IMUOP2. Off: "DISABLE DAP" and "KNOCK DOWN
 * TRACK, REFSMMAT, DRIFT FLAGS" (IBNKCALL RNDREFDR). */
static void imuop(void)
{
    if (A_is_zero()) { /* EXTEND; BZF IMUOP2 */
        imuop2();
        return;
    }
    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    ads(ADDRESS_OF(IMODES33));
    tc_at((word)(IMUOP + 5u)); /* TC IBNKCALL; CADR RNDREFDR */
    continue_at(IBNKCALL);
}

/* IMUOP +7, where RNDREFDR returns: "KNOCK DOWN RENDEZVOUS, IMUUSE FLAGS",
 * and "IF GOING OFF, ALARM IF PROG USING IMU" (214). */
void imuop_after_rndrefdr(void)
{
    set_A(negate(BITS7_8)); /* CS BITS7&8 */
#if LUMINARY099
    mask_at(ADDRESS_OF(FLAGWRD0));
    xch(ADDRESS_OF(FLAGWRD0));
#else
    mask_at(ADDRESS_OF(STATE));
    xch(ADDRESS_OF(STATE));
#endif
    com();
    mask(IMUSEFLG);
    if (ccs_A_positive()) {
        nxtifail();
        return;
    }
    tc_at((word)(IMUOP + 016u)); /* TC ALARM; OCT 214 */
    continue_at(ALARM);
}

/* IMUOP +020, where ALARM returns: TCF NXTIFAIL. */
void imuop_after_alarm(void)
{
    nxtifail();
}

/* IMUOP2 -- LM p. 176, CM p. 154.
 * "SEE IF FAILED ISS TURN-ON SEQ IN PROG. IF SO, DON'T PROCESS UNTIL
 * PRESENT 90 SECONDS EXPIRES"; else ITURNON2. */
static void imuop2(void)
{
    set_A(BIT2);
    mask_at(ADDRESS_OF(IMODES30));
    if (ccs_A_positive())
        nxtifail();
    else
        iturnon2();
}

/* ======================================================================
 * The routines of the channel 33 bits
 * ====================================================================== */

/* PIPFAIL -- LM p. 177, CM p. 155.
 * Bit 13, PIPA FAIL: "SET BIT10 IN IMODES30 SO ALL ISS WARNING INFO IS IN
 * ONE REGISTER", and SETISSW. */
static void pipfail(void)
{
    if (ccs_A_positive()) set_A(BIT10); /* CCS A; CAF BIT10 */
    xch(ADDRESS_OF(IMODES30));
    mask(MINUS_BIT10); /* MASK -BIT10 */
    ads(ADDRESS_OF(IMODES30));
    tc_at((word)(PIPFAIL + 5u)); /* TC SETISSW */
    if (setissw()) pipfail_after_setissw();
}

/* After PIPFAIL's SETISSW: "IF PIP FAIL DOESN'T LIGHT ISS WARNING, DO A
 * PROGRAM ALARM IF IMU OPERATING BUT NOT CAGED OR BEING TURNED ON" (212). */
static void pipfail_after_setissw(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT1);
    if (ccs_A_positive()) {
        nxtfl33();
        return;
    }
    ca(ADDRESS_OF(IMODES30));
    mask(OCT1720);
    if (ccs_A_positive()) { /* "ABOVE CONDITION NOT MET" */
        nxtfl33();
        return;
    }
    tc_at((word)(PIPFAIL + 016u)); /* TC ALARM; OCT 212 */
    continue_at(ALARM);
}

/* PIPFAIL +020, where ALARM returns: TCF NXTFL33. */
void pipfail_after_alarm(void)
{
    nxtfl33();
}

/* DNTMFAST -- LM p. 178, CM p. 156.
 * Bit 12: "DO PROG ALARM IF TM TOO FAST" (1105) when it goes to 0. */
static void dntmfast(void)
{
    if (ccs_A_positive()) {
        nxtfl33();
        return;
    }
    tc_at((word)(DNTMFAST + 2u)); /* TC ALARM; OCT 1105 */
    continue_at(ALARM);
}

/* DNTMFAST +4, where ALARM returns: TCF NXTFL33. */
void dntmfast_after_alarm(void)
{
    nxtfl33();
}

/* UPTMFAST -- LM p. 178, CM p. 156.
 * Bit 11: "SAME AS DNLINK TOO FAST WITH DIFFERENT ALARM CODE" (1106). */
static void uptmfast(void)
{
    if (ccs_A_positive()) {
        nxtfl33();
        return;
    }
    tc_at((word)(UPTMFAST + 2u)); /* TC ALARM; OCT 1106 */
    continue_at(ALARM);
}

/* UPTMFAST +4, where ALARM returns: TCF NXTFL33. */
void uptmfast_after_alarm(void)
{
    nxtfl33();
}

/* ======================================================================
 * The lamps and the cage
 * ====================================================================== */

/* SETISSW -- LM p. 179-180, CM p. 157.
 * "SET ISS WARNING USING THE FAIL BITS IN BITS 13, 12, AND 10 OF IMODES30
 * AND THE FAILURE INHIBIT BITS IN POSITIONS 4, 3, AND 1": MP BIT10 moves
 * the inhibits up beside the fail bits (into L), and ROR LCHAN or's them
 * in, so a 0 left in bits 13, 12 or 10 is a failure not inhibited. Then
 * ISSWON, or ISSWOFF: lamp off (DSALMOUT bit 1) "IF LAMP TEST" not "IN
 * PROGRESS", and return (TC Q). */
bool setissw(void)
{
    set_A(OCT15);
    mask_at(ADDRESS_OF(IMODES30));
    mp(BIT10); /* EXTEND; MP BIT10 */
    ca(ADDRESS_OF(IMODES30));
    ror_lchan(); /* EXTEND; ROR LCHAN */
    com();       /* "0 INDICATES FAILURE" */
    mask(OCT15000);
    if (ccs_A_positive()) { /* TCF ISSWON: "FAILURE" */
        isswon();
        return false;
    }
    /* ISSWOFF: "DON'T TURN OFF ISS WARNING IF LAMP TEST IN PROGRESS". */
    set_A(BIT1);
    mask_at(ADDRESS_OF(IMODES33));
    if (!ccs_A_positive()) {
        set_A(negate(BIT1)); /* CS BIT1 */
        wand_channel(DSALMOUT);
    }
    tc_through(Q_ADDRESS); /* TC Q */
    return true;
}

/* ISSWON -- LM p. 180, CM p. 157.
 * Keep the return (QXCH ITEMP6) and "TELL EVERYONE WHAT CAUSED THE ISS
 * WARNING" (VARALARM, which returns to ISSWON +3). */
static void isswon(void)
{
    qxch(ADDRESS_OF(ITEMP6));   /* EXTEND; QXCH ITEMP6 */
    tc_at((word)(ISSWON + 2u)); /* TC VARALARM */
    continue_at(VARALARM);
}

/* ISSWON +3, where VARALARM returns: the lamp on, and back to SETISSW's
 * caller (TC ITEMP6), going on there if it is in this file. */
void isswon_after_varalarm(void)
{
    set_A(BIT1);
    wor_channel(DSALMOUT);          /* EXTEND; WOR DSALMOUT */
    tc_through(ADDRESS_OF(ITEMP6)); /* TC ITEMP6 */
    if (Z == NXTIFAIL)
        nxtifail();
    else if (Z == ISSUP_TC_SETISSW + 1u)
        issup_after_setissw();
    else if (Z == PIPFAIL + 6u)
        pipfail_after_setissw();
}

/* CAGESUB -- LM p. 180, CM p. 158.
 * "SET OUTBITS AND INTERNAL FLAGS FOR SYSTEM TURN-ON OR CAGE. DISABLE THE
 * ERROR COUNTER AND REMOVE THE IMU DELAY COMP." (channel 12 bits 6 and
 * 15), and "SEND ZERO AND COARSE" (bits 4 and 5). On into CAGESUB1. */
static void cagesub(void)
{
    set_A(negate(BITS6_15)); /* CS BITS6&15 (the CM's BIT15+6) */
    wand_channel(CHAN12);
    set_A(BITS4_5); /* CAF BITS4&5 */
    wor_channel(CHAN12);
    cagesub1();
}

/* CAGESUB1 -- LM p. 180, CM p. 158: "TURN ON NO ATT LAMP" (DSPTAB +11D
 * bit 4, bit 15 to send it). On into CAGESUB2. */
static void cagesub1(void)
{
    cs(LAMPS);
    mask(OC40010);
    ads(LAMPS);
    cagesub2();
}

/* CAGESUB2 -- LM p. 180, CM p. 158.
 * "SET FLAGS TO INDICATE CAGING OR TURN-ON AND INHIBIT ALL ISS WARNING
 * INFO" (IMODES30 bits 6, 4, 3, 1), "DISABLE DAP AUTO AND HOLD MODES"
 * (IMODES33 bit 6), and return (TC Q). */
static void cagesub2(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(OCT75);
    ads(ADDRESS_OF(IMODES30));
    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    ads(ADDRESS_OF(IMODES33));
    tc_through(Q_ADDRESS); /* TC Q */
}

/* LAMPTEST -- LM p. 181, CM p. 159.
 * "SUBROUTINE TO SKIP IF LAMP TEST NOT IN PROGRESS" ("BIT 1 OF IMODES33 =
 * 1 IF LAMP TEST IN PROGRESS"): INCR Q, and TC Q. True if it skipped. The
 * CM's last three words are PINBALL's (TCF ZOPFIN3), the same three. */
static bool lamptest(void)
{
    bool skip;

    cs(ADDRESS_OF(IMODES33));
    mask(BIT1);
    skip = ccs_A_positive();   /* CCS A */
    if (skip) incr(Q_ADDRESS); /* INCR Q */
    tc_through(Q_ADDRESS);     /* TC Q */
    return skip;
}
