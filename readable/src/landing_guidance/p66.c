/* SPDX-License-Identifier: MIT */
/* LUNAR_LANDING_GUIDANCE_EQUATIONS.agc, pp. 816-819 (Luminary099 only):
 * P66's rate of descent mode (bank 32). Once a second RODCOMP takes the
 * astronaut's clicks of the rate of descent switch (RODCOUNT, which
 * DESCBITS counts) into the desired altitude rate (VDGVERT), brings the
 * velocity up to date from the PIPAs, and from the error in altitude rate
 * the acceleration to command (/AFC/), held inside what the engine can give
 * (MAXFORCE, MINFORCE). THROTTLE +3 sends it to the engine, and the
 * horizontal velocity goes to the display (VHORIZ, noun 60). The faithful
 * C is c64-port/src/lm_llge.c. */
#include "landing_guidance.h"
#include "indexing.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"
#include "../interpreter/notation.h"

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* RODTASK: the second's task, which starts RODCOMP as a job (FINDVAC,
 * priority 22, "EBANK= DVCNTR", 2CADR RODCOMP), */
void rodtask(void)
{
    set_A(PRIO22);
    tc_to(RODTASK + 1u, FINDVAC);
}

/* and ends (TCF TASKOVER). */
void rodtask_after_findvac(void)
{
    continue_at(TASKOVER);
}

/* P66VERTA: "TERMINATE GROUP 3." (PHASCHNG, OCT 00003), */
void p66verta(void)
{
    tc_to(P66VERTA, PHASCHNG);
}

/* and RODTASK in a second (TWIDDLE, ADRES RODTASK). */
void p66verta_after_phaschng(void)
{
    set_A(ONE_SEC);
    tc_to(P66VERTA + 3u, TWIDDLE);
}

/* RODCOMP, with interrupts held: the switch's clicks since the last pass
 * (RODCOUNT, cleared) times RODSCAL1, "UPDATE DESIRED ALTITUDE RATE." The
 * PIPAs: "SET OLDPIPAX,Y,Z = PIPAX,Y,Z" and "SET RUPTREG1,2,3 = OLDPIPAX,Y,Z"
 * (the last pass's), and "SNAPSHOT TIME OF PIPA READING." (THISTPIP).
 * MPAC gets the counts since READACCS (PIPA + PIPATMP), DELVROD those since
 * the last pass (TEMX, Y and Z, READACCS's, cleared "SO WE WILL KNOW WHEN
 * READACCS CHANGES THEM."). A TS that overflows skips the word after it,
 * and leaves A +1 or -1 for the next sum. Then the velocity's
 * interpretive code (ITRPNT1_CODE). */
void rodcomp(void)
{
    inhint();
    set_A(ZERO);
    xch(ADDRESS_OF(RODCOUNT));
    mp_at(ADDRESS_OF(RODSCAL1));
    das(ADDRESS_OF(VDGVERT)); /* "UPDATE DESIRED ALTITUDE RATE." */
    dca(ADDRESS_OF(PIPAX));
    dxch(ADDRESS_OF(OLDPIPAX));
    dxch(ADDRESS_OF(RUPTREG1));
    ca(ADDRESS_OF(PIPAZ));
    xch(ADDRESS_OF(OLDPIPAZ));
    xch(ADDRESS_OF(RUPTREG3));
    dca(ADDRESS_OF(TIME2)); /* "SNAPSHOT TIME OF PIPA READING." */
    dxch(ADDRESS_OF(THISTPIP));

    ca(ADDRESS_OF(OLDPIPAX));
    ad_at(ADDRESS_OF(PIPATMPX));
    if (!ts(ADDRESS_OF(MPAC))) /* "MPAC(X) = PIPAX + PIPATMPX" */
        ca(ADDRESS_OF(OLDPIPAY));
    ad_at(ADDRESS_OF(PIPATMPY));
    if (!ts(ADDRESS_OF(MPAC) + 3u)) /* "MPAC(Y) = PIPAY + PIPATMPY" */
        ca(ADDRESS_OF(OLDPIPAZ));
    ad_at(ADDRESS_OF(PIPATMPZ));
    if (!ts(ADDRESS_OF(MPAC) + 5u)) /* "MPAC(Z) = PIPAZ + PIPATMPZ" */
        cs(ADDRESS_OF(OLDPIPAX));
    ad_at(ADDRESS_OF(TEMX));
    ad_at(ADDRESS_OF(RUPTREG1));
    if (!ts(ADDRESS_OF(DELVROD))) cs(ADDRESS_OF(OLDPIPAY));
    ad_at(ADDRESS_OF(TEMY));
    ad_at(ADDRESS_OF(RUPTREG2));
    if (!ts(ADDRESS_OF(DELVROD) + 2u)) cs(ADDRESS_OF(OLDPIPAZ));
    ad_at(ADDRESS_OF(TEMZ));
    ad_at(ADDRESS_OF(RUPTREG3));
    if (!ts(ADDRESS_OF(DELVROD) + 4u)) set_A(ZERO);

    /* "ZERO LO-ORDER MPAC COMPONENTS" and TEMX, TEMY and TEMZ (A +0, or +1
     * or -1 after an overflow: these TS never skip). */
    ts(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(MPAC) + 4u);
    ts(ADDRESS_OF(MPAC) + 6u);
    ts(ADDRESS_OF(TEMX));
    ts(ADDRESS_OF(TEMY));
    ts(ADDRESS_OF(TEMZ));
    set_A(negate(ONE));
    ts(ADDRESS_OF(MODE)); /* (a vector) */
    tc_at(ITRPNT1 - 1u);
    intpret();
}

/* ITRPNT1_CODE: the velocity, the altitude rate and the commanded
 * acceleration. The PIPA counts scaled to velocity "M/CS *2(-7)" (KPIP1),
 * and the time since PIPTIME, the last average-G pass ("30-31D CONTAINS
 * TIME IN CS SINCE PIPTIME"): V plus the counts, less the gravity and
 * PIPA bias over that time (GDT/2, VBIAS), is the velocity now ("STORE
 * UPDATED VELOCITY IN 24-29D"). Along R's unit vector it is HDOTDISP
 * ("UPDATE HDOTDISP RATE FOR NOUN 63."), and with the time, the altitude
 * HCALC1 ("UPDATE HCALC1 FOR NOUN 63."). The acceleration to command:
 * the error in altitude rate over TAUROD, plus gravity, over the thrust
 * axis's vertical part (UNITX through CDU*NBSM, DOT 14D), to /AFC/. Then
 * the thrust the PIPAs measured since the last pass (DELVROD), corrected
 * for the throttle's lag (LAG/TAU) and the weighting THROTTLE left
 * (FWEIGHT), which /AFC/ is limited by: above MAXFORCE / MASS or below
 * MINFORCE / MASS, the limit (BMN, BPL AFCSPOT). The numbers on the right
 * of the listing are the push-down list's depth after each line. */
static const word itrpnt1_words[] = {
    /* ITRPNT1 */ ORDERS(VXSC, PDDL), /* "SCALE MPAC TO M/CS *2(-7) AND PUSH" (6) */
    ADDRESS_PLUS_ONE(KPIP1),
    ADDRESS_PLUS_ONE(THISTPIP),
    ORDER(DSU),
    ADDRESS_PLUS_ONE(PIPTIME),
    STORE_D(30),       /* "30-31D CONTAINS TIME IN CS SINCE PIPTIME" */
    ORDERS(DDV, PDVL), /* (8) */
    ADDRESS_PLUS_ONE(4SEC_28),
    ADDRESS_PLUS_ONE(GDT_2),
    ORDERS(VSU, VXSC), /* (6) */
    ADDRESS_PLUS_ONE(VBIAS),
    ORDERS(VSL2, VAD),
    ADDRESS_PLUS_ONE(V),
    ORDERS(VAD, STADR),       /* (0) */
    AFTER_STADR(STOVL_D(24)), /* "STORE UPDATED VELOCITY IN 24-29D" */
    ADDRESS_PLUS_ONE(R),
    ORDER(UNIT),
    STORE_D(14),
    ORDERS(DOT, SL1),
    ADDRESS_PLUS_ONE_D(24),
    STODL(HDOTDISP), /* "UPDATE HDOTDISP RATE FOR NOUN 63." */
    ADDRESS_PLUS_ONE_D(30),
    ORDERS(SL, DMP),
    SL_COUNT(11),
    ADDRESS_PLUS_ONE(HDOTDISP),
    ORDERS(DAD, DSU),
    ADDRESS_PLUS_ONE_D(36),
    ADDRESS_PLUS_ONE(_LAND_),
    STODL(HCALC1), /* "UPDATE HCALC1 FOR NOUN 63." */
    ADDRESS_PLUS_ONE(HDOTDISP),
    ORDERS(BDSU, DDV),
    ADDRESS_PLUS_ONE(VDGVERT),
    ADDRESS_PLUS_ONE(TAUROD),
    ORDERS(PDVL, ABVAL), /* (2) */
    ADDRESS_PLUS_ONE(GDT_2),
    ORDERS(DDV, SR2),
    ADDRESS_PLUS_ONE(GSCALE),
    STORE_D(20),
    ORDER(DAD),         /* (0) */
    ORDERS(PDVL, CALL), /* (2) */
    ADDRESS_PLUS_ONE(UNITX),
    ADDRESS(CDU_STARNBSM),
    ORDER(DOT),
    ADDRESS_PLUS_ONE_D(14),
    STORE_D(22),
    ORDERS(BDDV, STADR), /* (0) */
    AFTER_STADR(STOVL(_AFC_)),
    ADDRESS_PLUS_ONE(DELVROD),
    ORDERS(VXSC, VAD),
    ADDRESS_PLUS_ONE(KPIP1),
    ADDRESS_PLUS_ONE(VBIAS),
    ORDERS(ABVAL, PDDL), /* (2) */
    ADDRESS_PLUS_ONE(THISTPIP),
    ORDERS(DSU, PDDL), /* (4) */
    ADDRESS_PLUS_ONE(LASTTPIP),
    ADDRESS_PLUS_ONE(THISTPIP),
    STODL(LASTTPIP),   /* (2) */
    ORDERS(DDV, BDDV), /* (0) */
    ADDRESS_PLUS_ONE(SHFTFACT),
    ORDERS(PDDL, DMP), /* (2) */
    ADDRESS_PLUS_ONE(FWEIGHT),
    ADDRESS_PLUS_ONE(BIT1H),
    ORDERS(DDV, DDV),
    ADDRESS_PLUS_ONE(MASS),
    ADDRESS_PLUS_ONE(SCALEFAC),
    ORDERS(DAD, PDDL), /* (4) */
    ADDRESS_PLUS_ONE_D(0),
    ADDRESS_PLUS_ONE_D(20),
    ORDERS(DDV, DSU), /* (2) */
    ADDRESS_PLUS_ONE_D(22),
    ORDERS(DMP, DAD),
    ADDRESS_PLUS_ONE(LAG_TAU),
    ADDRESS_PLUS_ONE(_AFC_),
    ORDERS(PDDL, DDV), /* (4) */
    ADDRESS_PLUS_ONE(MAXFORCE),
    ADDRESS_PLUS_ONE(MASS),
    ORDERS(PDDL, DDV), /* (6) */
    ADDRESS_PLUS_ONE(MINFORCE),
    ADDRESS_PLUS_ONE(MASS),
    ORDERS(PUSH, BDSU), /* (8) */
    ADDRESS_PLUS_ONE_D(2),
    ORDERS(BMN, DLOAD), /* (6) */
    ADDRESS(AFCSPOT),
    ORDERS(DLOAD, PUSH), /* (6) */
    ORDERS(BDSU, BPL),
    ADDRESS_PLUS_ONE_D(2),
    ADDRESS(AFCSPOT),
    ORDER(DLOAD),               /* (4) */
    /* AFCSPOT */ ORDER(DLOAD), /* (2), (4), OR (6) */
    ORDER(SETPD),               /* (2) */
    ADDRESS_PLUS_ONE_D(2),
    STODL(_AFC_), /* (0) */
    /* ITRPNT2 */ ORDER(EXIT),
};

/* After the EXIT: "MPAC = MEASURED ACCELERATION." for THROTTLE +3, which
 * throttles the engine to /AFC/ (BANKCALL, CADR THROTTLE +3), */
void itrpnt2_after_exit(void)
{
    dxch(ADDRESS_OF(MPAC));
    tc_to(ITRPNT2 + 2u, BANKCALL);
}

/* then the horizontal velocity (VHORCOMP_CODE). */
void itrpnt2_after_throttle(void)
{
    tc_at(ITRPNT2 + 4u);
    intpret();
}

/* VHORCOMP_CODE: "PICK UP UPDATED VELOCITY VECTOR." (24D; or, from P67,
 * V), plus DELVS: the velocity less its part along R, whose length is
 * VHORIZ. */
static const word vhorcomp_words[] = {
    ORDER(VLOAD), /* "PICK UP UPDATED VELOCITY VECTOR." */
    ADDRESS_PLUS_ONE_D(24),
    /* VHORCOMP */ ORDERS(VSL2, VAD),
    ADDRESS_PLUS_ONE(DELVS),
    ORDERS(VSR2, PDVL),
    ADDRESS_PLUS_ONE(R),
    ORDERS(UNIT, VXSC),
    ADDRESS_PLUS_ONE(HDOTDISP),
    ORDERS(VSL1, BVSU),
    ORDER(ABVAL),
    STORE(VHORIZ),
    ORDER(EXIT),
};

/* After the EXIT: "PUT UP V06N60 DISPLAY BUT AVOID PHASCHNG" (BANKCALL,
 * CADR DISPEXIT +3). */
void vhorcomp_after_exit(void)
{
    tc_to(VHORCOMP + 012u, BANKCALL);
}

const struct interpretive_code ITRPNT1_CODE = {032u, ITRPNT1, COUNT(itrpnt1_words), itrpnt1_words};
const struct interpretive_code VHORCOMP_CODE = {032u, ITRPNT2 + 5u, COUNT(vhorcomp_words),
                                                vhorcomp_words};

#endif
