/* SPDX-License-Identifier: MIT */
/* SYSTEM_TEST_STANDARD_LEAD_INS.agc (Luminary099 pages 370-372, Comanche055
 * pages 420-422): "SPECIAL PROGRAMS TO EASE THE PANGS OF ERASABLE MEMORY
 * PROGRAMS". The faithful C is c64-port/src/standard_lead.c,
 * standard_lead_interp.c, standard_lead_wake.c and standard_lead_routines.c.
 *
 * A program in erasable memory (a test keyed in from the ground) cannot
 * use BANKCALL, CALL or JOBWAKE as fixed code does: each keeps or finds its
 * return by the caller's FBANK, and erasable code has none. These three
 * lead-ins keep the return in EBUF2 and EBUF2 +1 instead, "TWO ERASABLES
 * ... IN UNSWITCHED WHICH ARE UNSHARED BY INTERRUPTS AND OTHER EMEMORY
 * PROGRAMS". The flight program itself never calls them.
 *
 * Both missions' files are the same code: the LM's in bank 4, the CM's
 * E/BKCALL and E/CALL in bank 13 and its E/JOBWAK in bank 4. The CM's file
 * also holds its three system-test entry words in bank 33. */
#include "standard_lead.h"
#include "../agc/names.h"
#include "../executive/executive.h"
#include "../interpreter/interpreter.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ======================================================================
 * E/BKCALL -- LM p. 370, CM p. 420.
 * "FOR DOING BANKCALLS FROM AND RETURNING TO ERASABLE." The call is
 *
 *	TC	BANKCALL
 *	CADR	E/BKCALL
 *	CADR	ROUTINE		# WHERE TO WANT TO GO IN FIXED.
 *	RETURN HERE FROM DISPLAY TERMINATE, BAD STALL OR TC Q.
 *	RETURN HERE FROM DISPLAY PROCEED OR GOOD RETURN FROM STALL.
 *	RETURN HERE FROM DISPLAY ENTER OR RECYCLE.
 *
 * so E/BKCALL starts as BANKCALL leaves it: BUF2 holds the return, the
 * word after BANKCALL's CADR (the ROUTINE CADR's), and BUF2 +1 the
 * caller's FBANK. "A + L ARE PRESERVED THROUGH BANKCALL AND E/BKCALL."
 * ====================================================================== */

void e_bkcall(void)
{
    dxch(ADDRESS_OF(BUF2));      /* "SAVE A,L AND GET DP RETURN." */
    dxch(ADDRESS_OF(EBUF2));     /* "SAVE DP RETURN." */
    incr(ADDRESS_OF(EBUF2));     /* "RETURN +1 BECAUSE DOUBLE CADR." */
    ca(BBANK_ADDRESS);           /* the caller's FBANK, BANKCALL left, */
    mask(LOW10);                 /* "GET CURRENT EBANK.  (SBANK SOMEDAY)" */
    ads(ADDRESS_OF(EBUF2) + 1u); /* "FORM BBCON.  (WAS FBANK)" */
    /* NDX EBUF2; CA 0 -1: the word before the one EBUF2 now names, the
     * ROUTINE CADR. "GET CADR OF ROUTINE." */
    ca(indexed(EBUF2(0), negate(1u)));
    /* "GO TO ROUTINE, SETTING Q TO SWRETURN AND RESTORING A + L." SWCALL
     * returns to the three words after the TC, as the routine returns. */
    tc_at(E_BKCALL + 010u);
    continue_at(SWCALL);
}

/* E/SWITCH -- LM p. 370, CM p. 420.
 * EBUF2, EBUF2 +1 now hold the return and the caller's BBCON: DTCB goes
 * there, with the caller's EBANK and FBANK. */
static void e_switch(void)
{
    dxch(ADDRESS_OF(EBUF2));
    dtcb(E_SWITCH + 1u);
}

/* E/BKCALL +11: "TX Q, V34, OR BAD STALL RETURN." (TC +4): to the word
 * after the CADR. */
void e_bkcall_bad_return(void)
{
    tc_at(E_BKCALL + 011u);
    e_switch();
}

/* E/BKCALL +12: "PROCEED OR GOOD STALL RETURN." (TC +2, into the second
 * INCR): one word further. */
void e_bkcall_proceed_return(void)
{
    tc_at(E_BKCALL + 012u);
    incr(ADDRESS_OF(EBUF2));
    e_switch();
}

/* E/BKCALL +13: "ENTER OR RECYCLE RETURN.": two words further. */
void e_bkcall_enter_return(void)
{
    incr(ADDRESS_OF(EBUF2));
    incr(ADDRESS_OF(EBUF2));
    e_switch();
}

/* ======================================================================
 * E/CALL -- LM p. 371, CM p. 421.
 * "FOR CALLING A FIXED MEMORY INTERPRETIVE SUBROUTINE FROM ERASABLE AND
 * RETURNING TO ERASABLE." The call is
 *
 *	RTB
 *		E/CALL
 *	CADR	ROUTINE		# THE INTERPRETIVE SUBROUTINE YOU WANT.
 *				# RETURNS HERE IN INTERPRETIVE.
 *
 * so E/CALL starts as RTB leaves it, with LOC at RTB's operand: the word
 * after LOC is the CADR, and the interpretive code goes on after that.
 * ====================================================================== */

void e_call(void)
{
    lxch(ADDRESS_OF(LOC));     /* "ADRES -1 OF CADR." */
    ca(indexed(L_ADDRESS, L)); /* INDEX L; CA L: "CADR IN A." */
    incr(L_ADDRESS);
    incr(L_ADDRESS);         /* "RETURN ADRES IN L." */
    dxch(ADDRESS_OF(EBUF2)); /* "STORE CADR AND RETURN" */
    tc_at(E_CALL + 6u);
    continue_at(INTPRET);
}

/* E/CALL +7 and +11, interpretive. CALL EBUF2: an erasable address, so
 * CALL goes to the CADR it holds, leaving its return (the EXIT) in QPRET.
 * "INDIRECTLY EXECUTE ROUTINE.  IT MUST LEAVE VIA RVQ OR EQUIVALENT."
 * Then EXIT, back into basic code. */
static const word standard_lead_words[] = {
    ORDER(CALL),
    ADDRESS(EBUF2), /* "INDIRECTLY EXECUTE ROUTINE.  IT MUST" */
    ORDER(EXIT),    /* "LEAVE VIA RVQ OR EQUIVALENT." */
};
#if LUMINARY099
const struct interpretive_code STANDARD_LEAD_CODE = {04u, E_CALL + 7u, COUNT(standard_lead_words),
                                                     standard_lead_words};
#else
const struct interpretive_code STANDARD_LEAD_CODE = {013u, E_CALL + 7u, COUNT(standard_lead_words),
                                                     standard_lead_words};
#endif

/* E/CALL +12, after the EXIT: the caller's interpretive code goes on after
 * its CADR. TCF INTPRET +2 lands on INTPRET's QXCH LOC with no EXTEND
 * before it, so it runs as LXCH LOC: LOC gets L. */
void e_call_after_exit(void)
{
    lxch(ADDRESS_OF(EBUF2) + 1u); /* "PICK UP RETURN." */
    continue_at(INTPRET + 2u);    /* "SET LOC AND RETURN TO CALLER." */
}

/* ======================================================================
 * E/JOBWAK -- LM p. 372, CM p. 422.
 * "FOR WAKING UP ERASABLE MEMORY JOBS. THIS ROUTINE MUST BE CALLED IN
 * INTERRUPT OR WITH INTERRUPTS INHIBITED." The call is
 *
 *	INHINT
 *	...
 *	CA	WAKEADR		# ADDRESS OF SLEEPING JOB
 *	TC	IBNKCALL
 *	CADR	E/JOBWAK
 *	...			# RETURNS HERE
 *	RELINT			# IF YOU DID AND INHINT.
 *
 * A job asleep in erasable memory went to sleep (JOBSLEEP) with its
 * erasable address as its LOC. JOBWAKE finds it by that LOC and gives it
 * back a LOC as for fixed code, 2000 plus the address within its bank
 * (WAKETEST): for an erasable address, the fixed memory bit (BIT11) set,
 * which E/JOBWAK takes back out.
 * ====================================================================== */

void e_jobwak(void)
{
    tc_at(E_JOBWAK);
    jobwake(); /* "ARRIVE IWTH ADRES IN A." */
    /* JOBWAKE leaves LOCCTR at the job's core set. With no such job it is
     * -1, and the ADS changes the word before LOC instead, as on the AGC.
     * does. */
    set_A(negate(BIT11));                  /* CS BIT11 */
    ads(indexed(ADDRESS_OF(LOC), LOCCTR)); /* "KNOCK FIXED MEMORY BIT OUT OF ADRES." */
    tc_through(ADDRESS_OF(RUPTREG3));      /* "RETURN": IBNKCALL's */
}

#if COMANCHE055

/* ======================================================================
 * COMPVER, GTSCPSS1, REDO -- CM p. 422.
 * The system-test entry words, at fixed addresses in bank 33 that the
 * ground knows ("MUST BE 33,2000."): each is one TC. REDO's leaves Q at
 * the MM 07 after it, which NEWMODEX displays, and the test goes on into
 * IMUTEST ("FALL INTO IMUTEST").
 * ====================================================================== */

void compver(void)
{
    tc_at(COMPVER);
    continue_at(GCOMPVER);
}

void gtscpss1(void)
{
    tc_at(GTSCPSS1);
    continue_at(GTSCPSS);
}

void redo(void)
{
    tc_at(REDO);
    continue_at(NEWMODEX); /* "DISPLAY MM 07." */
}

#endif
