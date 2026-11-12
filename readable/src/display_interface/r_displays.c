/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 4: the perform displays, the R
 * displays and the copies of a display's words (LM pages 1360-1363, CM pages
 * 1471-1474), bank 10. The faithful C is c64-port/src/lanec_di_*.c.
 *
 * "ALL ROUTINES THAT END IN R HAVE AN IMMEDIATE RETURN TO THE USER." and
 * "ALL ROUTINES THAT END IN R START A SEPARATE JOB (MAKEPLAY) WITH USER'S
 * JOB PRIORITY." GODSPRS1 starts that job, BOTHJOBS gives it the user's
 * words, and the user goes on at the call +4 (+0 for GODSPR). */
#include "display_interface.h"

/* EXDSPRET -- LM p. 1360, CM p. 1471: "EXDSPRET IS USED TO DISPLAY A VERB
 * NOUN ARRIVING IN A WITH A RETURN MADE TO THE USER AFTER THE DISPLAY HAS
 * BEEN" sent out (bits 15 and 6). */
void exdspret(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT15_PLUS_6);
    goflash2();
}

/* GOPERFS -- LM p. 1360, CM p. 1471: "LEAVE ONLY FLASH, PERFORM,
 * BLANKING". */
void goperfs(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), PERFMASK);
    goflash2();
}

/* GOPERF1 -- LM p. 1360, CM p. 1471: "STORE DESIRED CHECKLIST VALUE", and
 * V01N25, "USED TO DISPLAY CHECKLIST VALUE IN R1". */
void goperf1(void)
{
    ts_caf(ADDRESS_OF(NORMTEM1), V01N25);
    goperfs();
}

/* GOPERF2 -- LM p. 1360, CM p. 1471: "DESIRED VERB-NOUN TO DISPLAY
 * R1,R2,R3". */
void goperf2(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), PERF2MSK);
    goflash2();
}

/* PURRS4 -- LM p. 1362, CM p. 1473: "DESIRED OPTION CODE" to OPTION1,
 * V04N06 and PERF4MSK ("FLASH, PERFORM AND EBANK R3"), and TC Q. */
static void purrs4(void)
{
    ts_caf(ADDRESS_OF(OPTION1), V04N06);
    ts_caf(ADDRESS_OF(PLAYTEM1), PERF4MSK);
    tc_through(Q_ADDRESS);
}

/* GOPERF4 -- LM p. 1361, CM p. 1471: PURRS4, and GOFLASH2. */
void goperf4(void)
{
    tc_at(GOPERF4);
    purrs4();
    goflash2();
}

/* VACDSP -- LM p. 1361, CM p. 1472: a flashing R display's job has a VAC
 * area: SPVAC, with the user's bank and superbank in L and MAKEPLAY's
 * address in A (MAKEGEN); SPVAC returns to BOTHJOBS. */
static void vacdsp(void)
{
    ca(BBANK_ADDRESS);
    ror_superbnk();
    ts(L_ADDRESS);
    set_A(MAKEGEN);
    tc_at((word)(VACDSP + 5u)); /* TC SPVAC */
    continue_at(SPVAC);
}

/* GODSPRS1 -- LM p. 1361, CM p. 1472: "IMMEDIATE RETURN IS CALL CADR +4"
 * (A, to RUPTREG3). "MAKE DISPLAY ONE HIGHER THAN USER": the user's
 * priority to NEWPRIO. "IS THIS A FLASHING R DISPLAY": "YES, MAKE DSPLAY
 * JOB A VAC"; "NO, MAKE DSPLAY JOB A NOVAC" (2CADR MAKEPLAY). */
void godsprs1(void)
{
    inhint();
    if (!ts(ADDRESS_OF(RUPTREG3))) ca(ADDRESS_OF(PRIORITY));
    mask(PRIO37);
    if (!ts(ADDRESS_OF(NEWPRIO))) ca(ADDRESS_OF(PLAYTEM4));
    mask(BIT4);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        vacdsp();
        return;
    }
    ca(ADDRESS_OF(NEWPRIO));
    tc_at((word)(GODSPRS1 + 012u)); /* TC NOVAC; 2CADR MAKEPLAY */
    continue_at(NOVAC);
}

/* GODSPRS -- LM p. 1361, CM p. 1472: the flags to PLAYTEM4, and GODSPRS1
 * with A 3, the offset of the immediate return from the CADR +1. */
void godsprs(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM4), THREE);
    godsprs1();
}

/* GOFLASHR -- LM p. 1361, CM p. 1471: "LEAVE ONLY FLASH BIT SET". */
void goflashr(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), BIT4);
    godsprs();
}

/* SAVELOCR -- LM p. 1362, CM p. 1473: the return to L, and MAKECADR (the
 * user's CADR), which comes back to SAVELOCS +7. */
static void savelocr(void)
{
    lxch(Q_ADDRESS);
    tc_at((word)(SAVELOCR + 1u)); /* TC MAKECADR */
    continue_at(MAKECADR);
}

/* SAVELOCS -- LM p. 1362, CM p. 1473: the "EBANK BITS" in the flags
 * (PLAYTEM4), and SAVELOCR. (An overflow in A skips the LXCH Q.) */
void savelocs(void)
{
    inhint();
    set_A(negate(OCT3400));
    mask_at(ADDRESS_OF(PLAYTEM4));
    ad_at(EBANK_ADDRESS);
    if (ts(ADDRESS_OF(PLAYTEM4))) {
        tc_at((word)(SAVELOCR + 1u)); /* TC MAKECADR */
        continue_at(MAKECADR);
        return;
    }
    savelocr();
}

/* BOTHJOBS -- LM p. 1361, CM p. 1472: where SPVAC returns, the new job's
 * register set at LOCCTR. SAVELOCS, which comes back to BOTHJOBS +1. */
void bothjobs(void)
{
    tc_at(BOTHJOBS);
    savelocs();
}

/* GODSPRS1 +015 -- LM p. 1361, CM p. 1472, where NOVAC returns: TCF
 * BOTHJOBS. */
void godsprs1_after_novac(void)
{
    bothjobs();
}

/* BOTHJOBS +1 -- LM p. 1361, CM p. 1472: "SAVE NVWORD AND" the user's "MPAC
 * +2", and the "CADR, FLAGS AND EBANK" in the new job's MPAC +1 to +4, and
 * its register set offset in the user's MPAC +5 (for BLANKET). SAVELOCR,
 * which comes back to BOTHJOBS +014: RELINT, and BANKJUMP to the "CALL CADR
 * +4". */
static void bothjobs_plus1(void)
{
    word job = fetch(ADDRESS_OF(LOCCTR));

    dca(ADDRESS_OF(MPAC) + 1u);
    dxch(indexed(ADDRESS_OF(MPAC) + 1u, job));
    dca(ADDRESS_OF(MPAC) + 3u);
    dxch(indexed(ADDRESS_OF(MPAC) + 3u, job));
    ca(ADDRESS_OF(LOCCTR));
    ts(ADDRESS_OF(MPAC) + 5u);
    tc_at((word)(BOTHJOBS + 013u)); /* TC SAVELOCR */
    savelocr();
}

/* SAVELOCS +7 -- LM p. 1362, CM p. 1473, where MAKECADR returns: the CADR to
 * PLAYTEM3, plus the immediate return's offset (RUPTREG3, "NOT USED FOR NON
 * R ROUTINES"), and TC L back to the caller of SAVELOCS: GOFLASH2 +2
 * (RELINT, MAKEPLAY), BOTHJOBS +1, or BOTHJOBS +014 (RELINT, BANKJUMP). (An
 * overflow in A skips the AD.) */
void savelocs_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(PLAYTEM3))) ad_at(ADDRESS_OF(RUPTREG3));
    tc_through(L_ADDRESS);
    if (Z == GOFLASH2 + 2u) {
        relint();
        continue_at(MAKEPLAY);
    } else if (Z == BOTHJOBS + 1u) {
        bothjobs_plus1();
    } else if (Z == BOTHJOBS + 014u) {
        relint();
        continue_at(BANKJUMP);
    } else {
        not_modelled();
    }
}

/* GOPERFRS -- LM p. 1362, CM p. 1473. */
void goperfrs(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), PERFMASK);
    godsprs();
}

/* GOPERF1R -- LM p. 1362, CM p. 1473: "DESIRED CHECKLIST VALUE", and
 * V01N25. */
void goperf1r(void)
{
    ts_caf(ADDRESS_OF(NORMTEM1), V01N25);
    goperfrs();
}

/* GOPERF2R -- LM p. 1362, CM p. 1473. */
void goperf2r(void)
{
    ts_caf(ADDRESS_OF(PLAYTEM1), PERF2MSK);
    godsprs();
}

/* GOPERF4R -- LM p. 1362, CM p. 1473: PURRS4, and GODSPRS. */
void goperf4r(void)
{
    tc_at(GOPERF4R);
    purrs4();
    godsprs();
}

/* COPIES2 -- LM p. 1362, CM p. 1473: the display's words into the set of
 * its kind (COPINDEX): the flags (PLAYTEM4) to DSPFLG, called EBANKSAV
 * here; the CADR (PLAYTEM3) to CADRFLSH for a flashing display or a
 * GODSPRET (CADRMASK), else SKIPADD; the "VERB NOUN" (PLAYTEM1) to NVWORD.
 * RELINTQ. */
void copies2(void)
{
    word kind = fetch(ADDRESS_OF(COPINDEX));

    inhint();
    ca(ADDRESS_OF(PLAYTEM4));
    ts(indexed(ADDRESS_OF(EBANKSAV), kind));
    mask(CADRMASK);
    if (!A_is_zero()) {
        ca(ADDRESS_OF(PLAYTEM3));
        ts(indexed(ADDRESS_OF(CADRFLSH), kind));
    }
    ca(ADDRESS_OF(PLAYTEM1)); /* SKIPADD */
    ts(indexed(ADDRESS_OF(NVWORD), kind));
    relintq();
}

/* COPIES -- LM p. 1362, CM p. 1473: the kind in A to COPINDEX, and
 * COPIES2. (An overflow in A would skip COPIES2's INHINT, which RELINTQ
 * undoes anyway.) */
void copies(void)
{
    ts(ADDRESS_OF(COPINDEX));
    copies2();
}

/* COPYNORM -- LM p. 1362, CM p. 1473: COPIES for the PRIO display (0). */
void copynorm(void)
{
    set_A(ZERO);
    copies();
}
