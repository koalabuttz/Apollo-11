/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 860-863 (Luminary099 only, bank 33): SERVICER, the
 * job READACCS sets up every two seconds. The velocity change's magnitude
 * (ABDELV) and the mass it implies, the PIPAs and CDUs compensated, the
 * state vector updated (AVERAGEG), the delta-V monitor (DVMON), which
 * decides whether the engine thrusts, and the exit to the guidance through
 * AVGEXIT. The faithful C is c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* Not modelled: a transfer the code computes (a TC Q, SERVOUT's DXCH Z)
 * into the interpreter's own code (06037-07707 but NEWOPS, SWCALL's
 * 04621-04630, the interpreter's parts of banks 0 and 1), where the
 * faithful port does not run on. */
static void not_into_the_interpreter(word address)
{
    unsigned at;

    if (address == NEWOPS || address >= 04000u) {
        if ((address >= 04621u && address <= 04630u) ||
            (address >= 06037u && address <= 07707u && address != NEWOPS))
            not_modelled();
        return;
    }
    at = fixed_physical(address);
    if (at <= 03734u - 02000u || (at >= 02000u + 0344u && at <= 02000u + 0600u)) not_modelled();
}

/* Not modelled: a computed transfer to an address that is not a fixed
 * one (02000-07777), or into the interpreter's own code. */
static void computed(reg16 address)
{
    if (address < 02000u || address > 07777u) not_modelled();
    not_into_the_interpreter((word)address);
}

/* TC Q: back to the TC in fixed memory that called. */
void return_to_q(void)
{
    word at = (word)Q;

    computed(Q);
    tc_at(Q_ADDRESS);
    continue_at(at);
}

/* TC A: to the address in A (QUIKFAZ5's return). */
void return_to_a(void)
{
    word at = (word)A;

    computed(A);
    tc_at(A_ADDRESS);
    continue_at(at);
}

/* TC to an erasable word that holds a return (SETPOS's TC LRADRET). */
void tc_through_computed(unsigned address)
{
    computed(fetch(address));
    tc_through(address);
}

/* ---- SERVICER -- pp. 860-861 ---- */

/* SERVICER: "RESTART REREADAC + SERVICER" (PHASCHNG, OCT 16035, OCT 20000,
 * 2CADR GETABVAL), ... */
void servicer(void)
{
    tc_to(SERVICER, PHASCHNG);
}

/* ... 1/PIPADT "IN CASE RESTART HAS CAUSED LASTBIAS TO BE SKIPPED.", and
 * the "PIPA COMPENSATION CALL" (1/PIPA, through BANKCALL), on to
 * GETABVAL. */
void servicer_after_phaschng(void)
{
    set_A(PRIO31);
    ts(ADDRESS_OF(1_PIPADT));
    tc_to(SERVICER + 7u, BANKCALL); /* CADR 1/PIPA */
}

/* GETABVAL: to the interpreter, for GETABVAL_CODE. */
void getabval(void)
{
    tc_at(GETABVAL);
    intpret();
}

/* The magnitude of the velocity change DELV in MPAC. */
static const word getabval_words[] = {
    /* GETABVAL +1 */ ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(DELV),
    ORDER(EXIT),
};

const struct interpretive_code GETABVAL_CODE = {033u, GETABVAL + 1u, COUNT(getabval_words),
                                                getabval_words};

/* After the EXIT: "ABDELV = CM/SEC*2(-14).", "ABDVCONV = M/CS * 2(-5).",
 * and MASS1 the mass, "IN CASE WE SKIP MASSMON"; on to MASSMON. */
void getabval_after_exit(void)
{
    ca(MPAC_AT(0u));
    ts(ADDRESS_OF(ABDELV));
    mp(KPIP); /* EXTEND; MP KPIP */
    dxch(ADDRESS_OF(ABDVCONV));
    dca(ADDRESS_OF(MASS));
    dxch(ADDRESS_OF(MASS1));
    massmon();
}

/* MASSMON: "ARE WE ON THE SURFACE?" (SURFFLAG): if so, "BYPASS MASS
 * MESS". Else the mass the burn used, from the rocket equation over one
 * cycle: the change ABDVCONV over the exhaust velocity, times the mass,
 * added to MASS1. The exhaust velocities are stored negative, so MASS1
 * goes down by the mass the cycle's burn used. The exhaust
 * velocity is the ascent engine's, APSVEX, with APSFLAG set; else the
 * descent engine's, DPSVEX. That is the word after APSVEX, and the code
 * reads it with the DCA's own word: with APSFLAG clear the CCS skips the
 * EXTEND and runs the DCA word as CA APSVEX +1. ("IF EXTEND IS EXECUTED,
 * APSVEX --> A, OTHERWISE DPSVEX --> A"; a MASK leaves no negative.) */
void massmon(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(SURFFBIT);
    if (A_is_zero()) { /* "YES:  BYPASS MASS MESS" */
        moonspot();
        return;
    }
    ca(ADDRESS_OF(FLGWRD10)); /* "NO:  WHICH VEX SHOULD BE USED?" */
    mask(APSFLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE)
        dca_constant(APSVEX);
    else
        set_A(DPSVEX);
    ts(Q_ADDRESS);
    dca(ADDRESS_OF(ABDVCONV));
    dv_at(Q_ADDRESS); /* OCT10002: "WHERE APPROPRIATE VEX RESIDES" */
    mp_at(ADDRESS_OF(MASS));
    das(ADDRESS_OF(MASS1));
    moonspot();
}

/* MOONSPOT: ABDELV, "TP MPAC = ABDELV AT 2(14) CM/SEC", "MULTIPLY BY
 * KPIP1 TO GET" "ABDELV AT 2(7) M/CS" (SHORTMP), ... */
void moonspot(void)
{
    set_A(KPIP1);
    tc_to(MOONSPOT + 1u, SHORTMP);
}

/* ... "UPDATE DVTOTAL FOR DISPLAY", and the CDUs of the PIPA reading into
 * CDUSPOT (TC TMPTOSPT); ... */
void moonspot_after_shortmp(void)
{
    dxch(MPAC_AT(0u));
    das(ADDRESS_OF(DVTOTAL));
    tc_to(MOONSPOT + 4u, TMPTOSPT);
}

/* ... their sines and cosines (QUICTRIG, through BANKCALL), ... */
void moonspot_after_tmptospt(void)
{
    tc_to(MOONSPOT + 5u, BANKCALL); /* CADR QUICTRIG */
}

/* ... the navigation base's axes from them, into XNBPIP (FLESHPOT, with
 * XNBPIPAD's ECADR XNBPIP in A), ... */
void moonspot_after_quictrig(void)
{
    set_A(XNBPIPAD);
    tc_to(MOONSPOT + 010u, BANKCALL); /* CADR FLESHPOT */
}

/* ... and the interpreter, for AVERAGEG_CODE. */
void moonspot_after_fleshpot(void)
{
    tc_at(MOONSPOT + 012u);
    intpret();
}

/* AVERAGEG: the state vector updated, by the landing's equations
 * (RVBOTH) with MUNFLAG set, else by the average-G equations (CALCRVG). */
static const word averageg_words[] = {
    /* AVERAGEG */ ORDERS(BON, CALL),
    SWITCH(BON, MUNFLAG),
    ADDRESS(RVBOTH),
    ADDRESS(CALCRVG),
    ORDER(EXIT),
};

const struct interpretive_code AVERAGEG_CODE = {033u, AVERAGEG, COUNT(averageg_words),
                                                averageg_words};

/* GOSERV: the restart point moved on (TC QUIKFAZ5), ... */
void goserv(void)
{
    tc_to(GOSERV, QUIKFAZ5);
}

/* ... COPYCYCL: the new state copied into the old (TC COPYCYC), ... */
void copycycl(void)
{
    tc_to(COPYCYCL, COPYCYC);
}

/* ... and PIPATMP cleared ("A IS ZERO ON RETURN FROM COPYCYC"). (A TS that
 * overflows, entered here with A so, skips the next TS: PIPATMPZ gets the
 * +1 or -1 the first leaves in A.) Then STEERSW cleared "PRIOR TO DVMON".
 * With IDLEFLAG set, AUXFLAG to 0 (NODVMON1); with AUXFLAG clear, AUXFLAG
 * to 1 (NODVMON2); else the delta-V monitor, DVMON. (The two CCSs test
 * MASK results: positive or +0.) */
void copycycl_after_copycyc(void)
{
    if (!ts(ADDRESS_OF(PIPATMPX))) ts(ADDRESS_OF(PIPATMPY));
    ts(ADDRESS_OF(PIPATMPZ));

    set_A(negate(STEERBIT));
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));

    set_A(IDLEFBIT); /* "IS THE IDLE FLAG SET?" */
    mask_at(ADDRESS_OF(FLAGWRD7));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        nodvmon1();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD6));
    mask(AUXFLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        nodvmon2();
        return;
    }
    dvmon();
}

/* ---- DVMON -- pp. 861-863 ---- */

/* DVMON: a velocity change no more than DVTHRUSH is low thrust
 * (LOTHRUST). Else STEERSW set, and DVCNTSET. */
void dvmon(void)
{
    cs(ADDRESS_OF(DVTHRUSH));
    ad_at(ADDRESS_OF(ABDELV));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF LOTHRUST */
        lothrust();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD2)); /* "SET STEERSW." */
    mask(STEERBIT);
    ads(ADDRESS_OF(FLAGWRD2));
    dvcntset();
}

/* DVCNTSET: DVCNTR 1: "ALLOW TWO PASSES MAXIMUM NOW THAT" "THRUST HAS BEEN
 * DETECTED." Then the jets (USEJETS) with APSFLAG set or with the gimbal
 * fail bit of channel 32 (bit 9) +0; else the trim gimbal, USEGTS. */
void dvcntset(void)
{
    set_A(ONE);
    ts(ADDRESS_OF(DVCNTR));
    ca(ADDRESS_OF(FLGWRD10)); /* "BRANCH IF APSFLAG IS SET." */
    mask(APSFLBIT);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        usejets();
        return;
    }
    set_A(BIT9); /* "CHECK GIMBAL FAIL BIT" */
    rand_channel(032u);
    if (A_is_zero()) {
        usejets();
        return;
    }
    usegts();
}

/* USEGTS: DAPBOOLS's USEQRJTS clear (the DAP trims with the gimbal), and
 * SERVOUT. */
void usegts(void)
{
    set_A(negate(USEQRJTS));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    servout();
}

/* NODVMON1: "SET AUXFLAG TO 0.", and USEJETS. */
void nodvmon1(void)
{
    set_A(negate(AUXFLBIT));
    mask_at(ADDRESS_OF(FLAGWRD6));
    ts(ADDRESS_OF(FLAGWRD6));
    usejets();
}

/* NODVMON2: "SET AUXFLAG TO 1.", and USEJETS. */
void nodvmon2(void)
{
    cs(ADDRESS_OF(FLAGWRD6));
    mask(AUXFLBIT);
    ads(ADDRESS_OF(FLAGWRD6));
    usejets();
}

/* LOTHRUST: the restart point moved on (TC QUIKFAZ5); ... */
void lothrust(void)
{
    tc_to(LOTHRUST, QUIKFAZ5);
}

/* ... with DVCNTR positive, one more pass (DECCNTR); with it +0 (the
 * passes spent), the COMFAIL job unless group 4 runs one already ("COMFAIL
 * JOB ACTIVE?": PHASE4 positive, "YES:  WON'T NEED ANOTHER."), else phase
 * 4.37, the "4.37SPOT FOR COMFAIL" (PHASCHNG, OCT 00374), and the job.
 *
 * The CCSs' other outcomes, as the AGC runs them: DVCNTR negative,
 * SERVOUT; DVCNTR -0, the PHASCHNG; PHASE4 -0, the job without the
 * PHASCHNG. Not modelled: PHASE4 negative, which would run the OCT 00374
 * as an instruction. */
void lothrust_after_quikfaz5(void)
{
    switch (ccs_at(ADDRESS_OF(DVCNTR))) {
    case POSITIVE: deccntr(); return;
    case IS_PLUS_ZERO: break;
    case NEGATIVE: servout(); return;
    case IS_MINUS_ZERO: tc_to(LOTHRUST + 5u, PHASCHNG); return;
    }
    switch (ccs_at(ADDRESS_OF(PHASE4))) {
    case POSITIVE: servout(); return;
    case IS_PLUS_ZERO: tc_to(LOTHRUST + 5u, PHASCHNG); return;
    case NEGATIVE: not_modelled(); return;
    case IS_MINUS_ZERO: lothrust_after_phaschng(); return;
    }
}

/* The COMFAIL job (NOVAC at priority 25, 2CADR COMFAIL), ... */
void lothrust_after_phaschng(void)
{
    set_A(PRIO25);
    tc_to(LOTHRUST + 010u, NOVAC);
}

/* ... and SERVOUT. */
void lothrust_after_novac(void)
{
    servout();
}

/* DECCNTR: DVCNTR less one (the CCS left it in A) kept in DVCNTR1 over
 * the restart point (TC QUIKFAZ5), ... (A TS that overflows, entered here
 * with A so, skips the TC.) */
void deccntr(void)
{
    if (ts(ADDRESS_OF(DVCNTR1))) {
        deccntr_after_quikfaz5();
        return;
    }
    tc_to(DECCNTR + 1u, QUIKFAZ5);
}

/* ... then in DVCNTR; and STOPRATE (through IBNKCALL, interrupts held):
 * "IF THRUST IS LOW, NO STEERING IS DONE" "AND THE DESIRED RATES ARE SET
 * TO ZERO." It returns to USEJETS. */
void deccntr_after_quikfaz5(void)
{
    ca(ADDRESS_OF(DVCNTR1));
    ts(ADDRESS_OF(DVCNTR));
    inhint();
    tc_to(DECCNTR + 5u, IBNKCALL); /* CADR STOPRATE */
}

/* USEJETS: DAPBOOLS's USEQRJTS set (the DAP trims with the jets), and
 * SERVOUT. */
void usejets(void)
{
    cs(ADDRESS_OF(DAPBOOLS));
    mask(USEQRJTS);
    ads(ADDRESS_OF(DAPBOOLS));
    servout();
}

/* SERVOUT: interrupts allowed, and 1/ACCS (through BANKCALL), the DAP's
 * accelerations for the new mass; ... */
void servout(void)
{
    relint();
    tc_to(SERVOUT + 1u, BANKCALL); /* CADR 1/ACCS */
}

/* ... the job's work area set up for the guidance: PUSHLOC the bottom of
 * its VAC area (PRIORITY's low nine bits), FIXLOC that too and the word
 * after it +0 (ZL; DXCH FIXLOC); the restart point moved on (TC
 * QUIKFAZ5); ... */
void servout_after_1_accs(void)
{
    ca(ADDRESS_OF(PRIORITY));
    mask(LOW9);
    ts(ADDRESS_OF(PUSHLOC));
    zl();
    dxch(ADDRESS_OF(FIXLOC));
    tc_to(SERVOUT + 010u, QUIKFAZ5);
}

/* ... and the guidance, through AVGEXIT's 2CADR (EXTEND; DCA AVGEXIT; DXCH
 * Z): "EXIT TO SELECTED ROUTINE WHETHER THERE IS THRUST OR NOT. THE STATE
 * OF STEERSW WILL CONVEY THIS INFORMATION." */
void servout_after_quikfaz5(void)
{
    dca(ADDRESS_OF(AVGEXIT));
    dtcb(SERVOUT + 013u);
    if (Z < 02000u) not_modelled(); /* (an erasable word) */
    not_into_the_interpreter(Z);
}

#endif
