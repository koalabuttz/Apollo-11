/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 5 (Luminary099 pp. 521-537): the rendezvous radar's
 * subroutines, from MAGSUB to RRZ2, with ENDRADAR, RDBADEND and RADNOOP
 * (pp. 561-563), which RRZERO and RRZ2 end at.
 *
 * MAGSUB to FLIP are in fixed-fixed memory, called with interrupts held:
 * MAGSUB compares a magnitude with a limit, RRLIMCHK checks a pair of
 * desired gimbal angles against the limits of the radar's present mode,
 * SETTRKF lights or puts out the TRACKER FAIL lamp. The rest (bank 25) are
 * waitlist tasks and the subroutines they call: RRTURNON and RRZERO zero
 * the radar's CDUs and find its mode (RRZEROSB); DORREPOS drives the
 * antenna to a safe position and REMODE from one mode to the other, one
 * axis at a time (RRTONLY, RRSONLY), every half second through RR1AX2 and
 * RROUT, which sends the commands to the CDUs' error counters.
 *
 * RADMODES holds the radar's modes: bit 1 the turn-on sequence running
 * (TURNONBT), bit 2 not in auto mode (AUTOMBIT), bit 4 no radar data
 * (RRDATABT), bit 7 the CDUs good (RCDUFBIT), bit 10 a designate waiting
 * (DESIGBIT), bit 11 a reposition running (REPOSBIT), bit 12 mode 2
 * (ANTENBIT), bit 13 the CDUs being zeroed (RCDU0BIT), bit 14 a remode
 * asked for (REMODBIT).
 *
 * As in parts 1 to 4, each call out of the file leaves Z at the routine
 * and Q at the word after the TC, and a TC or a branch to a label the
 * faithful port keys a routine at stops there. MAGSUB has no key: it runs
 * inside its caller's routine and ends at the TC Q +1 or Q +2 that takes
 * it back. No TS here overflows (each stores a constant, a masked word, a
 * CCS's result or a product), so none skips. */
#include "p20_p25.h"
#include "../agc/names.h"

#if LUMINARY099

/* ---- MAGSUB to FLIP, in fixed-fixed memory ---- */

/* MAGSUB -- p. 521: "RETURNS TO CALLER +2 IF THE ABSOLUTE VALUE OF VALUE
 * OF C(A) IS GREATER THAN THE NEGATIVE OF THE NUMBER AT CALLER +1.
 * OTHERWISE IT RETURNS TO CALLER +3." Q is the caller +1, its limit. */
void magsub(void)
{
    if (A_is_zero_or_minus()) com(); /* A's magnitude */
    ad(read_word(Q));                /* INDEX Q; AD 0: plus the negative limit */
    if (A_is_zero_or_minus())
        continue_at(Q_PLUS_2); /* "ABS(A) <= CONST GO TO L+3" */
    else
        continue_at(Q_PLUS_1); /* "ABS(A) > CONST GO TO L+2" */
}

/* RRLIMCHK -- p. 522: "CHECKS RR DESIRED GIMBAL ANGLES TO SEE IF THEY ARE
 * WITHIN THE LIMITS OF THE CURRENT MODE." The word after the TC is the
 * address of the trunnion angle, the shaft angle after it: they go to
 * ITEMP1 and ITEMP2, and the caller +2 to L. Mode 1 (ANTENBIT clear) is
 * "ABS(T) L 55 DEGS" and "ABS(S + 5.5 DEGS) L 64.5 DEGS"; mode 2 (MODE2CHK)
 * "ABS(T) G 125 DEGS" and "ABS(S + 82 DEGS) L 57 DEGS". Out of the limits,
 * TC L to the caller +2; within them, RRLIMOK to the caller +3. */
void rrlimchk(void)
{
    dca(read_word(Q)); /* EXTEND; INDEX Q; INDEX 0; DCA 0 */
    incr(Q_ADDRESS);
    dxch(ADDRESS_OF(ITEMP1));
    lxch(Q_ADDRESS); /* "L(CALLER +2) TO L." */
    set_A(ANTENBIT); /* "SEE WHICH MODE RR IS IN." */
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) {
        continue_at(MODE2CHK); /* TCF MODE2CHK */
        return;
    }
    ca(ADDRESS_OF(ITEMP1));
    tc_at(RRLIMCHK + 014u);
    magsub(); /* DEC -.30555: 55 degrees */
}

void rrlimchk_trunnion_out(void)
{
    tc_return_in(RRLIMCHK + 016u, L_ADDRESS);
}

void rrlimchk_trunnion_in(void)
{
    set_A(FIVE_5DEGS);
    ad_at(ADDRESS_OF(ITEMP2));
    tc_at(RRLIMCHK + 021u);
    magsub(); /* DEC -.35833: "64.5 DEGS" */
}

void rrlimchk_shaft_out(void)
{
    tc_return_in(RRLIMCHK + 023u, L_ADDRESS);
}

void rrlimchk_shaft_in(void)
{
    tc_to(RRLIMCHK + 024u, RRLIMOK); /* "IN LIMITS." */
}

/* MODE2CHK -- p. 523. */
void mode2chk(void)
{
    set_A(EIGHTY2DEGS);
    ad_at(ADDRESS_OF(ITEMP2));
    tc_at(MODE2CHK + 2u);
    magsub(); /* DEC -.31667: "(SHAFT LIMITS AT -25, -139 DEGS)" */
}

void mode2chk_shaft_out(void)
{
    tc_return_in(MODE2CHK + 4u, L_ADDRESS);
}

void mode2chk_shaft_in(void)
{
    ca(ADDRESS_OF(ITEMP1));
    tc_at(MODE2CHK + 6u);
    magsub(); /* DEC -.69444: "125 DEGS" */
}

/* RRLIMOK -- INDEX L; TC L "( = TC 1 )": L holds the caller +2, so the
 * indexed TC 1 goes to the caller +3. Not modelled: an L that does not
 * make it a TC to fixed memory. */
void rrlimok(void)
{
    word to = indexed_instruction(01u /* TC L */, overflow_corrected(L));

    if ((to & 070000u) || to < 02000u) not_modelled();
    tc_at(RRLIMOK + 1u);
    continue_at(to);
}

/* After MODE2CHK's trunnion check, MAGSUB's +2 (within 125 degrees): the
 * TC L after RRLIMOK's INDEX, to the caller +2. */
void mode2chk_trunnion_out(void)
{
    tc_return_in(RRLIMOK + 1u, L_ADDRESS);
}

/* SETTRKF -- p. 524: "UPDATES THE TRACKER FAIL LAMP ON THE DSKY" (DSPTAB
 * +11D bit 8), not during a lamp test (IMODES33 bit 1). The lamp is to be
 * on when the radar's CDUs failed in auto mode while not being zeroed (13,7,2
 * all clear: RADMODES bits 13, 7 and 2), or when there is no radar data
 * (RRDATABT). L holds the lamp's bit; the desired state (L, or +0 for off)
 * "HALF ADD"ed to the present one gives bit 8 set for a change, which FLIP
 * makes. */
void settrkf(void)
{
    set_A(BIT1); /* "NO ACTION IF DURING LAMP TEST" */
    mask_at(ADDRESS_OF(IMODES33));
    if (bit_set_in_A()) {
        tc_q();
        return;
    }
    set_A(BIT8); /* RRTRKF */
    ts(L_ADDRESS);
    set_A(BITS13_7_2); /* "SEE IF CDU FAILED." */
    mask_at(ADDRESS_OF(RADMODES));
    if (A_is_zero()) {
        ca(L_ADDRESS); /* TRKFLON: the lamp on */
    } else {
        set_A(RRDATABT); /* RRCHECK: "SEE IF RR DATA FAILED." */
        mask_at(ADDRESS_OF(RADMODES));
        if (bit_set_in_A()) ca(L_ADDRESS); /* the lamp on; else A +0, off */
    }
    ad_at(ADDRESS_OF(DSPTAB) + 11u); /* "HALF ADD DESIRED AND PRESENT STATES." */
    mask_at(L_ADDRESS);
    if (A_is_zero())
        continue_at(TCQ); /* "NO CHANGE." */
    else
        continue_at(FLIP);
}

/* FLIP: DSPTAB +11D with L's bit changed, and bit 15 set for the DSKY's
 * display routine to send it ("CAN'T USE LXCH DSPTAB +11D (RESTART
 * PROB)"). */
void flip(void)
{
    ca(ADDRESS_OF(DSPTAB) + 11u);
    rxor_lchan();
    mask(POSMAX);
    ad(BIT15);
    ts(ADDRESS_OF(DSPTAB) + 11u);
    tc_q();
}

/* ---- RRTURNON and RRZEROSB (bank 25) ---- */

/* RRTURNON -- p. 526, a waitlist task: "THE TURN-ON SEQUENCE WHICH, ALONG
 * WTIH RRZEROSB, ZEROES THE CDU'S AND DETERMINES THE RR MODE." Then, a
 * second later, the turn-on flag off "SO A MONITOR REPOSITION WON'T
 * ALARM." */
void rrturnon(void)
{
    tc_at(RRTURNON);
    rrzerosb();
}

void rrturnon_after_rrzerosb(void)
{
    tc_to(RRTURNON + 1u, FIXDELAY); /* DEC 100: "WAIT 1 SEC BEFORE REMOVING TURN ON FLAG" */
}

void rrturnon_after_fixdelay(void)
{
    set_A(negate(TURNONBT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    continue_at(TASKOVER); /* TCF TASKOVER */
}

/* RRZEROSB -- p. 527: "A CLOSED SUBROUTINE TO ZERO THE RR CDU'S, DETERMINE
 * THE RR MODE, AND TURN ON THE TRACKER FAIL LAMP IF REQUIRED." The return
 * waits in RRRET while the CDU zero discrete (channel 12 bit 1) is on for
 * 20 milliseconds and the CDUs then settle for 10 seconds. */
void rrzerosb(void)
{
    qxch(ADDRESS_OF(RRRET));
    set_A(BIT1);                    /* "BIT 13 OF RADMODES MUST BE SET BEFORE COMING HERE." */
    wor_channel(CHAN12);            /* "TURN ON ZERO RR CDU" */
    tc_to(RRZEROSB + 5u, FIXDELAY); /* DEC 2 */
}

void rrzerosb_after_zeroing(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(CDUT));
    ts(ADDRESS_OF(CDUS));
    set_A(negate(ONE)); /* "REMOVE ZEROING BIT." */
    wand_channel(CHAN12);
    tc_to(RRZEROSB + 015u, FIXDELAY); /* DEC 1000: "RESET FAIL INHIBIT IN 10 SECS." */
}

/* Mode 1 while the trunnion angle is within 90 degrees, else mode 2. */
void rrzerosb_after_settling(void)
{
    set_A(negate(RCDU0BIT)); /* "REMOVE ZEROING IN PROCESS BIT" */
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    ca(ADDRESS_OF(CDUT));
    tc_at(RRZEROSB + 023u);
    magsub(); /* DEC -.5 */
}

/* ANTENBIT (mode 2) or +0 (mode 1) into RADMODES bit 12, and the lamp. */
static void set_mode_bit(void)
{
    xch(ADDRESS_OF(RADMODES));
    mask(MINUS_BIT12);
    ads(ADDRESS_OF(RADMODES));
    tc_to(RRZEROSB + 034u, SETTRKF); /* "TRACKER LAMP MIGHT GO ON NOW." */
}

void rrzerosb_mode2(void)
{
    set_A(ANTENBIT); /* TCF +3: "IF MODE 2." */
    set_mode_bit();
}

void rrzerosb_mode1(void)
{
    set_A(ZERO);
    set_mode_bit();
}

void rrzerosb_after_settrkf(void)
{
    tc_return_in(RRZEROSB + 035u, ADDRESS_OF(RRRET)); /* "DONE." */
}

/* ---- DORREPOS and REMODE ---- */

/* DORREPOS -- p. 529, a waitlist task from RRGIMON when the antenna is out
 * of its mode's limits: "A SEQUENCE OF TASKS TO DRIVE THE RENDEZVOUS RADAR
 * TO A SAFE POSITION." The error counters enabled (SETRRECR), then 20
 * milliseconds later the trunnion to 0 degrees (180 in mode 2) and the
 * shaft to 0 (-90 in mode 2). ("ALARM 501 DELETED IN DANCE 279 PER PCR
 * 97.") */
void dorrepos(void)
{
    tc_to(DORREPOS, SETRRECR); /* "SET UP RR CDU ERROR COUNTERS." */
}

void dorrepos_after_setrrecr(void)
{
    tc_to(DORREPOS + 1u, FIXDELAY); /* DEC 2 */
}

void dorrepos_trunnion(void)
{
    set_A(ANTENBIT); /* "MANEUVER TRUNNION ANGLE TO NOMINAL POS." */
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) set_A(BIT15); /* "0 FOR MODE 1 AND 180 FOR MODE 2." */
    tc_at(DORREPOS + 7u);
    rrtonly();
}

void dorrepos_shaft(void)
{
    set_A(ANTENBIT); /* "NOT PUT SHAFT IN RIGHT POSITION" */
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) set_A(negate(HALF)); /* "-90 FOR MODE 2." */
    tc_at(DORREPOS + 014u);
    rrsonly();
}

/* REPOSRPT -- the reposition done (or cut short by a remode, from RR1AX2):
 * its flag off, then BEGDES if "SOMEONE IS WAITING TO DESIGNATE", else the
 * error counters disabled and the task over. */
void reposrpt(void)
{
    set_A(negate(REPOSBIT));
    mask_at(ADDRESS_OF(RADMODES)); /* "REMOVE REPOSITION BIT." */
    ts(ADDRESS_OF(RADMODES));
    mask(DESIGBIT); /* "SEE IF SOMEONE IS WAITING TO DESIGNATE." */
    if (bit_set_in_A()) {
        continue_at(BEGDES); /* TCF BEGDES */
        return;
    }
    /* "IF NO FURTHER ANTENNA CONTROL REQUIRED, REMOVE ERROR COUNTER ENABLE." */
    set_A(negate(BIT2));
    wand_channel(CHAN12);
    continue_at(TASKOVER); /* TCF TASKOVER */
}

/* SETRRECR -- "SET UP RR ERROR COUNTERS": channel 12 bit 2, with the last
 * commands (LASTYCMD, LASTXCMD) cleared, unless the counters are already
 * enabled. */
void setrrecr(void)
{
    set_A(BIT2);
    rand_channel(CHAN12);
    if (bit_set_in_A()) {
        tc_q(); /* "DO NOT CLEAR LAST COMMAND IF ERROR COUNTERS ARE ENABLED" */
        return;
    }
    ts(ADDRESS_OF(LASTYCMD)); /* A, +0 */
    ts(ADDRESS_OF(LASTXCMD));
    set_A(BIT2);
    wor_channel(CHAN12); /* "ENABLE RR CDU ERROR COUNTERS." */
    tc_q();
}

/* REMODE -- p. 531: "THE GENERAL REMODING SUBROUTINE. IT DRIVES THE
 * TRUNNION ANGLE TO 0 DEGREES IF THE CURRENT MODE IS MODE 1, 180 DEGREES
 * FOR MODE 2, THEN DRIVES THE SHAFT ANGLE TO -45 DEGREES, AND FINALLY
 * DRIVES THE TRUNNION ANGLE TO -130 DEGREES, TO PLACE THE RR IN MODE 2, -50
 * DEGREES FOR MODE 1, BEFORE INITIATING 2-AXIS CONTROL." Its comments give
 * the mode 1 angle, then the mode 2 one in parentheses. */
void remode(void)
{
    set_A(ANTENBIT); /* "DRIVE TRUNNION TO 0 (180)" */
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) set_A(BIT15);
    tc_at(REMODE + 4u);
    rrtonly();
}

void remode_shaft(void)
{
    set_A(MINUS_45DEGSR);
    tc_at(REMODE + 6u);
    rrsonly();
}

void remode_trunnion_out(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(ANTENBIT);
    if (bit_set_in_A()) set_A(MINUS_80DEGSR); /* "GO TO T = -130 (-50)." */
    ad(MINUS_50DEGSR);
    tc_at(REMODE + 014u);
    rrtonly();
}

void remode_trunnion_back(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(ANTENBIT);
    if (bit_set_in_A()) set_A(BIT15); /* "GO TO T = -180 (+0)." */
    tc_at(REMODE + 021u);
    rrtonly();
}

void remode_shaft_back(void)
{
    cs(ADDRESS_OF(RADMODES)); /* "GO TO S = -90 (+0)." */
    mask(ANTENBIT);
    if (bit_set_in_A()) set_A(negate(HALF));
    tc_at(REMODE + 026u);
    rrsonly();
}

void remode_invert(void)
{
    tc_at(REMODE + 027u);
    rmodinv();
}

void remode_end(void)
{
    set_A(negate(REMODBIT)); /* "END OF REMODE." */
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    set_A(DESIGBIT); /* "WAS REMODE CALLED DURING DESIGNATE?" */
    mask_at(ADDRESS_OF(RADMODES));
    if (A_is_zero()) {
        continue_at(RGOODEND); /* "NO -- RETURN TO CALLER WAITING IN RADSTALL" */
        return;
    }
    tc_to(REMODE + 037u, STDESIG); /* "YES -- RETURN TO DESIGNATE" */
}

/* RMODINV: "INVERT THE MODE STATUS." */
void rmodinv(void)
{
    lxch(ADDRESS_OF(RADMODES));
    set_A(ANTENBIT);
    rxor_lchan();
    ts(ADDRESS_OF(RADMODES));
    tc_q();
}

/* ---- RRTONLY, RRSONLY and RR1AX2 ---- */

/* RRTONLY and RRSONLY -- p. 533: "SUBROUTINES FOR DOING SINGLE AXIS RR
 * MANEUVERS FOR REMODE AND REPOSITION", A the desired angle. RRINDEX is the
 * axis (0 the trunnion, 1 the shaft); the return waits in RRRET while
 * RR1AX2 drives, every half second. */
static void rr1axis(void)
{
    ts(ADDRESS_OF(RRINDEX));
    qxch(ADDRESS_OF(RRRET));
    continue_at(RR1AX2); /* TCF RR1AX2 */
}

void rrtonly(void)
{
    ts(ADDRESS_OF(RDES)); /* "DESIRED TRUNNION ANGLE." */
    set_A(ZERO);
    rr1axis();
}

void rrsonly(void)
{
    ts(ADDRESS_OF(RDES)); /* "SHAFT COMMANDS ARE UNRESOLVED SINCE THIS */
    set_A(ONE);           /* ROUTINE ENTERED ONLY WHEN T = 0 OR 180." */
    rr1axis();
}

/* RRINDEX's axis: 0 or 1, as RRTONLY and RRSONLY set it. Not modelled: any
 * other, which would index past CDUT and CDUS. */
static unsigned rr_axis(unsigned address)
{
    word axis = fetch(ADDRESS_OF(RRINDEX));

    if (axis > 1u) not_modelled();
    return address + axis;
}

/* RR1AX2 -- p. 534: the error signal, "TRIES TO NULL .7 OF ERROR OVER NEXT
 * .5" second; done when within one degree or "IF RR OUT OF AUTO MODE". "IF
 * SOMEONE REQUESTS A DESIGNATE WHICH REQUIRES A REMODE AND A REPOSITION IS
 * IN PROGRESS, INTERRUPT IT AND START THE REMODE IMMEDIATELY" (PRIO22:
 * RADMODES bits 14 and 11). */
void rr1ax2(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(PRIO22);
    if (A_is_zero()) {
        continue_at(REPOSRPT);
        return;
    }
    ca(ADDRESS_OF(RDES));
    msu_at(rr_axis(ADDRESS_OF(CDUT))); /* EXTEND; INDEX RRINDEX; MSU CDUT */
    ts(ADDRESS_OF(ITEMP1));            /* "SAVE ERROR SIGNAL." */
    mp(RRSPGAIN);
    ts(L_ADDRESS);
    ca(ADDRESS_OF(RADMODES));
    mask(AUTOMBIT);
    xch(ADDRESS_OF(ITEMP1)); /* "STORE RR-OUT-OF-AUTO-MODE BIT." */
    tc_at(RR1AX2 + 017u);
    magsub(); /* "SEE IF WITHIN ONE DEGREE." DEC -.00555, "SCALED IN HALF-REVS." */
}

/* MAGSUB's +2: within one degree, back to the caller. */
void rr1ax2_done(void)
{
    tc_return_in(RR1AX2 + 022u, ADDRESS_OF(RRRET)); /* "RETURN TO CALLER." */
}

/* MAGSUB's +1: not yet. ITEMP1 holds AUTOMBIT out of auto mode (then the
 * TC RRRET above), else +0, and L the command. The other axis's command is
 * zero: TANG the trunnion's, TANG +1 the shaft's. Not modelled: an ITEMP1
 * or RRINDEX other than RR1AX2 and RR1AXIS leave. */
void rr1ax2_command(void)
{
    switch (ccs_at(ADDRESS_OF(ITEMP1))) {
    case POSITIVE: continue_at(RR1AX2 + 022u); return; /* "NO.  IF RR OUT OF AUTO MODE, EXIT." */
    case IS_PLUS_ZERO: break;
    default: not_modelled();
    }
    switch (ccs_at(ADDRESS_OF(RRINDEX))) { /* "COMMAND FOR OTHER AXIS IS ZERO." */
    case POSITIVE: break;                  /* TCF +2: "SETTING A TO 0." */
    case IS_PLUS_ZERO: xch(L_ADDRESS); break;
    default: not_modelled();
    }
    dxch(ADDRESS_OF(TANG));
    tc_to(RR1AX2 + 027u, RROUT);
}

void rr1ax2_after_rrout(void)
{
    tc_to(NXTRR1AX, FIXDELAY); /* TCF NXTRR1AX: DEC 50, "2 SAMPLES PER SECOND." */
}

/* ---- RROUT ---- */

/* RROUT -- p. 535, with interrupts held: "RECEIVES RR GYRO COMMANDS IN
 * TANG, TANG +1 IN RR ERROR COUNTER SCALING", limits each to 384 pulses and
 * sends the difference from the last command (LASTYCMD, LASTXCMD) to the
 * error counters (CDUTCMD, CDUSCMD): the shaft's first, ITEMP2 1, then the
 * trunnion's, ITEMP2 0. The return waits in L. */
void rrout(void)
{
    lxch(Q_ADDRESS); /* "SAVE RETURN" */
    set_A(ONE);      /* "LOOP TWICE." */
    rrout2();
}

/* ITEMP2's axis, 1 or 0. Not modelled: any other, which would index past
 * TANG, LASTYCMD and CDUTCMD's pairs. */
static unsigned rrout_axis(unsigned address)
{
    word axis = fetch(ADDRESS_OF(ITEMP2));

    if (axis > 1u) not_modelled();
    return address + axis;
}

void rrout2(void)
{
    ts(ADDRESS_OF(ITEMP2));
    ca(rrout_axis(ADDRESS_OF(TANG))); /* INDEX A; CA TANG */
    ts(ADDRESS_OF(ITEMP1));           /* "SAVE SIGN COMMAND FOR LIMITING." */
    tc_at(RROUT2 + 4u);
    magsub(); /* "SEE IF WITHIN LIMITS." -RRLIMIT, DEC -384 */
}

/* SETRRCTR +1: "COUNT OUT DIFFERENCE BETWEEN DESIRED STATE AND PRESENT
 * STATE AS RECORDED IN LASTYCMD AND LASTXCMD". */
static void count_out(void)
{
    xch(rrout_axis(ADDRESS_OF(LASTYCMD)));
    com();
    ad_at(ADDRESS_OF(ITEMP1));
    ad(NEG0);                                                /* "PREVENT +0 IN OUTCOUNTER" */
    if (ts(rrout_axis(ADDRESS_OF(CDUTCMD)))) not_modelled(); /* (commands of 384 pulses at most) */
    switch (ccs_at(ADDRESS_OF(ITEMP2))) {
    case POSITIVE: rrout2(); return; /* "PROCESS BOTH INPUTS." */
    case IS_PLUS_ZERO: break;
    default: not_modelled();
    }
    set_A(PRIO6);                             /* "ENABLE COUNTERS." */
    wor_channel(CHAN14);                      /* "PUT ON CDU DRIVES S AND T" */
    tc_return_in(SETRRCTR + 015u, L_ADDRESS); /* "RETURN." */
}

/* SETRRCTR: MAGSUB's +2, the command within the limit. */
void setrrctr(void)
{
    ca(ADDRESS_OF(ITEMP1));
    count_out();
}

/* RROUTLIM: MAGSUB's +1, "LIMIT COMMAND TO ABS VAL OF 384." (A zero ITEMP1,
 * which MAGSUB does not send here, stays +0.) */
void rroutlim(void)
{
    switch (ccs_at(ADDRESS_OF(ITEMP1))) {
    case POSITIVE: set_A(negate(MINUS_RRLIMIT)); break; /* CS -RRLIMIT */
    case NEGATIVE: set_A(MINUS_RRLIMIT); break;
    default: break; /* CCS leaves +0 */
    }
    ts(ADDRESS_OF(ITEMP1));
    count_out(); /* TCF SETRRCTR +1 */
}

/* ---- RRZERO and RRZ2 ---- */

/* RADNOOP -- p. 561: "NO FURTHER ACTION REQUESTED." RGOODEND as an
 * immediate task (2CADR RGOODEND). */
static void radnoop(void)
{
    set_A(ONE);
    tc_to(RADNOOP + 1u, WAITLIST);
}

/* RDBADEND -- p. 563: the radar job's bad end (POSTJUMP, CADR BADEND). */
void rdbadend(void)
{
    set_A(TWO);
    tc_to(RDBADEND + 1u, POSTJUMP);
}

/* ENDRADAR: "PROLOG TO CHECK RR CDU FAIL BEFORE END." */
void endradar(void)
{
    set_A(RCDUFBIT);
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A())
        continue_at(RGOODEND); /* TCF RGOODEND */
    else
        rdbadend(); /* TCF RDBADEND */
}

/* RRZERO -- p. 537, a radar request: "ROUTINE TO ZERO THE RR CDUS AND
 * DETERMINE THE ANTENNA MODE", as a task (RRZ2) in a hundredth of a second,
 * then back to the requester (SWRETURN), with alarm 510 if the radar is not
 * in auto mode. */
void rrzero(void)
{
    set_A(BIT11_1);                /* "SEE IF MONITOR REPOSITION OR NOT IN AUTO */
    mask_at(ADDRESS_OF(RADMODES)); /* IF SO, DON'T RE-ZERO CDUS." */
    if (bit_set_in_A()) {
        radnoop(); /* "(IMMEDIATE TASK TO RGOODEND)." */
        return;
    }
    inhint();
    set_A(negate(RCDU0BIT)); /* "SET FLAG TO SHOW ZEROING IN PROGRESS." */
    mask_at(ADDRESS_OF(RADMODES));
    ad(RCDU0BIT);
    ts(ADDRESS_OF(RADMODES));
    set_A(ONE);
    tc_to(RRZERO + 012u, WAITLIST); /* 2CADR RRZ2 */
}

void rrzero_after_waitlist(void)
{
    cs(ADDRESS_OF(RADMODES)); /* "SEE IF IN AUTO MODE." */
    mask(AUTOMBIT);
    if (bit_set_in_A()) {
        continue_at(ROADBACK); /* TCF ROADBACK */
        return;
    }
    tc_to(RRZERO + 021u, ALARM); /* OCT 510: "AUTO DISCRETE NOT PRESENT -- TRYING" */
}

void roadback(void)
{
    relint();
    continue_at(SWRETURN); /* TCF SWRETURN */
}

/* RRZ2: the task. "COMMON TO TURNON AND RRZERO." */
void rrz2(void)
{
    tc_at(RRZ2);
    rrzerosb();
}

void rrz2_after_rrzerosb(void)
{
    endradar(); /* TCF ENDRADAR */
}

#endif
