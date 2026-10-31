/* SPDX-License-Identifier: MIT */
/* Alarms and aborts: ALARM_AND_ABORT.agc of Luminary099 (LM, listing pages
 * 1381-1385) and Comanche055 (CM, pages 1493-1496). The faithful C is
 * c64-port/src/lm_alarm.c and cm_alarm.c.
 *
 * Both missions share ALARM through MULTFAIL, PRIOLARM, CURTAINS and
 * VARALARM. Their aborts differ: the CM's BAILOUT, POODOO and CCSHOLE
 * first save erasable memory for the debugging crew on the ground
 * (VAC5STOR), and its POODOO clears the mission program's flags itself;
 * the LM's POODOO sets a restart phase and goes to GOPOODOO, which clears
 * them (FLAGS) and puts the servicer to idle. Only the LM has BAILOUT1,
 * POODOO1, ALARM1 and ALMNCADR, for callers that pass the 2CADR to blame
 * in A and L. ABORT is WHIMPER in the LM and BAILOUT in the CM.
 *
 * The binding (src/binding/alarm_routines.c) hands these routines only
 * states the original's callers make: a return address and an alarm code
 * are words, so no TS here stores an overflow and skips. */
#include "alarm.h"
#include "../agc/names.h"

static void bortent(void);
static void prioent(void);
static void prioent_plus1(void);
static void larment(void);
static void chkfail1(void);
static void chkfail2(void);
static void fail3(void);
static void proglarm(void);
static void multexit(void);
static void multfail(void);
#if LUMINARY099
static void bothabrt(void);
#else
static void abort2(void);
#endif

/* The display table word with the PROG light (bit 9). Bit 15 set marks it
 * for sending to the DSKY. */
#define PROG_LIGHT_WORD (ADDRESS_OF(DSPTAB) + 11u) /* DSPTAB +11D */

/* INDEX Q; CA 0 (or CAF 0): the alarm code, the word after the caller's
 * TC, whose address Q holds. (INDEX adds Q as a word: an overflow in it,
 * which no caller leaves, is corrected.) */
static void ca_code_after_tc(void)
{
    ca(overflow_corrected(Q));
}

/* ======================================================================
 * Recording an alarm
 * ====================================================================== */

/* ALARM -- LM p. 1381, CM p. 1493.
 * "THE FOLLOWING SUBROUTINE MAY BE CALLED TO DISPLAY A NON-ABORTIVE ALARM
 * CONDITION. IT MAY BE CALLED EITHER IN INTERRUPT OR UNDER EXECUTIVE
 * CONTROL." The calling sequence is
 *     TC   ALARM
 *     OCT  AAANN      "ALARM NO. NN IN GENERAL AREA AAA."
 *                     "(RETURNS HERE)"
 * "ALARM TURNS ON THE PROGRAM ALARM LIGHT, BUT DOES NOT DISPLAY." Q, the
 * address of the code, is also where the alarm came from: it goes into
 * ALMCADR. */
void alarm(void)
{
    inhint();
    ca(Q_ADDRESS);
    alarm2();
}

/* ALARM2 -- LM p. 1381, CM p. 1493.
 * ALARM with the address to blame in A, not Q's: CURTAINS blames its own
 * caller that way. */
void alarm2(void)
{
    ts(ADDRESS_OF(ALMCADR));
    ca_code_after_tc();
    bortent();
}

/* BORTENT -- LM p. 1381, CM p. 1493.
 * The entry of the aborts: the code in A goes to L, where CHKFAIL1 looks
 * for it. */
static void bortent(void)
{
    ts(L_ADDRESS);
    prioent();
}

/* PRIOENT -- LM p. 1381, CM p. 1493.
 * VARALARM's entry, with the code already in L: ALMCADR +1 gets the bank
 * the alarm came from. */
static void prioent(void)
{
    ca(BBANK_ADDRESS);
    prioent_plus1();
}

/* PRIOENT +1 -- LM p. 1381, CM p. 1493.
 * PRIOLARM's entry, with its user's bank in A. */
static void prioent_plus1(void)
{
    ror_superbnk(); /* EXTEND; ROR SUPERBNK: "ADD SUPER BITS." */
    ts(ADDRESS_OF(ALMCADR) + 1u);
    larment();
}

/* LARMENT -- LM p. 1381, CM p. 1493.
 * "STORE RETURN FOR ALARM": ITEMP1 keeps Q while FAILREG is updated. The
 * CM also keeps the running job's interpretive location (LOC and BANKSET)
 * in LOCALARM and BANKALRM, for whoever reads the alarm later. */
static void larment(void)
{
    ca(Q_ADDRESS);
    ts(ADDRESS_OF(ITEMP1));
#if COMANCHE055
    ca(ADDRESS_OF(LOC));
    ts(ADDRESS_OF(LOCALARM));
    ca(ADDRESS_OF(BANKSET));
    ts(ADDRESS_OF(BANKALRM));
#endif
    chkfail1();
}

/* CHKFAIL1, CHKFAIL2, FAIL3 -- LM p. 1381, CM p. 1493.
 * Put the code (in L) into the first free FAILREG word. +0 is free. Each
 * test is the four-way CCS:
 *     CCS   FAILREG       "IS ANYTHING IN FAILREG"
 *     TCF   CHKFAIL2      positive: "YES TRY NEXT REG"
 *     LXCH  FAILREG       +0: take it ...
 *     TCF   PROGLARM      negative (and +0, after the LXCH)
 *     (CHKFAIL2)          -0: on to the next
 * The first alarm also turns the PROG light on; later ones leave it. */
static void chkfail1(void)
{
    switch (ccs_at(ADDRESS_OF(FAILREG))) {
    case POSITIVE: chkfail2(); return;
    case IS_PLUS_ZERO: lxch(ADDRESS_OF(FAILREG)); /* fall through */
    case NEGATIVE: proglarm(); return;            /* "TURN ALARM LIGHT ON FOR FIRST ALARM" */
    case IS_MINUS_ZERO: chkfail2(); return;
    }
}

static void chkfail2(void)
{
    switch (ccs_at(ADDRESS_OF(FAILREG) + 1u)) {
    case POSITIVE: fail3(); return;
    case IS_PLUS_ZERO: lxch(ADDRESS_OF(FAILREG) + 1u); /* fall through */
    case NEGATIVE: multexit(); return;
    case IS_MINUS_ZERO: fail3(); return;
    }
}

/* FAIL3: FAILREG +2 is free if its magnitude is +0 (its sign bit only
 * says whether MULTFAIL has been). A masked word is never negative. */
static void fail3(void)
{
    ca(ADDRESS_OF(FAILREG) + 2u);
    mask(POSMAX);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: multfail(); return;
    case IS_PLUS_ZERO: lxch(ADDRESS_OF(FAILREG) + 2u); /* fall through */
    case NEGATIVE: multexit(); return;
    case IS_MINUS_ZERO: proglarm(); return; /* never: the word after the CCS's four */
    }
}

/* PROGLARM -- LM p. 1382, CM p. 1494.
 * Turn on the PROG light: set bits 15 and 9 of DSPTAB +11D, those not set
 * already (CS; MASK picks the clear ones, and ADS adds just them, so no
 * carry). */
static void proglarm(void)
{
    cs(PROG_LIGHT_WORD);
    mask(OCT40400);
    ads(PROG_LIGHT_WORD);
    multexit();
}

/* MULTEXIT -- LM p. 1382, CM p. 1494.
 * "OBTAIN RETURN ADDRESS IN A", allow interrupts again, and return:
 *     INDEX  A
 *     TC     1
 * runs TC 1 plus A. A holds the Q that LARMENT kept, the address of the
 * code word, so this is TC to the word after it. A routine of this file
 * that called BORTENT or PRIOENT comes back to its own next word that way.
 * The LM's BAILOUT1 and POODOO1 keep an instruction in ITEMP1 instead,
 * TCF OCT40400 or TCF OCT77770 (ADR40400, ADR77770), which INDEX makes a
 * TCF to the word after BAILOUT's or POODOO's code. */
static void multexit(void)
{
    word instruction;

    xch(ADDRESS_OF(ITEMP1));
    relint();
    instruction = indexed(1u, a_word());
    if (instruction < 010000u) { /* TC */
        tc_at((word)(MULTEXIT + 3u));
        continue_at(instruction);
    } else { /* TCF */
        continue_at((word)(instruction & 07777u));
    }
}

/* MULTFAIL -- LM p. 1382, CM p. 1494.
 * All three words are taken: FAILREG +2 gets the latest code with its sign
 * bit set (AD BIT15), to show there were more. (A negative code would
 * overflow and the TS skip the TCF; no caller has one.) */
static void multfail(void)
{
    ca(L_ADDRESS);
    ad(BIT15);
    ts(ADDRESS_OF(FAILREG) + 2u);
    multexit();
}

/* ======================================================================
 * PRIOLARM: the alarm with a display
 * ====================================================================== */

/* PRIOLARM -- LM p. 1382, CM p. 1494 (bank 10).
 * "PRIOLARM DISPLAYS V05N09 VIA PRIODSPR WITH 3 RETURNS TO THE USER FROM
 * THE ASTRONAUT AT CALL LOC +1,+2,+3 AND AN IMMEDIATE RETURN TO THE USER
 * AT CALL LOC +4." The user calls it by BANKCALL, with the code in A:
 *     CAF   OCTXX         "ALARM CODE"
 *     TC    BANKCALL
 *     CADR  PRIOLARM
 * so BUF2 holds the user's 2CADR, the address and bank to blame. "KEEP IN
 * DISPLAY ROUTINES BANK." */
void priolarm(void)
{
    inhint();
    ts(L_ADDRESS);        /* "SAVE ALARM CODE" */
    ca(ADDRESS_OF(BUF2)); /* "2 CADR OF PRIOLARM USER" */
    ts(ADDRESS_OF(ALMCADR));
    ca(ADDRESS_OF(BUF2) + 1u);
    tc_at((word)(PRIOLARM + 5u)); /* TC PRIOENT +1: "LEAVE L ALONE" */
    prioent_plus1();
}

/* PRIOLARM, after the alarm -- LM p. 1382, CM p. 1494 (bank 10).
 * MULTEXIT comes back past -2SEC (DEC -200, "DONT MOVE"): show V05N09. */
void priolarm_after_alarm(void)
{
    set_A(V05N09);
    continue_at(PRIODSPR);
}

/* ======================================================================
 * CURTAINS and VARALARM: alarms that return through ALMCADR
 * ====================================================================== */

/* CURTAINS -- LM p. 1383, CM p. 1496.
 * Alarm 217 (OCT217, the word after the TC ALARM2), and back to the
 * caller. */
void curtains(void)
{
    inhint();
    ca(Q_ADDRESS);
    tc_at((word)(CURTAINS + 2u)); /* TC ALARM2; OCT 00217 */
    alarm2();
}

/* CURTAINS, after the alarm -- LM p. 1383, CM p. 1496.
 * TC ALMCADR: "RETURN TO USER". */
void curtains_after_alarm(void)
{
    tc_at((word)(CURTAINS + 4u));
    continue_at(ADDRESS_OF(ALMCADR));
}

/* VARALARM -- LM p. 1384, CM p. 1496.
 * "VARALARM TURNS ON PROGRAM ALARM LIGHT BUT DOES NOT DISPLAY." The code
 * comes in A:
 *     CAF   (ALARM)
 *     TC    VARALARM */
void varalarm(void)
{
    inhint();
    ts(L_ADDRESS); /* "SAVE USERS ALARM CODE" */
    ca(Q_ADDRESS); /* "SAVE USERS Q" */
    ts(ADDRESS_OF(ALMCADR));
    tc_at((word)(VARALARM + 4u)); /* TC PRIOENT; OCT 14, "DONT MOVE" */
    prioent();
}

/* VARALARM, after the alarm -- LM p. 1384, CM p. 1496.
 * TC ALMCADR: "RETURN TO USER". */
void varalarm_after_alarm(void)
{
    tc_at((word)(VARALARM + 6u));
    continue_at(ADDRESS_OF(ALMCADR));
}

/* ALMCADR, run as an instruction -- LM p. 1383, CM p. 1496.
 * TC ALMCADR goes to the erasable word, and the AGC runs what it holds,
 * the user's return address, as TC <address>. */
void almcadr_as_instruction(void)
{
    tc_through(ADDRESS_OF(ALMCADR));
}

/* ======================================================================
 * Aborts
 * ====================================================================== */

#if LUMINARY099

/* BAILOUT -- LM p. 1382.
 * Record the alarm, then a software restart: MULTEXIT comes back to the
 * word after OCT40400 (WHIMPER -1). */
void bailout(void)
{
    inhint();
    ca(Q_ADDRESS);
    ts(ADDRESS_OF(ALMCADR));
    ca_code_after_tc();          /* INDEX Q; CAF 0 */
    tc_at((word)(BAILOUT + 5u)); /* TC BORTENT; OCT 40400 */
    bortent();
}

#else

/* BAILOUT -- CM p. 1495. ABORT is BAILOUT.
 * Save erasable memory for debugging (VAC5STOR), record the alarm, then a
 * software restart: MULTEXIT comes back to WHIMPER -1. */
void bailout(void)
{
    inhint();
    ca(Q_ADDRESS);
    ts(ADDRESS_OF(ALMCADR));
    tc_at((word)(BAILOUT + 3u)); /* TC BANKCALL; CADR VAC5STOR */
    continue_at(BANKCALL);
}

/* BAILOUT, after VAC5STOR -- CM p. 1495. The code is the word after the
 * caller's TC, which ALMCADR now points at. */
void bailout_after_vac5stor(void)
{
    ca(ALMCADR(0));              /* INDEX ALMCADR; CAF 0: the code after the caller's TC */
    tc_at((word)(BAILOUT + 7u)); /* TC BORTENT; OCT 40400 */
    bortent();
}

#endif

/* WHIMPER -1 -- LM p. 1383, CM p. 1495.
 * Where BAILOUT's alarm comes back to (the CM's POODOO also comes here
 * while average G is on). */
void whimper_minus1(void)
{
    inhint();
    whimper();
}

/* WHIMPER -- LM p. 1383, CM p. 1495. ABORT is WHIMPER in the LM.
 * A software restart from anywhere, an interrupt routine too: ENEMA, by
 * POSTJUMP. First end any interrupt being served. RESUME does that, and
 * then runs the instruction an interrupt keeps in BRUPT; so BRUPT gets
 * TC WHIMPER +4, the TC POSTJUMP below:
 *     CA    TWO
 *     AD    Z           (Z is WHIMPER +2, the next instruction's address)
 *     TS    BRUPT
 *     RESUME
 *     TC    POSTJUMP    "RESUME SENDS CONTROL HERE"
 *     CADR  ENEMA */
void whimper(void)
{
    set_A(TWO);
    ad((word)(WHIMPER + 2u));
    ts(ADDRESS_OF(BRUPT));
    *agc.in_interrupt = 0; /* RESUME, running TC WHIMPER +4 */
    tc_at((word)(WHIMPER + 4u));
    continue_at(POSTJUMP);
}

#if LUMINARY099

/* POODOO -- LM p. 1383.
 * Record the alarm, then stop the mission program: GOPOODOO, after a
 * restart phase that comes back to it. */
void poodoo(void)
{
    inhint();
    ca(Q_ADDRESS);
    abort2();
}

/* ABORT2 -- LM p. 1383.
 * POODOO with the address to blame in A (CCSHOLE's caller). */
void abort2(void)
{
    ts(ADDRESS_OF(ALMCADR));
    ca_code_after_tc();
    tc_at((word)(ABORT2 + 3u)); /* TC BORTENT; OCT 77770, "DON'T MOVE" */
    bortent();
}

/* POODOO, after the alarm -- LM p. 1383.
 * "4.35SPOT FOR GOPOODOO": restart group 4 gets phase 35, which restarts
 * at GOPOODOO (-PHASE4 gets -35, PHASE4 35). */
void poodoo_after_alarm(void)
{
    set_A(OCT35);
    ts(L_ADDRESS);
    com();
    dxch(MINUS_PHASE4_ADDRESS);
    gopoodoo();
}

/* GOPOODOO -- LM p. 1383.
 * "RESET STATEFLG, REINTFLG, AND NODOFLAG." (FLAGS, in bank 13). */
void gopoodoo(void)
{
    inhint();
    tc_at((word)(GOPOODOO + 1u)); /* TC BANKCALL; CADR FLAGS */
    continue_at(BANKCALL);
}

/* GOPOODOO, after FLAGS -- LM p. 1383.
 * "IS SERVICER CURRENTLY IN OPERATION?" If so, STRTIDLE; if not, end the
 * mission program's jobs and tasks: V37KLEAN, then MR.KLEAN. */
void gopoodoo_after_flags(void)
{
    ca(ADDRESS_OF(FLAGWRD7));
    mask(V37FLBIT);
    /* CCS A: positive or +0 (a masked word is never negative). */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        set_A(BBSERVDL);              /* STRTIDLE */
        tc_at((word)(STRTIDLE + 1u)); /* TC SUPERSW */
        continue_at(SUPERSW);
        return;
    }
    tc_at((word)(GOPOODOO + 7u)); /* TC BANKCALL; CADR V37KLEAN: "TERMINATE GRPS 1, 3, 5, AND 6" */
    continue_at(BANKCALL);
}

/* GOPOODOO, after V37KLEAN -- LM p. 1383.
 * "TERMINATE GRPS 2, 4, 1, 3, 5, AND 6 (I.E., GRP 4 LAST)". */
void gopoodoo_after_v37klean(void)
{
    tc_at((word)(GOPOODOO + 9u)); /* TC BANKCALL; CADR MR.KLEAN */
    continue_at(BANKCALL);
}

/* GOPOODOO, after MR.KLEAN -- LM p. 1383. TCF WHIMPER. */
void gopoodoo_after_mr_klean(void)
{
    whimper();
}

/* STRTIDLE, after SUPERSW -- LM p. 1383.
 * "PUT SERVICER INTO ITS "GROUND" STATE AND PROCED TO GOTOPOOH." */
void strtidle_after_supersw(void)
{
    tc_at((word)(STRTIDLE + 2u)); /* TC BANKCALL; CADR SERVIDLE */
    continue_at(BANKCALL);
}

/* CCSHOLE -- LM p. 1383.
 * Where a CCS goes that should never: POODOO with alarm 1103, blaming the
 * CCS's place. */
void ccshole(void)
{
    inhint();
    ca(Q_ADDRESS);
    tc_at((word)(CCSHOLE + 2u)); /* TC ABORT2; OCT 1103 */
    abort2();
}

/* BAILOUT1 -- LM p. 1384.
 * BAILOUT for a caller that blames another 2CADR, passed in A and L (the
 * Executive's 1201 and 1202 blame the job that asked). BOTHABRT puts
 * ADR40400 in ITEMP1, so MULTEXIT goes on as BAILOUT's does. */
void bailout1(void)
{
    inhint();
    dxch(ADDRESS_OF(ALMCADR));
    set_A(ADR40400);
    bothabrt();
}

/* BOTHABRT -- LM p. 1384.
 * BAILOUT1's and POODOO1's: A is where MULTEXIT goes on (ITEMP1), the code
 * is after the caller's TC. */
static void bothabrt(void)
{
    ts(ADDRESS_OF(ITEMP1));
    ca_code_after_tc(); /* INDEX Q; CAF 0 */
    ts(L_ADDRESS);
    chkfail1();
}

/* POODOO1 -- LM p. 1384.
 * POODOO for a caller that blames another 2CADR, in A and L. */
void poodoo1(void)
{
    inhint();
    dxch(ADDRESS_OF(ALMCADR));
    set_A(ADR77770);
    bothabrt();
}

/* ALARM1 -- LM p. 1384.
 * ALARM for a caller that blames another 2CADR, in A and L. */
void alarm1(void)
{
    inhint();
    dxch(ADDRESS_OF(ALMCADR));
    almncadr();
}

/* ALMNCADR -- LM p. 1384.
 * ALARM that leaves ALMCADR as it is. */
void almncadr(void)
{
    inhint();
    ca_code_after_tc();
    ts(L_ADDRESS);
    larment();
}

/* FLAGS -- LM p. 1385 (bank 13).
 * Clear STATEFLG, REINTFLG and NODOFLAG (CS bit; MASK word; TS word), and
 * return (TC Q). */
static void clear_flag(word bit, unsigned flagword)
{
    set_A(negate(bit));
    mask(fetch(flagword));
    ts(flagword);
}

void flags(void)
{
    clear_flag(STATEBIT, ADDRESS_OF(FLAGWRD3));
    clear_flag(REINTBIT, ADDRESS_OF(FLGWRD10));
    clear_flag(NODOBIT, ADDRESS_OF(FLAGWRD2));
    tc_through(Q_ADDRESS);
}

#else

/* POODOO -- CM p. 1495.
 * Save erasable memory for debugging (VAC5STOR), record the alarm, then
 * end the mission program: clear its flags and its jobs and tasks
 * (MR.KLEAN), and restart (WHIMPER). */
void poodoo(void)
{
    inhint();
    ca(Q_ADDRESS);
    ts(ADDRESS_OF(ALMCADR));
    tc_at((word)(POODOO +
                 3u)); /* TC BANKCALL; CADR VAC5STOR, "STORE ERASABLES FOR DEBUGGING PURPOSES." */
    continue_at(BANKCALL);
}

/* POODOO, after VAC5STOR -- CM p. 1495. */
void poodoo_after_vac5stor(void)
{
    ca(ALMCADR(0)); /* INDEX ALMCADR; CAF 0 */
    abort2();
}

/* ABORT2 -- CM p. 1495. TC BORTENT; OCT 77770, "DONT MOVE". */
static void abort2(void)
{
    tc_at(ABORT2);
    bortent();
}

/* POODOO, after the alarm -- CM p. 1495.
 * "IS AVERAGE G ON": then "DONT DO POODOO. DO BAILOUT." Otherwise clear
 * STATEFLG, REINTFLG and NODOFLAG (DOWNFLAG, its flag's ADRES after the
 * TC), end the jobs and tasks (MR.KLEAN), and restart. */
void poodoo_after_alarm(void)
{
    set_A(V37FLBIT);
    mask(FLAGWRD7);
    /* CCS A: positive or +0 (a masked word is never negative). */
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at((word)(POODOO + 12u)); /* TC WHIMPER -1 */
        whimper_minus1();
        return;
    }
    tc_at((word)(POODOO + 13u)); /* TC DOWNFLAG; ADRES STATEFLG */
    continue_at(DOWNFLAG);
}

/* POODOO, after DOWNFLAG of STATEFLG -- CM p. 1495. */
void poodoo_after_stateflg(void)
{
    tc_at((word)(POODOO + 15u)); /* TC DOWNFLAG; ADRES REINTFLG */
    continue_at(DOWNFLAG);
}

/* POODOO, after DOWNFLAG of REINTFLG -- CM p. 1495. */
void poodoo_after_reintflg(void)
{
    tc_at((word)(POODOO + 17u)); /* TC DOWNFLAG; ADRES NODOFLAG */
    continue_at(DOWNFLAG);
}

/* POODOO, after DOWNFLAG of NODOFLAG -- CM p. 1495. */
void poodoo_after_nodoflag(void)
{
    tc_at((word)(POODOO + 19u)); /* TC BANKCALL; CADR MR.KLEAN */
    continue_at(BANKCALL);
}

/* POODOO, after MR.KLEAN -- CM p. 1495. TC WHIMPER. */
void poodoo_after_mr_klean(void)
{
    tc_at((word)(POODOO + 21u));
    whimper();
}

/* CCSHOLE -- CM p. 1496.
 * Where a CCS goes that should never: VAC5STOR, then POODOO with alarm
 * 1103, blaming the CCS's place. */
void ccshole(void)
{
    inhint();
    ca(Q_ADDRESS);
    ts(ADDRESS_OF(ALMCADR));
    tc_at((word)(CCSHOLE + 3u)); /* TC BANKCALL; CADR VAC5STOR */
    continue_at(BANKCALL);
}

/* CCSHOLE, after VAC5STOR -- CM p. 1496. */
void ccshole_after_vac5stor(void)
{
    set_A(OCT1103);
    tc_at((word)(CCSHOLE + 6u)); /* TC ABORT2; OCT 1103 */
    abort2();
}

#endif
