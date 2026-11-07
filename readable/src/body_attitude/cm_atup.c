/* SPDX-License-Identifier: MIT */
/* CM_BODY_ATTITUDE.agc, pp. 887-889 (Comanche055, bank 37): the "BASIC
 * SUBROUTINE TO UPDATE ATTITUDE ANGLES" (CM/ATUP, REDOPOSE), the end of
 * the pass (CM/POSE3) and CORANGOV. The faithful C is
 * c64-port/src/cm_body_attitude.c. */
#include "body_attitude.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* ---- CORANGOV -- pp. 888-889 ---- */

/* TC CORANGOV at `at`, "CORRECT FOR OVFL IF ANY": an angle (over 180, in
 * A) that overflowed comes back by LIMITS -1 or LIMITS +1 (as +1 or -1 the
 * overflow left in A indexes it: "COSTS 2 MCT TO USE.  SEE ANGOVCOR."),
 * added into L; the angle is in L either way. Then TC Q, back to the word
 * after the TC, which leaves Q 3. */
static void corangov(word at)
{
    tc_at(at);
    if (ts(L_ADDRESS)) {                           /* TS L skips the TC Q on an overflow */
        ca(indexed(ADDRESS_OF(LIMITS), a_word())); /* INDEX A; CA LIMITS */
        ads(L_ADDRESS);
    }
    Q = 3u; /* TC Q */
}

/* ---- CM/ATUP -- pp. 887-888 ---- */

/* CM/ATUP, after CM/POSE's EXIT (EBANK AOG's, EBAOG): GAMA from 10D,
 * negated, the old GAMA kept in L; inhibited "UNTIL UPDATE OF BODY"
 * "ANGLES, SO THAT GAMDIFSW IS VALID FIRST PASS" "INDICATOR.". GAMDIFSW
 * ("GAMDIFSW=94D BIT11", "INITLY=0") clear: set it and GAMDOT +0, "DONT
 * CALC GAMA DOT UNTIL HAVE FORMD" "ONE DIFFERENCE." Else DOGAMDOT: "DEL
 * GAMA/360= T GAMDOT/360", times TCDU ("TCDU = .1 SEC, T = 2 SEC."):
 * GAMDOT, "GAMA DOT TCDU / 180", kept if its size is 5 or more, else
 * +0: "IGNORE GAMDOT IF LEQ .5 DEG/SEC", "SET GAMDOT=+0 AS TAG IF TOO
 * SMALL." (Not modelled: a work area whose 10D is in fixed memory, or is
 * Z, which no program has; the faithful port refuses it too.) */
static void gamdot(void)
{
    unsigned at;

    set_A(EBAOG);
    ts(EBANK_ADDRESS);
    /* CMTR1 */ at = indexed(10u, FIXLOC);               /* INDEX FIXLOC; CS 10D: "(GAMA/180)/2" */
    if (at >= 02000u || at == Z_ADDRESS) not_modelled(); /* a work area no program has */
    cs(at);
    xch(ADDRESS_OF(GAMA));
    ts(L_ADDRESS);
    inhint();
    cs(ADDRESS_OF(CM_FLAGS));
    mask(BIT11);                   /* "GAMDIFSW=94D BIT11" */
    if (!A_is_zero()) {            /* EXTEND; BZF DOGAMDOT: "IS OK, GO ON." */
        ads(ADDRESS_OF(CM_FLAGS)); /* "KNOW BIT IS 0" */
        tc_at(CMTR1 + 012u);       /* TC NOGAMDOT: "SET GAMDOT = 0" */
    } else {
        /* DOGAMDOT */ cs(L_ADDRESS);
        ad_at(ADDRESS_OF(GAMA));          /* "DEL GAMA/360= T GAMDOT/360" */
        mp(TCDU);                         /* "TCDU = .1 SEC, T = 2 SEC." */
        ts(ADDRESS_OF(GAMDOT));           /* "GAMA DOT TCDU / 180" */
        if (!A_is_zero_or_minus()) com(); /* EXTEND; BZMF +2; COM */
        ad(FIVE);
        if (A_is_zero_or_minus()) return; /* EXTEND; BZMF +3 */
    }
    /* NOGAMDOT */ set_A(0u); /* "COME HERE INHINTED." */
    ts(ADDRESS_OF(GAMDOT));
}

/* Then "UPDATE ANGLES BY CORRECTING EUILER ANG" "FOR ACCRUED INCREMENT
 * SINCE PIPUP", "R = R EUIL + R(NOW) -R(PIPUP)", for the roll and ALFA
 * (each sum CORANGOV'd twice: "ONLY SINGLE OVFL POSSIBLE.") and BETA
 * ("OVFL NOT EXPECTED."), into TEMPROLL, TEMPALFA and TEMPBETA; REPOSADR
 * into PHSNAME5 ("THIS ASSUMES THAT THE" TC PHASCHNG "OCT 10035" "IS NOT
 * CHANGED IN" "SERVICER."), so a restart redoes REDOPOSE; on into it. */
void cm_atup(void)
{
    gamdot();
    cs(ADDRESS_OF(MPAC)); /* "GET (R EUL/180) /2" */
    double_A();           /* "POSSIBLE OVERFLOW" */
    corangov(CMTR1 + 032u);
    su_at(ADDRESS_OF(ROLL_PIP)); /* "GET INCR SINCE PIPUP" */
    ad_at(ADDRESS_OF(ROLL_180)); /* "ONLY SINGLE OVFL POSSIBLE." */
    corangov(CMTR1 + 036u);
    ts(ADDRESS_OF(TEMPROLL));
    cs(ADDRESS_OF(MPAC) + 2u); /* "GET (ALFA EUL/180) /2" */
    double_A();                /* "NEEDED FOR EXT ATM DAP" */
    corangov(CMTR1 + 042u);
    su_at(ADDRESS_OF(ALFA_PIP));
    ad_at(ADDRESS_OF(ALFA_180));
    corangov(CMTR1 + 046u);
    ts(ADDRESS_OF(TEMPALFA));
    cs(ADDRESS_OF(MPAC) + 1u); /* "GET (BETA EUL/180) /2" */
    /* CMTR2 */ double_A();
    su_at(ADDRESS_OF(BETA_PIP));
    ad_at(ADDRESS_OF(BETA_180));
    xch(ADDRESS_OF(TEMPBETA)); /* "OVFL NOT EXPECTED." */
    set_A(EBANK3);
    ts(EBANK_ADDRESS);
    dca(ADDRESS_OF(REPOSADR));
    dxch(ADDRESS_OF(PHSNAME5));
    set_A(EBAOG);
    ts(EBANK_ADDRESS);
    redopose();
}

/* REDOPOSE, "RE-STARTS COME HERE": the angles into ROLL/180, ALFA/180 and
 * BETA/180 at once, interrupts allowed again, and back into the
 * interpreter ("CANT TC DANZIG AFTER PHASCHNG."). */
void redopose(void)
{
    dca(ADDRESS_OF(TEMPROLL));
    dxch(ADDRESS_OF(ROLL_180));
    ca(ADDRESS_OF(TEMPBETA));
    ts(ADDRESS_OF(BETA_180));
    relint();
    tc_at(REDOPOSE + 6u);
    intpret();
}

/* ---- CM/POSE3 -- p. 888 ---- */

/* CM/POSE3, "RETURN FROM CM/ATUP.": the speed (VN, "2(-7) M/CS") to
 * VMAGI, "FOR DISPLAY ON CALL.", and on at POSEXIT: "ENDEXIT, STARTENT, OR
 * SCALEPOP." */
static const word cm_pose3_words[] = {
    /* CM/POSE3 */ ORDERS(VLOAD, ABVAL), /* "RETURN FROM CM/ATUP." */
    ADDRESS_PLUS_ONE(VN),                /* "2(-7) M/CS" */
    STORE(VMAGI),                        /* "FOR DISPLAY ON CALL." */
    ORDER(GOTO),
    ADDRESS(POSEXIT), /* "ENDEXIT, STARTENT, OR SCALEPOP." */
};

const struct interpretive_code CM_POSE3_CODE = {037u, CM_POSE3, COUNT(cm_pose3_words),
                                                cm_pose3_words};

#endif
