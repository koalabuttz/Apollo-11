/* SPDX-License-Identifier: MIT */
/* RCS_FAILURE_MONITOR.agc, Luminary099 only (listing pages 190-192): the
 * jet failure monitor. The faithful C is c64-port/src/lm_rcsmon.c.
 *
 * "THE CREW IS WARNED OF JET FAILURES BY LAMPS LIT BY THE GRUMMAN
 * FAILURE-DETECTION CIRCUITRY; THEY MAY RESPOND BY OPERATING SWITCHES
 * WHICH ISOLATE PAIRS OF JETS FROM THE PROPELLANT TANKS AND SET BITS IN
 * CHANNEL 32." Channel 32 is inverted: a 0 bit is a closed valve. Bit n
 * (1-8) of PVALVEST, CH 32's bit as last acted on, has a jet bit in
 * CH5MASK (5FAILTAB) and one in CH6MASK (6FAILTAB).
 *
 * The listing calls the routine restart-protected but for one case: a
 * restart after the masks change and before PVALVEST does, with the
 * switch back as it was by the next pass, leaves the masks wrong until
 * the crew work the switch again.
 *
 * The binding (src/binding/rcs_failure_monitor_routines.c) runs these in
 * bank 06, with channel 32 as the run latched it and the fixed words the
 * routine reads: BIT8 ... BIT1 and the two tables. */
#include "rcs_failure_monitor.h"
#include "../agc/names.h"

#if LUMINARY099

static void vopened(void);
static void one_accfix(void);

/* L as INDEX L adds it: the bit's number, 1 for bit 8 to 8 for bit 1. */
static word bit_number(void)
{
    return overflow_corrected(L);
}

/* RCSMON (RCSMONIT) -- LM p. 191.
 * "PICK UP + INVERT INVERTED CHANNEL 32" and "KEEP JET-FAIL BITS ONLY": C,
 * the valves closed now, in Q. "FORM PC + PC" (P, PVALVEST): the bits that
 * changed; "QUIT IF NO ACTION REQUIRED". Else find the highest one: MP
 * BIT7 moves bits 8-1 into bits 14-7 of L, XCH L brings them to A and
 * leaves zero in L, the bit count, which goes up with each DOUBLE until
 * one overflows ("BOUND TO GET OVERFLOW IN THIS LOOP"). "SAVE THE
 * RELEVANT BIT" in Q. Its PVALVEST bit 0: the valve was open and has
 * just closed; 1: it has just opened, VOPENED. */
void rcsmon(void)
{
    set_A(negate(ZERO));  /* CS ZERO */
    rxor_channel(CHAN32); /* EXTEND; RXOR CHAN32 */
    mask(LOW8);
    ts(Q_ADDRESS);
    cs(ADDRESS_OF(PVALVEST));
    mask_at(Q_ADDRESS);
    ts(L_ADDRESS);
    cs(Q_ADDRESS);
    mask_at(ADDRESS_OF(PVALVEST));
    ads(L_ADDRESS);    /* "RESULT NZ INDICATES ACTION REQUIRED" */
    if (A_is_zero()) { /* EXTEND; BZF RCSMONEX */
        continue_at(RCSMONEX);
        return;
    }
    mp(BIT7);       /* EXTEND; MP BIT7 */
    xch(L_ADDRESS); /* "ZERO TO L IN THE PROCESS" */
    do {
        incr(L_ADDRESS);
        double_A();
    } while (!A_has_overflow()); /* OVSK; TCF -3 */
    ca(indexed(ADDRESS_OF(BIT8) - 1u, bit_number())); /* INDEX L; CA BIT8 -1 */
    ts(Q_ADDRESS);
    mask_at(ADDRESS_OF(PVALVEST)); /* "LOOK AT PREVIOUS VALVE STATE BIT" */
    /* CCS A: one bit of PVALVEST, so +0 or positive, never negative. */
    if (ccs_at(A_ADDRESS) == POSITIVE) { /* TCF VOPENED */
        vopened();
        return;
    }
    /* +0: "THE VALVE HAS JUST BEEN CLOSED". */
    cs(ADDRESS_OF(CH5MASK));
    mask_at(indexed(FIVE_FAILTAB, bit_number())); /* INDEX L; MASK 5FAILTAB */
    ads(ADDRESS_OF(CH5MASK));                     /* "SET INHIBIT BIT FOR CHANNEL 5 JET" */
    cs(ADDRESS_OF(CH6MASK));
    mask_at(indexed(SIX_FAILTAB, bit_number())); /* INDEX L; MASK 6FAILTAB */
    ads(ADDRESS_OF(CH6MASK));                    /* "SET INHIBIT BIT FOR CHANNEL 6 JET" */
    ca(Q_ADDRESS);
    ads(ADDRESS_OF(PVALVEST)); /* "RECORD ACTION TAKEN" */
    one_accfix();              /* TCF 1/ACCFIX */
}

/* VOPENED -- LM p. 192.
 * "A VALVE HAS JUST BEEN OPENED": "REMOVE INHIBIT BIT" for its channel 5
 * and channel 6 jets, and clear its bit of PVALVEST ("RECORD ACTION
 * TAKEN"); on into 1/ACCFIX. */
static void vopened(void)
{
    cs(indexed(FIVE_FAILTAB, bit_number())); /* INDEX L; CS 5FAILTAB */
    mask_at(ADDRESS_OF(CH5MASK));
    ts(ADDRESS_OF(CH5MASK));
    cs(indexed(SIX_FAILTAB, bit_number())); /* INDEX L; CS 6FAILTAB */
    mask_at(ADDRESS_OF(CH6MASK));
    ts(ADDRESS_OF(CH6MASK));
    cs(Q_ADDRESS);
    mask_at(ADDRESS_OF(PVALVEST));
    ts(ADDRESS_OF(PVALVEST));
    one_accfix();
}

/* 1/ACCFIX -- LM p. 192.
 * "SET UP 1/ACCS SO THAT THE SWITCH CURVES FOR TJETLAW CAN BE MODIFIED IF
 * CH5MASK HAS BEEN ALTERED": TC NOVAC; 2CADR 1/ACCJOB, at PRIO27. */
static void one_accfix(void)
{
    set_A(PRIO27);                  /* CAF PRIO27 */
    tc_at((word)(ONE_ACCFIX + 1u)); /* TC NOVAC; 2CADR 1/ACCJOB */
    continue_at(NOVAC);
}

/* 1/ACCFIX +4, where NOVAC returns: TCF RCSMONEX ("EXIT"). */
void one_accfix_after_novac(void)
{
    continue_at(RCSMONEX);
}

#endif
