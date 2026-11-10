/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc, part 1: zeroing the CDUs, coarse and
 * fine align, and the program's use of the PIPAs (LM pages 1309-1318, CM
 * pages 1420-1429).
 *
 * A mode switch starts in a job, through BANKCALL: it sets the IMU's
 * channel bits, puts the rest on WAITLIST as tasks, and leaves by
 * MODEEXIT (RELINT; TCF SWRETURN) for the caller, which waits in a
 * stall (IMUSTALL) for the last task to end the switch at ENDIMU. Every
 * step first makes sure the IMU is not being caged (CAGETEST, CAGETSTQ,
 * CAGETSTJ, part 3). */
#include "subroutines.h"

/* DSPTAB +11D: the lamp relay word; bit 4 the NO ATT lamp, bit 6 the
 * GIMBAL LOCK lamp (BITS4&6), bit 15 set for the word to be sent. */
#define LAMPS (ADDRESS_OF(DSPTAB) + 11u)

/* ZEROICDU -- LM p. 1309, CM p. 1420.
 * "ZERO ICDU COUNTERS.": CDUX, CDUY and CDUZ to +0, and TC Q. */
void zeroicdu(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(CDUY));
    ts(ADDRESS_OF(CDUZ));
    tc_through(Q_ADDRESS); /* TC Q */
}

/* ======================================================================
 * Zeroing
 * ====================================================================== */

/* MODEEXIT -- LM p. 1311, CM p. 1422: "GENERAL MODE-SWITCHING EXIT." */
void modeexit(void)
{
    relint();
    continue_at(SWRETURN);
}

/* IMUZERO -- LM p. 1310, CM p. 1421.
 * "ROUTINE TO ZERO ICDUS.", but "DON'T ZERO CDUS IF IMU IN GIMBAL LOCK
 * AND COARSE ALIGN (GIMBAL RUNAWAY PROTECTION)": with both lamps' bits on
 * in DSPTAB +11D, alarm 206 and "IMMEDIATE FAILURE." (CAGETSTJ +4).
 * IMUZEROA: the DAP's auto and hold modes off (IMODES33), the IMU and
 * ICDU fail warnings inhibited (IMODES30), the zero discrete on with
 * coarse align and the error counters off (channel 12), the CDUs zeroed,
 * and IMUZERO2 in 320 ms "TO GIVE AGS ADEQUATE TIME TO RECEIVE ITS PULSE
 * TRAIN." */
void imuzero(void)
{
    inhint();
    cs(LAMPS);
    mask(BITS4_6);
    if (ccs_register(A_ADDRESS) != POSITIVE) { /* CCS A; TCF IMUZEROA */
        tc_at((word)(IMUZERO + 5u));           /* TC ALARM; OCT 00206 */
        continue_at(ALARM);
        return;
    }
    tc_at(IMUZEROA); /* TC CAGETSTJ */
    if (!cagetstj()) return;
    cs(ADDRESS_OF(IMODES33)); /* "DISABLE DAP AUTO AND HOLD MODES" */
    mask(SUPER011);
    ads(ADDRESS_OF(IMODES33));
    cs(ADDRESS_OF(IMODES30));
    mask(BITS3_4);
    ads(ADDRESS_OF(IMODES30));
    set_A(negate(BITS4_6)); /* CS BITS4&6 */
    wand_channel(CHAN12);
    tc_at((word)(IMUZEROA + 012u)); /* TC NOATTOFF: "TURN OFF NO ATT LAMP." */
    noattoff();
    set_A(BIT5);
    wor_channel(CHAN12);
    tc_at((word)(IMUZEROA + 016u)); /* TC ZEROICDU */
    zeroicdu();
    set_A(BIT6);
    tc_at((word)(IMUZEROA + 020u)); /* TC WAITLIST; 2CADR IMUZERO2 */
    continue_at(WAITLIST);
}

/* IMUZERO +7, where ALARM returns: TCF CAGETSTJ +4. */
void imuzero_after_alarm(void)
{
    cagetstj_failed();
}

/* IMUZEROA +023, where WAITLIST returns: "SEE IF IMU OPERATING AND ALARM
 * IF NOT." (IMODES30 bit 9), alarm 210; on to MODEEXIT either way. */
void imuzeroa_after_waitlist(void)
{
    cs(ADDRESS_OF(IMODES30));
    mask(BIT9);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* TCF MODEEXIT */
        modeexit();
        return;
    }
    tc_at((word)(IMUZEROA + 027u)); /* TC ALARM; OCT 210 */
    continue_at(ALARM);
}

/* IMUZERO2 -- LM p. 1311, CM p. 1422.
 * The task: the CDUs zeroed again, the zero discrete removed, and
 * IMUZERO3 in 10 seconds ("WAIT 10 SECS FOR CTRS TO FIND GIMBALS"). */
void imuzero2(void)
{
    tc_at(IMUZERO2); /* TC CAGETEST */
    if (!cagetest()) return;
    tc_at((word)(IMUZERO2 + 1u)); /* TC ZEROICDU */
    zeroicdu();
    set_A(negate(BIT5)); /* CS BIT5: "REMOVE ZERO DISCRETE." */
    wand_channel(CHAN12);
    set_A(BIT11);
    tc_at((word)(IMUZERO2 + 6u)); /* TC VARDELAY */
    continue_at(VARDELAY);
}

/* IMUZERO3 -- LM p. 1311, CM p. 1422.
 * "REMOVE IMUFAIL AND ICDUFAIL INHIBIT.", the DAP's modes back on, and
 * SETISSW for the ISS warning lamp. */
void imuzero3(void)
{
    tc_at(IMUZERO3); /* TC CAGETEST */
    if (!cagetest()) return;
    set_A(negate(BITS3_4)); /* CS BITS3&4 */
    mask_at(ADDRESS_OF(IMODES30));
    ts(ADDRESS_OF(IMODES30));
    set_A(negate(SUPER011)); /* CS SUPER011 */
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
    tc_at((word)(IMUZERO3 + 7u)); /* TC IBNKCALL; CADR SETISSW */
    continue_at(IBNKCALL);
}

/* IMUZERO3 +011, where SETISSW returns: TCF ENDIMU. */
void imuzero3_after_setissw(void)
{
    endimu();
}

/* ======================================================================
 * Coarse align
 * ====================================================================== */

/* IMUCOARS -- LM p. 1312, CM p. 1423: "IMU COARSE ALIGN MODE."
 * Coarse align on (SETCOARS), and COARS in 60 ms. */
void imucoars(void)
{
    inhint();
    tc_at((word)(IMUCOARS + 1u)); /* TC CAGETSTJ */
    if (!cagetstj()) return;
    tc_at((word)(IMUCOARS + 2u)); /* TC SETCOARS */
    setcoars();
    set_A(SIX);
    tc_at((word)(IMUCOARS + 4u)); /* TC WAITLIST; 2CADR COARS */
    continue_at(WAITLIST);
}

/* IMUCOARS +7, where WAITLIST returns: TCF MODEEXIT. */
void imucoars_after_waitlist(void)
{
    modeexit();
}

/* COARS, COARS1 -- LM p. 1312, CM p. 1423.
 * The task: "ENABLE ALL THREE ISS CDU ERROR COUNTERS", and for each CDU
 * (CDUIND 2, 1, 0) "COMPUTE THETAD -- THETAA IN 1'S COMPLEMENT FORM" in
 * COMMAND: the difference (MSU), shifted right 2 and rounded (DOUBLE the
 * low part; an overflow adds into it). Then COARS2 after "MINIMUM OF 4
 * MS WAIT". */
void coars(void)
{
    tc_at(COARS); /* TC CAGETEST */
    if (!cagetest()) return;
    set_A(BIT6);
    wor_channel(CHAN12);
    set_A(TWO); /* "SET CDU INDICATOR" */
    for (;;) {
        ts(ADDRESS_OF(CDUIND)); /* COARS1 */
        ca(indexed(ADDRESS_OF(THETAD), CDUIND));
        msu_at(indexed(ADDRESS_OF(CDUX), CDUIND)); /* EXTEND; INDEX CDUIND; MSU CDUX */
        mp(BIT13);                                 /* "SHIFT RIGHT 2" */
        xch(L_ADDRESS);                            /* "ROUND" */
        double_A();
        if (ts(ADDRESS_OF(ITEMP1))) ads(L_ADDRESS); /* TCF +2, or on overflow ADS L */
        lxch(indexed(ADDRESS_OF(COMMAND), CDUIND)); /* "DIFFERENCE TO BE COMPUTED" */
        if (ccs_at(ADDRESS_OF(CDUIND)) != POSITIVE) break;
        tc_at((word)(COARS1 + 020u)); /* TC COARS1 */
    }
    set_A(TWO);
    tc_at((word)(COARS1 + 022u)); /* TC VARDELAY */
    continue_at(VARDELAY);
}

/* COMZERO -- LM p. 1314, CM p. 1425: a command that fits in one pass, all
 * of it, and COMMAND to +0. */
static void comzero(void)
{
    set_A(ZERO);
    xch(indexed(ADDRESS_OF(COMMAND), CDUIND));
    tc_at((word)(COMZERO + 3u)); /* TC NEXTCDU */
}

/* COMPOS -- LM p. 1313, CM p. 1424.
 * A positive command: at most COMMAX pulses this pass ("COMMAX = MAX
 * NUMBER OF PULSES ALLOWED MINUS ONE"), the rest left in COMMAND. On to
 * NEXTCDU. */
static void compos(void)
{
    ad(MINUS_COMMAX);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF COMZERO */
        comzero();
        return;
    }
    ts(indexed(ADDRESS_OF(COMMAND), CDUIND)); /* "REDUCE COMMAND BY MAX NUMBER OF PULSES" */
    set_A(negate(MINUS_COMMAX_MINUS));        /* CS -COMMAX- */
}

/* COMNEG -- LM p. 1314, CM p. 1425: the same for a negative command. */
static void comneg(void)
{
    ad(MINUS_COMMAX);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF COMZERO */
        comzero();
        return;
    }
    com();
    ts(indexed(ADDRESS_OF(COMMAND), CDUIND));
    set_A(MINUS_COMMAX_MINUS);  /* CA -COMMAX- */
    tc_at((word)(COMNEG + 7u)); /* TC NEXTCDU */
}

/* COARS2, NEXTCDU -- LM p. 1313, CM p. 1424.
 * VARDELAY's task, again every 600 ms while pulses go out: for each CDU
 * the pulses of this pass into its command register (CDUXCMD, CDUYCMD,
 * CDUZCMD; AD NEG0 turns +0 into -0), ITEMP1 counting the CDUs given
 * any. Then SENDPULS ("13,14,15": the three drives on, channel 14) and
 * this again in 600 ms; or, none left, 1.5 s "FOR GIMBALS TO SETTLE." */
void coars2(void)
{
    tc_at(COARS2); /* TC CAGETEST: "DON'T CONTINUE IF CAGED." */
    if (!cagetest()) return;
    ts(ADDRESS_OF(ITEMP1)); /* "SET TO +0." */
    set_A(TWO);
    for (;;) {
        ts(ADDRESS_OF(CDUIND)); /* COARS2 +3 */
        switch (ccs_at(indexed(ADDRESS_OF(COMMAND), CDUIND))) {
        case POSITIVE:                  /* "GREATER THAN MAX ALLOWED" */
            tc_at((word)(COARS2 + 6u)); /* TC COMPOS */
            compos();
            incr(ADDRESS_OF(ITEMP1)); /* NEXTCDU */
            break;
        case IS_PLUS_ZERO: tc_at((word)(COARS2 + 7u)); break; /* TC NEXTCDU +1 */
        case NEGATIVE:
            tc_at((word)(COARS2 + 010u)); /* TC COMNEG */
            comneg();
            incr(ADDRESS_OF(ITEMP1)); /* NEXTCDU */
            break;
        case IS_MINUS_ZERO: tc_at((word)(COARS2 + 011u)); break; /* TC NEXTCDU +1 */
        }
        ad(NEG0);
        ts(indexed(ADDRESS_OF(CDUXCMD), CDUIND)); /* "SET UP COMMAND REGISTER." */
        if (ccs_at(ADDRESS_OF(CDUIND)) != POSITIVE) break;
        tc_at((word)(NEXTCDU + 5u)); /* TC COARS2 +3 */
    }
    if (ccs_at(ADDRESS_OF(ITEMP1)) == POSITIVE) { /* "SEE IF ANY PULSES TO GO OUT." */
        set_A(BITS13_14_15);                      /* SENDPULS: CAF 13,14,15 */
        wor_channel(CHAN14);
        set_A(SIX_HUNDRED_MS);      /* CAF 600MS */
        tc_at((word)(COARS2 - 1u)); /* TCF COARS2 -1: "THEN TO VARDELAY" */
        continue_at(VARDELAY);
        return;
    }
    tc_at((word)(NEXTCDU + 010u)); /* TC FIXDELAY; DEC 150 */
    continue_at(FIXDELAY);
}

/* NEXTCDU +012, where FIXDELAY returns; CHKCORS, CORSCHK2, COARSERR --
 * LM pp. 1313-1314, CM pp. 1424-1425.
 * "AT END OF COMMAND, CHECK TO SEE THAT GIMBALS ARE WITHIN 2 DEGREES OF
 * THETAD." (COARSTOL); a gimbal further off is alarm 211 ("COARSE ALIGN
 * ERROR."). All within: ENDIMU. */
void nextcdu_after_fixdelay(void)
{
    set_A(TWO);
    for (;;) {
        ts(ADDRESS_OF(ITEMP1)); /* CHKCORS */
        ca(indexed(ADDRESS_OF(CDUX), a_word()));
        msu_at(indexed(ADDRESS_OF(THETAD), ITEMP1)); /* EXTEND; INDEX ITEMP1; MSU THETAD */
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE:
        case NEGATIVE:
            ad(COARSTOL); /* COARSERR: "2 DEGREES." */
            if (!A_is_zero_or_minus()) {
                tc_at((word)(COARSERR + 3u)); /* TC ALARM; OCT 211 */
                continue_at(ALARM);
                return;
            }
            break;
        default: break;
        }
        if (ccs_at(ADDRESS_OF(ITEMP1)) != POSITIVE) break; /* CORSCHK2 */
    }
    endimu(); /* "END OF COARSE ALIGNMENT" */
}

/* COARSERR +5, where ALARM returns: TCF IMUBAD. */
void coarserr_after_alarm(void)
{
    imubad();
}

/* CA+ECE -- LM p. 1314, CM p. 1425.
 * A task: "ENABLE ALL THREE ISS CDU ERROR COUNTERS". */
void ca_plus_ece(void)
{
    set_A(BIT6);
    wor_channel(CHAN12);
    tc_at((word)(CA_PLUS_ECE + 3u)); /* TC TASKOVER */
    continue_at(TASKOVER);
}

/* SETCOARS -- LM p. 1315, CM p. 1426.
 * "BYPASS IF ALREADY IN COARSE ALIGN" (channel 12 bit 4). Otherwise
 * "CLEAR ISS ERROR COUNTERS", "KNOCK DOWN GYRO ACTIVITY", "PUT ISS IN
 * COARSE ALIGN", "TURN ON NO ATT LAMP", "DISABLE DAP AUTO AND HOLD
 * MODES" and "DISABLE IMUFAIL"; on into RNDREFDR. */
void setcoars(void)
{
    set_A(BIT4);
    rand_channel(CHAN12);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_through(Q_ADDRESS); /* TC Q */
        return;
    }
    set_A(negate(BIT6)); /* CS BIT6 */
    wand_channel(CHAN12);
    set_A(negate(BIT10)); /* CS BIT10 */
    wand_channel(CHAN14);
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(GYROCMD));
    set_A(BIT4);
    wor_channel(CHAN12);
    cs(LAMPS);
    mask(OCT40010);
    ads(LAMPS);
    cs(ADDRESS_OF(IMODES33));
    mask(BIT6);
    ads(ADDRESS_OF(IMODES33));
    cs(ADDRESS_OF(IMODES30));
    mask(BIT4);
    ads(ADDRESS_OF(IMODES30));
    rndrefdr();
}

/* RNDREFDR -- LM p. 1315, CM p. 1426.
 * "CLEAR TRACK FLAG", "CLEAR DRIFT FLAG", "CLEAR REFSMMAT FLAG"
 * (FLAGWRD1 bit 5, FLAGWRD2 bit 15, FLAGWRD3 bit 13), and TC Q. */
void rndrefdr(void)
{
#if LUMINARY099
    set_A(negate(TRACKBIT)); /* CS TRACKBIT */
#else
    set_A(negate(BIT5)); /* CS BIT5 */
#endif
    mask_at(ADDRESS_OF(FLAGWRD1));
    ts(ADDRESS_OF(FLAGWRD1));
#if LUMINARY099
    set_A(negate(DRFTBIT)); /* CS DRFTBIT */
#else
    set_A(negate(BIT15)); /* CS BIT15 */
#endif
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));
#if LUMINARY099
    set_A(negate(REFSMBIT)); /* CS REFSMBIT */
#else
    set_A(negate(BIT13)); /* CS BIT13 */
#endif
    mask_at(ADDRESS_OF(FLAGWRD3));
    ts(ADDRESS_OF(FLAGWRD3));
    tc_through(Q_ADDRESS); /* TC Q */
}

/* ======================================================================
 * Fine align
 * ====================================================================== */

/* IMUFINE -- LM p. 1316, CM p. 1427: "IMU FINE ALIGN MODE SWITCH."
 * "RESET ZERO AND COARSE", "INSURE DAP AUTO AND HOLD MODES ENABLED", the
 * NO ATT lamp off, and IFAILOK in 10 seconds: "IMU FAIL WAS INHIBITED
 * DURING THE PRESUMABLY PRECEDING COARSE ALIGN.  LEAVE IT ON FOR THE
 * FIRST 5 SECS OF FINE ALIGN". */
void imufine(void)
{
    inhint();
    tc_at((word)(IMUFINE + 1u)); /* TC CAGETSTJ: "SEE IF IMU BEING CAGED." */
    if (!cagetstj()) return;
    set_A(negate(BITS4TO5)); /* CS BITS4-5 */
    wand_channel(CHAN12);
    set_A(negate(BIT6)); /* CS BIT6 */
    mask_at(ADDRESS_OF(IMODES33));
    ts(ADDRESS_OF(IMODES33));
    tc_at((word)(IMUFINE + 010u)); /* TC NOATTOFF */
    noattoff();
    set_A(BIT10);
    tc_at((word)(IMUFINE + 012u)); /* TC WAITLIST; 2CADR IFAILOK */
    continue_at(WAITLIST);
}

/* IMUFINE +015, where WAITLIST returns: IMUFINED in 2 seconds. */
void imufine_after_waitlist(void)
{
    set_A(TWO_SECS);
    tc_at((word)(IMUFINE + 016u)); /* TC WAITLIST; 2CADR IMUFINED */
    continue_at(WAITLIST);
}

/* IMUFINE +021, where WAITLIST returns: TCF MODEEXIT. */
void imufine_after_waitlist2(void)
{
    modeexit();
}

/* IMUFINED -- LM p. 1316, CM p. 1427.
 * The task: "SEE THAT NO ONE HAS CAGED THE IMU.", and ENDIMU. */
void imufined(void)
{
    tc_at(IMUFINED); /* TC CAGETEST */
    if (!cagetest()) return;
    endimu();
}

/* PFAILOK2 -- LM p. 1317, CM p. 1428: IMODES30 and'ed with A (an inhibit
 * bit's complement), and SETISSW: "THE ISS WARNING LIGHT MAY COME ON NOW
 * THAT THE INHIBIT WAS BEEN REMOVED." */
static void pfailok2(void)
{
    mask_at(ADDRESS_OF(IMODES30));
    ts(ADDRESS_OF(IMODES30));
    tc_at((word)(PFAILOK2 + 2u)); /* TC IBNKCALL; CADR SETISSW */
    continue_at(IBNKCALL);
}

/* IFAILOK -- LM p. 1317, CM p. 1428.
 * The task: "ENABLE IMU FAIL UNLESS IMU BEING CAGED." or "IF SOMEONE HAS
 * GONE INTO COARSE ALIGN." (channel 12 bit 4): "RESET IMUFAIL." (IMODES30
 * bit 13) and its inhibit (bit 4). */
void ifailok(void)
{
    tc_at(IFAILOK); /* TC CAGETSTQ */
    if (!cagetstq()) {
        continue_at(TASKOVER); /* TCF TASKOVER: "IT IS." */
        return;
    }
    set_A(BIT4);
    rand_channel(CHAN12);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(TASKOVER); /* TCF TASKOVER */
        return;
    }
    cs(ADDRESS_OF(IMODES30));
    mask(BIT13);
    ads(ADDRESS_OF(IMODES30));
    set_A(negate(BIT4)); /* CS BIT4 */
    pfailok2();
}

/* PFAILOK2 +4, where SETISSW returns: TCF TASKOVER. */
void pfailok2_after_setissw(void)
{
    continue_at(TASKOVER);
}

/* PFAILOK -- LM p. 1317, CM p. 1428.
 * The task: "ENABLE PIP FAIL PROG ALARM." unless caged: "RESET IMU AND
 * PIPA FAIL BITS." (IMODES30 bit 10, IMODES33 bit 13), and the PIPA fail
 * inhibit (IMODES30 bit 5) by PFAILOK2. */
void pfailok(void)
{
    tc_at(PFAILOK); /* TC CAGETSTQ */
    if (!cagetstq()) {
        continue_at(TASKOVER); /* TCF TASKOVER */
        return;
    }
    cs(ADDRESS_OF(IMODES30));
    mask(BIT10);
    ads(ADDRESS_OF(IMODES30));
    cs(ADDRESS_OF(IMODES33));
    mask(BIT13);
    ads(ADDRESS_OF(IMODES33));
    set_A(negate(BIT5)); /* CS BIT5 */
    pfailok2();
}

/* NOATTOFF -- LM p. 1317, CM p. 1428.
 * "SUBROUTINE TO TURN OFF NO ATT LAMP.": DSPTAB +11D's bits 4 and 15
 * clear, bit 15 set again (to send it), and TC Q. */
void noattoff(void)
{
    set_A(negate(OCT40010)); /* CS OCT40010 */
    mask_at(LAMPS);
    ad(BIT15);
    ts(LAMPS);
    tc_through(Q_ADDRESS); /* TC Q */
}

/* ======================================================================
 * The PIPAs
 * ====================================================================== */

/* PIPFREE2 -- LM p. 1318, CM p. 1429: "ISS WARNING MIGHT COME ON NOW."
 * (SETISSW). */
static void pipfree2(void)
{
    tc_at(PIPFREE2); /* TC IBNKCALL; CADR SETISSW */
    continue_at(IBNKCALL);
}

/* PIPUSE, PIPUSE1 -- LM p. 1318, CM p. 1429.
 * "ROUTINES TO INITIATE AND TERMINATE PROGRAM USE OF THE PIPAS.": PIPUSE
 * zeroes the PIPA counters (to -0) first. "DO NOT ENABLE PIPA FAIL IF IMU
 * IS CAGED"; otherwise the PIPA fail inhibit (IMODES30 bit 1) off: "IF
 * PIPA FAILS FROM NOW ON (UNTIL PIPFREE), LIGHT ISS WARNING." */
void pipuse(void)
{
    set_A(negate(ZERO)); /* CS ZERO */
    ts(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(PIPAY));
    ts(ADDRESS_OF(PIPAZ));
    pipuse1();
}

void pipuse1(void)
{
    tc_at(PIPUSE1); /* TC CAGETSTQ */
    if (!cagetstq()) {
        continue_at(SWRETURN); /* TCF SWRETURN */
        return;
    }
    inhint();
    set_A(negate(BIT1)); /* CS BIT1 */
    mask_at(ADDRESS_OF(IMODES30));
    ts(ADDRESS_OF(IMODES30));
    pipfree2();
}

/* PIPFREE2 +2, where SETISSW returns: TCF MODEEXIT. */
void pipfree2_after_setissw(void)
{
    modeexit();
}

/* PIPFREE -- LM p. 1318, CM p. 1429.
 * "PROGRAM DONE WITH PIPAS.  DON'T LIGHT ISS WARNING." (IMODES30 bit 1).
 * "IF PIP FAIL ON, DO PROG ALSRM AND RESET ISS WARNING." (alarm 212). */
void pipfree(void)
{
    inhint();
    cs(ADDRESS_OF(IMODES30));
    mask(BIT1);
    ads(ADDRESS_OF(IMODES30));
    mask(BIT10);
    if (ccs_register(A_ADDRESS) == POSITIVE) { /* TCF MODEEXIT */
        modeexit();
        return;
    }
    tc_at((word)(PIPFREE + 7u)); /* TC ALARM; OCT 212 */
    continue_at(ALARM);
}

/* PIPFREE +011, where ALARM returns: INHINT, and TCF PIPFREE2. */
void pipfree_after_alarm(void)
{
    inhint();
    pipfree2();
}
