/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 7 (Comanche055 pp. 729-736, banks 20, 21 and 42): the
 * file's last routines. The faithful C is c64-port/src/cm_p40_p47.c.
 *
 * S40.6, a task before an SPS burn: the gimbal drive test, the engine
 * bell to +2, -2 and 0 degrees two seconds apart, in pitch and then in
 * yaw, for the crew to watch on the gimbal position indicator; then the
 * gimbal trim, the bell to the trim angles PACTOFF and YACTOFF. With
 * MRKRTMP negative, the trim only. PRE40.6, its restart entry, trims at
 * once, and only while the RCS autopilot has the attitude.
 *
 * S41.2 and S40.14 turn the crew's autopilot data (verb 48: noun 46,
 * DAPDATR1 and DAPDATR2, and noun 47, the masses) into the RCS
 * autopilot's terms: the rate and deadband, whether the LM is attached,
 * the quads for +X translation, the failed quads (S41.2), and the
 * inertia terms (S40.14). DAPFIG, after verb 46, starts the autopilot
 * the configuration names. RCSUP and RCSDAPON give T5 to the RCS
 * autopilot.
 *
 * Each call out of the file leaves Z at the routine and Q at the word
 * after the TC; a TCF out of it leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"

#if COMANCHE055

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q, a subroutine's return (as in programs.c): Z gets the address in
 * Q, which becomes 3. Not modelled: a Q that is no such address. */
static void tc_q(void)
{
    word at = overflow_corrected(Q);

    if (at < 010u || at > 07777u) not_modelled();
    Q = 3u;
    Z = at;
}

/* ---- S40.6 -- pp. 729-731 ---- */

/* INDEX CNTR before an address: CNTR is -0 for pitch and -1 for yaw, and
 * the yaw word is the one before the pitch word (TVCYAW, TVCPITCH; BIT12,
 * BIT11). */
static unsigned by_axis(unsigned address)
{
    return indexed(address, fetch(ADDRESS_OF(CNTR)));
}

void s40_6(void);
static void gimtrim(void);
static void gimtrim_plus_2(void);

/* PRE40.6, the "RESTART ENTRY TO S40.6": under the TVC autopilot (bits
 * 15 and 14 of FLAGWRD6 1 and 0) the task ends (TASKOVER), as "REDOTVC
 * WILL REESTABLISH INTERFACE"; under RCS, "GIMTRIM ONLY" (MRKRTMP -1),
 * CNTR +1 "TO INDICATE A RESTART ENTRY", and S40.6. */
void pre40_6(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (!A_is_zero_or_minus()) { /* EXTEND; BZMF +2 */
        continue_at(TASKOVER);   /* "TVC, REDOTVC WILL REESTABLISH INTERFACE" */
        return;
    }
    set_A(negate(BIT1)); /* "RCS, SO DO S40.6, GIMTRIM ONLY" */
    ts(ADDRESS_OF(MRKRTMP));
    set_A(BIT1); /* "FOR REVISED S40.6 TIMING FOR RESTARTS..." */
    ts(ADDRESS_OF(CNTR));
    s40_6();
}

/* S40.6: OPTIND -0, "INHIBIT OPTICS ACTIVITY"; channel 12 bit 2 off (the
 * optics error counters), bits 8 and 11 on ("TVC ENABLE", OCT02200);
 * "60MS PROCEDURAL DELAY" (FIXDELAY, DEC 6), */
void s40_6(void)
{
    cs(ZERO_ADDRESS); /* "INHIBIT OPTICS ACTIVITY" */
    ts(ADDRESS_OF(OPTIND));
    set_A(negate(BIT2));  /* "DISENABLE OPTICS ERROR COUNTERS (ZERO," */
    wand_channel(CHAN12); /* EXTEND */
    set_A(OCT02200);      /* "TVC ENABLE (SPS SERVO AMPS SEE DAC" */
    wor_channel(CHAN12);  /* EXTEND */
    tc_to(S40_6 + 010u, FIXDELAY);
}

/* then bit 2 on, "ENABLE OPTICS ERROR COUNTERS", and "20MS PROCEDURAL
 * DELAY" (FIXDELAY, DEC 2), */
void s40_6_after_fixdelay(void)
{
    set_A(BIT2);         /* "ENABLE OPTICS ERROR COUNTERS" */
    wor_channel(CHAN12); /* EXTEND */
    tc_to(S40_6 + 015u, FIXDELAY);
}

static void gdtsetup(void);

/* and RSTRTST, "CHECK FOR RESTART ENTRY (PRE40.6)": CNTR positive, the
 * trim at once (GIMTRIM +2); else CNTR from MRKRTMP, and the trim only
 * (GIMTRIM) or the test first (GDTSETUP) by its sign. */
void rstrtst(void)
{
    switch (ccs_at(ADDRESS_OF(CNTR))) {
    case POSITIVE: gimtrim_plus_2(); return; /* "RESTART ENTRY....BYPASS 4 SECOND DELAY" */
    case IS_PLUS_ZERO:
        ca(ADDRESS_OF(MRKRTMP)); /* CAE: "CHECK FOR TEST/TRIM OR TRIM ONLY" */
        /* fall through */
    case NEGATIVE:
        ts(ADDRESS_OF(CNTR)); /* "MRKRTMP SAVES CNTR FOR RESTARTS" */
        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF GIMTRIM: "(TRIM ONLY)" */
        gimtrim();
        return;
    }
    gdtsetup();
}

static void output(void);

/* GIMDTEST: "GIMBAL DRIVE TEST, 1ST INCREMENT", +2ACTDEG (TC OUTPUT), */
static void gimdtest(void)
{
    set_A(P2ACTDEG);
    tc_at(GIMDTEST + 1u);
    output();
}

/* GDTSETUP: "GIMBAL DRIVE TEST SETUP, FOR PITCH" (CNTR -0), GIMDTEST. */
static void gdtsetup(void)
{
    cs(ZERO_ADDRESS);
    ts(ADDRESS_OF(CNTR));
    gimdtest();
}

/* GIMDTEST +2: "2ND INCREMENT (LEAVES GIMBAL AT -2)", -4ACTDEG, */
void gimdtest_second(void)
{
    set_A(M4ACTDEG);
    tc_at(GIMDTEST + 3u);
    output();
}

/* GIMDTEST +4: "3RD INCREMENT (LEAVES GIMBAL AT -0)", +2ACTDEG, */
void gimdtest_third(void)
{
    set_A(P2ACTDEG);
    tc_at(GIMDTEST + 5u);
    output();
}

/* GIMDTEST +6: "CHECK FOR COMPLETION OF YAW TEST." (CNTR -1): done,
 * GIMTRIM; else "SET UP YAW TEST" (CNTR -1) and GIMDTEST again. */
void gimdtest_check(void)
{
    cs(ADDRESS_OF(CNTR));
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: gimtrim(); return; /* "COMPLETED, GO TO GIMBAL TRIM ROUTINE" */
    case IS_PLUS_ZERO:
        set_A(negate(BIT1)); /* "SET UP YAW TEST" */
        /* fall through */
    case NEGATIVE:
        ts(ADDRESS_OF(CNTR));
        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    gimdtest(); /* TCF: "FOR YAW TEST" */
}

/* OUTPUT: "OUTPUT THE INCREMENT....SAVE Q" (QXCH TEMPR60): the increment to
 * TVCPITCH or TVCYAW, its counter released (bit 11 or 12 of channel 14),
 * and "WAIT 2SEC, WHILE ASTRONAUT VERIFIES" (FIXDELAY, DEC 200), */
static void output(void)
{
    unsigned to;

    qxch(ADDRESS_OF(TEMPR60)); /* EXTEND */
    to = by_axis(ADDRESS_OF(TVCPITCH));
    /* Not modelled, as in the faithful port: a TS past erasable memory,
     * or to Z or a bank register; a CAF of Z, or of a fixed word other
     * than BIT12, BIT11 and BIT10. */
    if (to > 01777u || (to >= FBANK_ADDRESS && to <= BBANK_ADDRESS)) not_modelled();
    ts(to);
    to = by_axis(ADDRESS_OF(BIT11));
    if (to == Z_ADDRESS || (to >= 02000u && (to < ADDRESS_OF(BIT12) || to > ADDRESS_OF(BIT10))))
        not_modelled();
    ca(to);              /* CAF */
    wor_channel(CHAN14); /* EXTEND */
    tc_to(OUTPUT + 010u, FIXDELAY);
}

/* then back to GIMDTEST (TC TEMPR60: the word there, a TC, runs, so Z
 * gets its address and Q the word after TEMPR60). Not modelled: a word
 * that is no TC. */
void output_after_fixdelay(void)
{
    word at = fetch(ADDRESS_OF(TEMPR60));

    if (at < 010u || at > 07777u) not_modelled();
    Q = ADDRESS_OF(TEMPR60) + 1u;
    Z = at;
}

/* GIMTRIM: "WAIT 4 SECONDS BEFORE GIMBAL TRIM" (FIXDELAY, DEC 400), */
static void gimtrim(void)
{
    tc_to(GIMTRIM, FIXDELAY);
}

/* GIMTRIM +2: "PICK UP TRIM VALUES AND OUTPUT THEM" (-0 plus PACTOFF and
 * YACTOFF, "(AVOID +0)"), "RELEASE THE COUNTERS, BITS 11,12" (PRIO6 to
 * channel 14), and ENDS40.6: the task ends (TCF TASKOVER). */
static void gimtrim_plus_2(void)
{
    cs(ZERO_ADDRESS); /* "PICK UP TRIM VALUES AND OUTPUT THEM" */
    ad_at(ADDRESS_OF(PACTOFF));
    ts(ADDRESS_OF(TVCPITCH));
    cs(ZERO_ADDRESS);
    ad_at(ADDRESS_OF(YACTOFF));
    ts(ADDRESS_OF(TVCYAW));
    set_A(PRIO6);        /* "RELEASE THE COUNTERS, BITS 11,12" */
    wor_channel(CHAN14); /* EXTEND */
    continue_at(TASKOVER);
}

/* GIMTRIM +2, after GIMTRIM's FIXDELAY. */
void gimtrim_after_fixdelay(void)
{
    gimtrim_plus_2();
}

/* ---- S41.2 -- pp. 731-733 ---- */

/* S41.2, from verb 48's noun 46 (DONOUN46) or FRESHDAP: RATEINDX, twice
 * the rate (bits 1-2 of DAPDATR1). Under INHINT, bit 2 of FLAGWRD7 set
 * if the LM is attached (bits 14 and 13 of DAPDATR1 1 and 0: "ATTACHED,
 * SET FLAG FOR INTEGRATION"), else cleared. ADB, the deadband: DEC 46
 * "NARROW DB", or with bit 4 DEC 455 "WIDE DB". XTRANS, the quads for +X
 * translation: +1 for quads B and D (bit 7), -1 for A and C (bit 10), 0
 * for both or none. Under INHINT, NJETSFLG (bit 15 of FLAGWRD1) set "FOR
 * 2 JET ULLAGE (AC OR BD)", else cleared. ACORBD from bit 13 of
 * DAPDATR2, "MINUS FOR A-C, PLUS FOR B-D"; RACFAIL from its bits 10 and
 * 4 (quads A and C) and RBDFAIL from bits 7 and 1 (quads B and D): +1
 * with the first bit clear, -1 with only the second clear, else +0.
 * TC Q. */
void s41_2(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(THREE);
    double_A(); /* AD A */
    ts(ADDRESS_OF(RATEINDX));

    inhint();
    ca(ADDRESS_OF(DAPDATR1)); /* CAE: "IS LEM ATTACHED (BITS 14,13 OF DAPDATR1" */
    mask(PRIO30);
    ad(MBIT14);                   /* "(OCT57777)" */
    if (A_is_zero()) {            /* EXTEND; BZF TOGETHER: "YES" */
        cs(ADDRESS_OF(FLAGWRD7)); /* "ATTACHED, SET FLAG FOR INTEGRATION" */
        mask(BIT2);
        ads(ADDRESS_OF(FLAGWRD7));
    } else {
        set_A(negate(BIT2)); /* "NO, UNSET FLAG" */
        mask_at(ADDRESS_OF(FLAGWRD7));
        ts(ADDRESS_OF(FLAGWRD7));
    }
    relint();

    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT4);
    if (!A_is_zero_or_minus()) set_A(DEC409); /* EXTEND; BZMF +2: "DEC 46 MEANS NARROW DB" */
    ad(DEC46);                                /* "DEC 455 MEANS WIDE DB" */
    ts(ADDRESS_OF(ADB));

    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT7);                            /* "QUAD BD" */
    if (!A_is_zero_or_minus()) set_A(ONE); /* EXTEND; BZMF +2 */
    ts(ADDRESS_OF(XTRANS));
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT10);                                   /* "QUAD AC" */
    if (!A_is_zero_or_minus()) set_A(negate(ONE)); /* EXTEND; BZMF +2 */
    ads(ADDRESS_OF(XTRANS));

    inhint();
    if (A_is_zero()) {        /* EXTEND; BZF +5: "CLEAR NJETSFLG (4 JETS, OR NO JETS)" */
        set_A(negate(BIT15)); /* "KJETSFLG = 0 FOR 4 JET (OR 0 JET) ULLAGE" */
        mask_at(ADDRESS_OF(FLAGWRD1));
        ts(ADDRESS_OF(FLAGWRD1));
    } else {
        cs(ADDRESS_OF(FLAGWRD1)); /* "SET NJETSFLG (2 JETS, AC OR BD QUADS)" */
        mask(BIT15);              /* "NJETSFLG = 1 FOR 2 JET ULLAGE (AC OR BD)" */
        ads(ADDRESS_OF(FLAGWRD1));
    }
    relint();

    ca(ADDRESS_OF(DAPDATR2));
    mask(BIT13);
    if (A_is_zero_or_minus()) set_A(negate(ONE)); /* EXTEND; BZMF +2 */
    com();
    ts(ADDRESS_OF(ACORBD)); /* "MINUS FOR A-C, PLUS FOR B-D" */

    ca(ADDRESS_OF(DAPDATR2));
    mask(BIT10);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        set_A(ZERO);
        ts(ADDRESS_OF(RACFAIL));
        ca(ADDRESS_OF(DAPDATR2));
        mask(BIT4);
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: break; /* TCF BDFAIL */
        case IS_PLUS_ZERO:
            set_A(negate(ONE));
            ts(ADDRESS_OF(RACFAIL));
            break;
        default: not_modelled(); /* a masked word is neither */
        }
        break;
    case IS_PLUS_ZERO:
        set_A(ONE);
        ts(ADDRESS_OF(RACFAIL));
        break; /* TCF BDFAIL */
    default: not_modelled();
    }

    /* BDFAIL */
    ca(ADDRESS_OF(DAPDATR2));
    mask(BIT7);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE:
        set_A(ZERO);
        ts(ADDRESS_OF(RBDFAIL));
        ca(ADDRESS_OF(DAPDATR2));
        mask(BIT1);
        switch (ccs_register(A_ADDRESS)) {
        case POSITIVE: break;
        case IS_PLUS_ZERO:
            set_A(negate(ONE));
            ts(ADDRESS_OF(RBDFAIL));
            break;
        default: not_modelled();
        }
        break;
    case IS_PLUS_ZERO:
        set_A(ONE);
        ts(ADDRESS_OF(RBDFAIL));
        break;
    default: not_modelled();
    }
    tc_q();
}

/* ---- DAPFIG -- p. 734 ---- */

static void nodapup(void);
static void rcsdapup(void);

/* DAPFIG, a job from verb 46 (STABLISH, by POSTJUMP): channel 12 bit 9
 * off, "TURN OFF SIVB TAKEOVER"; then by the configuration (bits 13-14
 * of DAPDATR1, "RIGHT SHIFT 4 OCTAL DIGITS"): the CM alone, NODAPUP; the
 * CSM or CSM and LM, RCSDAPUP; else SATSTKON (POSTJUMP). */
void dapfig(void)
{
    set_A(negate(BIT9));                     /* "TURN OFF SIVB TAKEOVER" */
    wand_channel(CHAN12);                    /* EXTEND */
    ca(ADDRESS_OF(DAPDATR1));                /* CAE: "DETERMINE VEHICLE CONFIGURATION" */
    mp(BIT3);                                /* EXTEND: "RIGHT SHIFT 4 OCTAL DIGITS" */
    mask(THREE);                             /* "(IN CASE BIT 15 IS USED)" */
    switch (A) {                             /* INDEX A; TCF +1: "BRANCH BASED ON CONFIG...." */
    case 0: nodapup(); break;                /* "CM.......ACTIVATE NODAP" */
    case 1:                                  /* "CSM......ACTIVATE RCSDAP" */
    case 2: rcsdapup(); break;               /* "CSM/LEM..ACTIVATE RCSDAP" */
    default: tc_to(DAPFIG + 014u, POSTJUMP); /* CADR SATSTKON */
    }
}

/* RCSDAPUP: "CALL TO ACTIVATE RCSDAP, AND RETURN" (INHINT; IBNKCALL,
 * CADR RCSDAPON), */
static void rcsdapup(void)
{
    inhint();
    tc_to(RCSDAPUP + 1u, IBNKCALL); /* TCR; CADR RCSDAPON */
}

static void endfig(void);

/* then RELINT and ENDFIG. */
void rcsdapup_after_ibnkcall(void)
{
    relint();
    endfig();
}

/* NODAPUP: "T5 IDLE FOR NODAP" (T5LOC from T5IDLDAP), and "RESET
 * T5-USAGE FLAGS FOR NODAP" (DOWNFLAG, DAPBIT1), */
static void nodapup(void)
{
    dca(T5IDLDAP); /* EXTEND: "T5 IDLE FOR NODAP (DON'T WORRY ABOUT T)" */
    dxch(ADDRESS_OF(T5LOC));
    tc_to(NODAPUP + 3u, DOWNFLAG); /* "RESET T5-USAGE FLAGS FOR NODAP" */
}

/* (DOWNFLAG, DAPBIT2), */
void nodapup_after_dapbit1(void)
{
    tc_to(NODAPUP + 5u, DOWNFLAG);
}

/* "ZERO JET CHANNELS IN 14 MS" (INHINT; IBNKCALL, CADR ZEROJET), */
void nodapup_after_dapbit2(void)
{
    inhint();
    tc_to(NODAPUP + 010u, IBNKCALL); /* "ZERO JET CHANNELS IN 14 MS AND THEN" */
}

/* then RELINT, HOLDFLAG +1, "KILL KALCMANU JOB", and ENDFIG. */
void nodapup_after_zerojet(void)
{
    relint();
    set_A(BIT1); /* "KILL KALCMANU JOB" */
    ts(ADDRESS_OF(HOLDFLAG));
    endfig();
}

/* ENDFIG: "CAME IN VIA V46, GO OUT VIA GOPIN" (POSTJUMP, CADR GOPIN). */
static void endfig(void)
{
    tc_to(ENDFIG, POSTJUMP);
}

/* ---- S40.14 -- p. 735 ---- */

/* S40.14, from verb 48's noun 47 (DONOUN47) or FRESHDAP: J/M, J/M1 and
 * J/M2 from IXX and IAVG times CONTONE ("2PI/M"); KMJ, KMJ1 and KMJ2,
 * CONTTWO over IXX and IAVG. TC Q. */
void s40_14(void)
{
    ca(ADDRESS_OF(IXX)); /* CAE: "RCS ENTRY" */
    mp(CONTONE);         /* EXTEND */
    ts(ADDRESS_OF(J_M));
    ca(ADDRESS_OF(IAVG));
    mp(CONTONE); /* EXTEND */
    ts(ADDRESS_OF(J_M1));
    ts(ADDRESS_OF(J_M2));
    dca(CONTTWO);           /* EXTEND */
    dv_at(ADDRESS_OF(IXX)); /* EXTEND */
    ts(ADDRESS_OF(KMJ));
    dca(CONTTWO);            /* EXTEND */
    dv_at(ADDRESS_OF(IAVG)); /* EXTEND */
    ts(ADDRESS_OF(KMJ1));
    ts(ADDRESS_OF(KMJ2));
    tc_q();
}

/* ---- RCSUP and RCSDAPON -- p. 736 ---- */

/* RCSUP, a T5 interrupt's routine: BANKRUPT and QRUPT kept, RCSDAPON
 * (TCR), "ACTIVATE RCS DAP", */
void rcsup(void)
{
    lxch(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT)); /* EXTEND */
    tc_at(RCSUP + 3u);       /* TCR RCSDAPON */
    rcsdapon();
}

/* and the interrupt's end (TCF RESUME). */
void rcsup_after_rcsdapon(void)
{
    continue_at(RESUME);
}

/* RCSDAPON, under INHINT: TIME5 to 0.6 seconds, which "ALLOWS
 * TVCEXEC/ROLLDAP TO DIE", and on at +1. */
void rcsdapon(void)
{
    set_A(POINT6_SECT5); /* "0.6 SEC ALLOWS TVCEXEC/ROLLDAP TO DIE" */
    rcsdapon_plus_1();
}

/* RCSDAPON +1, "ENTRY FROM R00TOP00": A to TIME5 and, unless it
 * overflows, T5PHASE ("WILL CAUSE FRESHDAP (+1)"); bit 3 of RCSFLAGS set
 * "TO REINITIALIZE FDAI ERROR"; T5LOC from RCSADDR (RCSATT); bits 15 and
 * 14 of FLAGWRD6 0 and 1, "T5 TAKEOVER BY RCSDAP"; "RETURN TO CALLER"
 * (TC Q). Not modelled: an overflow at that last TS, which would skip
 * the TC Q (as in the faithful port). */
void rcsdapon_plus_1(void)
{
    if (!ts(ADDRESS_OF(TIME5))) ts(ADDRESS_OF(T5PHASE)); /* "WILL CAUSE FRESHDAP (+1)" */
    cs(ADDRESS_OF(RCSFLAGS));                            /* "SET BIT3 TO REINITIALIZE FDAI ERROR" */
    mask(BIT3);
    ads(ADDRESS_OF(RCSFLAGS));
    dca(RCSADDR); /* EXTEND; "(RCSATT)" */
    dxch(ADDRESS_OF(T5LOC));
    set_A(negate(OCT60000)); /* "SEE BITS 15,14 TO 01 TO INDICATE" */
    mask_at(ADDRESS_OF(FLAGWRD6));
    ad(BIT14);
    if (ts(ADDRESS_OF(FLAGWRD6))) not_modelled(); /* "KILLS TVCEXEC AND ROLLDAP STARTS" */
    tc_q();                                       /* "RETURN TO CALLER (TVCDAPOF OR RCSDAPUP)" */
}

#endif
