/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 2: CHKPRIO and the display entries
 * without an immediate return (LM pages 1356-1358, CM pages 1467-1469),
 * bank 10.
 *
 * Each entry stores the verb and noun in PLAYTEM1 and puts the display's
 * flags (DSPFLG's bits) in A for GOFLASH2, which runs MAKEPLAY as part of
 * the user's job: "ALL ROUTINES NOT ENDING IN R BRANCH DIRECTLY TO
 * MAKEPLAY WHICH MAKES THESE DISPLAYS A PART OF THE USER'S JOB." */
#include "display_interface.h"

/* CHKPRIO -- LM p. 1356, CM p. 1467: "MARK ATTEMPT DURING PRIO": if a PRIO
 * or a mark is going, MARSLEEP. Otherwise "SET MARK OVER NORM" (FLAGWRD4
 * bit 3), and SETNORM. */
void chkprio(void)
{
    ca(ADDRESS_OF(FLAGWRD4));
    mask(OCT24100);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(MARSLEEP);
        return;
    }
    cs(ADDRESS_OF(FLAGWRD4));
#if LUMINARY099
    mask(MKOVBIT);
#else
    mask(BIT3);
#endif
    inhint();
    ads(ADDRESS_OF(FLAGWRD4));
    continue_at(SETNORM);
}

/* MARKPERF -- LM p. 1357, CM p. 1468: the mark's verb (MARKNV), for the
 * second part of a mark perform display. */
void markperf(void)
{
    ca(ADDRESS_OF(MARKNV));
    mask(VERBMASK);
    continue_at(NV50DSP);
}

/* GODSP2 -- LM p. 1357, CM p. 1468: no flags. */
void godsp2(void)
{
    set_A(ZERO);
    goflash2();
}

/* GODSP -- LM p. 1357, CM p. 1468. "GODSP IS USED TO DISPLAY A VERB NOUN
 * ARRIVING IN A.  NO RETURN IS MADE TO THE USER." */
void godsp(void)
{
    if (ts(ADDRESS_OF(PLAYTEM1))) { /* an overflow skips GODSP2's CAF */
        goflash2();
        return;
    }
    godsp2();
}

/* GODSPRET -- LM p. 1357, CM p. 1468: "ENTRANCE FOR A GODSP WITH A PASTE",
 * "SET BIT6 TO GO BACK TO USER AFTER NVSUB". */
void godspret(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT6);
    goflash2();
}

/* GODSPR2 -- LM p. 1357, CM p. 1468: the flags to PLAYTEM4, and GODSPRS1
 * with the immediate return at the call +0 (A +0). */
void godspr2(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM4), ZERO);
    godsprs1();
}

/* GODSPR1 -- LM p. 1357, CM p. 1468: no flags. */
void godspr1(void)
{
    set_A(ZERO);
    godspr2();
}

/* GODSPR -- LM p. 1357, CM p. 1468. "GODSPR IS THE SAME AS GODSP ONLY
 * RETURN IS TO THE USER." */
void godspr(void)
{
    if (ts(ADDRESS_OF(PLAYTEM1))) { /* an overflow skips GODSPR1's CAF */
        godspr2();
        return;
    }
    godspr1();
}

/* REFLASH -- LM p. 1357, CM p. 1468: "FLASH AND PERMIT". "REFLASH IS THE
 * SAME AS GOFLASH ONLY REFLASH REPLACES ANY ACTIVE NORMAL DISPLAY IF ONE
 * WAS ACTIVE." */
void reflash(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), REDOMASK);
    goflash2();
}

/* CLEANDSP -- LM p. 1357, CM p. 1468: a REFLASH of verb and noun +0.
 * "CLEANDSP CLEANS OUT ALL NORMAL DISPLAYS (ACTIVE AND INACTIVE)." */
void cleandsp(void)
{
    set_A(ZERO);
    reflash();
}

#if LUMINARY099
/* REFLASHR -- LM p. 1357: "REFLASHR IS THE SAME AS REFLASH ONLY AN
 * IMMEDIATE RETURN IS MADE TO THE USER'S CALL CADR +4." */
void reflashr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), REDOMASK);
    godsprs();
}
#endif

/* REGODSP -- LM p. 1358, CM p. 1468: bit 14, "REFLASH OR REDO". */
void regodsp(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT14);
    goflash2();
}

/* REGODSPR -- LM p. 1358, CM p. 1469. */
void regodspr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT14);
    godspr2();
}

/* CLOCPLAY -- LM p. 1358, CM p. 1469: CLOCKCON, "FLASH, PERFORM, V99 OR
 * V97 PASTE, REFLASH". */
void clocplay(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), CLOCKCON);
    goflash2();
}

/* GOFLASH -- LM p. 1358, CM p. 1469: "LEAVE ONLY FLASH BIT SET". */
void goflash(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT4);
    goflash2();
}

/* GOFLASH2 -- LM p. 1358, CM p. 1469: the flags to PLAYTEM4, SAVELOCS (on
 * to MAKECADR, which comes back to GOFLASH2 +2: RELINT, and "BRANCH
 * DIRECT WITH NO SEPARATE JOB CALL" to MAKEPLAY). An overflow in A skips
 * the TC SAVELOCS. */
void goflash2(void)
{
    if (ts(ADDRESS_OF(PLAYTEM4))) {
        relint();
        continue_at(MAKEPLAY);
        return;
    }
    tc_at((word)(GOFLASH2 + 1u)); /* TC SAVELOCS */
    savelocs();
}

/* PRIODSPR -- LM p. 1358, CM p. 1469: the PRIO bits (BITS7+4) and
 * GODSPRS. "PRIODSPR IS THE SAME AS PRIODSP ONLY AN IMMEDIATE RETURN IS
 * MADE TO THE USER'S CALL CADR +4." */
void priodspr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BITS7_PLUS_4);
    godsprs();
}

/* SETPRIO -- LM p. 1358, CM p. 1469. */
void setprio(void)
{
    set_A(BITS7_PLUS_4);
    goflash2();
}

/* PRIODSP -- LM p. 1358, CM p. 1469. "PRIODSP IS USED AS A PRIORITY
 * DISPLAY." */
void priodsp(void)
{
    if (ts(ADDRESS_OF(PLAYTEM1))) { /* an overflow skips SETPRIO's CAF */
        goflash2();
        return;
    }
    setprio();
}
