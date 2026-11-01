/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 8: the lights test and ERROR
 * LIGHT RESET -- LM pp. 467-471, CM pp. 385-389. The faithful C is the
 * port's Pinball modules (c64-port/src/lanec_pin_head_lamp.c and
 * dsky_lamp_test.c for the test, lanec_pin_gaps.c for the words WAITLIST
 * and NOVAC return to, dsky_charin_reset.c for RSET).
 *
 * "VBTSTLTS TURNS ON ALL DISPLAY PANEL LIGHTS. AFTER 5 SEC, IT TURNS OFF
 * THE CAUTION AND STATUS LIGHTS." It is verb 35 ("VB35 TEST LIGHTS", in
 * VERBTAB). The test runs in three pieces: the keyboard job lights
 * everything and asks WAITLIST for TSTLTS2 in five seconds; that task asks
 * the Executive for a job, TSTLTS3; the job puts the lights out, the major
 * mode back on (DSPMM), the monitor killed (KILMONON), the flash off, and
 * goes to TSTLTS4 (part 6), which releases the display.
 *
 * ERROR LIGHT RESET, the RSET key, "TURNS OFF, UPLINK ACTIVITY, AUTO,
 * HOLD, FREE, OPERATOR ERROR, PROG ALM, TRACKER FAIL. LEAVES GIMBAL LOCK
 * AND NO ATT ALONE. IT ALSO ZEROES THE :TEST ALARM: OUT BIT, WHICH TURNS
 * OFF STBY, RESTART, ETC. IT ALSO SETS :CAUTION RESET: TO 1. IT ALSO
 * FORCES BIT 12 OF ALL DSPTAB ENTRIES TO 1."
 *
 * The missions differ in a few words: Comanche055's test first calls
 * CHKPOOH by BANKCALL (back at VBTSTLTS +2) and keeps interrupts inhibited
 * while it calls WAITLIST; for the radar fail flags (Luminary099's
 * RADMODES) Comanche055 sets the optics' bit 7 of OPTMODES.
 *
 * Each function runs from a label, or from the word a call returns to, to
 * a call or jump to another routine, which ends it with Q and Z set. A
 * call the faithful port runs within its step is a C call: KILMONON and
 * FLASHOFF at the end of the test. */
#include "pinball.h"
#include "../agc/names.h"

/* ---- The lights test, verb 35 (bank 41) ---- */

#if LUMINARY099
#define TSTLTS1_WAITLIST (TSTLTS1 + 15u) /* TC WAITLIST */
#define TSTLTS3_BANKCALL (TSTLTS3 + 25u) /* TC BANKCALL, CADR DSPMM */
#else
#define TSTLTS1_WAITLIST (TSTLTS1 + 13u)
#define TSTLTS3_BANKCALL (TSTLTS3 + 24u)
#endif

static void lights_on(void);

#if LUMINARY099
/* VBTSTLTS -- LM p. 467. */
void vbtstlts(void)
{
    lights_on();
}
#else
/* VBTSTLTS -- CM p. 385. CHKPOOH first, by BANKCALL. */
void vbtstlts(void)
{
    tc(VBTSTLTS, BANKCALL); /* CADR CHKPOOH */
}

/* VBTSTLTS +2, where CHKPOOH returns. */
void vbtstlts_plus2(void)
{
    lights_on();
}
#endif

/* The lights on, interrupts inhibited. "SET BIT 1 OF IMODES33 SO IMUMON
 * WONT TURN OUT ANY LAMPS." "TURN ON UPLINK ACTIVITY, TEMP, KEY RLSE, V/N
 * FLASH, OPERATOR ERROR" (TSTCON1, channel 11). "TURN ON NO ATT, GIMBAL
 * LOCK, TRACKER, PROG ALM" (TSTCON2, DSPTAB +11D). "TURN ON TEST ALARM
 * OUTBIT" (BIT10 of channel 13). Then the count of DSPTAB words, TEN,
 * into TSTLTS1, whose first pass the faithful port runs within this step:
 * a C call. */
static void lights_on(void)
{
    inhint();
    set_A(negate(BIT1));
    mask_at(ADDRESS_OF(IMODES33));
    ad(BIT1);
    ts(ADDRESS_OF(IMODES33));
    set_A(TSTCON1);
    wor_dsalmout();
    set_A(TSTCON2);
    ts(ADDRESS_OF(DSPTAB) + 11u);
    set_A(BIT10);
    wor_channel(CHAN13);
    set_A(TEN);
    tstlts1();
}

/* TSTLTS1 -- one DSPTAB word, the count in A, "DISPLAY ALL 8:S" (FULLDSP
 * complemented: a negative DSPTAB word is one still to be sent to the
 * DSKY), and back to TSTLTS1 for the next, down to DSPTAB +0. Then "TURN ON 3 PLUS
 * SIGNS" (FULLDSP1 in DSPTAB +1, +4 and +6), NOUT ELEVEN (all the words to
 * be sent), and WAITLIST for TSTLTS2 in five seconds (SHOLTS). Luminary099
 * lets interrupts in for a moment around CAF SHOLTS. */
void tstlts1(void)
{
    if (ts(ADDRESS_OF(ERCNT))) not_modelled();
    set_A(negate(FULLDSP));
    ts(indexed_erasable(ADDRESS_OF(DSPTAB), fetch(ADDRESS_OF(ERCNT))));
    switch (ccs_at(ADDRESS_OF(ERCNT))) {
    case POSITIVE: tc(TSTLTS1 + 5u, TSTLTS1); return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* a count of 0 to 10 is never negative */
    }
    set_A(negate(FULLDSP1));
    ts(ADDRESS_OF(DSPTAB) + 1u);
    ts(ADDRESS_OF(DSPTAB) + 4u);
    ts(ADDRESS_OF(DSPTAB) + 6u);
    set_A(ELEVEN);
    ts(ADDRESS_OF(NOUT));
#if LUMINARY099
    relint();
    set_A(SHOLTS);
    inhint();
#else
    set_A(SHOLTS);
#endif
    tc(TSTLTS1_WAITLIST, WAITLIST); /* 2CADR TSTLTS2 */
}

/* The word WAITLIST returns to: the keyboard job ends. "DSPLOCK IS LEFT
 * BUSY (FROM KEYBOARD ACTION) UNTIL TSTLTS3 TO INSURE THAT LIGHTS TEST
 * WILL BE SEEN." */
#if LUMINARY099
void tstlts1_plus18(void)
#else
void tstlts1_plus16(void)
#endif
{
    tc(TSTLTS1_WAITLIST + 3u, ENDOFJOB);
}

/* TSTLTS2 -- "CALLED BY WAITLIST": a job at CHARIN's priority (CHRPRIO)
 * for TSTLTS3. */
void tstlts2(void)
{
    set_A(CHRPRIO);
    tc(TSTLTS2 + 1u, NOVAC); /* 2CADR TSTLTS3 */
}

/* TSTLTS2 +4, where NOVAC returns: the task ends. */
void tstlts2_plus4(void)
{
    tc(TSTLTS2 + 4u, TASKOVER);
}

/* TSTLTS3 -- "CALLED BY EXECUTIVE". "TURN OFF UPLINK ACTIVITY, TEMP,
 * OPERATOR ERROR" (TSTCON3), "TURN OFF TEST ALARM OUTBIT". "MAKE NO ATT
 * FOLLOW BIT 4 OF CHANNEL 12 (NO ATT LIGHT ON IF IN COARSE ALIGN)" and
 * "TURN OFF AUTO, HOLD, FREE, SPARE, GIMBAL LOCK, SPARE, TRACKER, PROG ALM"
 * (DSPTAB +11D). "SET BITS TO INDICATE ALL LAMPS OUT. TEST LIGHTS
 * COMPLETE." in IMODES33 and IMODES30, and the radar's (LM) or the
 * optics' (CM) fail bits likewise. "REDISPLAY C(MODREG)" (DSPMM, by
 * BANKCALL). */
void tstlts3(void)
{
    set_A(negate(TSTCON3));
    inhint();
    wand_dsalmout();
    set_A(negate(BIT10));
    wand_channel(CHAN13);
    set_A(BIT4);
    rand_channel(CHAN12);
    ad(BIT15);
    ts(ADDRESS_OF(DSPTAB) + 11u);
    set_A(negate(BITS_13_11_1));
    mask_at(ADDRESS_OF(IMODES33));
    ad(PRIO16);
    ts(ADDRESS_OF(IMODES33));
    set_A(negate(OCT55000));
    mask_at(ADDRESS_OF(IMODES30));
    ad(PRIO15); /* "15000." */
    ts(ADDRESS_OF(IMODES30));
#if LUMINARY099
    set_A(negate(RFAILS2));
    mask_at(ADDRESS_OF(RADMODES));
    ad(RCDUFBIT);
    ts(ADDRESS_OF(RADMODES));
#else
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT7);
    ads(ADDRESS_OF(OPTMODES));
#endif
    relint();
    tc(TSTLTS3_BANKCALL, BANKCALL); /* CADR DSPMM */
}

/* The word DSPMM returns to. "TURN ON KILL MONITOR BIT." "TURN OFF V/N
 * FLASH." Then to TSTLTS4 in bank 40 (part 6), which "DOES RELDSP AND GOES
 * TO PINBRNCH IF ENDIDLE IS AWAITING OPERATOR RESPONSE." */
#if LUMINARY099
void tstlts3_plus27(void)
#else
void tstlts3_plus26(void)
#endif
{
    word at = TSTLTS3_BANKCALL + 2u;

    tc_at(at);
    kilmonon();
    tc_at(at + 1u);
    flashoff();
    tc(at + 2u, POSTJUMP); /* CADR TSTLTS4 */
}

/* ---- ERROR LIGHT RESET, the RSET key (bank 40) ---- */

static void force_bit12(void);

/* ERROR -- CHARIN's key table sends the RSET key here. "RESTORE ORIGINAL
 * C(DSPLOCK). THUS ERROR LIGHT RESET LEAVES DSPLOCK UNCHANGED." (CHARIN
 * kept it in 21/22REG.) "TURN ON :CAUTION RESET: OUTBIT" (BIT10 of channel
 * 11). "LEAVE GIMBAL LOCK AND NO ATT INTACT, TURNING OFF AUTO, HOLD, FREE,
 * PROG ALARM, AND TRACKER." "RESET FAIL BITS WHICH GENERATE PROG ALARM SO
 * THAT IF THE FAILURE STILL EXISTS, THE ALARM WILL COME BACK" (IMODES33,
 * IMODES30, and the radar's or the optics' fail bits). "TURN OFF :TEST
 * ALARM: OUTBIT." "TURN OFF UPLINK ACTIVITY, OPERATOR ERROR." (ERCON). */
void error_(void)
{
    xch(ADDRESS_OF(TWENTY_ONE_22REG));
    ts(ADDRESS_OF(DSPLOCK));
    inhint();
    set_A(BIT10);
    wor_dsalmout();
    set_A(GL_PLUS_NOATT);
    mask_at(ADDRESS_OF(DSPTAB) + 11u);
    ad(BIT15);
    ts(ADDRESS_OF(DSPTAB) + 11u);
    set_A(negate(PRIO16));
    mask_at(ADDRESS_OF(IMODES33));
    ad(PRIO16);
    ts(ADDRESS_OF(IMODES33));
    set_A(negate(BIT10));
    mask_at(ADDRESS_OF(IMODES30));
    ad(BIT10);
    ts(ADDRESS_OF(IMODES30));
#if LUMINARY099
    set_A(negate(RFAILS));
    mask_at(ADDRESS_OF(RADMODES));
    ad(RCDUFBIT);
    ts(ADDRESS_OF(RADMODES));
#else
    cs(ADDRESS_OF(OPTMODES));
    mask(BIT7);
    ads(ADDRESS_OF(OPTMODES));
#endif
    set_A(negate(BIT10));
    wand_channel(CHAN13);
    set_A(negate(ERCON));
    wand_dsalmout();
    force_bit12();
    set_A(ZERO);
    ts(ADDRESS_OF(FAILREG));
    ts(ADDRESS_OF(FAILREG) + 1u);
    ts(ADDRESS_OF(FAILREG) + 2u);
    ts(ADDRESS_OF(SFAIL));
    tc(ERCOM + 10u, ENDOFJOB);
}

/* TSTAB -- each DSPTAB word, DSPTAB +10 down to +0 (ERCNT counts from
 * BINCON, DEC 10), with bit 12 forced on. CCS gives the word's magnitude
 * less one, so AD ONE puts it back: a + word (or +0) by ERPLUS, CS A, MASK
 * NOTBIT12, CS A, has bit 12 set; a - word (or -0) by ERMINUS, CS A, MASK
 * NOTBIT12, is put back negative with bit 12 of its complement clear: so
 * set in the word the relays get. "MIGHT WANT TO RESET CLPASS, DECBRNCH,
 * ETC." Interrupts in between words. */
static void force_bit12(void)
{
    word ercnt;

    set_A(BINCON); /* "(DEC 10)" */
    for (;;) {
        /* TSTAB +1 */
        ts(ADDRESS_OF(ERCNT)); /* "ERCNT = COUNT" */
        inhint();
        ercnt = fetch(ADDRESS_OF(ERCNT));
        switch (ccs_at(indexed_by(ADDRESS_OF(DSPTAB), ercnt))) {
        case POSITIVE:
            ad(ONE);
            /* fall through */
        case IS_PLUS_ZERO:
            tc_at(ERMINUS - 2u); /* TC ERPLUS */
            com();               /* ERPLUS */
            mask(NOTBIT12);
            com();
            break;
        case NEGATIVE:
            ad(ONE);
            /* fall through */
        case IS_MINUS_ZERO:
            com(); /* ERMINUS */
            mask(NOTBIT12);
            tc_at(ERMINUS + 2u); /* TC ERCOM */
            break;
        }
        /* ERCOM */
        ts(indexed_erasable(ADDRESS_OF(DSPTAB), ercnt));
        relint();
        switch (ccs_at(ADDRESS_OF(ERCNT))) {
        case POSITIVE: tc_at(ERCOM + 4u); continue; /* TC TSTAB +1 */
        case IS_PLUS_ZERO: return;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* the count runs 10 to 0 */
        }
    }
}
