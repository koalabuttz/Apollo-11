/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 5: monitor and the alarms --
 * LM pp. 437-440 and 446, CM pp. 355-358 and 364. The faithful C is the
 * port's Pinball modules (c64-port/src/lanec_pw_*.c, lanec_pin_gaps.c).
 *
 * "MONITOR ALLOWS OTHER KEYBOARD ACTIVITY." A monitor verb (11-17) or an
 * NVSUB with one keeps a noun on the DSKY, shown again each second. MONITOR
 * saves the verb and noun in MONSAVE and the noun's address in MONSAVE1
 * (bit 15 the "KILL MONITOR SIGNAL (KILLER BIT)", bit 14 "THE EXTERNAL
 * MONITOR BIT"), and starts MONREQ as a WAITLIST task, unless one is
 * running. MONREQ, each second, takes the time for noun 65 (LODSAMPT),
 * calls itself again and starts MONDO as a job; it stops when the killer
 * bit is set. MONDO shows the noun with the monitor verb made the display
 * verb (11 to 01 and so on), through TESTNN, with ENTRET set so the display
 * comes back to PASTEVB, which puts the monitor verb (or the NVMONOPT
 * please verb) back in the verb lights. A busy DSKY (DSPLOCK) only lights
 * KEY RELEASE (MONBUSY).
 *
 * DSPFMEM (verb 27) shows a word of fixed memory in octal. DSPALARM, the
 * alarm of a display or load verb that cannot run, ends a monitor and
 * lights OPR ERR; CHARALRM, an illegal key, lights OPR ERR.
 *
 * Each function runs from a label, or from the word a call returns to, to
 * a call or jump to another routine or to the next label the faithful port
 * enters, which ends it with Q and Z set. The fixed-fixed services the
 * faithful port runs within its step (LEFT5, RELDSPON, FALTON, KILMONON,
 * LODSAMPT) are C calls. */
#include "pinball.h"
#include "../agc/names.h"

/* ---- MONITOR (LM pp. 437-438, CM pp. 355-356) ---- */

static void monit2(void);

/* MONITOR and MONIT1 -- LM p. 437, CM p. 355. The noun's address without
 * bits 15 and 14 into MPAC +1. ENTEXIT holds TC ENDOFJOB (ENDINST) for a
 * monitor keyed in: CS ENTEXIT, AD ENDINST gives -0, and bit 14 is set
 * ("EXTERNALLY INITIATED MONITOR") and the NVMONOPT options cleared. For
 * an NVSUB the CCS goes on to MONIT2; on +0 it runs the word BIT15/14 as
 * an instruction (OCT 60000, AD A, which leaves +0) on its way. */
void monitor(void)
{
    set_A(negate(BIT15_14));
    mask_at(ADDRESS_OF(NOUNCADR));
    if (ts(ADDRESS_OF(MPAC) + 1u)) not_modelled(); /* "TEMP STORAGE" */
    cs(ADDRESS_OF(ENTRET));                        /* ENTEXIT */
    ad(ENDINST);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc_at(MONITOR + 6u); break;
    case IS_PLUS_ZERO:
        ad_at(A_ADDRESS); /* BIT15/14, OCT 60000, run as AD A */
        tc_at(MONITOR + 010u);
        break;
    case NEGATIVE: tc_at(MONITOR + 010u); break;
    case IS_MINUS_ZERO:
        set_A(BIT14); /* "EXTERNALLY INITIATED MONITOR." */
        ads(ADDRESS_OF(MPAC) + 1u);
        set_A(ZERO);
        ts(ADDRESS_OF(MONSAVE2)); /* "ZERO NVMONOPT OPTIONS" */
        break;
    }
    monit2();
}

/* MONIT2 -- LM p. 437, CM p. 355. The verb (LEFT5, then CYL twice: left
 * seven) and noun together into MPAC, DSPLOCK +0 "SO MONITOR CAN RUN", and
 * KEY RELEASE out (RELDSP1) when CADRSTOR is empty. */
static void monit2(void)
{
    set_A(LOW7);
    mask_at(ADDRESS_OF(VERBREG));
    tc_at(MONIT2 + 2u);
    left5();
    if (ts(ADDRESS_OF(CYL))) not_modelled(); /* (LEFT5 leaves no overflow) */
    cs(ADDRESS_OF(CYL));
    xch(ADDRESS_OF(CYL));
    ad_at(ADDRESS_OF(NOUNREG));
    if (ts(ADDRESS_OF(MPAC))) /* "TEMP STORAGE" */
        not_modelled_state("TS MPAC with overflow (MONIT2)");
    set_A(ZERO);
    ts(ADDRESS_OF(DSPLOCK)); /* "+0 INTO DSPLOCK SO MONITOR CAN RUN." */
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE:
        tc_at(MONIT2 + 013u); /* TC +2 */
        continue_at(MONIT2 + 015u);
        return;
    case IS_PLUS_ZERO: tc(MONIT2 + 014u, RELDSP1); return;
    case NEGATIVE: continue_at(MONIT2 + 015u); return;
    case IS_MINUS_ZERO: continue_at(MONIT2 + 016u); return;
    }
}

/* MONIT2 +015 -- LM p. 438, CM p. 356, where RELDSP1 returns: interrupts
 * held off while MONSAVE is read and written. */
void monit2_plus015(void)
{
    inhint();
    continue_at(MONIT2 + 016u);
}

/* MONIT2 +016 -- LM p. 438, CM p. 356. "IF MONSAVE WAS +, NO REQUEST"; "IF
 * MONSAVE WAS 0, REQUEST MONREQ" in 10 ms (WAITLIST, 2CADR MONREQ). */
void monit2_plus016(void)
{
    switch (ccs_at(ADDRESS_OF(MONSAVE))) {
    case POSITIVE:
        tc_at(MONIT2 + 017u); /* TC +5 */
        continue_at(MONIT2 + 024u);
        return;
    case IS_PLUS_ZERO:
        set_A(ONE);
        tc(MONIT2 + 021u, WAITLIST);
        return;
    case NEGATIVE: tc(MONIT2 + 021u, WAITLIST); return;
    case IS_MINUS_ZERO: tc(MONIT2 + 022u, MONREQ); return; /* the 2CADR's word, TC MONREQ */
    }
}

/* MONIT2 +024 -- LM p. 438, CM p. 356, where WAITLIST returns. "PLACE
 * MONITOR VERB AND NOUN INTO MONSAVE", the address into MONSAVE1 ("ZERO THE
 * KILL MONITOR BIT", "SET UP EXTERNAL MONITOR BIT"), and back through
 * ENTRET. */
void monit2_plus024(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(MONSAVE));
    relint();
    tc(MONIT2 + 027u, ADDRESS_OF(ENTRET));
}

/* MONREQ -- LM p. 438, CM p. 356. "CALLED BY WAITLIST": the time for noun
 * 65 (LODSAMPT); with the killer bit off, itself again in a second
 * (MONDEL) and MONDO as a job; with it on, KILLMON. */
void monreq(void)
{
    tc_at(MONREQ);
    lodsampt();
    switch (ccs_at(ADDRESS_OF(MONSAVE1))) {
    case POSITIVE: tc_at(MONREQ + 2u); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at(MONREQ + 3u); break; /* TC +3 */
    case NEGATIVE: tc(MONREQ + 4u, KILLMON); return;
    case IS_MINUS_ZERO: tc(MONREQ + 5u, KILLMON); return;
    }
    set_A(MONDEL);
    tc(MONREQ + 7u, WAITLIST); /* "ENTER WAITLIST REQUEST FOR MONREQ" */
}

void monreq_plus10(void)
{
    set_A(CHRPRIO);
    tc(MONREQ + 013u, NOVAC); /* "ENTER EXEC REQUEST FOR MONDO" */
}

void monreq_plus14(void)
{
    tc(MONREQ + 016u, TASKOVER);
}

/* KILLMON -- LM p. 438, CM p. 356. "ZERO MONSAVE AND TURN KILLER BIT OFF",
 * and the external monitor bit. */
void killmon(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MONSAVE));
    ts(ADDRESS_OF(MONSAVE1));
    tc(KILLMON + 3u, TASKOVER);
}

/* MONDO -- LM pp. 438-439, CM pp. 356-357. "CALLED BY EXEC": ends "IN CASE
 * TERMINATE CAME SINCE LAST MONREQ"; a busy DSKY ("NVSUB IS BUSY") goes to
 * MONBUSY; otherwise the noun into NOUNREG and on the DSKY (UPDATNN -1). */
void mondo(void)
{
    switch (ccs_at(ADDRESS_OF(MONSAVE1))) {
    case POSITIVE: tc_at(MONDO + 1u); break;     /* TC +4 */
    case IS_PLUS_ZERO: tc_at(MONDO + 2u); break; /* TC +3 */
    case NEGATIVE: tc(MONDO + 3u, ENDOFJOB); return;
    case IS_MINUS_ZERO: tc(MONDO + 4u, ENDOFJOB); return;
    }
    switch (ccs_at(ADDRESS_OF(DSPLOCK))) {
    case POSITIVE: tc(MONDO + 6u, MONBUSY); return;
    case IS_PLUS_ZERO:
        set_A(LOW7);
        mask_at(ADDRESS_OF(MONSAVE));
        break;
    case NEGATIVE: mask_at(ADDRESS_OF(MONSAVE)); break;
    case IS_MINUS_ZERO: break;
    }
    tc(MONDO + 011u, UPDATNN - 1u); /* "PLACE NOUN INTO NOUNREG AND DISPLAY IT" */
}

/* MONDO +012 -- LM p. 439, CM p. 357, where UPDATNN returns. "CHANGE MONITOR
 * VERB TO DISPLAY VERB": the verb, in bits 8-14, less ten, right seven by
 * EDOP, into VERBREG; ENTRET to "RETURN TO PASTEVB AFTER DATA DISPLAY"; the
 * address into MPAC +2 for INTMCTBS; and the display by TESTNN. */
void mondo_plus012(void)
{
    set_A(MID7);
    mask_at(ADDRESS_OF(MONSAVE));
    ad(MONREF);                               /* "-DEC10, STARTING IN BIT8" */
    if (ts(ADDRESS_OF(EDOP))) not_modelled(); /* "RIGHT 7" */
    ca(ADDRESS_OF(EDOP));
    ts(ADDRESS_OF(VERBREG));
    set_A(MONBACK);
    ts(ADDRESS_OF(ENTRET));
    set_A(negate(BIT15_14));
    mask_at(ADDRESS_OF(MONSAVE1));
    ts(ADDRESS_OF(MPAC) + 2u);
    tc(ENDMONDO, TESTNN);
}

/* MONBUSY -- LM p. 439, CM p. 358. "TURN KEY RELEASE LIGHT" on. */
void monbusy(void)
{
    tc_at(MONBUSY);
    reldspon();
    tc(MONBUSY + 1u, ENDOFJOB);
}

/* ---- PASTEVB (LM p. 439, CM p. 357) ---- */

/* PASTEVB -- LM p. 439, CM p. 357 and PASTEOPT. "PASTE MONITOR VERB" back
 * into VERBREG and the verb lights, or, with an NVMONOPT paste option
 * (MONSAVE2 bits 8-14), "PASTE PLEASE VERB". */
void pastevb(void)
{
    set_A(MID7);
    mask_at(ADDRESS_OF(MONSAVE2)); /* "NVMONOPT PASTE OPTION" */
    if (A_is_zero()) {             /* EXTEND; BZF +2 */
        ca(ADDRESS_OF(MONSAVE));
    } else {
        tc_at(PASTEVB + 4u); /* TC PASTEOPT */
    }
    if (ts(ADDRESS_OF(EDOP))) not_modelled(); /* "RIGHT 7" */
    ca(ADDRESS_OF(EDOP));
    tc(PASTEOPT + 2u, BANKCALL); /* CADR UPDATVB -1 */
}

/* PASTEOPT +4 -- LM p. 439, CM p. 357, where UPDATVB returns. "ZERO REQRET
 * SO THAT PASTED VERBS CAN BE EXECUTED BY OPERATOR", and the NVMONOPT blank
 * option (BLANKSUB). */
void pasteopt_plus4(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(REQRET));
    ca(ADDRESS_OF(MONSAVE2));
    tc(PASTEOPT + 7u, BLANKSUB);
}

/* PASTEOPT +010 -- LM p. 439, CM p. 357, where BLANKSUB returns when the
 * DSKY is busy. */
void pasteopt_plus010(void)
{
    tc_at(PASTEOPT + 010u); /* TC +1 */
    continue_at(ENDPASTE);
}

void endpaste(void)
{
    tc(ENDPASTE, ENDOFJOB);
}

/* ---- DSPFMEM (LM p. 440, CM p. 358) ---- */

/* DSPFMEM -- LM p. 440, CM p. 358: "USED TO DISPLAY (IN OCTAL) ANY FIXED
 * REGISTER", verb 27 with noun 01: the word at the FCADR in NOUNCADR, with
 * the superbank bits "PRELOADED INTO 3RD COMPONENT OF NOUN 26" (DSPTEM1 +2),
 * fetched by SUPDACAL and shown in R1. */
void dspfmem(void)
{
    set_A(R1D1);
    ts(ADDRESS_OF(DSPCOUNT));
    ca(ADDRESS_OF(DSPTEM1) + 2u);
    ts(L_ADDRESS);
    ca(ADDRESS_OF(NOUNCADR)); /* "ORIGINAL FCADR LOADED STILL IN NOUNCADR." */
    tc(DSPFMEM + 5u, SUPDACAL);
}

void dspfmem_plus6(void)
{
    tc(DSPFMEM + 6u, DSPOCTWD);
}

/* ENDSPF -- LM p. 440, CM p. 358, where DSPOCTWD returns. */
void endspf(void)
{
    tc(ENDSPF, ENDOFJOB);
}

/* ---- The alarms (LM p. 446, CM p. 364) ---- */

/* PREDSPAL -- LM p. 446, CM p. 364: R1 the register for the alarm's display
 * (CS VD1). */
void predspal(void)
{
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    continue_at(DSPALARM);
}

/* DSPALARM -- LM p. 446, CM p. 364: "FINDS TC NVSUBEND IN ENTRET FOR NVSUB
 * INITIATED ROUTINES", which "ABORT WITH 01501"; "IF THIS IS A MONITOR, KILL
 * IT" (ENTRET TC PASTEVB): KILMONON, OPR ERR, and the monitor verb back
 * (PASTEVB); otherwise CHARALRM. */
void dspalarm(void)
{
    set_A(negate(NVSBENDL));
    ad_at(ADDRESS_OF(ENTRET)); /* ENTEXIT */
    if (A_is_zero()) {         /* EXTEND; BZF CHARALRM +2 */
        tc(CHARALRM + 2u, POODOO);
        return;
    }
    set_A(negate(MONADR));
    ad_at(ADDRESS_OF(ENTRET));
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        tc(DSPALARM + 010u, CHARALRM);
        return;
    }
    tc_at(DSPALARM + 011u);
    kilmonon();
    tc_at(DSPALARM + 012u);
    falton();
    tc(DSPALARM + 013u, PASTEVB); /* "PUT MONITOR VERB BACK IN VERBREG" */
}

/* CHARALRM -- LM p. 446, CM p. 364: "NOT NVSUB INITIATED. TURN ON OPR
 * ERROR". */
void charalrm(void)
{
    tc_at(CHARALRM);
    falton();
    tc(CHARALRM + 1u, ENDOFJOB);
}

/* ---- Fixed-fixed services (KILMONON LM p. 454, CM p. 372; LODSAMPT LM p. 462, CM p. 380) ---- */

/* KILMONON -- LM p. 454, CM p. 372: "THIS IS THE KILL MONITOR BIT" (BIT15)
 * into MONSAVE1, which turns off bit 14, "THE EXTERNAL MONITOR BIT". */
void kilmonon(void)
{
    set_A(BIT15);
    ts(ADDRESS_OF(MONSAVE1));
    tc_return(Q_ADDRESS);
}

/* LODSAMPT -- LM p. 462, CM p. 380: the time (TIME2, TIME1) into SAMPTIME,
 * noun 65's. */
void lodsampt(void)
{
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(SAMPTIME));
    tc_return(Q_ADDRESS);
}
