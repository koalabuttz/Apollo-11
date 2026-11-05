/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 873-879 (Luminary099 only, bank 33): the landing's
 * part of each pass. HIGATASK, which starts the landing radar's move to
 * position 2 before high gate; MUNRETRN, where MUNRVG comes back to, and
 * the landing radar checks after it (35KCHK, XORCHK, HITEST, POS2CHK,
 * HIGATCHK, POS1CHK, CONTSERV); R29? and the altitude, altitude rate and
 * state for R10 (NOR29NOW, COPYCYC2); and ALTCHK, which compares the
 * altitude with a table of criteria. The faithful C is
 * c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* ---- HIGATASK -- p. 873 ---- */

/* HIGATASK: "IS ENTERED APPROXIMATELY 6 SECS PRIOR TO HIGATE DURING THE
 * DESCENT PHASE." "SET HIGATE AND LR INHIBIT FLAGS" (PRIO3, bits 11 and 10
 * of FLGWRD11), and "SET LR POSITIONING JOB (POS2)" (FINDVAC, HIGATJOB),
 * interrupts held; ... */
void higatask(void)
{
    inhint();
    set_A(negate(PRIO3));
    mask_at(ADDRESS_OF(FLGWRD11));
    ad(PRIO3);
    ts(ADDRESS_OF(FLGWRD11));
    set_A(PRIO32);
    tc_to(HIGATASK + 6u, FINDVAC);
}

/* ... and "CONTINUE SERVICER". */
void higatask_after_findvac(void)
{
    contserv();
}

/* ---- MUNRETRN -- pp. 874-875 ---- */

/* MUNRETRN: "THE RETURN LOC FROM SPECIAL AVE G ROUTINE (MUNRVG)", its
 * EXIT. */
static const word munretrn_words[] = {
    /* MUNRETRN */ ORDER(EXIT),
};

const struct interpretive_code MUNRETRN_CODE = {033u, MUNRETRN, COUNT(munretrn_words),
                                                munretrn_words};

/* After it: "BYPASS LR LOGIC IF BIT15 IS SET." (LRBYBIT, CS FLGWRD11: on to
 * COPYCYC1). Else "SEE IF ALT < 35000 FT LAST CYCLE" (READLBIT): "ALT WAS >
 * 35000 FT LAST CYCLE   CHK NOW" (35KCHK); "WERE WE BELOW 30000 FT LAST
 * PASS?" (XORFLBIT): "NO -- TEST THIS PASS" (XORCHK); else HITEST. */
void munretrn_after_exit(void)
{
    cs(ADDRESS_OF(FLGWRD11));
    mask(LRBYBIT);
    if (A_is_zero()) {
        copycyc1();
        return;
    }
    set_A(READLBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        thirty_five_kchk();
        return;
    }
    set_A(XORFLBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        xorchk();
        return;
    }
    hitest();
}

/* HITEST: "CHECK FOR HIGATE" (PSTHIBIT): "NOT AT HIGATE LAST CYCLE -- CHK
 * THIS CYCLE" (HIGATCHK); else POS2CHK. */
void hitest(void)
{
    set_A(PSTHIBIT);
    mask_at(ADDRESS_OF(FLGWRD11));
    if (A_is_zero()) {
        higatchk();
        return;
    }
    pos2chk();
}

/* POS2CHK: "VERIFY LR IN POS2" (bit 7 of channel 33 +0): "IT IS -- CHECK
 * FOR LR UPDATE" (UPDATCHK). Else with bit 13 of channel 12 +0, "LR NOT IN
 * POS2 OR REPOSITIONING -- BAD" (LRPOSALM); "LR BEING REPOSITIONED --
 * CONTINUE SERV". */
void pos2chk(void)
{
    set_A(BIT7);
    rand_channel(033u);
    if (A_is_zero()) {
        continue_at(UPDATCHK);
        return;
    }
    set_A(BIT13);
    rand_channel(012u);
    if (A_is_zero()) {
        lrposalm();
        return;
    }
    contserv();
}

/* HIGATCHK: "IS TTF > CRITERION?  (TTF IS NEGATIVE)" (TTF/8 plus
 * RPCRTIME): "NO" (POS1CHK). Else XNBPIP, "UXBXP IN GSOP CH5", from E bank
 * 4 ("MUST SWITCH EBANKS", the old one saved in L and restored): with
 * RPCRTQSW less it +0 or less, "IF UXBXP > QSW, THEN REPOSITION"
 * (HIGATASK); else POS1CHK. */
void higatchk(void)
{
    ca(ADDRESS_OF(TTF_8));
    ad_at(ADDRESS_OF(RPCRTIME));
    if (A_is_zero_or_minus()) {
        pos1chk();
        return;
    }
    set_A(EBANK4);
    xch(EBANK_ADDRESS);
    ts(L_ADDRESS);
    cs(ADDRESS_OF(XNBPIP));
    lxch(EBANK_ADDRESS);
    ad_at(ADDRESS_OF(RPCRTQSW));
    if (A_is_zero_or_minus()) {
        higatask();
        return;
    }
    pos1chk();
}

/* POS1CHK: "HIGATE NOT IN SIGHT -- DO POS1 CHK" (bit 6 of channel 33 +0):
 * "LR IN POS1 -- CHECK FOR LR UPDATE" (UPDATCHK); else LRPOSALM. */
void pos1chk(void)
{
    set_A(BIT6);
    rand_channel(033u);
    if (A_is_zero()) {
        continue_at(UPDATCHK);
        return;
    }
    lrposalm();
}

/* LRPOSALM: "LR NOT IN PROPER POS-ALARM-BYPASS UPDATE" "AND CONTINUE
 * SERVICER" (ALARM, OCT 511, which returns to CONTSERV). */
void lrposalm(void)
{
    tc_to(LRPOSALM, ALARM);
}

/* CONTSERV: "CLEAR LR MEASUREMENT MADE DISCRETES." (BITS4-7 of FLGWRD11),
 * interrupts held, and "SET LR LITES PROPERLY" (R12LITES, through
 * IBNKCALL), which returns to COPYCYC1. */
void contserv(void)
{
    inhint();
    set_A(negate(BITS4_7));
    mask_at(ADDRESS_OF(FLGWRD11));
    ts(ADDRESS_OF(FLGWRD11));
    tc_to(CONTSERV + 4u, IBNKCALL); /* CADR R12LITES */
}

/* ---- COPYCYC1, R29?, NOR29NOW, COPYCYC2 -- pp. 876-877 ---- */

/* COPYCYC1: the restart point (QUIKFAZ5), which returns to R29?. */
void copycyc1(void)
{
    tc_to(COPYCYC1, QUIKFAZ5);
}

/* R29?: "IS NOR29FLG OR READRFLG SET?" (NR29&RDR of FLAGWRD3): "YES, SO
 * DON'T DESIGNATE." (R29NODES). "IS THE RR NOT ZEROING ITS CDUS, AND IS
 * THE RENDEZVOUS RADAR IN AUTO MODE?" (OCT10002 of RADMODES +0): "NO, SO
 * DON'T DESIGNATE." "IS RR REPOSITIONING OR REMODING?" (PRIO22 of
 * RADMODES): "YES:  COME BACK IN 2 SECONDS & TRY AGAIN." (NOR29NOW). Else
 * R29. (The CCSs test MASK results: positive or +0.) */
void r29_q(void)
{
    ca(ADDRESS_OF(FLAGWRD3));
    mask(NR29_RDR);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        r29nodes();
        return;
    }
    ca(ADDRESS_OF(RADMODES));
    mask(OCT10002);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        r29nodes();
        return;
    }
    ca(ADDRESS_OF(RADMODES));
    mask(PRIO22);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        nor29now();
        return;
    }
    continue_at(R29);
}

/* R29NODES: "R29 NOT ALLOWED THIS CYCLE.", interrupts held: "SHOW THAT
 * DESIGNATION IS OFF." (DESIGBIT of RADMODES). */
void r29nodes(void)
{
    inhint();
    set_A(negate(DESIGBIT));
    mask_at(ADDRESS_OF(RADMODES));
    ts(ADDRESS_OF(RADMODES));
    nor29now();
}

/* NOR29NOW: to the interpreter ("INTPRET DOES A RELINT."), for
 * NOR29NOW_CODE. */
void nor29now(void)
{
    tc_at(NOR29NOW);
    intpret();
}

/* The altitude, HCALC and HCALC1 ("NEW HCALC*2(24)M."), the length of R1S
 * less /LAND/, and "ALTITUDE FOR R10 IN BIT UNITS." (ALTBITS, through
 * SGNAGREE); "DOWNRANGE HALF-UNIT VECTOR FOR R10." (UHZP); "TEMP. REF.
 * POSITION VECTOR*2(29)M." and "TEMP. REF. VELOCITY VECTOR 2(7) M/CS." (RN1
 * and VN1, R1S and V1S in the reference frame); and the altitude rate for
 * R10, from UNIT/R/ cross V1S, scaled by ARCONV1: COPYCYC2's EXIT "LEAVE
 * ALTITUDE RATE COMPENSATION IN MPAC". */
static const word nor29now_words[] = {
    /* NOR29NOW +1 */ ORDERS(VLOAD, ABVAL), /* "MPAC = ABVAL( NEW SM. POSITION VECTOR )" */
    ADDRESS_PLUS_ONE(R1S),
    ORDERS(PUSH, DSU),
    ADDRESS_PLUS_ONE(_LAND_),
    STORE(HCALC),
    STORE(HCALC1),
    ORDERS(DMPR, RTB),
    ADDRESS_PLUS_ONE(ALTCONV),
    ADDRESS(SGNAGREE),
    STOVL(ALTBITS),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VXV, UNIT),
    ADDRESS_PLUS_ONE(UHYP),
    STOVL(UHZP),
    ADDRESS_PLUS_ONE(R1S),
    ORDERS(VXM, VSR4),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(RN1),
    ADDRESS_PLUS_ONE(V1S),
    ORDERS(VXM, VSL1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STOVL(VN1),
    ADDRESS_PLUS_ONE(UNIT_R_),
    ORDERS(VXV, ABVAL),
    ADDRESS_PLUS_ONE(V1S),
    ORDERS(SL1, DSQ),
    ORDER(DDV),
    ORDERS(DMPR, RTB),
    ADDRESS_PLUS_ONE(ARCONV1),
    ADDRESS(SGNAGREE),
    /* COPYCYC2 */ ORDER(EXIT),
};

const struct interpretive_code NOR29NOW_CODE = {033u, NOR29NOW + 1u, COUNT(nor29now_words),
                                                nor29now_words};

/* After COPYCYC2's EXIT, interrupts held: "UPDATE RUNIT FOR R10." (UNIT/R/,
 * its three high words), "LOAD NEW DALTRATE FOR R10." (MPAC), R and V from
 * R1S and V1S, and "COMPLETE THE COPYCYCL." */
void copycyc2_after_exit(void)
{
    unsigned i;

    inhint();
    for (i = 0; i < 3u; ++i) {
        ca(ADDRESS_OF(UNIT_R_) + 2u * i);
        ts(ADDRESS_OF(RUNIT) + i);
    }
    ca(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(DALTRATE));
    for (i = 0; i < 6u; i += 2u) {
        dca(ADDRESS_OF(R1S) + i);
        dxch(ADDRESS_OF(R) + i);
    }
    for (i = 0; i < 6u; i += 2u) {
        dca(ADDRESS_OF(V1S) + i);
        dxch(ADDRESS_OF(V) + i);
    }
    copycycl();
}

/* ---- ALTCHK, XORCHK, 35KCHK -- pp. 878-879 ---- */

/* ALTCHK "COMPARES CURRENT ALTITUDE (IN HCALC) WITH A SPECIFIED ALTITUDE
 * FROM A TABLE BEGINNING AT ALTCRIT.", the criterion N words on (in A):
 * 25KFT, 50KFT, 50FT, 30KFT. The criterion less HCALC to MPAC +1 and +2,
 * and BRANCH +4 on it (and MPAC +2)... */
void altchk(void)
{
    if (A % 2u != 0u || A > 6u) not_modelled(); /* (N, its callers' 2 and 6, is one of these) */
    dca(indexed(ALTCRIT, A));
    dxch(ADDRESS_OF(MPAC) + 1u);
    dcs(ADDRESS_OF(HCALC));
    das(ADDRESS_OF(MPAC) + 1u);
    tc_to(ALTCHK + 7u, BRANCH + 4u);
}

/* ... positive (HCALC under the criterion): A +0 ("BETTER THAN A NOOP,
 * PERHAPS"), ... */
void altchk_after_branch(void)
{
    set_A(ZERO);
    altchk_after_branch_zero();
}

/* ... and with +0 too, the return a word on ("RETURN HERE IF HCALC < OR =
 * SPECIFIED CRITERION."); ... */
void altchk_after_branch_zero(void)
{
    incr(ADDRESS_OF(BUF2));
    altchk_after_branch_negative();
}

/* ... back through SWRETURN ("ALTCHK MUST BE BANKCALLED EVEN FROM ITS OWN
 * BANK."). */
void altchk_after_branch_negative(void)
{
    continue_at(SWRETURN);
}

/* XORCHK: "ARE WE BELOW 30000 FT?" (ALTCHK, 30KFT): ... */
void xorchk(void)
{
    set_A(SIX);
    tc_to(XORCHK + 1u, BANKCALL); /* CADR ALTCHK */
}

/* ... no: "CONTINUE LR UPDATE" (HITEST); ... */
void xorchk_after_altchk(void)
{
    hitest();
}

/* ... "YES: INHIBIT X-AXIS OVERRIDE" (XOVINFLG), and XORFLG set; ... */
void xorchk_below(void)
{
    tc_to(XORCHK + 4u, UPFLAG); /* ADRES XOVINFLG */
}

void xorchk_after_xovinflg(void)
{
    tc_to(XORCHK + 6u, UPFLAG); /* ADRES XORFLG */
}

/* ... and "CONTINUE LR UPDATE". */
void xorchk_after_xorflg(void)
{
    hitest();
}

/* 35KCHK: "ARE WE BELOW 35000 FT?" (ALTCHK, 50KFT): ... */
void thirty_five_kchk(void)
{
    set_A(TWO);
    tc_to(THIRTY_FIVE_KCHK + 1u, BANKCALL); /* CADR ALTCHK */
}

/* ... no: CONTSERV; ... */
void thirty_five_kchk_after_altchk(void)
{
    contserv();
}

/* ... yes: "SET READLR FLAG TO ENABLE LR READING.", ... */
void thirty_five_kchk_below(void)
{
    tc_to(THIRTY_FIVE_KCHK + 4u, UPFLAG); /* ADRES READLR */
}

/* ... and CONTSERV. */
void thirty_five_kchk_after_readlr(void)
{
    contserv();
}

#endif
