/* SPDX-License-Identifier: MIT */
/* P70-P71.agc, pp. 829-830 and 835-836 (Luminary099 only, bank 21): R10,R11,
 * the task that runs the landing radar lights and watches for an abort,
 * P70 and P71 and their start, and LEGAL?, which says whether an abort may
 * start. The faithful C is c64-port/src/lm_p70_p71_file.c. */
#include "p70_p71.h"
#include "../agc/names.h"

#if LUMINARY099

static void flashh_q(void);
static void p71now_q(void);
static void p70now_q(void);
static void start_abort(void);
static void abortalm(void);

/* ---- R10,R11 -- pp. 829-830 ---- */

/* R10,R11: "IS SERVICER STILL RUNNING?" (AVEGFLAG clear: CS FLAGWRD7 leaves
 * its bit set). If not, "LET AVGEND TAKE CARE OF GROUP 2" (TASKOVER). If
 * so, the pass count PIPCTR: on its "LAST PASS", +0, LRHTASK; otherwise
 * PIPCTR1 gets it less one, and R10,R11 waits for itself again (TWIDDLE,
 * OCT31 centiseconds). (A MASK leaves no negative word; and CCS of PIPCTR
 * -0 skips the TS, leaving PIPCTR1 alone.) */
void r10_r11(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(AVEGFBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        continue_at(TASKOVER);
        return;
    }
    switch (ccs_at(ADDRESS_OF(PIPCTR))) {
    case POSITIVE:
    case NEGATIVE: ts(ADDRESS_OF(PIPCTR1)); break;   /* (A, CCS's result, never overflows) */
    case IS_PLUS_ZERO: continue_at(LRHTASK); return; /* "LAST PASS. CALL LRHTASK." */
    case IS_MINUS_ZERO: break;
    }
    set_A(OCT31);
    tc_to(R10_R11 + 011u, TWIDDLE); /* ADRES R10,R11 */
}

/* R10,R11A: "IF LAMP TEST, DO NOT CHANGE LR LITES" (IMODES33's bit 1
 * clear): then on at 10,11; otherwise FLASHH?. */
void r10_r11a(void)
{
    cs(ADDRESS_OF(IMODES33));
    mask(BIT1);
    if (A_is_zero())
        ten_11();
    else
        flashh_q();
}

/* FLASHH?: the altitude light's flash bit (FLGWRD11 bit 1: "C(A) = 1 =
 * HFLASH BIT"). Off, "LEAVE ALONE" (FLASHV?); on, "FLIP H LITE" (FLIP,
 * with HLITE in L). (A, a MASK's, never overflows: TS L does not skip.) */
static void flashh_q(void)
{
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        flashv_q();
        return;
    }
    set_A(HLITE);
    ts(L_ADDRESS);
    tc_to(FLASHH_Q + 5u, FLIP);
}

/* FLASHV?: the velocity light's (VFLSHBIT: "MUST BE BIT 2"). Off, on at
 * 10,11; on, "FLIP V LITE". */
void flashv_q(void)
{
    set_A(VFLSHBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        ten_11();
        return;
    }
    set_A(VLITE);
    ts(L_ADDRESS);
    tc_to(FLASHV_Q + 6u, FLIP);
}

/* 10,11: "IS THE LETABORT FLAG SET ?" No: "PROCEED TO R10" (LANDISP). Yes:
 * P71NOW?. */
void ten_11(void)
{
    ca(ADDRESS_OF(FLAGWRD9));
    mask(LETABBIT);
    if (A_is_zero())
        continue_at(LANDISP);
    else
        p71now_q();
}

/* P71NOW?: "ARE WE IN P71 NOW?" Yes: R10 (LANDISP). No: "IS AN ABORT STAGE
 * COMMANDED?" (channel 30's bit 4 clear; L keeps the channel
 * complemented): yes, P71A; no, P70NOW?. (READ leaves the channel's word
 * sign-extended, so COM cannot overflow, and TS L does not skip; a MASK
 * leaves no negative word.) */
static void p71now_q(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(ONE_DEC71);
    if (A_is_zero()) {
        continue_at(LANDISP);
        return;
    }
    read_channel(CHAN30);
    com();
    ts(L_ADDRESS);
    mask(BIT4);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        p71a();
    else
        p70now_q();
}

/* P70NOW?: "ARE WE IN P70 NOW?" Yes: R10. No: "IS AN ABORT COMMANDED?"
 * (channel 30's bit 1 clear, from L): yes, P70A; no, R10. */
static void p70now_q(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(ONE_DEC70);
    if (A_is_zero()) {
        continue_at(LANDISP);
        return;
    }
    ca(L_ADDRESS);
    mask(BIT1);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        p70a();
    else
        continue_at(LANDISP);
}

/* ---- P70 and P71 -- p. 830 ---- */

/* P70, from verb 37: LEGAL?, whose return is P70A. */
void p70(void)
{
    tc_at(P70);
    legal_q();
}

/* P70A: Q -0 for P70 ... */
void p70a(void)
{
    set_A(negate(ZERO));
    start_abort();
}

/* P71: LEGAL?, whose return is P71A. */
void p71(void)
{
    tc_at(P71);
    legal_q();
}

/* P71A: ... and two for P71. */
void p71a(void)
{
    set_A(TWO);
    start_abort();
}

/* P70A +3 (P71A +1): Q gets A, then with interrupts inhibited CONTABRT
 * (DTCB of CNTABTAD, its 2CADR). (A is a constant: TS Q does not skip.) */
static void start_abort(void)
{
    ts(Q_ADDRESS);
    inhint();
    set_L(CNTABTAD[1]);
    set_A(CNTABTAD[0]);
    dtcb(P70A + 010u);
}

/* ---- LEGAL? -- pp. 835-836 ---- */

/* LEGAL?: "IS THE DESIRED PGM ALREADY IN PROGRESS?" (MMNUMBER, the program
 * asked for, MODREG): then ABORTALM. "ARE THE ABORTS ENABLED?" (LETABORT
 * set) and "IS SERVICER ON THE AIR?" (AVEGFLAG): if both, "ALL IS WELL"
 * (TC Q, back to the caller); otherwise ABORTALM. (Each MASK leaves no
 * negative word.) */
void legal_q(void)
{
    cs(ADDRESS_OF(MMNUMBER));
    ad_at(ADDRESS_OF(MODREG));
    if (A_is_zero()) {
        abortalm();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD9));
    mask(LETABBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE) {
        abortalm();
        return;
    }
    ca(ADDRESS_OF(FLAGWRD7));
    mask(AVEGFBIT);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        tc_q(); /* "YES. ALL IS WELL." */
    else
        abortalm();
}

/* ABORTALM: the operator error light (FALTON), ... */
static void abortalm(void)
{
    tc_to(ABORTALM, FALTON);
}

/* ... the display released (RELDSP), ... */
void abortalm_after_falton(void)
{
    tc_to(ABORTALM + 1u, RELDSP);
}

/* ... and back to Pinball (POSTJUMP, CADR PINBRNCH). */
void abortalm_after_reldsp(void)
{
    tc_to(ABORTALM + 2u, POSTJUMP);
}

#endif
