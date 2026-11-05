/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 814-815 (Luminary099 only):
 * "GUIDANCE LOOP DISPLAYS": each pass ends at DISPEXIT, which puts up the
 * phase's display (WHATDISP: P63's V06N63, P64's V06N64, flashing until
 * the crew permits redesignations, or V06N60 for P65-P67) and ends the
 * job. The faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099

/* DISPEXIT: "KILL GROUP 3: DISPLAYS WILL BE RESTORED BY NEXT GUIDANCE
 * CYCLE." (-PHASE3 and PHASE3 get NEG0's -0 and +0.) */
void dispexit(void)
{
    dca(ADDRESS_OF(NEG0));
    dxch(ADDRESS_OF(MPHASE3));
    dispexit_plus3();
}

/* DISPEXIT +3: "IF FLUNDISP IS SET, NO DISPLAY THIS PASS" (ENDLLJOB);
 * else the phase's WHATDISP word, by WCHPHOLD. */
void dispexit_plus3(void)
{
    cs(ADDRESS_OF(FLAGWRD8));
    mask(FLUNDBIT);
    if (A_is_zero())
        endlljob(); /* "TO PICK UP THE TAG" */
    else
        tcf_by_phase(WHATDISP, ADDRESS_OF(WCHPHOLD));
}

/* P63DISPS -2: "KILL GROUP 5". */
void p63disps_phaschng(void)
{
    tc_to(P63DISPS - 2u, PHASCHNG);
}

/* P63DISPS: V06N63. */
void p63disps(void)
{
    set_A(V06N63);
    dispcomn();
}

/* DISPCOMN: the display (REGODSPR, through BANKCALL), and ENDLLJOB. */
void dispcomn(void)
{
    tc_to(DISPCOMN, BANKCALL);
}

/* ENDLLJOB: the job ends. */
void endlljob(void)
{
    continue_at(ENDOFJOB);
}

/* P64DISPS: "HAS TREDES REACHED ZERO?" Then RED-OVER, which ends the
 * redesignations. Else, "IS REDFLAG SET?" (REDES-OK's static display);
 * "OTHERWISE USE FLASHING DISPLAY" (REFLASHR, through BANKCALL), whose
 * returns are the crew's answer. */
void p64disps(void)
{
    ca(ADDRESS_OF(TREDES));
    if (A_is_zero()) {
        red_over(); /* "YES: CLEAR REDESIGNATION FLAG" */
        return;
    }
    cs(ADDRESS_OF(FLAGWRD6));
    mask(REDFLBIT);
    if (A_is_zero()) {
        redes_ok(); /* "YES: DO STATIC DISPLAY" */
        return;
    }
    set_A(V06N64); /* "OTHERWISE USE FLASHING DISPLAY" */
    tc_to(P64DISPS + 010u, BANKCALL);
}

/* REFLASHR's returns: TERMINATE (GOTOPOOH), PROCEED (P64CEED: "PERMIT
 * REDESIGNATIONS"), RECYCLE (P64DISPS) and, last, ENDLLJOB. */
void p64disps_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* P64CEED: ELINCR1 and AZINCR1 cleared, and UPFLAG REDFLAG: "ENABLE
 * REDESIGNATION LOGIC". (A is +0: the TSs never skip.) */
void p64ceed(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(ELINCR1));
    ts(ADDRESS_OF(AZINCR1));
    tc_to(P64CEED + 3u, UPFLAG);
}

/* UPFLAG's return: the job ends. */
void p64ceed_after_upflag(void)
{
    continue_at(ENDOFJOB);
}

/* RED-OVER: DOWNFLAG REDFLAG, and on to REDES-OK. */
void red_over(void)
{
    tc_to(RED_OVER, DOWNFLAG);
}

/* REDES-OK: V06N64. */
void redes_ok(void)
{
    set_A(V06N64);
    dispcomn();
}

/* VERTDISP: V06N60. */
void vertdisp(void)
{
    set_A(V06N60);
    dispcomn();
}

#endif
