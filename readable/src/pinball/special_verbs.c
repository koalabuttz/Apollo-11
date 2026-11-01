/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 6: the major mode change, the
 * request verbs, proceed and terminate, and key release -- LM pp. 447-450,
 * CM pp. 365-368. The faithful C is the port's Pinball modules
 * (c64-port/src/lanec_pw_*.c, lanec_pin_head_*.c, lanec_pin_keys.c).
 *
 * Verb 37 asks for a new major mode: MMCHANG flashes the noun lights blank
 * and waits for two digits (REQMM, returning to MMCHANG +1 through REQRET),
 * then goes to V37 (FRESH_START_AND_RESTART) with the mode in A. Verbs 30
 * and 31 (VBRQEXEC, VBRQWAIT) start a job or a task at the address and
 * priority or delay preloaded into noun 26. Verbs 32-34 (VBRESEQ, VBPROC,
 * VBTERM) answer a flashing display: "PROCEED WITHOUT DATA", terminate, or
 * "MAKE IT LOOK LIKE DATA IN", each through LOADSTAT for the routine
 * waiting in ENDIDLE; PROCKEY is the PRO key's version of verb 33.
 * VBRELDSP is the KEY REL key.
 *
 * Each function runs from a label, or from the word a call returns to, to
 * a call or jump to another routine or to the next label the faithful port
 * enters, which ends it with Q and Z set. A call the faithful port runs
 * within its step (FLASHON after REQMM's 2BLANK, RELDSP at TSTLTS4,
 * RELDSP1 at UNSUSPEN) is a C call. */
#include "pinball.h"
#include "../agc/names.h"

/* ---- Verb 37, the major mode change (LM p. 447, CM p. 365) ---- */

/* MMCHANG -- "ENTPASHI ASSUMES THE TC REQMM AT MMCHANG": the mode's two
 * digits are asked for, and come back at MMCHANG +1. */
void mmchang(void)
{
    tc(MMCHANG, REQMM);
}

/* MMCHANG +1. "DEMAND THAT 2 NUM CHAR WERE PUNCHED IN" (DSPCOUNT = -ND2,
 * BIT5 being OCT20, ND2), or ALMCYCLE; the mode out of NOUNREG into MPAC,
 * and the noun lights blanked (ND1, 2BLANK). */
void mmchang_plus1(void)
{
    set_A(BIT5); /* "OCT20 = ND2." */
    ad_at(ADDRESS_OF(DSPCOUNT));
    if (!A_is_zero()) {             /* EXTEND; BZF +2 */
        tc(MMCHANG + 5u, ALMCYCLE); /* "DSPCOUNT NOT= -ND2. ALARM AND RECYCLE." */
        return;
    }
    set_A(ZERO);
    xch(ADDRESS_OF(NOUNREG));
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    set_A(ND1);
    ts(ADDRESS_OF(DSPCOUNT));
    tc(MMCHANG + 11u, BANKCALL); /* CADR 2BLANK */
}

/* MMCHANG +13, where 2BLANK returns. "BLOCK NUM CHAR IN" (DSPCOUNT -VD1),
 * and to V37 with the mode in A ("GO THRU STANDARD LOC.", MODROUTB). */
void mmchang_plus13(void)
{
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    ca(ADDRESS_OF(MPAC));
    tc(MMCHANG + 16u, POSTJUMP); /* CADR MODROUTB */
}

/* REQMM -- the return, complemented, into REQRET (minus for ENTER's
 * higher pass), the noun lights blanked and flashing, and DECBRNCH "SET
 * FOR DEC". */
void reqmm(void)
{
    cs(Q_ADDRESS);
    if (ts(ADDRESS_OF(REQRET))) not_modelled();
    set_A(ND1);
    ts(ADDRESS_OF(DSPCOUNT));
    set_A(ZERO);
    ts(ADDRESS_OF(NOUNREG));
    tc(REQMM + 6u, BANKCALL); /* CADR 2BLANK */
}

void reqmm_plus8(void)
{
    tc_at(REQMM + 8u);
    flashon();
    set_A(ONE);
    ts(ADDRESS_OF(DECBRNCH));            /* "SET FOR DEC" */
    tc(REQMM + 11u, ADDRESS_OF(ENTRET)); /* TC ENTEXIT */
}

/* ---- Verbs 30 and 31, the requests (LM pp. 447-448, CM p. 366) ---- */

static void reqex1_plus1(void);

/* VBRQEXEC -- "ENTERS REQUEST TO EXEC FOR ANY ADDRESS WITH ANY PRIORITY":
 * noun 26's first component is the priority, "BIT1=0 FOR NOVAC, BIT1=1
 * FOR FINDVAC". TC NOVAC or TC FINDVAC goes into MPAC (REQEX1). */
void vbrqexec(void)
{
    set_A(BIT1);
    mask_at(ADDRESS_OF(DSPTEM1));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: tc(VBRQEXEC + 3u, SETVAC); return; /* "IF BIT1 = 1, FINDVAC" */
    case IS_PLUS_ZERO:
        set_A(TCNOVAC); /* "IF BIT1 = 0, NOVAC" */
        continue_at(REQEX1);
        return;
    case NEGATIVE: continue_at(REQEX1); return;
    case IS_MINUS_ZERO: reqex1_plus1(); return;
    }
}

/* REQEX1 -- "TC NOVAC OR TC FINDVAC INTO MPAC". */
void reqex1(void)
{
    if (ts(ADDRESS_OF(MPAC))) not_modelled();
    reqex1_plus1();
}

/* REQEX1 +1: the priority, without bit 1. */
static void reqex1_plus1(void)
{
    set_A(negate(BIT1));
    mask_at(ADDRESS_OF(DSPTEM1));
    continue_at(REQEX1 + 3u);
}

/* REQEX1 +3 (REQUESTC -1, where verb 31 comes in with the delay): "PRIO
 * INTO MPAC+4 AS A TEMP". */
void reqex1_plus3(void)
{
    if (ts(ADDRESS_OF(MPAC) + 4u)) not_modelled();
    continue_at(REQUESTC);
}

/* REQUESTC -- the display system released (RELDSP). */
void requestc(void)
{
    tc(REQUESTC, RELDSP);
}

/* REQUESTC +1. "TC ENDOFJOB INTO MPAC+3", the "JOB ADRES INTO MPAC+1"
 * and "BBCON INTO MPAC+2", the priority or delay in A, and with interrupts
 * off TC MPAC: NOVAC, FINDVAC or WAITLIST with the 2CADR after it, coming
 * back to MPAC +3's TC ENDOFJOB. */
void requestc_plus1(void)
{
    set_A(ENDINST);
    ts(ADDRESS_OF(MPAC) + 3u);
    dca(ADDRESS_OF(DSPTEM1) + 1u);
    dxch(ADDRESS_OF(MPAC) + 1u);
    ca(ADDRESS_OF(MPAC) + 4u); /* "PRIO IN A" */
    inhint();
    tc(REQUESTC + 8u, ADDRESS_OF(MPAC));
}

/* SETVAC -- TC FINDVAC for REQEX1. */
void setvac(void)
{
    set_A(TCFINDVC);
    tc(SETVAC + 1u, REQEX1);
}

/* VBRQWAIT -- "ENTERS REQUEST TO WAITLIST FOR ANY ADDRESS WITH ANY DELAY":
 * "TC WAITLIST INTO MPAC", and the "TIME DELAY" in A. */
void vbrqwait(void)
{
    set_A(TCWAIT);
    ts(ADDRESS_OF(MPAC));
    ca(ADDRESS_OF(DSPTEM1));
    continue_at(ENDRQWT);
}

void endrqwt(void)
{
    tc(ENDRQWT, REQUESTC - 1u);
}

/* ---- Verbs 32-34 and PRO (LM pp. 448-449, CM pp. 366-367) ---- */

/* VBPROC -- verb 34, "PROCEED WITHOUT DATA": LOADSTAT +1. Verb 34 (VBTERM)
 * sets it -1 and verb 32 (VBRESEQ) -0 at VBPROC +1. Then the monitor is
 * killed (KILMONON), the display system released, the flash turned off,
 * and RECALTST wakes the routine waiting in ENDIDLE. */
void vbproc(void)
{
    set_A(ONE);
    continue_at(VBPROC + 1u);
}

void vbproc_plus1(void)
{
    if (ts(ADDRESS_OF(LOADSTAT))) not_modelled();
    tc(VBPROC + 2u, KILMONON); /* "TURN ON KILL MONITOR BIT" */
}

void vbproc_plus3(void)
{
    tc(VBPROC + 3u, RELDSP);
}

void vbproc_plus4(void)
{
    tc(VBPROC + 4u, FLASHOFF);
}

void vbproc_plus5(void)
{
    tc(VBPROC + 5u, RECALTST); /* "SEE IF THERE IS ANY RECALL FROM ENDIDLE" */
}

/* VBTERM -- verb 34: "TERM VERB SETS LOADSTAT NEG". */
void vbterm(void)
{
    set_A(negate(ONE));
    tc(VBTERM + 1u, VBPROC + 1u);
}

/* PROCKEY -- "PERFORMS THE SAME FUNCTION AS VBPROC": "SET REQRET FOR ENTER
 * PASS 0" and "BLOCK NUMERICAL CHARACTERS, SIGNS, CLEAR". */
void prockey(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(REQRET));
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    tc(PROCKEY + 4u, VBPROC);
}

/* VBRESEQ -- verb 32: "MAKE IT LOOK LIKE DATA IN" (CS ZERO, -0). */
void vbreseq(void)
{
    set_A(negate(ZERO));
    tc(VBRESEQ + 1u, VBPROC + 1u);
}

/* ---- Key release (LM p. 450, CM p. 368) ---- */

/* VBRELDSP -- "ALWAYS TURNS OFF THE UPACT LIGHT" (bit 3 of channel 11).
 * With the old DSPLOCK (21/22REG, as KEYRUPT left it) and the external
 * monitor bit both set, UNSUSPEN; otherwise TSTLTS4. Each CCS's
 * other outcomes run on in the listing's words. */
void vbreldsp(void)
{
    set_A(negate(BIT3));
    wand_dsalmout();                                /* "TURN OFF UPACT LITE" */
    switch (ccs_at(ADDRESS_OF(TWENTY_ONE_22REG))) { /* "OLD DSPLOCK" */
    case POSITIVE:
        set_A(BIT14);
        mask_at(ADDRESS_OF(MONSAVE1));
        break;
    case IS_PLUS_ZERO: mask_at(ADDRESS_OF(MONSAVE1)); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: tc(VBRELDSP + 7u, UNSUSPEN); return;
    }
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
        tc(VBRELDSP + 7u, UNSUSPEN); /* "OLD DSPLOCK AND EMB BOTH 1, UNSUSPEND." */
        return;
    case IS_PLUS_ZERO: continue_at(TSTLTS4); return;
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); return; /* into TSTLTS4 +1 and +2 */
    }
}

/* TSTLTS4 -- "NOT UNSUSPENDING EXTERNAL MONITOR, RELEASE DISPLAY SYSTEM
 * AND DO RE-ESTABLISH IF CADRSTOR IS FULL": PINBRNCH re-runs the NVSUB
 * calls of the job waiting in ENDIDLE. */
void tstlts4(void)
{
    tc_at(TSTLTS4);
    reldsp();
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: tc_at(TSTLTS4 + 2u); break; /* TC +2 */
    case IS_PLUS_ZERO: tc(TSTLTS4 + 3u, ENDOFJOB); return;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); return; /* the CADR PINBRNCH word */
    }
    tc(TSTLTS4 + 4u, POSTJUMP); /* CADR PINBRNCH */
}

/* UNSUSPEN -- "EXTERNAL MONITOR IS SUSPENDED, JUST UNSUSPEND IT BY
 * CLEARING DSPLOCK", and "TURN KEY RELEASE LIGHT OFF IF BOTH CADRSTOR AND
 * DSPLIST ARE EMPTY" (RELDSP1). */
void unsuspen(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DSPLOCK));
    switch (ccs_at(ADDRESS_OF(CADRSTOR))) {
    case POSITIVE: tc(UNSUSPEN + 3u, ENDOFJOB); return;
    case IS_PLUS_ZERO:
        tc_at(UNSUSPEN + 4u);
        reldsp1();
        break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); return; /* past ENDRELDS */
    }
    tc(UNSUSPEN + 5u, ENDOFJOB);
}
