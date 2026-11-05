/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 800-801 (Luminary099 only):
 * "GUILDENSTERN: AUTO-MODES MONITOR (R13)". Each guidance pass starts
 * here. "ON EVERY APPEARANCE OR DISAPPEARANCE OF THE MANUAL THROTTLE
 * DISCRETE TO SELECT P67 OR P66 RESPECTIVELY: ON EVERY APPEARANCE OF THE
 * ATTITUDE-HOLD DISCRETE TO SELECT P66 UNLESS THE CURRENT PROGRAM IS P67
 * IN WHICH CASE THERE IS NO CHANGE". P66 and P67 start the vertical phase
 * (VRTSTART); otherwise the pass goes on at GUILDRET (pass.c). The
 * faithful C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void stabl_q(void);
static void p66now_q(void);
static void restart_q(void);
static void vrtstart(void);

/* GUILDEN: "IS UN-AUTO-THROTTLE DISCRETE PRESENT?" (channel 30's bit 5).
 * If so, STARTP67; if not, P67NOW?: "ARE WE IN P67 NOW?" (CHECKMM). (A,
 * a masked bit, is never negative: CCS A takes it as positive or +0.) */
void guilden(void)
{
    read_channel(CHAN30);
    mask(BIT5);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        startp67();
    else
        tc_to(P67NOW_Q, CHECKMM); /* DEC 67 */
}

/* CHECKMM's return when not in P67: TCF STABL?. */
void p67now_q_plus2(void)
{
    stabl_q();
}

/* STARTP66 (CHECKMM's return when in P67, and P66NOW?'s): FASTCHNG, ... */
void startp66(void)
{
    tc_at(STARTP66);
    fastchng();
}

/* ... then NEWMODEX, the program P66 (DEC66), ... */
void startp66_after_fastchng(void)
{
    tc_to(STARTP66 + 1u, NEWMODEX);
}

/* ... "SET DESIRED ALTITUDE RATE = CURRENT ALTITUDE RATE" (VDGVERT gets
 * HDOTDISP), and STRTP66A. */
void startp66_after_newmodex(void)
{
    dca(ADDRESS_OF(HDOTDISP));
    dxch(ADDRESS_OF(VDGVERT));
    strtp66a();
}

/* STRTP66A: the interpretive code, ... */
void strtp66a(void)
{
    tc_at(STRTP66A);
    intpret();
}

/* ... the rate of descent mode's start: the PIPA biases (PBIASX, PBIASY,
 * PBIASZ) as a vector times BIASFACT to VBIAS, RODFLAG set, TEMX
 * complemented to OLDPIPAX, DELVROD zeroed (ZEROVECS), RODSCALE to
 * RODSCAL1 and PIPTIME to LASTTPIP. */
static const word strtp66a_words[] = {
    /* STRTP66A +1 */ ORDERS(SLOAD, PUSH),
    ADDRESS_PLUS_ONE(PBIASZ),
    ORDERS(SLOAD, PUSH),
    ADDRESS_PLUS_ONE(PBIASY),
    ORDERS(SLOAD, VDEF),
    ADDRESS_PLUS_ONE(PBIASX),
    ORDERS(VXSC, SET),
    ADDRESS_PLUS_ONE(BIASFACT),
    SWITCH(SET, RODFLAG),
    STOVL(VBIAS),
    ADDRESS_PLUS_ONE(TEMX),
    ORDER(VCOMP),
    STOVL(OLDPIPAX),
    ADDRESS_PLUS_ONE(ZEROVECS),
    STODL(DELVROD),
    ADDRESS_PLUS_ONE(RODSCALE),
    STODL(RODSCAL1),
    ADDRESS_PLUS_ONE(PIPTIME),
    STORE(LASTTPIP),
    ORDER(EXIT),
};

const struct interpretive_code STRTP66A_CODE = {031u, STRTP66A + 1u, COUNT(strtp66a_words),
                                                strtp66a_words};

/* After the EXIT: FCOLD, FWEIGHT and WCHVERT (VRTSTART) zero, for P66.
 * (A is +0: no TS skips.) */
void strtp66a_after_exit(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(FCOLD));
    ts(ADDRESS_OF(FWEIGHT));
    ts(ADDRESS_OF(FWEIGHT) + 1u);
    vrtstart();
}

/* VRTSTART: WCHVERT gets A (+0 for P66, ten for P67), and "WCHPHASE = 2
 * ---> VERTICAL: P65,P66,P67" (WCHPHOLD too); then STOPRATE (BANKCALL:
 * "TEMPORARY, I HOPE HOPE HOPE"). (No TS skips: A is +0, ten or two.) */
static void vrtstart(void)
{
    ts(ADDRESS_OF(WCHVERT));
    set_A(TWO);
    ts(ADDRESS_OF(WCHPHOLD));
    ts(ADDRESS_OF(WCHPHASE));
    tc_to(VRTSTART + 4u, BANKCALL); /* CADR STOPRATE */
}

/* STOPRATE's return: "PERMIT X-AXIS OVERRIDE" (DOWNFLAG XOVINFLG), ... */
void vrtstart_after_stoprate(void)
{
    tc_to(VRTSTART + 6u, DOWNFLAG);
}

/* ... DOWNFLAG REDFLAG, ... */
void vrtstart_after_xovinflg(void)
{
    tc_to(VRTSTART + 010u, DOWNFLAG);
}

/* ... and VERTGUID (vertguid.c). */
void vrtstart_after_redflag(void)
{
    vertguid();
}

/* STARTP67: NEWMODEX, the program P67: "NO HARM IN "STARTING" P67 OVER
 * AND OVER SO NO NEED FOR A FASTCHNG AND NO NEED TO SEE IF ALREADY IN
 * P67." */
void startp67(void)
{
    tc_to(STARTP67, NEWMODEX); /* DEC 67 */
}

/* NEWMODEX's return: RODCOUNT zero, and VRTSTART with ten. */
void startp67_after_newmodex(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(RODCOUNT));
    set_A(TEN);
    vrtstart();
}

/* STABL?: "IS UN-ATTITUDE-HOLD DISCRETE PRESENT?" (channel 31's bit 13).
 * "YES ALL'S WELL": GUILDRET; if not, P66NOW?. (A, a masked bit, is never
 * negative.) */
static void stabl_q(void)
{
    set_A(BIT13);
    rand_channel(CHAN31);
    if (ccs_at(A_ADDRESS) == POSITIVE)
        guildret();
    else
        p66now_q();
}

/* P66NOW?: in P66 already, RESTART?; if not, "HAS THE ROD SWITCH BEEN
 * "CLICKED"?" (RODCOUNT): "NO. CONTINUE WITH AUTOMATIC LANDING"
 * (GUILDRET), "YES. SWITCH INTO THE ROD MODE." (STARTP66). */
static void p66now_q(void)
{
    cs(ADDRESS_OF(MODREG));
    ad(DEC66);
    if (A_is_zero()) {
        restart_q();
        return;
    }
    ca(ADDRESS_OF(RODCOUNT));
    if (A_is_zero())
        guildret();
    else
        startp66();
}

/* RESTART?: "HAS THERE BEEN A RESTART?" (RODFLAG clear): "YES.
 * REINITIALIZE BUT LEAVE VDGVERT AS IS." (STRTP66A); "NO: CONTINUE WITH
 * R.O.D." (VERTGUID). */
static void restart_q(void)
{
    ca(ADDRESS_OF(FLAGWRD1));
    mask(RODFLBIT);
    if (A_is_zero())
        strtp66a();
    else
        vertguid();
}

#endif
