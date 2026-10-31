/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 4: the optics monitor, zeroing and drive of
 * T4RUPT_PROGRAM.agc, Comanche055 only (listing pages 160-169). The
 * faithful C is c64-port/src/cm_t4rupt_optics.c.
 *
 * OPTMODES keeps the optics' inbits as last sampled (bit 7, channel 30's
 * OCDU FAIL; bits 4 and 5, channel 33's mode switch) and its modes: bit
 * 10 zeroed since the last fresh start or restart, bit 9 return to coarse
 * optics, bit 3 zeroing in progress, bit 2 OCDU FAIL inhibited, bit 1 the
 * zeroing task working.
 *
 * Many of the listing's TCs are jumps that set Q on the way; the
 * functions below make each one as a call, so Q is what the listing
 * leaves. A CCS goes four ways (positive, +0, negative, -0), and several
 * here land on the code after the four words: each switch says where.
 *
 * The binding (src/binding/t4rupt_optics_routines.c) runs these in bank
 * 06 (OPTMON to OPFAILOF) or bank 10 (OPTDRIVE on), on the states the
 * faithful routines take. */
#include "optics.h"
#include "../agc/names.h"

#if COMANCHE055

/* DSPTAB +11D, the word of the DSKY's lamps (bit 8 OCDU FAIL), as T4RUPT
 * sends it. */
#define LAMPS (ADDRESS_OF(DSPTAB) + 11u)

/* Where ZOPFINI and COARSLOK go back to: the word after the TC (L+1) or
 * the one after that (L+2); or nowhere, ZOPFINI having gone to RESUME. */
enum back_to { NOWHERE, L_PLUS_1, L_PLUS_2 };

static void optmon33(void);
static void setsamp(void);
static void processw(void);
static void zoptdes(void);
static void ztoman(void);
static void ztocsc(void);
static enum back_to coarslok(void);
static enum back_to zopfini(void);
static enum back_to zopfin2(void);
static void canzopt(void);
static void manudes(void);
static void mantoman(void);
static void mantocsc(void);
static void mantocsc_plus3(void);
static void cscdes(void);
static void csctoman(void);
static void cancoars(void);
static void optzero(void);
static void setcnt(void);
static void setdesmd(void);
static void setzoend(void);
static void zeropcdu(void);
static void initzopt(void);
static void ocduftst(void);
static void opfailon(void);
static void setoff(void);
static void opfailof(void);
static void ozone(void);
static void contdrve(void);
static void opt2(void);
static void getopcmd(void);
static void storcmd(void);
static void cmdsetup(void);
static void cmdsetup_plus1(void);
static void truncmd(void);
static void posopcmd(void);
static void nextopt(void);
static void nextopt_plus1(void);
static void negopcmd(void);
static void delopcmd(void);
static void sendocmd(void);
static void setbit(void);

/* A word MASKed to one bit is + or +0, so a CCS of it either goes on
 * (the bit set) or skips one word (clear). */
static bool ccs_A_positive(void)
{
    return ccs_at(A_ADDRESS) == POSITIVE;
}

/* The listing's ABS(X) less a constant (CCS X; AD K; TCF +2; TCF -2, K
 * negative): after the CCS A holds |X| - 1, and both nonzero ways go
 * through AD K; for +0 or -0, A is left +0. */
static void abs_less(enum sign x, word k)
{
    if (x == POSITIVE || x == NEGATIVE) ad(k);
}

/* ======================================================================
 * The monitor (bank 06)
 * ====================================================================== */

/* OPTMON -- CM p. 160.
 * "MONITOR OPTICS INBITS IN CHAN 30 AND 33": "LOOK FOR OCDU FAIL BIT
 * CHANGE", keeping it in RUPTREG1 ("STORE CHANGE BIT"); with one,
 * OCDUFTST ("PROCESS OCDUFAIL BIT CHANGE"). On into 33OPTMON. */
void optmon(void)
{
    ca(ADDRESS_OF(OPTMODES));
    rxor_channel(CHAN30); /* EXTEND; RXOR CHAN30 */
    mask(BIT7);
    ts(ADDRESS_OF(RUPTREG1));
    if (ccs_A_positive()) {         /* CCS A */
        tc_at((word)(OPTMON + 6u)); /* TC OCDUFTST */
        ocduftst();
    }
    optmon33();
}

/* 33OPTMON -- CM p. 160.
 * "BYPASS IF TVC TAKEOVER" (OPTIND -0). Else "LOOK FOR OPTICS MODE SWITCH
 * CHANGE": add the switch bits' changes to RUPTREG1 ("STORE INBIT
 * CHANGES") and "UPDATE OPTMODES TO SHOW BIT CHANGES". Then "SAMPLE
 * CURRENT SWITCH SETTING" (the bits are 0 when set) into SWSAMPLE:
 * "MANUAL-SET ZERO", "CSC-SET SWSAMPLE POS", "ZOPTICS-SET SWSAMPLE (-1)". */
static void optmon33(void)
{
    if (ccs_at(ADDRESS_OF(OPTIND)) == IS_MINUS_ZERO) { /* TCF +4, +3, +2; TCF RESUME */
        continue_at(RESUME);
        return;
    }
    ca(ADDRESS_OF(OPTMODES));
    rxor_channel(CHAN33); /* EXTEND; RXOR CHAN33 */
    mask(OCTHIRTY);
    ads(ADDRESS_OF(RUPTREG1));
    lxch(ADDRESS_OF(OPTMODES));
    rxor_lchan(); /* EXTEND; RXOR LCHAN */
    ts(ADDRESS_OF(OPTMODES));
    com();
    mask(OCTHIRTY);
    if (A_is_zero()) { /* EXTEND; BZF SETSAMP */
        setsamp();
        return;
    }
    mask(BIT5); /* "SEE IF CSC" */
    if (ccs_A_positive())
        tc_at((word)(SETSAMP - 2u)); /* TC +2 */
    else
        set_A(NEGONE); /* CAF NEGONE */
    setsamp();
}

/* SETSAMP -- CM p. 160: "CURRENT OPTICS SWITCH SETTING". On into
 * PROCESSW. */
static void setsamp(void)
{
    ts(ADDRESS_OF(SWSAMPLE));
    processw();
}

/* PROCESSW -- CM p. 160: "BRANCH ON PREVIOUS SETTING" (DESOPMOD): CSC,
 * manual, zero optics. */
static void processw(void)
{
    switch (ccs_at(ADDRESS_OF(DESOPMOD))) {
    case POSITIVE:
        tc_at((word)(PROCESSW + 1u)); /* TC CSCDES */
        cscdes();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(PROCESSW + 2u)); /* TC MANUDES */
        manudes();
        return;
    case NEGATIVE:
        tc_at((word)(PROCESSW + 3u)); /* TC ZOPTDES */
        zoptdes();
        return;
    case IS_MINUS_ZERO: /* on into ZOPTDES */ zoptdes(); return;
    }
}

/* ZOPTDES -- CM p. 161.
 * The switch was at zero optics: "IS SWITCH STILL AT ZOPTICS". Now at CSC,
 * ZTOCSC; at manual, ZTOMAN. Still there: if zeroing is in progress
 * (ZOPFINI), count the 32 samples down (ZOPTCNT), and at the end set up
 * the zeroing's end (SETZOEND). */
static void zoptdes(void)
{
    enum back_to back = L_PLUS_1;

    switch (ccs_at(ADDRESS_OF(SWSAMPLE))) {
    case POSITIVE:
        tc_at((word)(ZOPTDES + 1u)); /* TC ZTOCSC: "NOW AT CSC" */
        ztocsc();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(ZOPTDES + 2u)); /* TC ZTOMAN: "MANUAL" */
        ztoman();
        return;
    case NEGATIVE:
        tc_at((word)(ZOPTDES + 3u)); /* TC ZOPFINI: "ZOPTICS-SEE IF ZOPT PROCESSING" */
        back = zopfini();
        break;
    case IS_MINUS_ZERO: /* lands on the TC SETDESMD */ break;
    }
    if (back == NOWHERE) return;
    if (back == L_PLUS_1) {
        tc_at((word)(ZOPTDES + 4u)); /* TC SETDESMD: "ZOPT NOT PROCESSING-NO ACTION" */
        setdesmd();
        return;
    }
    switch (ccs_at(ADDRESS_OF(ZOPTCNT))) { /* "ZOPT PROCESSING-CHECK COUNTER" */
    case POSITIVE:
        tc_at((word)(ZOPTDES + 6u)); /* TC SETCNT: "32 SAMPLE NOT FINISHED-SET COUNTER" */
        setcnt();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(ZOPTDES + 7u)); /* TC SETZOEND: "32 SAMPLE WAIT COMPLETED" */
        setzoend();
        return;
    case NEGATIVE: /* on into ZTOMAN */ ztoman(); return;
    case IS_MINUS_ZERO: /* lands on ZTOMAN's TC SETDESMD */
        tc_at((word)(ZTOMAN + 1u));
        setdesmd();
        return;
    }
}

/* ZTOMAN -- CM p. 161.
 * "ZOP TO MANUAL-IS ZOPT DONE": "YES-NORMAL EXIT". Else ZOPALARM: alarm
 * 116, "SWITCHED ALTERED WHILE ZOPTICS PROCESSING". */
static void ztoman(void)
{
    tc_at(ZTOMAN); /* TC ZOPFINI */
    switch (zopfini()) {
    case NOWHERE: return;
    case L_PLUS_1:
        tc_at((word)(ZTOMAN + 1u)); /* TC SETDESMD */
        setdesmd();
        return;
    case L_PLUS_2:
        tc_at(ZOPALARM); /* TC ALARM; OCT 00116 */
        continue_at(ALARM);
        return;
    }
}

/* ZOPALARM +2 -- CM p. 161, where ALARM returns: "SET RETURN OPTION"
 * (WTOPTION 11, how many passes the switch may be back at zero optics to
 * go on with the zeroing), "CANCEL ZOPT", and SETDESMD. */
void zopalarm_after_alarm(void)
{
    set_A(OCT13); /* CAF OCT13 */
    ts(ADDRESS_OF(WTOPTION));
    tc_at((word)(ZOPALARM + 4u)); /* TC CANZOPT */
    canzopt();
    tc_at((word)(ZOPALARM + 5u)); /* TC SETDESMD */
    setdesmd();
}

/* ZTOCSC -- CM p. 161.
 * "SEE IF ZOPT PROCESSING": no, "CHECK RETURN TO COARS OPT" (MANTOCSC
 * +3); yes, alarm 116, then cancel the zeroing and MANTOCSC. */
static void ztocsc(void)
{
    tc_at(ZTOCSC); /* TC ZOPFINI */
    switch (zopfini()) {
    case NOWHERE: return;
    case L_PLUS_1:
        tc_at((word)(ZTOCSC + 1u)); /* TC MANTOCSC +3 */
        mantocsc_plus3();
        return;
    case L_PLUS_2:
        tc_at((word)(ZTOCSC + 2u)); /* TC ALARM; OCT 00116: "ZOPT PROCESSING-ALARM" */
        continue_at(ALARM);
        return;
    }
}

/* ZTOCSC +4, where ALARM returns: "CANCEL ZOPT", then MANTOCSC ("ZERO
 * CNT-LOOK FOR COARS OPT RETURN"). */
void ztocsc_after_alarm(void)
{
    tc_at((word)(ZTOCSC + 4u)); /* TC CANZOPT */
    canzopt();
    tc_at((word)(ZTOCSC + 5u)); /* TC MANTOCSC */
    mantocsc();
}

/* COARSLOK -- CM p. 161: "IF COARS OPT SINCE FSTART GO TO L+2, IF NOT GO
 * TO L+1" (OPTMODES bit 9). */
static enum back_to coarslok(void)
{
    set_A(BIT9); /* CAF BIT9; TCF ZOPFIN2 */
    return zopfin2();
}

/* ZOPFINI -- CM p. 161 (typed ZOPFIN1 in this repository's copy of the
 * listing, which notes that it should read ZOPFINI).
 * "SEE IF END ZOPT TASK WORKING" (OPTMODES bit 1): if so, "WAIT ONE
 * SAMPLE PERIOD" (RESUME). Else "TEST IF ZOPTICS PROCESSING" (bit 3):
 * L+2 if so, L+1 if not. */
static enum back_to zopfini(void)
{
    set_A(BIT1);
    mask_at(ADDRESS_OF(OPTMODES));
    if (ccs_A_positive()) {          /* CCS A */
        tc_at((word)(ZOPFINI + 3u)); /* TC RESUME */
        continue_at(RESUME);
        return NOWHERE;
    }
    set_A(BIT3); /* CAF BIT3 */
    return zopfin2();
}

/* ZOPFIN2, ZOPFIN3 -- CM p. 161: the OPTMODES bit in A set, return to
 * L+2 (INCR Q; TC Q), else to L+1. */
static enum back_to zopfin2(void)
{
    bool set;

    mask_at(ADDRESS_OF(OPTMODES));
    set = ccs_A_positive();   /* ZOPFIN3: CCS A */
    if (set) incr(Q_ADDRESS); /* INCR Q */
    tc_through(Q_ADDRESS);    /* TC Q */
    return set ? L_PLUS_2 : L_PLUS_1;
}

/* CANZOPT -- CM p. 161: "CANCEL ZERO OPTICS": "ZERO ZOPT PROCESSING
 * BIT-ENABLE OCDUFAIL" (OPTMODES bits 3 and 2), and "MAKE SURE ZERO OCDU
 * IS OFF" (channel 12 bit 1). */
static void canzopt(void)
{
    set_A(negate(SIX)); /* CS SIX */
    mask_at(ADDRESS_OF(OPTMODES));
    ts(ADDRESS_OF(OPTMODES));
    set_A(negate(BIT1)); /* CS BIT1 */
    wand_channel(CHAN12);
    tc_through(Q_ADDRESS); /* TC Q */
}

/* MANUDES -- CM p. 162.
 * The switch was at manual: "SEE IF SWITCH STILL IN MANUAL MODE". Now at
 * CSC, MANTOCSC; still manual, MANTOMAN. Now at zero optics: "LOOK AT
 * ZOPTICS RETURN OPTION": with time left, "CONTINUE ZOPTICS"; without,
 * "ZOPTICS MUST START ANEW" (OPTZERO). */
static void manudes(void)
{
    switch (ccs_at(ADDRESS_OF(SWSAMPLE))) {
    case POSITIVE:
        tc_at((word)(MANUDES + 1u)); /* TC MANTOCSC: "NOW AT CSC" */
        mantocsc();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(MANUDES + 2u)); /* TC MANTOMAN: "STILL MANUAL" */
        mantoman();
        return;
    case NEGATIVE:
        switch (ccs_at(ADDRESS_OF(WTOPTION))) {
        case POSITIVE:
            tc_at((word)(MANUDES + 4u)); /* TC +2: "5 SEC RETURN GOOD-CONTINUE ZOPTICS" */
            break;
        case IS_PLUS_ZERO:
            tc_at((word)(MANUDES + 5u)); /* TC OPTZERO */
            optzero();
            return;
        case NEGATIVE: /* lands on the TC INITZOPT */ break;
        case IS_MINUS_ZERO: /* lands on the TC SETDESMD */
            tc_at((word)(MANUDES + 7u));
            setdesmd();
            return;
        }
        break;
    case IS_MINUS_ZERO: /* lands on the TC +2 */ tc_at((word)(MANUDES + 4u)); break;
    }
    tc_at((word)(MANUDES + 6u)); /* TC INITZOPT: "SHOW ZERO OPTICS PROCESSING" */
    initzopt();
    tc_at((word)(MANUDES + 7u)); /* TC SETDESMD: "NORMAL EXIT" */
    setdesmd();
}

/* MANTOMAN -- CM p. 162: "DECREMENT RETURN OPTION TIME" (WTOPTION), and
 * SETDESMD. A negative WTOPTION lands on MANTOCSC; -0 on MANTOCSC +1, with
 * A +0, as MANTOCSC's CAF ZERO leaves it. */
static void mantoman(void)
{
    switch (ccs_at(ADDRESS_OF(WTOPTION))) {
    case POSITIVE: ts(ADDRESS_OF(WTOPTION)); break;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
    case IS_MINUS_ZERO: mantocsc(); return;
    }
    tc_at((word)(MANTOMAN + 2u)); /* TC SETDESMD */
    setdesmd();
}

/* MANTOCSC -- CM p. 162: "CANCEL ZOPT RETURN OPTION IF SET" (and the
 * sample count), then MANTOCSC +3. */
static void mantocsc(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(WTOPTION));
    ts(ADDRESS_OF(ZOPTCNT));
    mantocsc_plus3();
}

/* MANTOCSC +3 -- CM p. 162: "CHECK FOR COARS OPT RETURN" (COARSLOK). With
 * none, "NO ACTION"; else "SET COARS OPT WORKING" (OPTIND +1) and "ENABLE
 * OPTICS CDU ERROR CNTS" (channel 12 bit 2). */
static void mantocsc_plus3(void)
{
    tc_at((word)(MANTOCSC + 3u)); /* TC COARSLOK */
    if (coarslok() == L_PLUS_1) {
        tc_at((word)(MANTOCSC + 4u)); /* TC SETDESMD */
        setdesmd();
        return;
    }
    set_A(ONE); /* CAF ONE */
    ts(ADDRESS_OF(OPTIND));
    set_A(BIT2); /* CAF BIT2 */
    wor_channel(CHAN12);
    tc_at((word)(MANTOCSC + 012u)); /* TC SETDESMD */
    setdesmd();
}

/* CSCDES, CSCTOZOP -- CM p. 162.
 * The switch was at CSC: "SEE IF SWITCH STILL AT CSC". Now manual,
 * CSCTOMAN. Now zero optics (CSCTOZOP): "INITIALIZE FOR ZOPT" (32
 * samples), then on into CSCTOMAN as INITZOPT returns. */
static void cscdes(void)
{
    switch (ccs_at(ADDRESS_OF(SWSAMPLE))) {
    case POSITIVE:
        tc_at((word)(CSCDES + 1u)); /* TC SETDESMD: "STILL AT CSC" */
        setdesmd();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(CSCDES + 2u)); /* TC CSCTOMAN: "MANUAL" */
        csctoman();
        return;
    case NEGATIVE: set_A(OCT40); break; /* CSCTOZOP: CAF OCT40 */
    case IS_MINUS_ZERO: break;          /* lands on its TS ZOPTCNT, A +0 */
    }
    ts(ADDRESS_OF(ZOPTCNT));
    tc_at((word)(CSCTOZOP + 2u)); /* TC INITZOPT */
    initzopt();
    csctoman();
}

/* CSCTOMAN -- CM p. 162: "SEE IF COARS WORKING" (OPTIND + or +0): "SWITCH
 * NOT CSC-KILL COARS". Else "NO COARS-NORMAL EXIT". */
static void csctoman(void)
{
    switch (ccs_at(ADDRESS_OF(OPTIND))) {
    case POSITIVE:
        tc_at((word)(CSCTOMAN + 1u)); /* TC CANCOARS */
        cancoars();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(CSCTOMAN + 2u)); /* TC CANCOARS */
        cancoars();
        return;
    case NEGATIVE: tc_at((word)(CSCTOMAN + 3u)); break; /* TC +1 */
    case IS_MINUS_ZERO: break;
    }
    tc_at((word)(CSCTOMAN + 4u)); /* TC SETDESMD */
    setdesmd();
}

/* CANCOARS -- CM p. 163: "SET OPTIND (-1) TO SHOW NOT WORKING", "DISABLE
 * OCDU ERR CNTS" (channel 12 bit 2), and "SET RETURN-TO-COARS BIT"
 * (OPTMODES bit 9). */
static void cancoars(void)
{
    set_A(NEGONE); /* CA NEGONE */
    ts(ADDRESS_OF(OPTIND));
    set_A(negate(BIT2)); /* CS BIT2 */
    wand_channel(CHAN12);
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT9);
    ads(ADDRESS_OF(OPTMODES));
    tc_at((word)(CANCOARS + 010u)); /* TC SETDESMD */
    setdesmd();
}

/* OPTZERO -- CM p. 163: "INITIALIZE ZERO OPTICS" and "SET UP 32 SAMPLE
 * WAIT". On into SETCNT. */
static void optzero(void)
{
    tc_at(OPTZERO); /* TC INITZOPT */
    initzopt();
    set_A(OCT40); /* CA OCT40 */
    setcnt();
}

/* SETCNT -- CM p. 163: the sample count (ZOPTCNT). On into SETDESMD. */
static void setcnt(void)
{
    ts(ADDRESS_OF(ZOPTCNT));
    setdesmd();
}

/* SETDESMD -- CM p. 163: "SET CURRENT SWITCH INDICATION-RESUME"
 * (DESOPMOD). */
static void setdesmd(void)
{
    ca(ADDRESS_OF(SWSAMPLE));
    ts(ADDRESS_OF(DESOPMOD));
    tc_at((word)(SETDESMD + 2u)); /* TC RESUME */
    continue_at(RESUME);
}

/* SETZOEND -- CM p. 163: "SEND ZERO OPTICS CDU" (channel 12 bit 1), and
 * "HOLD ZERO CDU FOR 200 MS": ENDZOPT as a task in 200 ms. */
static void setzoend(void)
{
    set_A(BIT1); /* CAF BIT1 */
    wor_channel(CHAN12);
    set_A(K200MS);                /* CA 200MS */
    tc_at((word)(SETZOEND + 4u)); /* TC WAITLIST; 2CADR ENDZOPT */
    continue_at(WAITLIST);
}

/* SETZOEND +7, where WAITLIST returns: "SHOW ZOPTICS TASK WORKING"
 * (OPTMODES bit 1), and SETDESMD. */
void setzoend_after_waitlist(void)
{
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT1);
    ads(ADDRESS_OF(OPTMODES));
    tc_at((word)(SETZOEND + 012u)); /* TC SETDESMD */
    setdesmd();
}

/* ENDZOPT -- CM p. 163, a task: "ZERO OCDU COUNTERS", "TURN OFF ZERO
 * OCDU" (channel 12 bit 1), and "DELAY 200MS FOR CDUS TO RESYNCHRONIZE"
 * (VARDELAY). */
void endzopt(void)
{
    tc_at(ENDZOPT); /* TC ZEROPCDU */
    zeropcdu();
    set_A(negate(BIT1)); /* CS BIT1 */
    wand_channel(CHAN12);
    set_A(K200MS);               /* CAF 200MS */
    tc_at((word)(ENDZOPT + 5u)); /* TC VARDELAY */
    continue_at(VARDELAY);
}

/* ENDZOPT +6 -- CM p. 163-164, where VARDELAY returns: "SHOW ZOPTICS SINCE
 * LAST FRESH START OR RESTART" (OPTMODES bit 10), "ENABLE OCDUFAIL-SHOW
 * OPTICS COMPLETE" (bits 3, 2 and 1 cleared), "CHECK OCDU FAIL BIT AFTER
 * ENABLE" (OCDUFTST), and end the task. */
void endzopt_after_vardelay(void)
{
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT10);
    ads(ADDRESS_OF(OPTMODES));
    set_A(negate(SEVEN)); /* CS SEVEN */
    mask_at(ADDRESS_OF(OPTMODES));
    ts(ADDRESS_OF(OPTMODES));
    tc_at((word)(ENDZOPT + 014u)); /* TC OCDUFTST */
    ocduftst();
    tc_at((word)(ENDZOPT + 015u)); /* TC TASKOVER */
    continue_at(TASKOVER);
}

/* ZEROPCDU -- CM p. 164: "ZERO IN CDUS, -20 IN CDUT" (degrees), and
 * "INITIALIZE SHAFT MONITOR ZONE" (ZONE +0). */
static void zeropcdu(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(CDUS));
    ts(ADDRESS_OF(ZONE));
    set_A(negate(K20DEGS)); /* CS 20DEGS */
    ts(ADDRESS_OF(CDUT));
    tc_through(Q_ADDRESS); /* TC Q */
}

/* INITZOPT -- CM p. 164: "INITIALIZE ZOPTICS-INHIBIT OCDUFAIL AND SHOW
 * OPTICS PROCESSING" (WTOPTION +0, OPTMODES bits 3 and 2). */
static void initzopt(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(WTOPTION));
    cs(ADDRESS_OF(OPTMODES));
    mask(SIX);
    ads(ADDRESS_OF(OPTMODES));
    tc_through(Q_ADDRESS); /* TC Q */
}

/* OCDUFTST -- CM p. 165.
 * "SEE IF OCDUFAIL ON OR OFF" (channel 30 bit 7, 0 when failed). Off:
 * OPFAILOF. On: "OCDUFAIL LIGHT ON UNLESS INHIBITED" (OPTMODES bit 2),
 * OPFAILON. Returns (TC Q). */
static void ocduftst(void)
{
    set_A(BIT7); /* CAF BIT7 */
    rand_channel(CHAN30);
    if (ccs_A_positive()) { /* TCF OPFAILOF: "OCDUFAIL LIGHT OFF" */
        opfailof();
        return;
    }
    set_A(BIT2); /* CAF BIT2 */
    mask_at(ADDRESS_OF(OPTMODES));
    if (ccs_A_positive()) {    /* "OCDUFAIL INHIBITED" */
        tc_through(Q_ADDRESS); /* TC Q */
        return;
    }
    opfailon();
}

/* OPFAILON -- CM p. 165: "ON BIT" (bit 8 of the lamps' word, if not on
 * already). On into SETOFF. */
static void opfailon(void)
{
    set_A(BIT8); /* CAF BIT8 */
    ad_at(LAMPS);
    mask(BIT8);
    setoff();
}

/* SETOFF -- CM p. 165: A is the lamp bit to flip, or +0 for "NO CHANGE".
 * Flip it in the lamps' word, with bit 15 to send it ("SHOW ACTION
 * WANTED"). */
static void setoff(void)
{
    if (A_is_zero()) {         /* EXTEND; BZF TCQ */
        tc_through(Q_ADDRESS); /* TCQ is a TC Q */
        return;
    }
    ts(L_ADDRESS);
    ca(LAMPS);
    rxor_lchan(); /* EXTEND; RXOR LCHAN */
    mask(POSMAX);
    ad(BIT15);
    ts(LAMPS);
    tc_through(Q_ADDRESS); /* TC Q */
}

/* OPFAILOF -- CM p. 165: "DON'T TURN OFF IF LAMP TEST" (IMODES33 bit 1).
 * Else "TURN OFF OCDUFAIL LIGHT" (bit 8, if on), at SETOFF. */
static void opfailof(void)
{
    set_A(BIT1); /* CAF BIT1 */
    mask_at(ADDRESS_OF(IMODES33));
    if (ccs_A_positive()) {    /* "LAMP TEST IN PROGRESS" */
        tc_through(Q_ADDRESS); /* TC Q */
        return;
    }
    set_A(BIT8); /* CAF BIT8 */
    mask_at(LAMPS);
    setoff(); /* TCF SETOFF */
}

/* ======================================================================
 * The drive (bank 10)
 * ====================================================================== */

/* OPTDRIVE -- CM p. 166.
 * "SHAFT STOP MONITOR-ZONE UPDATE": with the shaft CDU (CDUS) "LESS THAN
 * 45 DEG", zone 0 (OZONE). Past 45 degrees, a zone 0 becomes the shaft's
 * sign ("GREATER THAN 45 DEG-SET ZONE TO SIGN CDU"); "OTHERWISE DON'T MESS
 * WITH ZONE". On into CONTDRVE. */
void optdrive(void)
{
    ca(ADDRESS_OF(CDUS));
    ts(L_ADDRESS);
    abs_less(ccs_at(A_ADDRESS), MINUS_45DEGS); /* "ABS(CDUS) - 45 DEG" */
    if (A_is_zero_or_minus()) {                /* EXTEND; BZMF OZONE */
        ozone();
        return;
    }
    ca(ADDRESS_OF(ZONE));
    if (!A_is_zero()) { /* EXTEND; BZF +2; TCF CONTDRVE */
        contdrve();
        return;
    }
    xch(L_ADDRESS);
    ts(ADDRESS_OF(ZONE)); /* TCF OZONE +1 */
    contdrve();
}

/* OZONE -- CM p. 166: "ZONE ZERO". On into CONTDRVE. */
static void ozone(void)
{
    set_A(ZERO); /* CAF ZERO */
    ts(ADDRESS_OF(ZONE));
    contdrve();
}

/* CONTDRVE -- CM p. 166.
 * "WORK COARS OPTICS" while OPTIND is + or +0 ("NO OPT" when -), and the
 * switch is at CMC (SWSAMPLE positive: "ZERO (-1) MANUAL (+0)" resume).
 * "SEE IF OCDUS ZEROED SINCE LAST FSTART" (OPTMODES bit 10): if not,
 * alarm 120 first. */
static void contdrve(void)
{
    switch (ccs_at(ADDRESS_OF(OPTIND))) {
    case POSITIVE: tc_at((word)(CONTDRVE + 1u)); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at((word)(CONTDRVE + 2u)); break; /* TC +3 */
    case NEGATIVE:
        tc_at((word)(CONTDRVE + 3u)); /* TC RESUME */
        continue_at(RESUME);
        return;
    case IS_MINUS_ZERO:
        tc_at((word)(CONTDRVE + 4u)); /* TC RESUME */
        continue_at(RESUME);
        return;
    }
    ca(ADDRESS_OF(SWSAMPLE));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF RESUME */
        continue_at(RESUME);
        return;
    }
    set_A(BIT10); /* CAF BIT10 */
    mask_at(ADDRESS_OF(OPTMODES));
    if (!ccs_A_positive()) {            /* CCS A */
        tc_at((word)(CONTDRVE + 014u)); /* TC ALARM; OCT 00120: "OPTICS NOT ZEROED" */
        continue_at(ALARM);
        return;
    }
    tc_at((word)(CONTDRVE + 013u)); /* TC +3 */
    contdrve_after_alarm();
}

/* CONTDRVE +016 -- CM p. 166, also where ALARM returns: "SEE IF ERR CNTS
 * ENABLED" (channel 12 bit 2). If not, SETBIT. Else the commands, shaft
 * first (OPTIND 1, then 0). */
void contdrve_after_alarm(void)
{
    set_A(BIT2); /* CA BIT2 */
    rand_channel(CHAN12);
    if (A_is_zero()) { /* EXTEND; BZF SETBIT: "CNTS NOT ENABLED-DO IT AND RESUME" */
        setbit();
        return;
    }
    set_A(ONE); /* CAF ONE: "INITIALIZE OPTIND" */
    opt2();
}

/* OPT2 -- CM p. 167: OPTIND from A; at 0, "CHECK TRUNION COMMAND"
 * (TRUNCMD). Else GETOPCMD. */
static void opt2(void)
{
    ts(ADDRESS_OF(OPTIND));
    if (A_is_zero()) { /* EXTEND; BZF TRUNCMD */
        truncmd();
        return;
    }
    getopcmd();
}

/* GETOPCMD -- CM p. 167.
 * The CDU OPTIND names (1 the shaft, 0 the trunnion): "PICK UP DESIRED OPT
 * ANGLE" (DESOPTS or DESOPTT) and "GET DIFFERENCE" from the CDU angle
 * (MSU), scaled by MP BIT13 and DOUBLE, its overflow kept in L. On into
 * STORCMD. */
static void getopcmd(void)
{
    word cdu = fetch(ADDRESS_OF(OPTIND));

    ca(indexed(ADDRESS_OF(DESOPTT), cdu));  /* INDEX OPTIND; CA DESOPTT */
    msu_at(indexed(ADDRESS_OF(CDUT), cdu)); /* EXTEND; INDEX OPTIND; MSU CDUT */
    mp(BIT13);                              /* EXTEND; MP BIT13 */
    xch(L_ADDRESS);
    double_A();
    if (ts(ADDRESS_OF(ITEMP1))) /* TS ITEMP1; TCF +2: "NO OVFL" */
        ads(L_ADDRESS);         /* "WITH OVFL" */
    storcmd();
}

/* STORCMD -- CM p. 167: "STORE COMMAND" (COMMANDO +OPTIND), then "GET NEXT
 * COMMAND" (OPT2) while OPTIND is 1. After the trunnion's (OPTIND 0, +0
 * after the CCS): "INITIALIZE SEND INDICATOR TO ZERO" (ITEMP1), and the
 * shaft stop avoidance. */
static void storcmd(void)
{
    lxch(
        indexed(ADDRESS_OF(COMMANDO), fetch(ADDRESS_OF(OPTIND)))); /* INDEX OPTIND; LXCH COMMANDO */
    if (ccs_at(ADDRESS_OF(OPTIND)) == POSITIVE) {                  /* CCS OPTIND; TCF OPT2 */
        opt2();
        return;
    }
    ts(ADDRESS_OF(ITEMP1));

    /* "SHAFT STOP AVOIDANCE": "IF CDUS GREATER THAN + OR - 90 DEG CHECK FOR
     * POSSIBLE STOP PROBLEM": in a nonzero zone, a shaft command whose
     * sign is the zone's, to a DESOPTS "IN FIRST OR FOURTH QUAD", is
     * "REVERSE[D]". */
    abs_less(ccs_at(ADDRESS_OF(CDUS)), NEG1_2);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF CMDSETUP: "CDU LESS THAN 90 DEG, NO PROBLEMS" */
        cmdsetup();
        return;
    }
    ca(ADDRESS_OF(ZONE));
    if (A_is_zero()) { /* EXTEND; BZF CMDSETUP: "NORMAL COMMAND" */
        cmdsetup();
        return;
    }
    mask(BIT15); /* "GRAB SIGN OF ZONE" */
    ts(L_ADDRESS);
    ca(ADDRESS_OF(COMMANDO) + 1u);
    mask(BIT15);  /* "GRAB SIGN OF SHAFT COMMAND" */
    rxor_lchan(); /* EXTEND; RXOR LCHAN */
    /* Signs alike leave +0. Signs that differ leave A negative, landing on
     * the DESOPTS test's AD NEG1/2 with |A| - 1, positive after it:
     * CMDSETUP all the same. */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: cmdsetup(); return; /* TCF CMDSETUP: "SIGN ZONE NOT EQUAL TO SIGN COMMAND" */
    case IS_PLUS_ZERO:
        abs_less(ccs_at(ADDRESS_OF(DESOPTS)), NEG1_2); /* "ABS(DESOPTS) - 90 DEG" */
        break;
    case NEGATIVE: ad(NEG1_2); break;
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        cs(ADDRESS_OF(COMMANDO) + 1u);
        ts(ADDRESS_OF(COMMANDO) + 1u);
    }
    cmdsetup();
}

/* CMDSETUP -- CM p. 168: "SET OPTIND" 1, the shaft. */
static void cmdsetup(void)
{
    set_A(ONE); /* CAF ONE */
    cmdsetup_plus1();
}

/* CMDSETUP +1 -- CM p. 168: OPTIND from A, and by the "SIGN OF COMMAND"
 * (COMMANDO +OPTIND) POSOPCMD or NEGOPCMD; a "ZERO COMMAND-SKIP SEND
 * INDICATOR" (NEXTOPT +1). */
static void cmdsetup_plus1(void)
{
    ts(ADDRESS_OF(OPTIND));
    switch (ccs_at(indexed(ADDRESS_OF(COMMANDO), a_word()))) { /* INDEX A; CCS COMMANDO */
    case POSITIVE:
        tc_at((word)(CMDSETUP + 4u)); /* TC POSOPCMD */
        posopcmd();
        return;
    case IS_PLUS_ZERO:
        tc_at((word)(CMDSETUP + 5u)); /* TC NEXTOPT +1 */
        nextopt_plus1();
        return;
    case NEGATIVE:
        tc_at((word)(CMDSETUP + 6u)); /* TC NEGOPCMD */
        negopcmd();
        return;
    case IS_MINUS_ZERO:
        tc_at((word)(CMDSETUP + 7u)); /* TC NEXTOPT +1 */
        nextopt_plus1();
        return;
    }
}

/* TRUNCMD -- CM p. 168.
 * "IF COMMAND GREATER THAN 45 DEG-COMMAND 45 DEG": DESOPTT - CDUT
 * overflowing (TS Q skips) is "GREATER THAN 45 DEG-USE POSMAX WITH CORRECT
 * SIGN" (A +1 or -1 after the TS) as the trunnion's command, at STORCMD.
 * Else "LESS THAN 45 DEG-NORMAL OPERATION" (GETOPCMD). */
static void truncmd(void)
{
    cs(ADDRESS_OF(CDUT));
    ad_at(ADDRESS_OF(DESOPTT));
    if (!ts(Q_ADDRESS)) {
        tc_at((word)(TRUNCMD + 3u)); /* TC GETOPCMD */
        getopcmd();
        return;
    }
    if (ccs_A_positive()) {          /* CCS A */
        set_A(POSMAX);               /* CA POSMAX */
        tc_at((word)(TRUNCMD + 6u)); /* TC +2 */
    } else {
        set_A(negate(POSMAX)); /* CS POSMAX */
    }
    ts(L_ADDRESS);
    tc_at((word)(TRUNCMD + 011u)); /* TC STORCMD */
    storcmd();
}

/* POSOPCMD -- CM p. 168: a command (A, its size less one) more than
 * MAXPLS1's pulses goes out as MAXPLS's ("GREATER THAN MAX PULSE-USE MAX
 * PULSE"); one less, whole (DELOPCMD). */
static void posopcmd(void)
{
    ad(MAXPLS1);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DELOPCMD: "COMMAND LESS THAN MAX PULSE" */
        delopcmd();
        return;
    }
    set_A(negate(MAXPLS)); /* CS MAXPLS */
    nextopt();
}

/* NEXTOPT -- CM p. 168: "SET SEND INDICATOR" (ITEMP1). On into NEXTOPT
 * +1. */
static void nextopt(void)
{
    incr(ADDRESS_OF(ITEMP1));
    nextopt_plus1();
}

/* NEXTOPT +1 -- CM p. 168: "MAKE SURE ZERO COMMAND IS -ZERO", "STORE PULSE
 * IN SEND REG" (CDUTCMD +OPTIND), and "GET NEXT OPT" while OPTIND is 1.
 * Then, if "ANY PULSES TO GO" (ITEMP1, counted up from +0), SENDOCMD. */
static void nextopt_plus1(void)
{
    ad(NEG0);
    ts(indexed(ADDRESS_OF(CDUTCMD), fetch(ADDRESS_OF(OPTIND)))); /* INDEX OPTIND; TS CDUTCMD */
    if (ccs_at(ADDRESS_OF(OPTIND)) == POSITIVE) {                /* CCS OPTIND */
        tc_at((word)(NEXTOPT + 5u));                             /* TC CMDSETUP +1 */
        cmdsetup_plus1();
        return;
    }
    if (ccs_at(ADDRESS_OF(ITEMP1)) == POSITIVE) { /* CCS ITEMP1; TCF SENDOCMD */
        sendocmd();
        return;
    }
    tc_at((word)(NEXTOPT + 010u)); /* TC RESUME: "NO" */
    continue_at(RESUME);
}

/* NEGOPCMD -- CM p. 168: as POSOPCMD, for a negative command. */
static void negopcmd(void)
{
    ad(MAXPLS1);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF DELOPCMD: "LESS THAN MAX PULSE" */
        delopcmd();
        return;
    }
    set_A(MAXPLS); /* CA MAXPLS: "MAX PULSES"; TCF NEXTOPT */
    nextopt();
}

/* DELOPCMD -- CM p. 169: "SET UP SMALL COMMAND": the whole command
 * (COMMANDO +OPTIND, which keeps A, what is left of it after MAXPLS1). */
static void delopcmd(void)
{
    xch(indexed(ADDRESS_OF(COMMANDO), fetch(ADDRESS_OF(OPTIND)))); /* INDEX OPTIND; XCH COMMANDO */
    nextopt();
}

/* SENDOCMD -- CM p. 169: "SEND OCDU DRIVE COMMANDS" (channel 14 bits 11
 * and 12). */
static void sendocmd(void)
{
    set_A(BITS11_12); /* CAF 11,12 */
    wor_channel(CHAN14);
    tc_at((word)(SENDOCMD + 3u)); /* TC RESUME */
    continue_at(RESUME);
}

/* SETBIT -- CM p. 169: "ENABLE OCDU ERR CNTS" (channel 12 bit 2), and
 * "START COARS NEXT TIME AROUND". */
static void setbit(void)
{
    set_A(BIT2); /* CAF BIT2 */
    wor_channel(CHAN12);
    tc_at((word)(SETBIT + 3u)); /* TC RESUME */
    continue_at(RESUME);
}

#endif
