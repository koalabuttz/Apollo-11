/* SPDX-License-Identifier: MIT */
/* ASCENT_GUIDANCE.agc, pp. 852-854 (Luminary099 only, banks 14 and 30):
 * the burn's end. ENGOFF times the engine's shutdown to the time to go;
 * ENGOFF1 shuts it off and starts CUTOFF, the post-burn job: FLRCS set,
 * verb 16 noun 63 and then noun 85 flashing (CUTOFF2), and TERMASC, back
 * to P00. The faithful C is c64-port/src/lm_ascent.c. */
#include "ascent.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

#if LUMINARY099

/* ---- ENGOFF -- pp. 852-853 (bank 14) ---- */

/* ENGOFF (from MAINENG, under 4 seconds to go): DT, the time from now to
 * the shutdown at PIPTIME + TGO. The time now (LOADTIME), less PIPTIME,
 * plus TTOGO (which is -TGO), negated; EXIT. */
static const word engoff_words[] = {
    /* ENGOFF */ ORDER(RTB),   /* the time now, */
    ADDRESS(LOADTIME),         /* (RTB's routine) */
    ORDERS(DSU, DAD),          /* less PIPTIME, plus TTOGO, */
    ADDRESS_PLUS_ONE(PIPTIME), /* (DSU's operand) */
    ADDRESS_PLUS_ONE(TTOGO),   /* (DAD's) */
    ORDERS(DCOMP, EXIT),       /* negated: DT */
};

const struct interpretive_code ENGOFF_CODE = {014u, ENGOFF, COUNT(engoff_words), engoff_words};

/* After the EXIT: "FORCE SIGN AGREEMENT ON MPAC, MPAC +1." (TPAGREE). */
void engoff_after_exit(void)
{
    tc_to(ENGOFF + 6u, TPAGREE);
}

/* E bank 7, interrupts held, and ENGOFFDT, the delay: DT, or 1 (one
 * centisecond, BIT1) when DT is not positive. CCS leaves DT - 1 for a
 * positive DT, +0 for +0 or -0, and CAF ZERO makes it +0 for a negative
 * one; AD BIT1 adds the 1 back. (Neither TS overflows, so neither skips.)
 * TWIDDLE starts ENGOFF1 then (ADRES ENGOFF1). */
void engoff_after_tpagree(void)
{
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    inhint();
    switch (ccs_at(MPAC_AT(1u))) {
    case POSITIVE: break;     /* "C(A) = DT - 1 BIT" */
    case IS_PLUS_ZERO: break; /* "C(A) = 0" */
    case NEGATIVE: set_A(ZERO); break;
    case IS_MINUS_ZERO: break;
    }
    ad(BIT1); /* "C(A) = 1 BIT OR DT." */
    ts(ADDRESS_OF(ENGOFFDT));
    tc_to(ENGOFF + 020u, TWIDDLE); /* ADRES ENGOFF1 */
}

/* Restart group 4 at ENGOFF1, ENGOFFDT from now (PHASCHNG, OCT 47014,
 * -GENADR ENGOFFDT, 2CADR ENGOFF1), ... */
void engoff_after_twiddle(void)
{
    tc_to(ENGOFF + 022u, PHASCHNG);
}

/* (PHASCHNG returns past the three words after the OCT. A program that
 * resumed at the first of them would run them as instructions, as the AGC would: the
 * -GENADR ENGOFFDT as a MASK of the word it names in fixed-fixed memory,
 * and the 2CADR's first word as TC ENGOFF1.) */
void engoff_restart_words(void)
{
    mask_at(ADDRESS_FIELD(fetch_fixed(ENGOFF + 024u)));
    tc_at(ENGOFF + 025u);
    engoff1();
}

/* ... and the interpretive code (ENGOFF_SET_CODE): ... */
void engoff_intpret(void)
{
    tc_at(ENGOFF + 027u);
    intpret();
}

/* ... "DISABLE DELTA-V MONITOR" (IDLEFLAG set), and T2TEST, the steering
 * on to the end. */
static const word engoff_set_words[] = {
    /* ENGOFF +030 */ ORDERS(SET, GOTO),
    SWITCH(SET, IDLEFLAG), /* "DISABLE DELTA-V MONITOR" */
    ADDRESS(T2TEST),
};

const struct interpretive_code ENGOFF_SET_CODE = {014u, ENGOFF + 030u, COUNT(engoff_set_words),
                                                  engoff_set_words};

/* ENGOFF1, the task: "SHUT OFF THE ENGINE." (ENGINOF2, through IBNKCALL),
 * ... */
void engoff1(void)
{
    tc_to(ENGOFF1, IBNKCALL); /* CADR ENGINOF2 */
}

/* ... "SET UP A JOB FOR THE ASCENT GUIDANCE POSTBURN LOGIC." (CUTOFF, at
 * priority 17, with a VAC area: FINDVAC), ... */
void engoff1_after_enginof2(void)
{
    set_A(PRIO17);
    tc_to(ENGOFF1 + 3u, FINDVAC); /* 2CADR CUTOFF */
}

/* ... restart group 4 at that job (PHASCHNG, OCT 07024, OCT 17000, 2CADR
 * CUTOFF), ... */
void engoff1_after_findvac(void)
{
    tc_to(ENGOFF1 + 6u, PHASCHNG);
}

/* ... and the task ends (TASKOVER). */
void engoff1_after_phaschng(void)
{
    continue_at(TASKOVER);
}

/* ---- CUTOFF -- p. 853 (bank 14) ---- */

/* CUTOFF, the job: "SET FLRCS FLAG." (UPFLAG), ... */
void cutoff(void)
{
    tc_to(CUTOFF, UPFLAG); /* ADRES FLRCS */
}

/* ... and verb 16 noun 63 flashing (V16N63, GOFLASH through BANKCALL). */
void cutoff_after_upflag(void)
{
    set_A(V16N63);
    tc_to(CUTOFF + 3u, BANKCALL); /* CADR GOFLASH */
}

/* GOFLASH's returns: terminate, to TERMASC (POSTJUMP); ... */
void cutoff_terminate(void)
{
    tc_to(CUTOFF + 010u, POSTJUMP); /* CADR TERMASC */
}

/* ... proceed, CUTOFF1: the attitude errors zeroed before the deadband
 * is reduced (ZATTEROR, through IBNKCALL, interrupts held), ... */
void cutoff_proceed(void)
{
    inhint();
    tc_to(CUTOFF1 + 1u, IBNKCALL); /* CADR ZATTEROR */
}

/* ... or enter: the display again. */
void cutoff_enter(void)
{
    cutoff_after_upflag();
}

/* CUTOFF1 +3: the minimum deadband (SETMINDB), ... */
void cutoff1_after_zatteror(void)
{
    tc_to(CUTOFF1 + 3u, IBNKCALL); /* CADR SETMINDB */
}

/* ... and CUTOFF2, in bank 30 (POSTJUMP). */
void cutoff1_after_setmindb(void)
{
    tc_to(CUTOFF1 + 5u, POSTJUMP); /* CADR CUTOFF2 */
}

/* ---- CUTOFF2 and TERMASC -- p. 854 (bank 30) ---- */

/* CUTOFF2: restart group 4 at 4.2 (PHASCHNG, OCT 04024), ... */
void cutoff2(void)
{
    tc_to(CUTOFF2, PHASCHNG);
}

/* ... and verb 16 noun 85 flashing (V16N85C: the velocity to be gained in
 * the LM's axes, VGBODY). */
void cutoff2_after_phaschng(void)
{
    set_A(V16N85C);
    tc_to(CUTOFF2 + 3u, BANKCALL); /* CADR GOFLASH */
}

/* GOFLASH's returns: terminate, TERMASC; ... */
void cutoff2_terminate(void)
{
    termasc();
}

/* ... "PROCEED", TERMASC; ... */
void cutoff2_proceed(void)
{
    termasc();
}

/* ... or enter: CUTOFF2 again. */
void cutoff2_enter(void)
{
    cutoff2();
}

/* TERMASC: restart group 4 at 4.2, ... */
void termasc(void)
{
    tc_to(TERMASC, PHASCHNG); /* OCT 04024 */
}

/* ... "RESTORE DEADBAND DESIRED BY ASTRONAUT." (RESTORDB, through
 * IBNKCALL, interrupts held), ... */
void termasc_after_phaschng(void)
{
    inhint();
    tc_to(TERMASC + 3u, IBNKCALL); /* CADR RESTORDB */
}

/* ... "DISALLOW ABORTS AT THIS TIME." (LETABORT clear), ... */
void termasc_after_restordb(void)
{
    tc_to(TERMASC + 5u, DOWNFLAG); /* ADRES LETABORT */
}

/* ... and P00 (GOTOPOOH). */
void termasc_after_downflag(void)
{
    continue_at(GOTOPOOH);
}

#endif
