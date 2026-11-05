/* SPDX-License-Identifier: MIT */
/* THROTTLE_CONTROL_ROUTINES.agc, pp. 793-797 (Luminary099 only): THROTTLE,
 * the throttling bands (WHERETO), the command (DOPIF, DOIT) and its
 * weighting for the next pass (FWEIGHT), FLATOUT and WHATOUT, and
 * MASSMULT. The faithful C is c64-port/src/lm_throttle.c. */
#include "throttle.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"

/* TC at, to a routine out of the file: Q gets the return and the routine
 * runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* Every return word these routines keep (Q at FLATOUT, WHATOUT and
 * MASSMULT, RTNHOLD at DOPIF's return) is a TC to fixed memory, 02000 to
 * 07777: TC RTNHOLD and TC BUF run it as an instruction. Any other is not
 * modelled. */
static void need_tc_return(word return_word)
{
    if (return_word < 02000u || return_word > 07777u) not_modelled();
}

static void whereto(void);
static void flatout1(void);
static void flatout2(void);
static void doit(void);

/* ---- FC and FP -- p. 793 ---- */

/* THROTTLE: "HERE FC, DESIRED THRUST, AND FP, PRESENT THRUST, UNWEIGHTED,
 * ARE COMPUTED." ABDELV, the present acceleration, times /AF/CNST: "IN
 * UNITS OF 2(-4) M/CS/CS, SAVING SERVICER TROUBLE". */
void throttle(void)
{
    ca(ADDRESS_OF(ABDELV));
    mp(SLASH_AF_CNST);
    throttle_plus3();
}

/* THROTTLE +3: the caller's return to RTNHOLD, and MASSMULT turns the
 * acceleration in A,L into a force. */
void throttle_plus3(void)
{
    qxch(ADDRESS_OF(RTNHOLD));
    tc_to(AFDUMP, MASSMULT);
}

/* "FP = PRESENT THRUST"; then the guidance's acceleration, /AFC/, to
 * MASSMULT. */
void afdump_after_massmult(void)
{
    dxch(ADDRESS_OF(FP));
    dca(ADDRESS_OF(_AFC_));
    tc_to(AFDUMP + 4u, MASSMULT);
}

/* "FC = THRUST DESIRED BY GUIDANCE", and "FCODD = WHAT IT IS GOING TO GET"
 * (MASSMULT leaves A a word, so TS FC never skips). "IF IT HAS BEEN LESS
 * THAN 3 SECONDS SINCE THE LAST THROTTLING, AUGMENT FP USING THE FWEIGHT
 * CALCULATED" last pass. */
void afdump_after_second_massmult(void)
{
    if (!ts(ADDRESS_OF(FC))) dxch(ADDRESS_OF(FCODD));
    cs(ADDRESS_OF(TTHROT));   /* "THIS CODING ASSUMES A FLATOUT WITHIN" */
    ad_at(ADDRESS_OF(TIME1)); /* "80 SECONDS BEFORE FIRST THROTTLE CALL" */
    mask(POSMAX);
    com();
    ad(THREE_SECS);
    /* BZMF WHERETO: "BRANCH IF (TIME1-TTHROT +1) > 3 SECONDS" */
    if (!A_is_zero_or_minus()) {
        dca(ADDRESS_OF(FWEIGHT));
        das(ADDRESS_OF(FP));
    }
    whereto();
}

/* ---- The throttling bands -- pp. 793-794 ---- */

/* WHERETO: "THIS LOGIC DETERMINES THE THROTTLING IN THE REGION 10% - 94%."
 * The low and high critical thrusts, LOWCRIT and HIGHCRIT in E-bank 5,
 * go to L*WCR*T and H*GHCR*T (BUF, BUF +1), and PIFPSET to -0. Then:
 *   - FCOLD, the last command, above H*GHCR*T and FCODD above L*WCR*T:
 *     FCODD gets FP, so PIF will be zero ("SEE NOTE 1"), and PIFPSET
 *     FEXTRA (FLATOUT1);
 *   - FCOLD above, FCODD at or below L*WCR*T (FCOMPSET): PIFPSET gets
 *     -(FMAXODD - FP) ("SEE NOTE 2");
 *   - FCOLD at or below H*GHCR*T (LOWFCOLD), FCODD above it: "THROTTLE-UP",
 *     FCODD FMAXPOS and PIFPSET FEXTRA;
 *   - both at or below: DOPIF, PIFPSET left -0. */
static void whereto(void)
{
    set_A(EBANK5);     /* "INITIALIZE L*WCR*T AND H*GHCR*T FROM" */
    ts(EBANK_ADDRESS); /* "PAD LOADED ERASABLES IN W-MATRIX" */
    dca(ADDRESS_OF(LOWCRIT));
    dxch(ADDRESS_OF(L_STAR_WCR_STAR_T));
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    set_A(negate(ZERO)); /* "INITIALIZE PIFPSET" */
    ts(ADDRESS_OF(PIFPSET));
    cs(ADDRESS_OF(H_STAR_GHCR_STAR_T));
    ad_at(ADDRESS_OF(FCOLD));
    if (A_is_zero_or_minus()) { /* LOWFCOLD: "BRANCH IF FCOLD < OR = HIGHCRIT" */
        cs(ADDRESS_OF(H_STAR_GHCR_STAR_T));
        ad_at(ADDRESS_OF(FCODD));
        if (A_is_zero_or_minus()) { /* "BRANCH IF FC < OR = HIGHCRIT" */
            tc_to(DOPIF, FASTCHNG);
            return;
        }
        set_A(FMAXPOS); /* "NO:  THROTTLE-UP" */
        flatout1();
        return;
    }
    cs(ADDRESS_OF(L_STAR_WCR_STAR_T));
    ad_at(ADDRESS_OF(FCODD));
    if (A_is_zero_or_minus()) { /* FCOMPSET: "BRANCH IF FC < OR = LOWCRIT" */
        set_A(negate(FMAXODD));
        ad_at(ADDRESS_OF(FP));
        flatout2();
        return;
    }
    ca(ADDRESS_OF(FP));
    flatout1();
}

/* FLATOUT1: FCODD gets the thrust in A, and PIFPSET FEXTRA. */
static void flatout1(void)
{
    dxch(ADDRESS_OF(FCODD));
    set_A(FEXTRA);
    flatout2();
}

/* FLATOUT2: PIFPSET, and DOPIF's TC FASTCHNG, which an overflow at TS
 * PIFPSET skips. */
static void flatout2(void)
{
    if (ts(ADDRESS_OF(PIFPSET)))
        continue_at(DOPIF + 1u);
    else
        tc_to(DOPIF, FASTCHNG);
}

/* ---- The command -- pp. 794-796 ---- */

/* DOPIF, after FASTCHNG: FCOLD gets FCODD, and "PIF = FC - FP, NEVER
 * EQUALS +0". (DCA leaves A a word, so TS FCOLD never skips.) */
void dopif_after_fastchng(void)
{
    need_tc_return(fetch(ADDRESS_OF(RTNHOLD)));
    dca(ADDRESS_OF(FCODD));
    if (!ts(ADDRESS_OF(FCOLD))) dxch(ADDRESS_OF(PIF));
    dcs(ADDRESS_OF(FP));
    das(ADDRESS_OF(PIF));
    doit();
}

/* DOIT: PIF and PIFPSET to the engine: PSEUDO55 and THRUST, the drive's
 * count ("ADD IN PIFPSET, WITHOUT CHANGING PIF"; an overflow at TS
 * PSEUDO55 skips TS THRUST), channel 14 bit 4 to send it, and TTHROT the
 * time. Then FWEIGHT for the next pass:
 *
 *               PIF(PPROCESS + TL)     PIF /PIF/
 *     FWEIGHT = ------------------ + -------------
 *                     PGUID           2 PGUID FRATE
 *
 * "WHERE PPROCESS IS THE TIME BETWEEN PIPA READING AND THE START OF
 * THROTTLING", PGUID the guidance period (in P66, from THISTPIP +1 and
 * 2SECS; otherwise from PIPTIME +1 and 4SECS) and TL the engine's lag
 * (THROTLAG). And THDUMP: TC RTNHOLD, to the caller. */
static void doit(void)
{
    ca(ADDRESS_OF(PIF));
    ad_at(ADDRESS_OF(PIFPSET));
    if (ts(ADDRESS_OF(PSEUDO55)) || !ts(ADDRESS_OF(THRUST))) set_A(BIT4);
    wor_channel(CHAN14);
    ca(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TTHROT));
    ca(ADDRESS_OF(THISTPIP) + 1u); /* "INITIALIZE FWEIGHT COMP AS IF FOR P66" */
    ts(ADDRESS_OF(BUF));
    cs(ADDRESS_OF(MODREG)); /* "ARE WE IN FACT IN P66?" */
    ad(DEC66);
    if (A_is_zero()) { /* FWCOMP: "YES" */
        set_A(TWO_SECS);
    } else {
        ca(ADDRESS_OF(PIPTIME) + 1u); /* "NO:  INITIALIZE FOR TWO SECOND PERIOD" */
        ts(ADDRESS_OF(BUF));
        set_A(K4SECS);
    }
    /* FWCOMP +1 */
    ts(Q_ADDRESS);
    mp(BIT6);
    lxch(ADDRESS_OF(BUF) + 1u);
    cs(ADDRESS_OF(BUF)); /* "TIME OF LAST PIPA READING." */
    ad_at(ADDRESS_OF(TIME1));
    ad(THROTLAG); /* "COMPENSATE FOR ENGINE RESPONSE LAG" */
    mask(LOW8);   /* "MAKE SURE SMALL AND POSITIVE" */
    zl();
    dv_at(Q_ADDRESS);
    mp_at(ADDRESS_OF(PIF));
    double_A();
    dxch(ADDRESS_OF(FWEIGHT));
    /* |PIF|: CCS leaves |PIF| - 1, and AD ONE restores it. */
    switch (ccs_at(ADDRESS_OF(PIF))) {
    case POSITIVE:
    case NEGATIVE: ad(ONE); break;
    case IS_PLUS_ZERO:
    case IS_MINUS_ZERO: break;
    }
    mp_at(ADDRESS_OF(PIF));
    dv_at(ADDRESS_OF(BUF) + 1u);
    zl();
    das(ADDRESS_OF(FWEIGHT));
    tc_through(ADDRESS_OF(RTNHOLD)); /* THDUMP */
}

/* FLATOUT "THROTTLES UP THE DESCENT ENGINE, AND IS CALLED AS A BASIC
 * SUBROUTINE": BIT13, "4096 PULSES", to WHATOUT. */
void flatout(void)
{
    set_A(BIT13);
    whatout();
}

/* WHATOUT: the throttle setting in A to PIFPSET ("USE PIFPSET SO FWEIGHT
 * WILL BE ZERO"), FCOLD and PIF -0, the caller's return to RTNHOLD, and
 * DOIT. */
void whatout(void)
{
    need_tc_return(Q);
    if (!ts(ADDRESS_OF(PIFPSET))) set_A(negate(ZERO));
    ts(ADDRESS_OF(FCOLD));
    ts(ADDRESS_OF(PIF));
    qxch(ADDRESS_OF(RTNHOLD));
    doit();
}

/* ---- MASSMULT -- p. 796 ---- */

/* MASSMULT "SCALES ACCELERATION, ARRIVING IN A AND L IN UNITS OF 2(-4)
 * M/CS/CS, TO FORCE IN PULSE UNITS": the interpreter's DMP by MASS and by
 * SCALEFAC ("LEAVES PROPERLY SCALED FORCE IN MPAC"), TPAGREE, and the
 * force in A,L, or POSMAX when its high word is not zero. The return is
 * kept in BUF. */
void massmult(void)
{
    need_tc_return(Q);
    qxch(ADDRESS_OF(BUF));
    dxch(ADDRESS_OF(MPAC));
    tc_at(MASSMULT + 3u);
    dmp(); /* ADRES MASS */
    tc_at(MASSMULT + 5u);
    dmp(); /* ADRES SCALEFAC */
    tc_at(MASSMULT + 7u);
    tpagree();
    ca(ADDRESS_OF(MPAC));
    if (A_is_zero())
        dxch(ADDRESS_OF(MPAC) + 1u);
    else
        set_A(POSMAX);
    tc_through(ADDRESS_OF(BUF));
}

#endif
