/* SPDX-License-Identifier: MIT */
/* TVCEXECUTIVE.agc (Comanche055 pp. 945-950, bank 16): the TVC DAP's
 * half-second task, TVCEXEC: the roll DAP's errors and task, the needles,
 * the variable gains and mass, and the trim corrections (the one-shot and
 * the repetitive ones). The faithful C is c64-port/src/cm_tvc_exec.c. */
#include "tvc.h"
#include "../agc/names.h"

#if COMANCHE055

/* DSALMOUT, channel 11, whose bit 13 is the engine-on bit. */
#define DSALMOUT_CHANNEL 011u

static void tvcexfin(void);
static void repchek(void);
static void oneshotok(void);

/* ---- TVCEXEC and ROLLPREP -- p. 946 ---- */

/* TVCEXEC, every half second from TVCINIT4: "CHECK FOR TERMINATION" (bits
 * 15 and 14 of FLAGWRD6 read 10 while the TVC DAP is on, so CS and MASK
 * leave a positive word; +0 or negative, "TERMINATE"). Otherwise WAITLIST
 * TVCEXEC again in half a second, "TO PERPETUATE TVCEXEC". */
void tvcexec(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TVCEXFIN */
        tvcexfin();
        return;
    }
    set_A(HALF_SEC);
    tc_to(TVCEXEC + 5u, WAITLIST); /* 2CADR TVCEXEC */
}

/* ROLLPREP, after the WAITLIST: "UPDATE ROLL LADDERS" (CDUX to OGANOW, the
 * last OGANOW to OGAPAST); AK the roll needle's "FLY-TO OGA ERROR" (OGAD
 * less OGANOW, "SC.AT B-1 REVS"), and OGAERR the roll DAP's
 * "PHASE-PLANE (FLY-FROM) OGAERROR" (AK times -BIT14). Then the roll DAP
 * task in three centiseconds (WAITLIST, 2CADR ROLLDAP). */
void rollprep(void)
{
    ca(ADDRESS_OF(CDUX));
    xch(ADDRESS_OF(OGANOW));
    xch(ADDRESS_OF(OGAPAST));
    ca(ADDRESS_OF(OGAD));
    msu_at(ADDRESS_OF(OGANOW));
    ts(ADDRESS_OF(AK));
    mp(MBIT14);
    ts(ADDRESS_OF(OGAERR));
    set_A(THREE); /* "SET UP ROLL DAP TASK (ALLOW SOME TIME)" */
    tc_to(ROLLPREP + 013u, WAITLIST);
}

/* NEEDLEUP, after the WAITLIST: "DO A NEEDLES UPDATE" (IBNKCALL, CADR
 * NEEDLER; "NEEDLES RESTARTS ITSELF"). */
void needleup(void)
{
    tc_to(NEEDLEUP, IBNKCALL);
}

/* ---- VARGAINS and EXECCOPY -- pp. 946-947 ---- */

/* VARGAINS +5, the bypass of the "MASS/GAIN UPDATES": TVCEXPHS 2, and on
 * to 1SHOTCHK. */
static void bypass_updates(void)
{
    set_A(TWO);
    ts(ADDRESS_OF(TVCEXPHS));
    oneshotchk();
}

/* EXECCOPY: "RESTART-PROTECT THE COPYCYCLE" (TVCEXPHS 1 more), then the
 * copycycle itself. */
static void execcopy(void)
{
    incr(ADDRESS_OF(TVCEXPHS));
    execcopy_copycycle();
}

/* VARGAINS, after NEEDLER: "CHECK ENGINE-ON BIT TO INHIBIT VARIABLE"
 * "GAINS AND MASS IF ENGINE OFF" (bit 13 of DSALMOUT; with it +0, "OFF, SO
 * BYPASS MASS/GAIN UPDATES"). With the engine on, VCNTR counts down the
 * half seconds "TEST FOR GAIN OF UPDATE TIME": positive, "NOT YET", and
 * VCNTRTMP its count less one and MASSTMP CSMMASS, to "PROTECT VCNTR AND"
 * "CSMMASS DURING AN IMPULSIVE BURN"; +0, GAINCHNG; -0, "NO, LOTHRUST
 * (S40.8 R40)", the bypass. Negative, "NOT USED", is a TCF +0, a TC to
 * itself the hardware's TC trap ends in a restart: not modelled (the
 * faithful port refuses it too). */
void vargains(void)
{
    set_A(BIT13);
    rand_channel(DSALMOUT_CHANNEL);
    if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) { /* (A is BIT13 or +0) */
        bypass_updates();
        return;
    }
    switch (ccs_at(ADDRESS_OF(VCNTR))) {
    case POSITIVE: /* +4 */
        ts(ADDRESS_OF(VCNTRTMP));
        ca(ADDRESS_OF(CSMMASS));
        ts(ADDRESS_OF(MASSTMP));
        execcopy();
        return;
    case IS_PLUS_ZERO: gainchng(); return;
    case NEGATIVE:
        not_modelled(); /* TCF +0 */
        return;
    case IS_MINUS_ZERO: bypass_updates(); return;
    }
}

/* GAINCHNG: "UPDATE IXX, IAVG, IAVG/TLX" (IBNKCALL, CADR FIXCW: the
 * "MASSPROP ENTRY  (ALREADY INITIALIZED)"). */
void gainchng(void)
{
    tc_to(GAINCHNG, IBNKCALL);
}

/* GAINCHNG +2, after FIXCW: "UPDATE 1/CONACC, VARK" (IBNKCALL, CADR
 * S40.15). */
void gainchng_after_fixcw(void)
{
    tc_to(GAINCHNG + 2u, IBNKCALL);
}

/* GAINCHNG +4, after S40.15: MASSTMP the "MASS FOR NEXT 10 SEC. OF BURN"
 * (CSMMASS less TENMDOT, "KG B+16"), and VCNTRTMP NINETEEN to "RESET THE
 * VARIABLE-GAIN UPDATE COUNTER" (an overflow at the TS MASSTMP skips the
 * CAF NINETEEN, leaving +1 or -1 in A). Then EXECCOPY. */
void gainchng_after_s40_15(void)
{
    cs(ADDRESS_OF(TENMDOT));
    ad_at(ADDRESS_OF(CSMMASS));
    if (!ts(ADDRESS_OF(MASSTMP))) set_A(NINETEEN);
    ts(ADDRESS_OF(VCNTRTMP));
    execcopy();
}

/* EXECCOPY +1, the copycycle: MASSTMP to CSMMASS, VCNTRTMP to VCNTR and
 * V97VCNTR ("FOR ENGFAIL (R41) MASS UPDATES AT SPSOFF"); "COPYCYCLE OVER"
 * (TVCEXPHS 1 more); on to 1SHOTCHK. */
void execcopy_copycycle(void)
{
    ca(ADDRESS_OF(MASSTMP));
    ts(ADDRESS_OF(CSMMASS));
    ca(ADDRESS_OF(VCNTRTMP));
    ts(ADDRESS_OF(VCNTR));
    ts(ADDRESS_OF(V97VCNTR));
    incr(ADDRESS_OF(TVCEXPHS));
    oneshotchk();
}

/* ---- 1SHOTCHK to CNTRCOPY -- pp. 947-949 ---- */

/* 1SHOTCHK: "CHECK TIME FOR ONE-SHOT OR REPCORR" by CNTR. Positive, "NOT
 * YET": CNTRTMP its count less one ("COUNT DOWN") and TVCEXPHS 6, "FOR
 * ENTRY AT CNTRCOPY". +0, "NOW"; -0, "NOW  (ONE-SHOT ONLY, NO REPCORR)":
 * 1SHOTOK. Negative, "ONE-SHOT OVER, ON TO REPCORR": REPCHEK. */
void oneshotchk(void)
{
    switch (ccs_at(ADDRESS_OF(CNTR))) {
    case POSITIVE: /* +4 */
        ts(ADDRESS_OF(CNTRTMP));
        set_A(SIX);
        ts(ADDRESS_OF(TVCEXPHS));
        cntrcopy();
        return;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: oneshotok(); return;
    case NEGATIVE: repchek(); return;
    }
}

/* REPCHEK: "CHECK FOR REPETITIVE UPDATES" by REPFRAC: +0 or negative, "NO,
 * OVER-AND-OUT"; positive, TEMPDAP +1 the "CORRECTION FRACTION", TVCEXPHS
 * 4 "FOR ENTRY AT CORSETUP", and CORSETUP. */
static void repchek(void)
{
    ca(ADDRESS_OF(REPFRAC));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TVCEXFIN */
        tvcexfin();
        return;
    }
    ts(ADDRESS_OF(TEMPDAP) + 1u);
    set_A(FOUR);
    ts(ADDRESS_OF(TVCEXPHS));
    corsetup();
}

/* 1SHOTOK: "CHECK ENGINE-ON BIT, NOT PERMITTING" "ONE-SHOT DURING
 * ENGINE-SHUTDOWN" (bit 13 of DSALMOUT +0: "NO, TERMINATE"); TVCEXPHS 1
 * more, and TEMPSET. */
static void oneshotok(void)
{
    set_A(BIT13);
    rand_channel(DSALMOUT_CHANNEL);
    if (ccs_register(A_ADDRESS) == IS_PLUS_ZERO) { /* (A is BIT13 or +0) */
        tvcexfin();
        return;
    }
    incr(ADDRESS_OF(TVCEXPHS));
    tempset();
}

/* TEMPSET: TEMPDAP +1 the one-shot's "CORRECTION FRACTION", FCORFRAC;
 * TVCEXPHS 1 more, and CORSETUP. */
void tempset(void)
{
    set_A(FCORFRAC);
    ts(ADDRESS_OF(TEMPDAP) + 1u);
    incr(ADDRESS_OF(TVCEXPHS));
    corsetup();
}

/* CG.CORR, one axis's "TMC LOOP": in acttmp the offset deloff plus
 * TEMPDAP times 4(delbar - deloff), the difference kept in TTMP1 (each a
 * double word; MP takes TTMP1's high word). */
static void cg_corr(unsigned deloff, unsigned delbar, unsigned acttmp)
{
    dca(deloff);
    dxch(acttmp);
    dcs(deloff);
    ddoubl();
    ddoubl();
    dxch(ADDRESS_OF(TTMP1));
    dca(delbar);
    ddoubl();
    ddoubl();
    das(ADDRESS_OF(TTMP1));
    dca(ADDRESS_OF(TTMP1));
    mp_at(ADDRESS_OF(TEMPDAP));
    das(acttmp);
}

/* CORSETUP: "CHECK FOR LEM-OFF/ON" (bit 13 of DAPDATR1, "SHOWS LEM-OFF"):
 * TEMPDAP the correction fraction, "LEM IS ON,  PICK UP   TEMPDAP+1" or
 * "LEM IS OFF, PICK UP 2(TEMPDAP+1)"; CNTRTMP -1, "ONE-SHOT DONE" (an
 * overflow at the TS TEMPDAP skips the CAF NEGONE, leaving +1 or -1 in
 * A). CG.CORR for pitch then yaw, and CORCOPY: "RESTART PROTECT THE
 * COPYCYCLE" (TVCEXPHS 1 more) and the copycycle. */
void corsetup(void)
{
    ca(ADDRESS_OF(DAPDATR1));
    mask(BIT13);
    if (!A_is_zero()) ca(ADDRESS_OF(TEMPDAP) + 1u); /* EXTEND; BZF +2 */
    ad_at(ADDRESS_OF(TEMPDAP) + 1u);
    if (!ts(ADDRESS_OF(TEMPDAP))) set_A(NEGONE);
    ts(ADDRESS_OF(CNTRTMP));
    cg_corr(ADDRESS_OF(PDELOFF), ADDRESS_OF(DELPBAR), ADDRESS_OF(PACTTMP)); /* "PITCH TMC LOOP" */
    cg_corr(ADDRESS_OF(YDELOFF), ADDRESS_OF(DELYBAR), ADDRESS_OF(YACTTMP)); /* "YAW TMC LOOP" */
    incr(ADDRESS_OF(TVCEXPHS));                                             /* CORCOPY */
    corcopy_copycycle();
}

/* CORCOPY +1, the copycycle: the "TRIM-ESTIMATES, AND" "TRIMS", PACTTMP to
 * PDELOFF and its high word to PACTOFF, YACTTMP to YDELOFF and YACTOFF;
 * TVCEXPHS 1 more, and CNTRCOPY. */
void corcopy_copycycle(void)
{
    dca(ADDRESS_OF(PACTTMP));
    ts(ADDRESS_OF(PACTOFF));
    dxch(ADDRESS_OF(PDELOFF));
    dca(ADDRESS_OF(YACTTMP));
    ts(ADDRESS_OF(YACTOFF));
    dxch(ADDRESS_OF(YDELOFF));
    incr(ADDRESS_OF(TVCEXPHS));
    cntrcopy();
}

/* CNTRCOPY: "UPDATE CNTR (RESTARTS OK, FOLLOWS CPYCY)" from CNTRTMP; then
 * TVCEXFIN. */
void cntrcopy(void)
{
    ca(ADDRESS_OF(CNTRTMP));
    ts(ADDRESS_OF(CNTR));
    tvcexfin();
}

/* TVCEXFIN: "RESET TVCEXPHS", and TASKOVER, "TVCEXECUTIVE FINISHED". */
static void tvcexfin(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(TVCEXPHS));
    continue_at(TASKOVER);
}

#endif
