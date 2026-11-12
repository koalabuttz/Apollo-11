/* SPDX-License-Identifier: MIT */
/* IMU_MODE_SWITCHING_ROUTINES.agc, part 4: R02, the IMU status check (LM
 * pages 1332-1333, bank 04; CM pages 1443-1444, bank 07), and P06 and P05,
 * the computer to standby and back (LM pages 1334-1337, bank 37; CM pages
 * 1445-1448, bank 26). The faithful C is c64-port/src/imu_mode_switching.c.
 * */
#include "subroutines.h"

/* ======================================================================
 * R02
 * ====================================================================== */

/* R02ZERO -- LM pp. 1332-1333, CM p. 1444: the IMUSE flag up (UPFLAG), and
 * SWRETURN. P51-P53 call it through BANKCALL as well. */
void r02zero(void)
{
    tc_at(R02ZERO); /* TC UPFLAG; ADRES IMUSE */
    continue_at(UPFLAG);
}

/* R02ZERO +2 -- LM p. 1333, CM p. 1444, where UPFLAG returns: TCF
 * SWRETURN. */
void r02zero_after_upflag(void)
{
    continue_at(SWRETURN);
}

/* R02BOTH -- LM p. 1332, CM p. 1443.
 * "TO CHECK WHETHER IMU IS ON AND IF ON WHETHER IT IS ALIGNED TO AN
 * ORIENTATION KNOWN BY THE CMC.": with the REFSMMAT flag up, R02ZERO.
 * Otherwise an alarm, 210 if the IMU is not "INITIALIZED" (IMODES30 bit
 * 9), 220 if REFSMMAT is not known, and GOTOPOOH. */
void r02both(void)
{
#if LUMINARY099
    set_A(REFSMBIT);
    mask_at(ADDRESS_OF(FLAGWRD3));
#else
    set_A(BIT13);
    mask_at(ADDRESS_OF(STATE) + 3u); /* FLAGWRD3 */
#endif
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at((word)(R02BOTH + 3u)); /* TC R02ZERO: "ZERO IMUS" */
        r02zero();
        return;
    }
    ca(ADDRESS_OF(IMODES30));
    mask(BIT9);
    if (!A_is_zero()) set_A(negate(BIT4)); /* EXTEND; BZF +2; CS BIT4 */
    ad(OCT220);
    tc_at((word)(R02BOTH + 012u)); /* TC VARALARM */
    continue_at(VARALARM);
}

/* R02BOTH +013 -- LM p. 1332, CM p. 1443, where VARALARM returns: TC
 * GOTOPOOH. */
void r02both_after_varalarm(void)
{
    tc_at((word)(R02BOTH + 013u));
    continue_at(GOTOPOOH);
}

/* ======================================================================
 * P06 and P05
 * ====================================================================== */

/* P06 -- LM p. 1335, CM p. 1446: "TRANSFER THE ISS/CMC FROM THE OPERATE
 * TO THE STANDBY CONDITION." "SET NODOV37 BIT", and on to PRESTAND. */
void p06(void)
{
    tc_at(P06); /* TC UPFLAG; ADRES NODOFLAG */
    continue_at(UPFLAG);
}

/* SCALPREP -- LM p. 1336, CM p. 1447.
 * The caller's return to MPAC +2, and the scaler (FINETIME +1). */
void scalprep(void)
{
    qxch(ADDRESS_OF(MPAC) + 2u);
    tc_at((word)(SCALPREP + 2u)); /* TC FINETIME +1 */
    continue_at((word)(FINETIME + 1u));
}

/* PRESTAND -- LM p. 1335, CM p. 1446.
 * "PRESTAND PREPARES FOR STANDBY BY SNAPSHOTTING THE SCALER AND TIME1
 * TIME2": "SNAPSHOT TIME1TIME2", and SCALPREP. */
void prestand(void)
{
    inhint();
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(TIME2SAV));
    tc_at((word)(PRESTAND + 4u)); /* TC SCALPREP */
    scalprep();
}

/* PRESTAND +5 -- LM p. 1335, CM p. 1446, SCALPREP's L+1: "T1,T2,SCALER NOT
 * COMPATIBLE". Again. */
void prestand_retry(void)
{
    tc_at((word)(PRESTAND + 5u)); /* TC PRESTAND */
    prestand();
}

/* PRESTAND +6 -- LM p. 1335, CM p. 1446, SCALPREP's L+2: "T1,T2 AND SCALER
 * OK". "STORE SCALER", and RNDREFDR: "REFSMM, DRIFT, TRACK FLAGS DOWN". */
void prestand_compatible(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(SCALSAVE));
    inhint();
    tc_at((word)(PRESTAND + 011u)); /* TC BANKCALL; CADR RNDREFDR */
    continue_at(BANKCALL);
}

/* PRESTAND +013 -- LM p. 1335, CM p. 1446, where RNDREFDR returns: "IMUSE
 * DOWN". */
void prestand_after_rndrefdr(void)
{
    tc_at((word)(PRESTAND + 013u)); /* TC DOWNFLAG; ADRES IMUSE */
    continue_at(DOWNFLAG);
}

/* PRESTAND +015 -- LM p. 1335, CM p. 1446, where DOWNFLAG returns: "RNDVZFLG
 * DOWN". */
void prestand_after_downflag(void)
{
    tc_at((word)(PRESTAND + 015u)); /* TC DOWNFLAG; ADRES RNDVZFLG */
    continue_at(DOWNFLAG);
}

/* PRESTAND +017 -- LM p. 1335, CM p. 1446, where DOWNFLAG returns: "SET
 * STANDBY ENABLE BIT" (channel 13 bit 11), and "SET RESTART TO POSTAND WHEN
 * STANDBY RECOVERS". */
void prestand_after_downflag2(void)
{
    set_A(BIT11);
    wor_channel(CHAN13);
    tc_at((word)(PRESTAND + 022u)); /* TC PHASCHNG; OCT 07024; OCT 20000; 2CADR POSTAND */
    continue_at(PHASCHNG);
}

/* PRESTAND +027 -- LM p. 1335, CM p. 1446, where PHASCHNG returns: GOPERF1
 * with OCT62 ("DEC 50"), again whichever of its three returns comes back. */
void prestand_after_phaschng(void)
{
    set_A(OCT62);
    tc_at((word)(PRESTAND + 030u)); /* TC BANKCALL; CADR GOPERF1 */
    continue_at(BANKCALL);
}

/* PRESTAND +032 -- LM p. 1335, CM p. 1446 to +034, GOPERF1's returns: TCF
 * -3, TCF -4, TCF -5. */
void prestand_after_goperf1(void)
{
    prestand_after_phaschng();
}

/* SCALPREP +3 -- LM p. 1336, CM p. 1447, where FINETIME returns, with the
 * scaler in A,L. "ADD 5 MS TO THE SCALER READING." and "SET LOW 5 BITS OF
 * (SCALER+5MS) TO ZERO AND STORE RESULTS IN MPAC,+1." "IS IT 1ST INTERVAL
 * AFTER T1 INCREMENT": if not, the caller's L+2, if so L+1 (TC MPAC +2). */
void scalprep_after_finetime(void)
{
    relint();
    dxch(ADDRESS_OF(MPAC));
    set_A(BIT5); /* CA BIT5 */
    ts(L_ADDRESS);
    set_A(ZERO); /* CA ZERO */
    das(ADDRESS_OF(MPAC));
    set_A(negate(LOW5)); /* CS LOW5 */
    mask_at(ADDRESS_OF(MPAC) + 1u);
    xch(ADDRESS_OF(MPAC) + 1u);
    mask(LOW5);
    if (ccs_register(A_ADDRESS) == POSITIVE) incr(ADDRESS_OF(MPAC) + 2u); /* "NO" */
    tc_through(ADDRESS_OF(MPAC) + 2u);                                    /* "YES" */
}

/* POSTAND +3 -- LM p. 1336, CM p. 1447: "CLEAR TIME1TIME2", and
 * SCALPREP. */
static void postand_plus3(void)
{
    inhint();
    set_A(ZERO); /* CA ZERO */
    ts(L_ADDRESS);
    dxch(ADDRESS_OF(TIME2));
    tc_at((word)(POSTAND + 7u)); /* TC SCALPREP */
    scalprep();
}

/* POSTAND -- LM p. 1336, CM p. 1447: the restart PRESTAND set.
 * "RECOVER TIME AFTER STANDBY.": "CLEAR STANDBY ENABLE BIT", then
 * POSTAND +3. */
void postand(void)
{
    set_A(negate(BIT11)); /* CS BIT11 */
    wand_channel(CHAN13);
    postand_plus3();
}

/* POSTAND +010 -- LM p. 1336, CM p. 1447, SCALPREP's L+1: "T1,T2,SCALER NOT
 * COMPATIBLE". Again. */
void postand_retry(void)
{
    tc_at((word)(POSTAND + 010u)); /* TC POSTAND +3 */
    postand_plus3();
}

/* POSTAND +011 -- LM p. 1336, CM p. 1447, SCALPREP's L+2: "FORM DP
 * DIFFERENCE OF POSTSTANDBY SCALER" "MINUS PRESTANDBY SCALER AND SHIFT RIGHT
 * 5 TO ALIGN BITS WITH TIME1TIME2." (SHORTMP by BIT10). */
void postand_compatible(void)
{
    dcs(ADDRESS_OF(SCALSAVE));
    das(ADDRESS_OF(MPAC));
    set_A(BIT10);
    tc_at((word)(POSTAND + 015u)); /* TC SHORTMP */
    continue_at(SHORTMP);
}

/* POSTAND +016 -- LM p. 1337, CM p. 1448, where SHORTMP returns: "MAKE DP
 * DIFF AGREE" (TPAGREE, MPAC +2 "NEEDED FOR TP AGREE"). */
void postand_after_shortmp(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 2u);
    tc_at((word)(POSTAND + 020u)); /* TC TPAGREE */
    continue_at(TPAGREE);
}

/* POSTCOM -- LM p. 1337, CM p. 1448: "C(MPAC,+1) IS MAGNITUDE OF DELTA
 * SCALER." The "PRESTANDBY TIME1TIME2" added, and "FORCE SIGN
 * AGREEMENT". */
static void postcom(void)
{
    dca(ADDRESS_OF(TIME2SAV));
    das(ADDRESS_OF(MPAC));
    tc_at((word)(POSTCOM + 3u)); /* TC TPAGREE */
    continue_at(TPAGREE);
}

/* POSTAND +021 -- LM p. 1337, CM p. 1448, where TPAGREE returns.
 * "IF DP DIFF NET +, NO SCALER OVERFLOW BETWEEN PRE AND POST STANDBY."
 * "IF DP DIFF NET -, SCALER OVERFLOWED. ADD BIT 10 TO HIGH DIFF TO
 * CORRECT." */
void postand_after_tpagree(void)
{
    switch (ccs_at(ADDRESS_OF(MPAC))) {
    case POSITIVE: tc_at((word)(POSTAND + 022u)); break;     /* TC POSTCOM */
    case IS_PLUS_ZERO: tc_at((word)(POSTAND + 023u)); break; /* TC POSTCOM */
    case NEGATIVE:
        tc_at((word)(POSTAND + 024u)); /* TC +1 */
        /* fall through */
    case IS_MINUS_ZERO:
        set_A(BIT10);
        ads(ADDRESS_OF(MPAC));
        break;
    }
    postcom();
}

/* POSTCOM +4 -- LM p. 1337, CM p. 1448, where TPAGREE returns: the "UPDATED
 * VALUE FOR T1,T2." added into TIME2, TIME1, and "CLEAR NODOFLAG". */
void postcom_after_tpagree(void)
{
    dxch(ADDRESS_OF(MPAC));
    das(ADDRESS_OF(TIME2));
    tc_at((word)(POSTCOM + 6u)); /* TC DOWNFLAG; ADRES NODOFLAG */
    continue_at(DOWNFLAG);
}

/* POSTCOM +010 -- LM p. 1337, CM p. 1448, where DOWNFLAG returns: TC
 * GOTOPOOH. */
void postcom_after_downflag(void)
{
    tc_at((word)(POSTCOM + 010u));
    continue_at(GOTOPOOH);
}
