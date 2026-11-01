/* SPDX-License-Identifier: MIT */
/* RADAR_LEADIN_ROUTINES.agc (Luminary099 pp. 490-491): "RADAR SAMPLING
 * LOOP." The radar tests (R04, R77) set RSAMPDT, the sampling period, and
 * start RADSAMP as a task. Each time it runs, it puts itself on the
 * WAITLIST again, starts DORSAMP as a job and works out RTSTDEX, which
 * measurement to take. DORSAMP reads the radar once (VARADAR and RADSTALL)
 * and, except in R77, keeps the sample in RSTACK; RTSTLOC steps through
 * RSTACK's double words. "+0 INSERTED MANUALLY TERMINATES TEST."
 *
 * Each call out of the file (WAITLIST, NOVAC, SWCALL, BANKCALL) leaves Z
 * at the routine and Q at the word after the TC, where the routine comes
 * back to; WAITLIST and NOVAC come back after the 2CADR. TASKOVER and
 * ENDOFJOB don't come back. */
#include "radar_leadin.h"
#include "../agc/names.h"

#if LUMINARY099

/* The value of a word as a signed number (-0 is 0): how INDEX moves an
 * address. */
static long signed_value(word value)
{
    return (value & 040000u) ? -(long)(~value & 037777u) : (long)value;
}

/* RADSAMP -- LM p. 490. CCS RSAMPDT: the sampling period ("TIMES NORMAL
 * ONCE-PER-SECOND SAMPLING"), less one, is the delay to the next sample (a
 * negative one works the same way). "+0 INSERTED MANUALLY TERMINATES TEST." -0 is
 * not modelled: the AGC would run the 2CADR after the TC WAITLIST as an
 * instruction. */
void radsamp(void)
{
    switch (ccs_at(ADDRESS_OF(RSAMPDT))) {
    case POSITIVE: /* TCF +2 */
    case NEGATIVE:
        tc_at(RADSAMP + 3u);
        continue_at(WAITLIST); /* 2CADR RADSAMP */
        return;
    case IS_PLUS_ZERO: continue_at(TASKOVER); return;
    case IS_MINUS_ZERO: not_modelled();
    }
}

/* RADSAMP +6 -- the sample itself, a job: DORSAMP at priority 25. */
void radsamp_after_waitlist(void)
{
    set_A(PRIO25);
    tc_at(RADSAMP + 7u);
    continue_at(NOVAC); /* 2CADR DORSAMP */
}

/* RADSAMP +012 -- "FOR CYCLIC SAMPLING, RTSTDEX = RTSTLOC/2 + RTSTBASE",
 * RTSTBASE "0 FOR RR, 2 FOR LR": the CAF BIT14 and MP halve RTSTLOC. An
 * overflow at the TS skips the TCF TASKOVER into DORSAMP. */
void radsamp_after_novac(void)
{
    set_A(BIT14);
    mp_at(ADDRESS_OF(RTSTLOC)); /* EXTEND; MP RTSTLOC */
    ad_at(ADDRESS_OF(RTSTBASE));
    if (ts(ADDRESS_OF(RTSTDEX))) {
        dorsamp();
        return;
    }
    continue_at(TASKOVER);
}

/* DORSAMP -- LM p. 490. "DO THE ACTUAL RADAR SAMPLE": VARADAR "SELECTS
 * VARIABLE RADAR CHANNEL" and comes back to DORSAMP +1. */
void dorsamp(void)
{
    tc_at(DORSAMP);
    varadar();
}

/* DORSAMP +1 -- RADSTALL waits for the reading. */
void dorsamp_after_varadar(void)
{
    tc_at(DORSAMP + 1u);
    continue_at(BANKCALL); /* CADR RADSTALL */
}

/* DORSAMP +3 -- RADSTALL's bad return: "ADVANCE FAIL COUNTER BUT ACCEPT BAD
 * DATA". */
void dorsamp_bad_return(void)
{
    incr(ADDRESS_OF(RFAILCNT));
    dorsamp2();
}

/* INDEX RTSTLOC; DXCH RSTACK: the sample into RSTACK's double word
 * RTSTLOC. Not modelled: an index that takes the DXCH out of erasable
 * memory or into the central registers below ZERO (the program keeps
 * RTSTLOC 0 to RTSTMAX, "STORAGE IS DP"). */
static void dxch_rstack_indexed(void)
{
    long at = (long)ADDRESS_OF(RSTACK) + signed_value(fetch(ADDRESS_OF(RTSTLOC)));

    if (at < ZERO_ADDRESS || at > 01776) not_modelled();
    dxch((unsigned)at);
}

/* DORSAMP2 -- LM pp. 490-491. "DON'T UPDATE RSTACK IF IN R77" (FLAGWRD5's
 * R77FLAG); otherwise the sample, in SAMPLSUM, goes to RSTACK. Then "CYCLE
 * RTSTLOC": two words on, or back to 0 at RTSTMAX. An overflow at the TS
 * skips the TCF ENDOFJOB into VARADAR. */
void dorsamp2(void)
{
    inhint();
    ca(ADDRESS_OF(FLAGWRD5));
    mask(R77FLBIT);
    switch (ccs_register(A_ADDRESS)) {
    case POSITIVE: break; /* TCF +4 */
    case IS_PLUS_ZERO:
        dxch(ADDRESS_OF(SAMPLSUM));
        dxch_rstack_indexed();
        break;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* (a masked bit is never negative) */
    }
    cs(ADDRESS_OF(RTSTLOC));
    ad_at(ADDRESS_OF(RTSTMAX));
    if (!A_is_zero()) { /* EXTEND; BZF +3 */
        ca(ADDRESS_OF(RTSTLOC));
        ad(TWO); /* "STORAGE IS DP" */
    }
    if (ts(ADDRESS_OF(RTSTLOC))) {
        varadar();
        return;
    }
    continue_at(ENDOFJOB); /* "CONTINUOUS SAMPLING AND 2N TRIES - GONE." */
}

/* INDEX RTSTDEX; CAF RDRLOCS: the CADR of the radar routine, RRRANGE (0),
 * RRRDOT, LRVELX, LRVELY, LRVELZ or LRALT (5). An index below the table
 * reaches erasable memory, where CAF is CA, as the faithful port runs it;
 * not modelled: Z (the CA would read its own address) and fixed words
 * outside the table. */
static void caf_rdrlocs_indexed(void)
{
    long at = (long)RDRLOCS + signed_value(fetch(ADDRESS_OF(RTSTDEX)));

    if (at >= RDRLOCS && at <= RDRLOCS + 5)
        set_A(fetch_fixed((word)at));
    else if (at >= 0 && at < 02000 && at != Z_ADDRESS)
        ca((unsigned)at);
    else
        not_modelled();
}

/* VARADAR -- LM p. 491. "VARIABLE RADAR DATA CALLER FOR ONE MEASUREMENT
 * ONLY": BUF2 = 1 "WILL BE SENT TO RADAR ROUTINE IN A BY SWCALL", which
 * goes to the routine RTSTDEX names, "NOT TOUCHING Q". */
void varadar(void)
{
    set_A(ONE);
    (void)ts(ADDRESS_OF(BUF2));
    caf_rdrlocs_indexed();
    continue_at(SWCALL);
}

#endif
