/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 3 (Luminary099 pp. 513-516): R23 and R24.
 *
 * R23, "MANUAL ACQUISITION ROUTINE R23LEM": "TO ACQUIRE THE CSM BY MANUAL
 * OPERATION OF THE RENDEZVOUS RADAR". The radar's tracker enabled, verb
 * 50 noun 25 flashes checklist code 205 (GOPERF1) while the crew slews
 * the antenna by hand: "SELECTION OF ENTER WILL RECYCLE R23", a preferred
 * tracking attitude maneuver (R61) and the display again; "SELECTION OF
 * PROC WILL CONTINUE R23": if the antenna is within its limits
 * (RRLIMCHK), "RADAR IS LOCKED ON CONTINUE IN P20", else alarm 501;
 * "SELECTION OF TERM WILL TERMINATE R23 + P20" (verb 56).
 *
 * R24, "SEARCH ROUTINE R24LEM": "TO ACQUIRE THE CSM BY A SEARCH PATTERN
 * WHEN THE RENDEZVOUS RADAR HAS FAILED TO ACQUIRE TEH CSM IN THE AUTOMATIC
 * TRACKING MODE AND TO ALLOW THE ASTRONAUT TO CONFIRM THAT REACQUISITION
 * HAS NOT BEEN IN SIDELOBE." Verb 16 noun 80 shows DATAGOOD and OMEGAD
 * while LRS24.1 designates the antenna round the search pattern: proceed
 * accepts the lock-on ("EXIT ROUTINE AND RETURN TO AUTO MODE IN P20"),
 * enter maneuvers to the preferred tracking attitude (R61, not on the
 * lunar surface) and searches again, terminate ends P20 (verb 56).
 *
 * CLRADMOD, in fixed-fixed memory, clears RADMODES's bits 10 and 15 (the
 * designate's) and the radar error counters' enable, channel 12 bit 2.
 * R21, R24 and the radar routines call it.
 *
 * As in parts 1 and 2, each call out of the file leaves Z at the routine
 * and Q at the word after the TC, and a TC or a branch to a label the
 * faithful port keys a routine at stops there. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- R23 ---- */

/* R23LEM -- p. 513. "SET NO ANGLE MONITOR FLAG": UPFLAG, NORRMON. */
void r23lem(void)
{
    tc_at(R23LEM);
    continue_at(UPFLAG); /* ADRES NORRMON */
}

/* R23LEM +2 -- "SELECT MINIMUM DEADBAND": SETMINDB, with interrupts
 * held. */
void r23lem_after_upflag(void)
{
    inhint();
    tc_to(R23LEM + 3u, IBNKCALL); /* CADR SETMINDB */
}

/* R23LEM1 -- "ENABLE TRACKER" (channel 12 bit 14), and verb 50 noun 25
 * flashing the checklist code 205 (GOPERF1: please perform). */
static void r23lem1(void)
{
    set_A(BIT14);
    wor_channel(CHAN12);
    set_A(OCT205);
    tc_to(R23LEM1 + 4u, BANKCALL); /* CADR GOPERF1 */
}

/* R23LEM +5. */
void r23lem_after_setmindb(void)
{
    relint();
    r23lem1();
}

/* R23LEM2 -- "CLEAR NO ANGLE MONITOR FLAG", "AND EXIT VIA V56". */
static void r23lem2(void)
{
    tc_to(R23LEM2, DOWNFLAG); /* ADRES NORRMON */
}

void r23lem2_after_downflag(void)
{
    tc_to(R23LEM2 + 2u, GOTOV56);
}

/* R23LEM1 +6 -- "TERMINATE". */
void r23lem1_terminate(void)
{
    tc_at(R23LEM1 + 6u);
    r23lem2();
}

/* R23LEM1 +7 -- "PROCEDE". */
void r23lem1_proceed(void)
{
    tc_to(R23LEM1 + 7u, R23LEM11);
}

/* R23LEM1 +010 -- "ENTER -- DO ANOTHER MANEUVER". */
void r23lem1_enter(void)
{
    tc_to(R23LEM1 + 010u, R23LEM3);
}

/* R23LEM11 -- "YES -- CHECK IF ANTENNA IS WITHIN LIMITS": RRLIMCHK, with
 * interrupts held, on the radar's CDU angles (CDUT and CDUS). */
void r23lem11(void)
{
    inhint();
    tc_to(R23LEM11 + 1u, RRLIMCHK); /* ADRES CDUT */
}

/* R23LEM11 +3 -- "NOT WITHIN LIMITS". */
void r23lem11_out_of_limits(void)
{
    tc_to(R23LEM11 + 3u, OUTOFLIM);
}

/* R23LEM11 +4 -- within them: "RESTORE DEADBAND TO ASTRONAUT SELECTED
 * VALUE". */
void r23lem11_within_limits(void)
{
    tc_to(R23LEM11 + 4u, IBNKCALL); /* CADR RESTORDB */
}

/* R23LEM11 +6 -- "CLEAR NO ANGLE MONITOR FLAG". */
void r23lem11_after_restordb(void)
{
    relint();
    tc_to(R23LEM11 + 7u, DOWNFLAG); /* ADRES NORRMON */
}

/* R23LEM11 +011 -- "RADAR IS LOCKED ON CONTINUE IN P20". */
void r23lem11_after_downflag(void)
{
    tc_to(R23LEM11 + 011u, P20LEMB1);
}

/* OUTOFLIM +1 -- "ISSUE ALARM -- RR ANTENNA NOT WITHIN LIMITS": alarm 501
 * (PRIOLARM). */
static void outoflim_alarm(void)
{
    set_A(OCT501PV);
    tc_to(OUTOFLIM + 2u, BANKCALL); /* CADR PRIOLARM */
}

/* OUTOFLIM -- p. 513. */
void outoflim(void)
{
    relint();
    outoflim_alarm();
}

/* OUTOFLIM +4 -- "TERMINATE -- EXIT R23 TO R00 (GO TO POOH)". */
void outoflim_terminate(void)
{
    tc_at(OUTOFLIM + 4u);
    r23lem2();
}

/* OUTOFLIM +5 -- "PROCEED ILLEGAL": TC OUTOFLIM +1, the alarm again. */
void outoflim_proceed(void)
{
    tc_at(OUTOFLIM + 5u);
    outoflim_alarm();
}

/* OUTOFLIM +6 -- "RECYCLE -- TO ANOTHER MANEUVER". */
void outoflim_enter(void)
{
    tc_to(OUTOFLIM + 6u, R23LEM3);
}

/* OUTOFLIM +7 -- PRIOLARM's immediate return. */
void outoflim_immediate_return(void)
{
    tc_to(OUTOFLIM + 7u, ENDOFJOB);
}

/* R23LEM3 -- R61, the preferred tracking attitude maneuver, and the
 * display again. */
void r23lem3(void)
{
    tc_to(R23LEM3, BANKCALL); /* CADR R61LEM */
}

void r23lem3_after_r61lem(void)
{
    tc_at(R23LEM3 + 2u);
    r23lem1();
}

/* ---- R24 ---- */

/* R24LEM -- p. 515. "SET SRCHOPT FLAG": UPFLAG, SRCHOPTN. */
void r24lem(void)
{
    tc_at(R24LEM);
    continue_at(UPFLAG); /* ADRES SRCHOPTN */
}

/* R24LEM +2 -- "RESET LOS BEING COMPUTED FLAG TO MAKE SURE DODES DOESN'T
 * GO TO R21". */
void r24lem_after_upflag(void)
{
    tc_to(R24LEM + 2u, DOWNFLAG); /* ADRES LOSCMFLG */
}

/* R24LEM2 -- restart group 2 at P20LEM1. */
static void r24lem2(void)
{
    tc_to(R24LEM2, PHASCHNG); /* OCT 04022 */
}

/* R24LEM1 -- "ZERO OUT DATA INDICATOR", "ZERO OMEGA DISPLAY REGS". (A is
 * +0: no TS skips.) */
void r24lem1(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DATAGOOD));
    ts(ADDRESS_OF(OMEGAD));
    ts(ADDRESS_OF(OMEGAD) + 1u);
    r24lem2();
}

/* R24LEM2 +2 -- verb 16 noun 80, a priority display (PRIODSPR). */
void r24lem2_after_phaschng(void)
{
    set_A(V16N80);
    tc_to(R24LEM2 + 3u, BANKCALL); /* CADR PRIODSPR */
}

/* R24LEM2 +5 -- terminate: "REJECT BY TERMINATING", verb 56. */
void r24lem2_terminate(void)
{
    tc_to(R24LEM2 + 5u, GOTOV56);
}

/* R24LEM2 +6 -- "PROCEED EXIT R24 TO P20LEM1". */
void r24lem2_proceed(void)
{
    tc_to(R24LEM2 + 6u, R24END);
}

/* R24LEM3 -- p. 516. Restart group 2 at P20LEM1. */
static void r24lem3(void)
{
    tc_to(R24LEM3, PHASCHNG); /* OCT 04022 */
}

/* R24LEM2 +7 -- "RECYCLE -- CALL R61 TO MANEUVER S/C". */
void r24lem2_enter(void)
{
    tc_at(R24LEM2 + 7u);
    r24lem3();
}

/* R24LEM2 +010 -- PRIODSPR's immediate return: the search, LRS24.1. */
void r24lem2_immediate_return(void)
{
    tc_to(R24LEM2 + 010u, BANKCALL); /* CADR LRS24.1 */
}

/* R24END -- the designate's next task killed (CALLDGCH). */
void r24end(void)
{
    tc_to(R24END, KILLTASK); /* CADR CALLDGCH */
}

/* R24END +2 -- "CLEAR BITS 10 & 15 OF RADMODES." */
void r24end_after_killtask(void)
{
    tc_to(R24END + 2u, CLRADMOD);
}

/* R24END +3 -- "AND GO TO 400 MI. RANGE CHECK IN P20". */
void r24end_after_clradmod(void)
{
    continue_at(P20LEM1); /* TCF P20LEM1 */
}

/* R24LEM3 +2 -- "KILL WAITLIST FOR NEXT POINT IN PATTERN". */
void r24lem3_after_phaschng(void)
{
    tc_to(R24LEM3 + 2u, KILLTASK); /* CADR CALLDGCH */
}

/* R24LEM3 +4 -- "CLEAR BITS 10 + 15 OF RADMODES TO KILL HALF SECOND
 * DESIGNATE LOOP". */
void r24lem3_after_killtask(void)
{
    tc_to(R24LEM3 + 4u, CLRADMOD);
}

/* R24LEM3 +5 -- "WAIT FOR DESIGNATE LOOP TO DIE": half a second
 * (DELAYJOB). */
void r24lem3_after_clradmod(void)
{
    relint();
    set_A(HALF_SEC);
    tc_to(R24LEM3 + 7u, BANKCALL); /* CADR DELAYJOB */
}

/* R24LEM3 +011 -- "CHECK IF ON LUNAR SURFACE". */
void r24lem3_after_delayjob(void)
{
    tc_at(R24LEM3 + 011u);
    lunsfchk();
}

/* R24LEM3 +012 -- "YES -- DON'T DO ATTITUDE MANEUVER". */
void r24lem3_on_surface(void)
{
    tc_to(R24LEM3 + 012u, R24LEM4);
}

/* R24LEM3 +013 -- "CALL R61 TO DO PREFERRED TRACKING ATTITUDE MANEUVER". */
void r24lem3_in_orbit(void)
{
    tc_to(R24LEM3 + 013u, BANKCALL); /* CADR R61LEM */
}

/* R24LEM4 -- "ZERO OUT RADCADR (WHICH WAS SET BY ENDRADAR WHEN DESIGNATE
 * STOPPED) SO THAT RRDESSM WILL RETURN TO CALLER", "AND GO BACK TO PUT UP
 * V16 N80 DISPLAY". (A is +0: the TS does not skip.) */
void r24lem4(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RADCADR));
    tc_at(R24LEM4 + 2u);
    r24lem2();
}

/* ---- CLRADMOD (fixed-fixed) ---- */

/* CLRADMOD -- p. 516. RADMODES's bits 10 and 15 cleared, with interrupts
 * held ("USER WILL RELINT"), and "DISABLE RR ERROR COUNTERS". (A and
 * RADMODES are ordinary words: the TS does not skip.) */
void clradmod(void)
{
    set_A(negate(BIT10_15));
    inhint();
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    set_A(negate(BIT2));
    wand_channel(CHAN12);
    tc_q();
}

#endif
