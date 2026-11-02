/* SPDX-License-Identifier: MIT */
/* P20-P25.agc, part 6 (Luminary099 pp. 538-552): the rendezvous radar's
 * designate, from RRDESSM to DODESEND.
 *
 * RRDESSM, an interpretive routine, points the antenna along a line of
 * sight known in stable member axes (RRTARGET): RRANGLES finds its gimbal
 * angles in each mode, RRLIMCHK checks them against the present mode's
 * limits, and TRYSWS against the other mode's (a remode first, RADMODES
 * bit 14). If neither will do, it returns to the caller +2: "VEHICLE
 * MANEUVER REQUIRED". RRDESNB, verb 41's, designates to angles given in
 * TANG and TANG +1, checked by RRLIMNB (RRLIMCHK's twin, with "THE MODE 1
 * SHAFT LOWER LIMIT IS -85 INSTEAD OF -70 DEGREES"), else alarm 502. Both
 * go on to STARTDES, which sets the designate bit (RADMODES bit 10), the
 * error counters and the task BEGDES, and returns to the caller +3 (+2 for
 * RRDESNB) through DESRET.
 *
 * BEGDES (a remode first if one is wanted) and STDESIG then run every half
 * second: they stop when a reposition has taken over or the designate is
 * done (ENDRADAR), stop at RDBADEND after DESCOUNT's 60 passes, and else
 * start the job DODES. DODES computes the gyro commands that turn the
 * antenna's line of sight onto the target's ("+ SHAFT = LOS . (COS(S), 0,
 * -SIN(S))", the trunnion's from "LOS . (SIN(T)SIN(S), COS(T),
 * SIN(T)COS(S))"), puts them out (RROUT) and, when the antenna points
 * within half a degree, ends the designate or enables the tracker for
 * lock-on. In R21 it also moves the line of sight on by the relative
 * velocity, and goes back to R21LEM2 for a new one when LOSCOUNT runs out.
 *
 * As in parts 1 to 5, each call out of the file leaves Z at the routine
 * and Q at the word after the TC, and a TC or a branch to a label the
 * faithful port keys a routine at stops there. STARTDES, TRYSWS, RRLIMNB
 * and RMODINV have no keys: they run inside their callers' routines. */
#include "p20_p25.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- RRDESSM ---- */

/* RRDESSM -- p. 538: the return in DESRET (STQ); RRNBSW clear (RRTARGET
 * is in stable member axes); "COMPUTES SINES AND COSINES, ORDER Y Z X" of
 * the IMU's CDUs, the line of sight to navigation base axes (*SMNB*), and
 * RRANGLES: "GET RR GIMBAL ANGLES IN PRESENT AND ALTERNATE MODE." (MODEA
 * and MODEB). RRANGLES's overflow return is LUNDESCH. */
static const word rrdessm_words[] = {
    /* RRDESSM */ ORDERS(STQ, CLEAR),
    ADDRESS(DESRET),
    SWITCH(CLEAR, RRNBSW),
    ORDER(CALL),
    ADDRESS(CDUTRIG),
    ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(RRTARGET),
    ADDRESS(STAR_SMNB_STAR),
    ORDER(CALL),
    ADDRESS(RRANGLES),
    ORDER(EXIT),
};

/* RRDESSM +013 -- the present mode's angles (MODEA) against its limits. */
void rrdessm_after_exit(void)
{
    inhint();
    tc_to(RRDESSM + 014u, RRLIMCHK); /* ADRES MODEA */
}

/* RRLIMCHK's +2: "NOT IN CURRENT MODE": TC +3, on with Q at OKDESSM. */
void rrdessm_not_in_mode(void)
{
    tc_to(OKDESSM - 1u, OKDESSM + 2u);
}

/* STARTDES -- p. 540: "STARTDES IS ENTERED WHEN WE ARE READY TO BEGIN
 * DESIGNATION." The return one on; the designate bit set (CS, MASK, ADS:
 * set if clear); during a reposition the counters are already set up
 * ("ECTR ALREADY SET UP."). Else SETRRECR, "SET UP ERROR COUNTERS.", and
 * BEGDES in 20 ms. */
static void startdes_plus1(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(DESIGBIT);
    ads(ADDRESS_OF(RADMODES));
    mask(REPOSBIT); /* "SEE IF REPOSITIONING IN PROGRESS." */
    if (bit_set_in_A())
        continue_at(DESRETRN); /* TCF DESRETRN */
    else
        tc_to(STARTDES + 7u, SETRRECR);
}

static void startdes(void)
{
    incr(ADDRESS_OF(DESRET));
    startdes_plus1();
}

/* OKDESSM: "INCREMENT SAYS NO VEHICLE MANEUVER REQ.", and STARTDES: "SHOW
 * DESIGNATE REQUIRED". */
void okdessm(void)
{
    incr(ADDRESS_OF(DESRET));
    tc_at(OKDESSM + 1u);
    startdes();
}

void startdes_after_setrrecr(void)
{
    set_A(TWO);
    tc_to(STARTDES + 011u, WAITLIST); /* 2CADR BEGDES */
}

/* DESRTRN: "SET EXIT" to the caller's return, one on again, through
 * BANKJUMP. */
static void desrtrn(void)
{
    relint();
    incr(ADDRESS_OF(DESRET));
    ca(ADDRESS_OF(DESRET));
    continue_at(BANKJUMP); /* TCF BANKJUMP */
}

/* DESRETRN -- p. 541: "FIRST PASS THRU DESIGNATE" (no one waiting in
 * RADSTALL): return to the caller; else the job ends. */
void desretrn(void)
{
    ca(ADDRESS_OF(RADCADR));
    if (A_is_zero())
        desrtrn(); /* "YES	SET EXIT" */
    else
        tc_to(DESRETRN + 3u, ENDOFJOB); /* "NO" */
}

/* NORDSTAL: "ZERO RADCADR TO WIPE OUT ANYONE WAITING IN RADSTALL SINCE WE
 * ARE NOW RETURNING TO P20 AND MAY DO NEW RADSTALL". (A +0: the TS does
 * not skip.) */
static void nordstal(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RADCADR));
    desrtrn();
}

/* TRYSWS -- p. 542: "SEE IF RRDESSM CAN BE ACCOMPLISHED AFTER A REMODE."
 * The mode bit inverted, the other mode's angles (MODEB) checked. */
static void trysws(void)
{
    tc_at(TRYSWS);
    rmodinv(); /* "(NOTE RUPT INHIBIT)" */
}

/* OKDESSM +2 (STARTDES's +1 return, and RRLIMCHK's +2 by TC +3): "CHECK
 * IF ON LUNAR SURFACE (SURFFLAG=P22F)": there "CANNOT DESIGNATE IN MODE
 * 2", NORDSTAL. Else TRYSWS. */
void okdessm_after_startdes(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(SURFFBIT);
    if (A_is_zero()) { /* EXTEND; BZF NORDSTAL */
        nordstal();
        return;
    }
    tc_at(OKDESSM + 6u);
    trysws();
}

/* NODESSM: "RE-INVERT MODE AND RETURN TO CALLER +2". */
static void nodessm(void)
{
    tc_at(NODESSM);
    rmodinv();
}

void nodessm_after_rmodinv(void)
{
    incr(ADDRESS_OF(DESRET));
    nordstal();
}

/* LUNDESCH: "OVERFLOW RETURN FROM RRANGLES". On the surface, NORDSTAL:
 * "RETURN TO CALLER -- ALARM 527"; with the rendezvous flag set, "NOT ON
 * MOON -- CALL FOR ATTITUDE MANEUVER" (NODESSM); else the job ends: "...
 * BUT NOT IN R29." */
void lundesch(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(SURFFBIT);
    if (A_is_zero()) { /* EXTEND; BZF NORDSTAL */
        nordstal();
        return;
    }
    ca(ADDRESS_OF(STATE));
    mask(RNDVZBIT);
    if (bit_set_in_A()) { /* "TEST RNDVZFLG" */
        tc_at(LUNDESCH + 7u);
        nodessm();
    } else {
        continue_at(LUNDESCH + 010u);
    }
}

void lundesch_not_rendezvous(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* TRYSWS +1, the mode bit inverted: "TRY DIFFERENT MODE." */
void trysws_after_rmodinv(void)
{
    tc_to(TRYSWS + 1u, RRLIMCHK); /* ADRES MODEB */
}

/* RRLIMCHK's +2: "VEHICLE MANEUVER REQUIRED". */
void trysws_out_of_limits(void)
{
    nodessm(); /* TCF NODESSM */
}

/* RRLIMCHK's +3: "RESET BIT12", then "SET FLAG FOR REMODE." */
void trysws_within_limits(void)
{
    tc_at(TRYSWS + 4u);
    rmodinv();
}

void trysws_after_rmodinv2(void)
{
    set_A(REMODBIT);
    ads(ADDRESS_OF(RADMODES));
    continue_at(OKDESSM); /* TCF OKDESSM */
}

/* ---- RRDESNB and RRLIMNB ---- */

/* RRDESNB -- p. 543: "DESIGNATE TO SPECIFIC RR GIMBAL ANGLES (INDEPENDENT
 * OF VEHICLE MOTION).  ENTER WITH DESIRED ANGLES IN TANG AND TANG +1." The
 * caller's return, as a CADR, to DESRET. */
void rrdesnb(void)
{
    tc_to(RRDESNB, MAKECADR);
}

void rrdesnb_after_makecadr(void)
{
    if (ts(ADDRESS_OF(DESRET))) not_modelled(); /* (A as MAKECADR leaves it does not overflow) */
    tc_to(RRDESNB + 2u, DOWNFLAG); /* "RESET FLAG TO PREVENT DODES FROM GOING BACK TO R21" */
}

/* RRLIMNB -- p. 543: "THIS ROUTINE IS IDENTICAL TO RRLIMCHK EXCEPT THAT
 * THE MODE 1 SHAFT LOWER LIMIT IS -85 INSTEAD OF -70 DEGREES": "MODE 1 IS
 * DEFINED AS 1 ABS(T) L 55 DEGS 2 SHAFT LIMITS AT +59, -85 DEGS". The word
 * after the TC is the angles' address (TANG); "L(CALLER +2) TO L". Mode 2
 * goes to RRLIMCHK's MODE2CHK. Its callers are in this bank, so its INDEX
 * Q reads their ADRES TANG. */
static void rrlimnb(void)
{
    word angles = read_word(Q); /* INDEX Q; CAF 0 */

    incr(Q_ADDRESS);
    dca(angles); /* EXTEND; INDEX A; DCA 0: "READ GIMBAL ANGLES INTO ITEMP STORAGE" */
    dxch(ADDRESS_OF(ITEMP1));
    lxch(Q_ADDRESS);
    set_A(ANTENBIT); /* "SEE WHICH MODE RR IS IN" */
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) {
        continue_at(MODE2CHK); /* "MODE 2 CAN USE RRLIMCHK CODING" */
        return;
    }
    ca(ADDRESS_OF(ITEMP1));
    tc_at(RRLIMNB + 015u);
    magsub(); /* DEC -.30555 */
}

void rrlimnb_trunnion_out(void)
{
    tc_return_in(RRLIMNB + 017u, L_ADDRESS);
}

/* "IF NEGATIVE SHAFT ANGLE, ADD 20.5 DEGS" (NEGSHAFT), else 5.5; then
 * SHAFTLIM: within "64.5 DEGREES"? */
void rrlimnb_trunnion_in(void)
{
    ca(ADDRESS_OF(ITEMP2));     /* "LOAD SHAFT ANGLE" */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF NEGSHAFT */
        continue_at(NEGSHAFT);
        return;
    }
    ad(FIVE_5DEGS);
    tc_at(SHAFTLIM);
    magsub(); /* DEC -.35833 */
}

void rrlimnb_shaft_out(void)
{
    tc_return_in(SHAFTLIM + 2u, L_ADDRESS); /* "NOT IN LIMITS" */
}

void rrlimnb_shaft_in(void)
{
    tc_to(SHAFTLIM + 3u, RRLIMOK); /* "IN LIMITS" */
}

/* NEGSHAFT: "MAKE NEGATIVE SHAFT LIMIT -85 DEGREES". */
void negshaft(void)
{
    ad(TWENTY_5DEGS);
    tc_at(SHAFTLIM); /* TCF SHAFTLIM */
    magsub();
}

/* RRDESNB +4 -- "SET TIME LIMIT COUNTER FOR DESIGNATE", and with
 * interrupts held, "SEE IF CURRENT MODE OK.": "DO SPECIAL V41 LIMIT
 * CHECK". (A, MAXTRYS, does not overflow: the TS does not skip.) */
void rrdesnb_after_downflag(void)
{
    set_A(MAXTRYS);
    ts(ADDRESS_OF(DESCOUNT));
    inhint();
    tc_at(RRDESNB + 7u);
    rrlimnb(); /* ADRES TANG */
}

/* TRYSWN: "SEE IF OTHER MODE WILL DO." */
void rrdesnb_not_in_mode(void)
{
    tc_at(TRYSWN); /* TCF TRYSWN */
    rmodinv();
}

/* OKDESNB: the angles to TANGNB, and "GET LOS IN NB COORDS." */
void okdesnb(void)
{
    relint();
    dca(ADDRESS_OF(TANG)); /* EXTEND; DCA TANG */
    dxch(ADDRESS_OF(TANGNB));
    tc_at(OKDESNB + 4u);
    intpret();
}

/* OKDESNB +5 -- RRNB: the line of sight from the angles, in navigation
 * base axes, to RRTARGET; RRNBSW set (DODES then needs no *SMNB*). */
static const word okdesnb_words[] = {
    ORDER(CALL),     ADDRESS(RRNB), /* "GET LOS IN NB COORDS." */
    STORE(RRTARGET), ORDERS(SET, EXIT), SWITCH(SET, RRNBSW),
};

/* OKDESNB +012 -- STARTDES +1, with interrupts held (DESRET, a CADR, is
 * not incremented here: RRDESNB returns to the caller +2). */
void okdesnb_after_exit(void)
{
    inhint();
    startdes_plus1(); /* TCF STARTDES +1 */
}

void tryswn_after_rmodinv(void)
{
    tc_at(TRYSWN + 1u);
    rrlimnb(); /* ADRES TANG */
}

/* RRLIMNB's +2: "NOT POSSIBLE." */
void tryswn_out_of_limits(void)
{
    tc_at(NODESNB); /* TCF NODESNB */
    rmodinv();      /* "REINVERT MODE BIT." */
}

/* RRLIMNB's +3: the mode bit back, and "CALL FOR REMODE." */
void tryswn_within_limits(void)
{
    tc_at(TRYSWN + 4u);
    rmodinv();
}

void tryswn_after_rmodinv2(void)
{
    set_A(REMODBIT);
    ads(ADDRESS_OF(RADMODES));
    continue_at(OKDESNB); /* TCF OKDESNB */
}

/* NODESNB +1: "BAD INPUT ANGLES.", the designate's bits cleared, and the
 * job ends: "AVOID 503 ALARM." */
void nodesnb_after_rmodinv(void)
{
    tc_to(NODESNB + 1u, ALARM); /* OCT 502 */
}

void nodesnb_after_alarm(void)
{
    tc_to(NODESNB + 3u, CLRADMOD);
}

void nodesnb_after_clradmod(void)
{
    tc_to(NODESNB + 4u, ENDOFJOB);
}

/* ---- BEGDES and STDESIG ---- */

/* BEGDES -- p. 545, the task STARTDES starts: a remode first if one is
 * wanted (REMODE returns to STDESIG), else STDESIG. */
void begdes(void)
{
    cs(ADDRESS_OF(RADMODES));
    mask(REMODBIT);
    if (bit_set_in_A())
        tc_to(BEGDES + 3u, STDESIG);
    else
        continue_at(BEGDES + 4u);
}

void begdes_remode(void)
{
    tc_to(BEGDES + 4u, REMODE);
}

/* DESLOOP: "2 SAMPLES PER SECOND." */
void desloop(void)
{
    tc_to(DESLOOP, FIXDELAY); /* DEC 50 */
}

/* BADDES: "REMOVE DESIGNATE FLAG", and RDBADEND. */
static void baddes(void)
{
    set_A(negate(DESIGBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    rdbadend(); /* TCF RDBADEND */
}

/* MOREDES +1: DODES as a job, "UPDATE GYRO TORQUE COMMANDS.", and again in
 * half a second (DESLOOP). */
static void moredes_plus1(void)
{
    set_A(PRIO26);
    tc_to(MOREDES + 2u, FINDVAC); /* 2CADR DODES */
}

/* MOREDES: one pass fewer in DESCOUNT (A, CCS's result, does not
 * overflow). */
static void moredes(void)
{
    ts(ADDRESS_OF(DESCOUNT));
    moredes_plus1();
}

/* STDESIG1: "SEE IF THE TIME LIMIT HAS EXPIRED". Out of time: "IF OUT OF
 * TIME, REMOVE ECR ENABLE + TRKR" (channel 12 bits 14 and 2), and BADDES.
 * With DESCOUNT negative the WAND takes CCS's result in A, as the listing
 * runs it. Not modelled: DESCOUNT -0, which runs WAND CHAN12 as a WRITE. */
static void stdesig1(void)
{
    switch (ccs_at(ADDRESS_OF(DESCOUNT))) {
    case POSITIVE: moredes(); return; /* TCF MOREDES */
    case IS_PLUS_ZERO: set_A(negate(B14_B2)); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    wand_channel(CHAN12);
    baddes();
}

/* STDESIG -- p. 546: "SEE IF GIMBAL LIMIT MONITOR HAS FOUND US OUT OF
 * BOUNDS.  IF SO, THIS BIT SHOWS A REPOSITION TO BE IN PROGRESS.": BADDES.
 * "SEE IF CONTINUOUS DESIGNATE WANTED." (RADMODES negative: bit 15): "IF
 * SO, DON'T CHECK BIT 10 TO SEE IF IN LIMITS BUT GO RIGHT TO FINDVAC
 * ENTRY." Else "SEE IF END OF PROBLEM (DATA GOOD IF LOCK-ON WANTED OR
 * WITHIN LIMITS IF NOT).  IF SO, EXIT AFTER CHECKING RR CDU FAIL."
 * (ENDRADAR), or the time limit. */
void stdesig(void)
{
    set_A(REPOSBIT);
    mask_at(ADDRESS_OF(RADMODES));
    if (bit_set_in_A()) {
        baddes(); /* TCF BADDES */
        return;
    }
    if (ccs_at(ADDRESS_OF(RADMODES)) == NEGATIVE) {
        moredes_plus1(); /* TCF MOREDES +1 */
        return;
    }
    cs(ADDRESS_OF(RADMODES)); /* (TCF +3, TCF +2, or on: here) */
    mask(DESIGBIT);
    if (bit_set_in_A()) {
        endradar(); /* TCF ENDRADAR */
        return;
    }
    stdesig1();
}

void moredes_after_findvac(void)
{
    continue_at(DESLOOP); /* TCF DESLOOP */
}

/* ---- DODES ---- */

/* DODES -- p. 548: the job: the radar's CDU angles to TANG, TANG +1. */
void dodes(void)
{
    dca(ADDRESS_OF(CDUT)); /* EXTEND; DCA CDUT */
    dxch(ADDRESS_OF(TANG));
    tc_at(DODES + 3u);
    intpret();
}

/* DODES +4 -- the line of sight in navigation base axes to 32D: from
 * RRTARGET, unless RRNBSW says it is there already (DONBRD), "MULTIPLY UNIT
 * LOS BY MAGNITUDE" (MLOSV) and "ADD ONE SECOND RELATIVE VELOCITY TO LOS"
 * (LOSVEL, MCTOMS), a unit vector through *SMNB*. Then the commands from
 * the CDU angles (CDULOGIC, from two's complement): "SHAFT COMMAND =
 * V(32D).(COS(S), 0, -SIN(S))." ("SIN(S) TO 0 AND COS(S) TO 2.") to TANG +1,
 * and the vector U = (SIN(T)SIN(S), COS(T), SIN(T)COS(S)) ("COS(T) TO 4.",
 * "SIN(T) TO 6."), dotted with the line of sight: "DOT U WITH LOS TO GET
 * TRUNNION COMMAND." */
static const word dodes_words[] = {
    ORDERS(SETPD, VLOAD),
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE(RRTARGET),
    ORDERS(BON, VXSC),
    SWITCH(BON, RRNBSW),
    ADDRESS(DONBRD), /* "TARGET IN NAV-BASE COORDINATES" */
    ADDRESS_PLUS_ONE(MLOSV),
    ORDERS(VSL1, PDVL),
    ADDRESS_PLUS_ONE(LOSVEL),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(MCTOMS),
    ORDERS(UNIT, CALL),
    ADDRESS(CDUTRIG),
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR),
    /* DONBRD */ STODL_D(32),
    ADDRESS_PLUS_ONE_AT(TANG, 1),
    ORDERS(RTB, PUSH),
    ADDRESS(CDULOGIC),
    ORDERS(SIN, PDDL),
    ORDERS(COS, PUSH),
    ORDERS(DMP, PDDL),
    ADDRESS_PLUS_ONE_D(32),
    ADDRESS_PLUS_ONE_D(36),
    ORDERS(DMP, BDSU),
    ADDRESS_PLUS_ONE_D(0),
    ORDER(STADR),
    AFTER_STADR(STORE_AT(TANG, 1)), /* "SHAFT COMMAND" */
    ORDERS(SLOAD, RTB),
    ADDRESS_PLUS_ONE(TANG),
    ADDRESS(CDULOGIC),
    ORDERS(PUSH, COS),
    ORDERS(PDDL, SIN),
    ORDERS(PUSH, DMP),
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(SL1, PDDL),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(PDDL, DMP),
    ADDRESS_PLUS_ONE_D(6),
    ADDRESS_PLUS_ONE_D(0),
    ORDERS(SL1, VDEF),
    ORDERS(DOT, EXIT),
    ADDRESS_PLUS_ONE_D(32),
};

/* DODES +057 -- "DOT WAS NEGATIVE OF DESREG ANGLE.": the commands scaled
 * by RDESGAIN, which "TRIES TO NULL .5 ERROR IN .5 SEC."; "SCALING ON
 * INPUT ANGLE WAS 4 RADIANS." In mode 2 (RADMODES bit 12) "A RELAY IN THE
 * RR REVERSES POLARITY OF THE SHAFT COMMANDS": "COMPENSATE FOR THIS SWITCH
 * BY CHANGING THE POLARITY OF OUR COMMAND." (A, a product, does not
 * overflow: neither TS skips.) */
void dodes_after_exit(void)
{
    cs(ADDRESS_OF(MPAC));
    mp(RDESGAIN);
    ts(ADDRESS_OF(TANG)); /* "TRUNNION COMMAND." */
    cs(ADDRESS_OF(RADMODES));
    mask(BIT12);
    if (A_is_zero()) /* EXTEND; BZF +3 */
        cs(ADDRESS_OF(TANG) + 1u);
    else
        ca(ADDRESS_OF(TANG) + 1u);
    mp(RDESGAIN);
    ts(ADDRESS_OF(TANG) + 1u); /* "SHAFT COMMAND FOR RROUT" */
    tc_at(DODES + 075u);
    intpret();
}

/* DODES +076 -- the radar's pointing axis, URR, from the angles: its Z
 * component COS(S)COS(T), its Y -SIN(T), its X SIN(S)COS(T): "FORM URR IN
 * NB AXES.", then "GET URR IN SM AXES." (*NBSM*, which "BYPASS NBSM
 * CONVERSION IN VERB 41" skips: RRNBSW), and "GET COSIN OF ANGLE BETWEEN
 * RR AND LOS". */
static const word dodes_urr_words[] = {
    ORDERS(DLOAD, DMP),
    ADDRESS_PLUS_ONE_D(2), /* "COS(S)." */
    ADDRESS_PLUS_ONE_D(4), /* "COS(T)." */
    ORDERS(SL1, PDDL),     /* "Z COMPONENT OF URR." */
    ORDERS(DCOMP, PDDL),   /* "Y COMPONENT = -SIN(T)" */
    ADDRESS_PLUS_ONE_D(0), /* "SIN(S)." */
    ORDERS(DMP, SL1),
    ADDRESS_PLUS_ONE_D(4),
    ORDERS(VDEF, BON),
    SWITCH(BON, RRNBSW),
    ADDRESS_AT(DONBRD, 070u), /* +3 */
    ORDER(CALL),
    ADDRESS(STAR_NBSM_STAR),
    ORDERS(DOT, EXIT),
    ADDRESS_PLUS_ONE(RRTARGET),
};

/* DORROUT -- p. 551 (bits 12 and 14 of FLAGWRD2: LOSCMFLG and the search
 * flag): in R21 or R24, "MULTIPLY UNIT LOS BY MAGNITUDE" and "ADD .5 SEC.
 * OF VELOCITY TO LOS VECTOR", stored again; else NOTP20. */
static void dorrout(void)
{
    ca(ADDRESS_OF(FLAGWRD2));
    mask(BIT12_14);
    if (A_is_zero()) { /* EXTEND; BZF NOTP20 */
        continue_at(NOTP20);
        return;
    }
    tc_at(DORROUT + 4u);
    intpret();
}

static const word dorrout_words[] = {
    ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(RRTARGET),
    ADDRESS_PLUS_ONE(MLOSV),
    ORDERS(VSL1, PUSH),
    ORDERS(VLOAD, VXSC),
    ADDRESS_PLUS_ONE(LOSVEL),
    ADDRESS_PLUS_ONE(MCTOMS),
    ORDERS(VSR1, VAD),
    ORDER(UNIT),
    STODL(RRTARGET), /* "STORE VELOCITY-CORRECTED LOS (UNIT)" */
    ADDRESS_PLUS_ONE_D(36),
    STORE(MLOSV), /* "AND STORE MAGNITUDE" */
    ORDER(EXIT),
};

/* RRDESDUN: "WHEN PROBLEM DONE, REMOVE BIT 10 SO NEXT WAITLIST TASK WE
 * WILL GO TO RGOODEND."; LOSCMFLG reset (no new line of sight after data
 * good). (A, a masked word, does not overflow: the TS does not skip.) */
static void rrdesdun(void)
{
    set_A(negate(BIT10));
    mask_at(ADDRESS_OF(RADMODES));
    inhint();
    ts(ADDRESS_OF(RADMODES));
    tc_to(RRDESDUN + 4u, DOWNFLAG); /* ADRES LOSCMFLG */
}

/* "TURN OFF ENABLE RR ERROR COUNTER", and the job ends "WITH ECTR
 * DISABLED." */
void rrdesdun_after_downflag(void)
{
    set_A(negate(BIT2));
    wand_channel(CHAN12);
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* DGOODCHK -- p. 551: "SEE IF DATA GOOD RECEIVED YET" (channel 33 bit 4,
 * clear when it has): done, RRDESDUN; not yet, DORROUT. */
static void dgoodchk(void)
{
    set_A(BIT4);
    rand_channel(CHAN33);
    if (bit_set_in_A())
        dorrout();
    else
        rrdesdun();
}

/* SIGNLCHK: "SEE IF BOTH AXES WERE WITHIN .5 DEGS." Not: DGOODCHK. Yes:
 * "IF WITHIN LIMITS AND NO LOCK-ON WANTED, PROBLEM IS FINISHED."; with
 * lock-on wanted, "ENABLE THE TRACKER" and DGOODCHK. */
static void signlchk(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC) + 1u)) {
    case POSITIVE: dgoodchk(); return; /* TCF DGOODCHK */
    case IS_PLUS_ZERO: break;
    default: not_modelled(); /* (MPAC +1 is 0 or 1) */
    }
    cs(ADDRESS_OF(STATE));
    mask(LOKONBIT);
    if (bit_set_in_A()) {
        rrdesdun(); /* TCF RRDESDUN */
        return;
    }
    set_A(BIT14);
    wor_channel(CHAN12);
    dgoodchk();
}

/* DODES +0115 -- the cosine of the angle between the radar's axis and the
 * line of sight, less that of half a degree: "DIFFERENCE OF COSINES,
 * SCALED B-2." to MPAC +1 as "ZERO IF RR IS POINTED OK, ONE IF NOT." ("IF
 * COS ERROR BIGGER, ERROR IS SMALLER"). "IF CONTINUOUS DESIGNATE WANTED,
 * PUT OUT COMMANDS WITHOUT CHECKING MAGNITUDE OF ERROR SIGNALS" (DORROUT);
 * else SIGNLCHK. */
void dodes_after_dot(void)
{
    dcs(COS1_2DG); /* EXTEND; DCS COS1/2DG */
    das(ADDRESS_OF(MPAC));
    set_A(ccs_at(ADDRESS_OF(MPAC)) == NEGATIVE ? ONE : ZERO); /* CCS MPAC, CA ZERO or CA ONE */
    ts(ADDRESS_OF(MPAC) + 1u);
    if (ccs_at(ADDRESS_OF(RADMODES)) == NEGATIVE)
        dorrout(); /* TCF DORROUT */
    else
        signlchk(); /* TCF SIGNLCHK, or on */
}

/* NOTP20 -- p. 552: "PUT OUT COMMAND UNLESS MONITOR REPOSITION HAS TAKEN OVER". */
void notp20(void)
{
    inhint();
    cs(ADDRESS_OF(RADMODES));
    mask(REPOSBIT);
    if (bit_set_in_A())
        tc_to(NOTP20 + 4u, RROUT);
    else
        continue_at(NOTP20 + 5u);
}

/* NOTP20 +5 -- "IF LOSCMFLG NOT SET, DON'T TEST LOS COUNTER", the job
 * ends. Else "TEST LOS COUNTER TO SEE IF TIME TO GET A NEW LOS": counting
 * down, DODESEND; out (+0), the task that schedules DODES killed and R21's
 * R21LEM2 for a new one. LOSCOUNT negative skips the INHINT, as the
 * listing runs it. Not modelled: LOSCOUNT -0, which runs the CADR after
 * KILLTASK as an instruction. */
void notp20_after_rrout(void)
{
    ca(ADDRESS_OF(FLAGWRD2));
    mask(LOSCMBIT);
    if (A_is_zero()) { /* EXTEND; BZF ENDOFJOB */
        continue_at(ENDOFJOB);
        return;
    }
    switch (ccs_at(ADDRESS_OF(LOSCOUNT))) {
    case POSITIVE: tc_to(NOTP20 + 012u, DODESEND); return;
    case IS_PLUS_ZERO: continue_at(NOTP20 + 013u); return;
    case NEGATIVE: tc_to(NOTP20 + 014u, KILLTASK); return; /* CADR DESLOOP +2 */
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* NOTP20 +013: "YES -- KILL TASK WHICH SCHEDULES DODES". */
void notp20_new_los(void)
{
    inhint();
    tc_to(NOTP20 + 014u, KILLTASK); /* CADR DESLOOP +2 */
}

/* NOTP20 +016 -- a waiting job of higher priority first (CHANG1), then
 * R21LEM2. Not modelled: NEWJOB negative, which runs the CADR after BANKCALL
 * as an instruction. */
void notp20_after_killtask(void)
{
    relint();
    switch (ccs_at(ADDRESS_OF(NEWJOB))) {
    case POSITIVE: tc_to(NOTP20 + 020u, CHANG1); return;
    case IS_PLUS_ZERO: continue_at(NOTP20 + 021u); return;
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: continue_at(DODESEND); return;
    }
}

void notp20_after_chang1(void)
{
    tc_to(NOTP20 + 021u, BANKCALL); /* CADR R21LEM2 */
}

/* DODESEND: LOSCOUNT one less (CCS's result), and the job ends. Not
 * modelled: an A that overflows, where the TS would skip. */
void dodesend(void)
{
    if (ts(ADDRESS_OF(LOSCOUNT))) not_modelled();
    tc_to(DODESEND + 1u, ENDOFJOB);
}

/* ---- The interpretive code ---- */

const struct interpretive_code RRDESSM_CODE = {025u, RRDESSM, COUNT(rrdessm_words), rrdessm_words};
const struct interpretive_code OKDESNB_CODE = {025u, OKDESNB + 5u, COUNT(okdesnb_words),
                                               okdesnb_words};
const struct interpretive_code DODES_CODE = {025u, DODES + 4u, COUNT(dodes_words), dodes_words};
const struct interpretive_code DODES_URR_CODE = {025u, DODES + 076u, COUNT(dodes_urr_words),
                                                 dodes_urr_words};
const struct interpretive_code DORROUT_CODE = {025u, DORROUT + 5u, COUNT(dorrout_words),
                                               dorrout_words};

#endif
