/* SPDX-License-Identifier: MIT */
/* T4RUPT, part 1: the display and dispatch core of T4RUPT_PROGRAM.agc,
 * Luminary099 (LM, listing pages 155-160) and Comanche055 (CM, pages
 * 133-138). The faithful C is c64-port/src/t4rupt_core.c.
 *
 * The two missions run the same code, two words apart from OPTTEST on
 * (the CM's T4JUMP table holds TCF OPTTEST where the LM's holds TC
 * RCSMONIT, and OPTTEST's two words come after it). Only T4JUMP's
 * targets differ.
 *
 * How DSPTAB drives the relays. The DSKY's digits and lights are latching
 * relays, set eleven bits at a time: a relay word is a 4-bit row number
 * (RELTAB's upper bits, "RELAYWORD CODE") and 11 relay bits. DSPTAB holds
 * one word per row; the display programs store a new one complemented
 * (negative), meaning "to be sent". DSPOUTSB scans DSPTAB downwards from
 * DSPCNT for a negative word, sends it, and stores it back positive
 * ("REPLACE POSITIVELY"); NOUT counts the words still to send. DSPTAB +11D
 * (the lights' word) goes out at every 120 ms phase.
 *
 * The phases. DSRUPTSW counts 7, 6, ... 0 (one T4RUPT each) and round
 * again; a negative DSRUPTSW (HANG20 subtracts 14,11,9 from it) is the 20
 * ms phase after a relay word, which turns the relays off again (QUIKOFF)
 * or sends another word straight away (QUIKDSP).
 *
 * The binding (src/binding/t4rupt_routines.c) runs these inside the
 * interrupt in bank 06, with DSRUPTSW a phase count (8 at most) or
 * negative, DSPCNT 0-012 or -0 and no DSPTAB word +0 or -0, as the
 * display programs keep them. */
#include "t4rupt.h"
#include "../agc/names.h"

static void cdrve(void);
static void dspout(word at);
static bool dspoutsb(word at);
static void dsplay(void);
static void hang20(void);
static void hang20_plus1(void);
static void settime4(void);
static void proceede(void);
static void nodspout(void);
static void quikdsp(void);
static void quikrupt(void);
static void nodspy(void);
static void quikoff(void);

/* T4RUPT -- LM p. 155, CM p. 133.
 * The T4RUPT lead-in comes here with the interrupted BBANK in A. Save it
 * (BANKRUPT) and Q (QRUPT). A negative DSRUPTSW is the 20 ms phase
 * (QUIKDSP). Else count DSRUPTSW down, "7(-1)0 AROUND AND AROUND", keeping
 * the count in RUPTREG1 for T4JUMP, and drive the relays (CDRVE). */
void t4rupt(void)
{
    ts(ADDRESS_OF(BANKRUPT));
    qxch(ADDRESS_OF(QRUPT)); /* EXTEND; QXCH QRUPT */
    switch (ccs_at(ADDRESS_OF(DSRUPTSW))) {
    case POSITIVE: break; /* TCF NORMT4 +1, with DSRUPTSW - 1 */
    case NEGATIVE: quikdsp(); return;
    case IS_PLUS_ZERO: /* TCF NORMT4 */
    case IS_MINUS_ZERO: set_A(SEVEN); break;
    }
    /* NORMT4 +1 -- LM p. 155, CM p. 133. */
    ts(ADDRESS_OF(RUPTREG1));
    ts(ADDRESS_OF(DSRUPTSW));
    cdrve();
}

/* CDRVE -- LM p. 156, CM p. 134.
 * The lights' word, DSPTAB +11D: if it is to be sent (negative), store it
 * back positive and send it with RELTAB11's row number, then HANG20.
 * Otherwise send a display word, if any is waiting (DSPOUT). */
static void cdrve(void)
{
    unsigned lights = ADDRESS_OF(DSPTAB) + 11u;

    switch (ccs_at(lights)) {
    case POSITIVE: dspout((word)(CDRVE + 1u)); return;
    case IS_PLUS_ZERO: dspout((word)(CDRVE + 2u)); return;
    case NEGATIVE: xch(lights); break;
    case IS_MINUS_ZERO: break;
    }
    mask(LOW11);
    ts(lights);
    ad(RELTAB11);
    write_channel(OUT0); /* EXTEND; WRITE OUT0 */
    tc_at((word)(CDRVE + 011u));
    hang20();
}

/* DSPOUT -- LM p. 157, CM p. 135.
 * "IS DSKY FLAG ON": with DSKYFLAG (FLAGWRD5 bit 15) off, clear the relays
 * (NODSPOUT). Else, if NOUT says words are waiting, send one (DSPOUTSB)
 * and wait 20 ms (HANG20); with none, clear the relays. */
static void dspout(word at)
{
    tc_at(at); /* TC DSPOUT */
    switch (ccs_at(ADDRESS_OF(FLAGWRD5))) {
    case POSITIVE:
        set_A(ZERO); /* "NO" */
        nodspout();
        return;
    case IS_PLUS_ZERO: nodspout(); return;
    case NEGATIVE: /* "YES" */
        switch (ccs_at(ADDRESS_OF(NOUT))) {
        case POSITIVE: break;
        case IS_PLUS_ZERO: nodspout(); return; /* "NO DISPLAY REQUESTS" */
        case NEGATIVE: hang20(); return;
        case IS_MINUS_ZERO: hang20_plus1(); return;
        }
        break;
    case IS_MINUS_ZERO: break;
    }
    if (dspoutsb((word)(DSPOUT + 4u)))
        hang20(); /* a relay word went out */
    else
        nodspout();
}

/* DSPOUTSB -- LM p. 157, CM p. 135.
 * "DSPOUT PROGRAM, PUTS OUT DISPLAYS". A is NOUT - 1, the count after this
 * word. Scan DSPTAB down from DSPCNT for a word to send (negative) and
 * send it (DSPLAY), returning by TCF Q+1: true. The scan wraps round once
 * (DSRUPTEM, -0 on the first pass, +0 on the second); finding nothing,
 * set NOUT +0 and return by TC Q: false. */
static bool dspoutsb(word at)
{
    bool count_down = true; /* in at DSPSCAN -2, or (wrapping) at -1 */

    tc_at(at); /* TC DSPOUTSB */
    ts(ADDRESS_OF(NOUT));
    set_A(negate(ZERO));      /* CS ZERO */
    ts(ADDRESS_OF(DSRUPTEM)); /* "SET TO -0 FOR 1ST PASS THRU DSPTAB" */
    xch(ADDRESS_OF(DSPCNT));
    for (;;) {
        if (count_down) ad(NEG0); /* DSPSCAN -2: "TO PREVENT +0" */
        ts(ADDRESS_OF(DSPCNT));   /* DSPSCAN -1 */
        count_down = true;
        /* DSPSCAN: INDEX DSPCNT; CCS DSPTAB. */
        switch (ccs_at(indexed(ADDRESS_OF(DSPTAB), fetch(ADDRESS_OF(DSPCNT))))) {
        case IS_PLUS_ZERO: continue;
        case NEGATIVE: dsplay(); return true;             /* "IF DSPTAB ENTRY -, DISPLAY" */
        case IS_MINUS_ZERO: not_modelled(); return false; /* runs TABLNTH (OCT 12) as TC 12 */
        case POSITIVE: break;                             /* "IF DSPTAB ENTRY +, SKIP" */
        }
        switch (ccs_at(ADDRESS_OF(DSPCNT))) {
        case POSITIVE: continue; /* "IF DSPCNT +, TRY AGAIN" */
        case IS_PLUS_ZERO: dsplay(); return true;
        case NEGATIVE: not_modelled(); return false; /* runs TABLNTH: no producer */
        case IS_MINUS_ZERO: break;
        }
        switch (ccs_at(ADDRESS_OF(DSRUPTEM))) {
        case POSITIVE: not_modelled(); return false; /* runs 120MRUPT: no producer */
        case IS_PLUS_ZERO: /* "2ND PASS THRU DSPTAB (DSPCNT = 0). +0 INTO NOUT." */
            ts(ADDRESS_OF(NOUT));
            tc_through(Q_ADDRESS);
            return false;
        case NEGATIVE: tc_through(Q_ADDRESS); return false;
        case IS_MINUS_ZERO: /* "1ST PASS THRU DSPTAB (DSPCNT=0). +0 INTO DSRUPTEM. PASS AGAIN" */
            ts(ADDRESS_OF(DSRUPTEM));
            set_A(TABLNTH);
            count_down = false; /* TCF DSPSCAN -1 */
            break;
        }
    }
}

/* DSPLAY -- LM p. 157, CM p. 135.
 * The word at DSPTAB + DSPCNT is negative, its relay bits complemented
 * plus one less: AD ONE and store it back "POSITIVELY", then send its 11
 * relay bits with the row number from RELTAB. Return past the TC's next
 * word: TCF Q+1. */
static void dsplay(void)
{
    /* INDEX DSPCNT: the row (-0 indexes as +0). */
    unsigned row = indexed(ADDRESS_OF(DSPTAB), fetch(ADDRESS_OF(DSPCNT))) - ADDRESS_OF(DSPTAB);
    word to;

    ad(ONE);
    ts(ADDRESS_OF(DSPTAB) + row); /* INDEX DSPCNT; TS DSPTAB */
    mask(LOW11);                  /* "REMOVE BITS 12 TO 15" */
    ts(ADDRESS_OF(DSRUPTEM));
    set_A(HI5);
    mask(RELTAB[row]); /* INDEX DSPCNT; MASK RELTAB */
    ad_at(ADDRESS_OF(DSRUPTEM));
    write_channel(OUT0); /* EXTEND; WRITE OUT0 */
    /* TCF Q+1: INTERPRETER.agc's INDEX Q; TC 1. */
    to = (word)indexed(1u, overflow_corrected(Q));
    tc_at((word)(Q_PLUS_1 + 1u));
    continue_at(to);
}

/* HANG20 -- LM p. 157, CM p. 135.
 * A relay word went out: make DSRUPTSW negative (minus 14,11,9) for the 20
 * ms phase, and set TIME4 for 20 ms. */
static void hang20(void)
{
    set_A(negate(BITS_14_11_9)); /* CS 14,11,9 */
    hang20_plus1();
}

/* HANG20 +1: DSPOUT's CCS NOUT comes here with NOUT -0 (A +0). */
static void hang20_plus1(void)
{
    ads(ADDRESS_OF(DSRUPTSW));
    set_A(TWENTY_MS_RUPT); /* CAF 20MRUPT */
    settime4();
}

/* SETTIME4 -- LM p. 157, CM p. 135.
 * TIME4 from A: 20MRUPT or 120MRUPT, counts short of overflow. */
static void settime4(void)
{
    ts(ADDRESS_OF(TIME4));
    proceede();
}

/* PROCEEDE -- LM p. 158, CM p. 136.
 * "MONITOR FOR PROCEED BUTTON": the PRO key is channel 32 bit 14,
 * IMODES33 bit 14 its state last time. "IF PREV OFF AND NOW ON -- UPDATE
 * IMODES33 AND PROCESS VIA PINBALL": start PROCKEY as a job (TC NOVAC,
 * 2CADR PROCKEY), which returns to T4JUMP. On any other change, update
 * IMODES33; with none, "BYPASS". */
static void proceede(void)
{
    ca(ADDRESS_OF(IMODES33));
    rxor_channel(CHAN32); /* EXTEND; RXOR CHAN32 */
    mask(BIT14);
    if (A_is_zero()) { /* EXTEND; BZF T4JUMP: "NO CHANGE" */
        t4jump();
        return;
    }
    lxch(ADDRESS_OF(IMODES33));
    rxor_lchan();                               /* EXTEND; RXOR LCHAN */
    if (!ts(ADDRESS_OF(IMODES33))) mask(BIT14); /* "UPDATE IMODES33" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: t4jump(); return;          /* "WAS ON -- NOW OFF" */
    case IS_PLUS_ZERO: set_A(CHRPRIO); break; /* "WAS OFF -- NOW ON" */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); return; /* runs the 2CADR: no producer */
    }
    tc_at((word)(PROCEEDE + 016u)); /* TC NOVAC; 2CADR PROCKEY */
    continue_at(NOVAC);
}

/* T4JUMP -- LM p. 159, CM p. 137.
 * "JUMP TO APPROPRIATE ONCE-PER SECOND (0.96 SEC ACTUALLY) ACTIVITY":
 * INDEX RUPTREG1; TCF +1, into a table of eight, the four checks twice.
 * The LM's: TC RCSMONIT (the RCS jet failure monitor, which returns to
 * TCF RRAUTCHK), TCF RRAUTCHK (the rendezvous radar), TCF IMUMON, TCF
 * DAPT4S (the autopilot's). The CM's: TCF OPTTEST and TCF OPTMON (the
 * optics), TCF IMUMON, TCF RESUME. Also PROCKEY's NOVAC returns here. */
void t4jump(void)
{
    word to = (word)indexed(T4JUMP + 2u, RUPTREG1);

    switch ((to - (T4JUMP + 2u)) % 4u) {
#if LUMINARY099
    case 0:
        tc_at(to); /* TC RCSMONIT */
        continue_at(RCSMONIT);
        break;
    case 1: continue_at(RRAUTCHK); break;
    case 2: continue_at(IMUMON); break;
    default: continue_at(DAPT4S); break;
#else
    case 0: opttest(); break;
    case 1: continue_at(OPTMON); break;
    case 2: continue_at(IMUMON); break;
    default: continue_at(RESUME); break;
#endif
    }
}

#if LUMINARY099
/* T4JUMP +3 and +7 -- LM p. 159: RCSMONIT's returns, TCF RRAUTCHK. */
void t4jump_plus3(void)
{
    continue_at(RRAUTCHK);
}

void t4jump_plus7(void)
{
    continue_at(RRAUTCHK);
}
#else
/* OPTTEST -- CM p. 137: TC IBNKCALL; CADR OPTDRIVE, the optics' drive. */
void opttest(void)
{
    tc_at(OPTTEST);
    continue_at(IBNKCALL);
}
#endif

/* NODSPOUT -- LM p. 160, CM p. 137.
 * Clear the relays (A is +0 or the empty word DSPOUTSB left) and wait
 * 120 ms: "SET FOR NEXT CDRIVE". */
static void nodspout(void)
{
    write_channel(OUT0);      /* EXTEND; WRITE OUT0 */
    set_A(HUNDRED20_MS_RUPT); /* CAF 120MRUPT */
    settime4();
}

/* QUIKDSP -- LM p. 160, CM p. 137.
 * The 20 ms phase. With DSRUPTSW bit 14 clear the last T4RUPT sent a word:
 * turn the relays off (QUIKOFF). Else send the next word now, if any
 * (DSPOUTSB), with bit 14 cleared from DSRUPTSW so the next 20 ms phase
 * turns it off; with none, clear the relays (NODSPY). */
static void quikdsp(void)
{
    set_A(BIT14);
    mask_at(ADDRESS_OF(DSRUPTSW));
    if (A_is_zero()) { /* "WROTE LAST TIME, NOW TURN OFF RELAYS." */
        quikoff();
        return;
    }
    switch (ccs_at(ADDRESS_OF(NOUT))) {
    case POSITIVE:
        if (!dspoutsb((word)(QUIKDSP + 5u))) { /* "BAD RETURN FROM DSPOUTSB" */
            nodspy();
            return;
        }
        break;                           /* "GOOD RETURN (WE DISPLAYED SOMETHING)" */
    case IS_PLUS_ZERO: nodspy(); return; /* "NOUT=0" */
    case NEGATIVE: break;
    case IS_MINUS_ZERO: quikrupt(); return;
    }
    set_A(negate(BIT14)); /* CS BIT14 */
    quikrupt();
}

/* QUIKRUPT -- LM p. 160, CM p. 137.
 * DSRUPTSW plus A, TIME4 for 20 ms, DSRUPTSW plus BIT9, and RESUME. */
static void quikrupt(void)
{
    ads(ADDRESS_OF(DSRUPTSW));
    set_A(TWENTY_MS_RUPT);
    ts(ADDRESS_OF(TIME4));
    set_A(BIT9);
    ads(ADDRESS_OF(DSRUPTSW));
    tc_at((word)(QUIKRUPT + 5u)); /* TC RESUME */
    continue_at(RESUME);
}

/* NODSPY and SYNCT4 -- LM p. 160, CM p. 137-138.
 * Clear the relays, then add 20 ms to TIME4 and BIT9 to DSRUPTSW until
 * DSRUPTSW is no longer negative, and RESUME. CCS on +0 runs OCT37737 as
 * CA 7737 before TC SYNCT4; the CAF after it overwrites A, so it is not
 * run here (README.md, Departures). */
static void nodspy(void)
{
    write_channel(OUT0); /* EXTEND; WRITE OUT0 */
    for (;;) {
        set_A(TWENTY_MS_RUPT); /* SYNCT4 */
        ads(ADDRESS_OF(TIME4));
        set_A(BIT9);
        ads(ADDRESS_OF(DSRUPTSW));
        switch (ccs_at(ADDRESS_OF(DSRUPTSW))) {
        case POSITIVE:
            tc_at((word)(SYNCT4 + 5u)); /* TC RESUME */
            continue_at(RESUME);
            return;
        case IS_MINUS_ZERO:
            tc_at((word)(SYNCT4 + 8u)); /* TC RESUME */
            continue_at(RESUME);
            return;
        case IS_PLUS_ZERO:                                /* OCT37737 */
        case NEGATIVE: tc_at((word)(SYNCT4 + 7u)); break; /* TC SYNCT4 */
        }
    }
}

/* QUIKOFF -- LM p. 160, CM p. 138.
 * Turn the relays off: "RESET DSRUPTSW TO SEND DISPLAY NEXT PASS". */
static void quikoff(void)
{
    write_channel(OUT0); /* EXTEND; WRITE OUT0 */
    set_A(BIT14);
    quikrupt();
}
