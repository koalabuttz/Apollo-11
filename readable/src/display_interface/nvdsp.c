/* SPDX-License-Identifier: MIT */
/* DISPLAY_INTERFACE_ROUTINES.agc, part 6: PINBRNCH and the display going out
 * (NVDSP, GOPLAY), and WITCHONE and XCHNYLOC (LM pages 1364-1367, CM pages
 * 1475-1478), bank 10. The faithful C is c64-port/src/lanec_di_*.c.
 *
 * NVDSP puts the kind's verb and noun out by NVMONOPT, then for a
 * flashing display waits in ENDIDLE (FLASHSUB, endidle.c) for the
 * astronaut. A perform display goes out in two parts: the verb and noun,
 * then the flashing V50, "PLEASE PERFORM". */
#include "display_interface.h"

/* NORMBNCH -- LM p. 1365, CM p. 1476: "SET PINBRANCH BIT" (UPFLAG;
 * ADRES PINBRFLG). */
void normbnch(void)
{
    tc_at(NORMBNCH);
    continue_at(UPFLAG);
}

/* PINBRNCH -- LM p. 1364, CM p. 1475. "PINBRNCH REESTABLISHES THE LAST
 * ACTIVE FLASHING DISPLAY.  IF THERE IS NO ACTIVE FLASHING DISPLAY, THE
 * DSKY IS BLANKED AND CONTROL IS SENT TO ENDOFJOB." MPAC +2 back from
 * MARK2PAC ("NEEDED TO SAVE MPAC +2 FOR MARK USERS"), and by FLAGWRD4's
 * "PINBRANCH CONDITION" (bits 13-15): ERASER ("NOTHING IN ENDIDLE"),
 * MARKPLAY, or NORMBNCH. */
void pinbrnch(void)
{
    relint();
    ca(ADDRESS_OF(MARK2PAC));
    ts(ADDRESS_OF(MPAC) + 2u);
    ca(ADDRESS_OF(FLAGWRD4));
    mask(PINMASK);
    switch (ccs_register(A_ADDRESS)) {
    case IS_PLUS_ZERO: continue_at(ERASER); return;
    case NEGATIVE: continue_at(MARKPLAY); return;
    default: break; /* TCF +3 */
    }
    normbnch();
}

/* NORMBNCH +2, where UPFLAG returns: "PRIO INTERRUPTED": KEEPPRIO, else
 * PLAYJUM1. */
void normbnch_after_upflag(void)
{
#if LUMINARY099
    set_A(PRIODBIT);
#else
    set_A(BIT14);
#endif
    mask_at(ADDRESS_OF(FLAGWRD4));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(KEEPPRIO);
        return;
    }
    continue_at(PLAYJUM1);
}

/* NVDSP (GOPLAY) -- LM p. 1365, CM p. 1476: COPYPACS. */
void nvdsp(void)
{
    tc_at(NVDSP);
    continue_at(COPYPACS);
}

/* NV50DSP -- LM p. 1365, CM p. 1476: the verb and noun in A out by
 * NVMONOPT (blanking by L). */
void nv50dsp(void)
{
    tc_at(NV50DSP);
    continue_at(NVMONOPT);
}

/* NVDSP1 -- LM p. 1365, CM p. 1476: the verb and noun one more (CCS took
 * one off). */
void nvdsp1(void)
{
    ad(ONE);
    nv50dsp();
}

/* NVDSP +023: "IN CASE MARKPLAY AWAKENED AFTER SLEEPING", the mark's verb
 * and noun back in MARKNV, and a V05 (V06 for a decimal mark perform,
 * TEM1's bit 8) with its noun. (An overflow in A skips the MASK.) */
void nvdsp_mark_plus1(void)
{
    if (!ts(ADDRESS_OF(MARKNV))) mask(LOW7);
    ad(V05N00M1);
    ad_at(ADDRESS_OF(TEM1));
    nvdsp1();
}

/* NVDSP +022: a complemented verb and noun (a mark perform). */
void nvdsp_mark(void)
{
    cs(ADDRESS_OF(MARKNV));
    nvdsp_mark_plus1();
}

/* NVDSP +1, where COPYPACS returns, with TEMPOR2 the kind's flags. "SET UP
 * BLANK BITS FOR NVMONOPT IN CASE USER REQUESTS BLANKING MONITOR" (L);
 * bit 13 down in DSPFLG (the first part of a perform); "BIT8 SET IF DEC
 * MARK PERFORM DISPLAY" to TEM1; MPAC +2 saved. By the kind's NVWORD:
 * NVDSP1, CLEANEND for none, or a mark perform's complemented word. */
void nvdsp_after_copypacs(void)
{
    word kind = display_kind();

    ca(ADDRESS_OF(TEMPOR2));
    mask(SEVEN);
    ts(L_ADDRESS);
    set_A(negate(BIT13));
    mask_at(indexed(ADDRESS_OF(DSPFLG), kind));
    ts(indexed(ADDRESS_OF(DSPFLG), kind));
    mask(BIT8);
    ts(ADDRESS_OF(TEM1));
    ca(ADDRESS_OF(MPAC) + 2u);
    ts(ADDRESS_OF(MPAC2SAV));
    ts(ADDRESS_OF(MARK2PAC));
    switch (ccs_at(indexed(ADDRESS_OF(NVWORD), kind))) {
    case POSITIVE: nvdsp1(); return;
    case IS_PLUS_ZERO: continue_at(CLEANEND); return;
    case NEGATIVE: nvdsp_mark(); return;
    case IS_MINUS_ZERO: nvdsp_mark_plus1(); return;
    }
}

/* NV50DSP +1, NVMONOPT's return when the DSKY is busy: REST. */
void nv50dsp_busy(void)
{
    continue_at(REST);
}

/* NV50DSP +2, where NVMONOPT returns: FLASHOFF, "IN CASE OF EXTENDED VERB
 * NON-FLASH". */
void nv50dsp_after_nvmonopt(void)
{
    tc_at((word)(NV50DSP + 2u));
    continue_at(FLASHOFF);
}

/* NV50DSP +3, where FLASHOFF returns: COPYTOGO, "MPACS DESTROYED BY
 * NVSUB". */
void nv50dsp_after_flashoff(void)
{
    tc_at((word)(NV50DSP + 3u));
    continue_at(COPYTOGO);
}

/* NV50DSP +4, where COPYTOGO returns: "UNSET SLEEPING BITS", MRKNVFLG. */
void nv50dsp_after_copytogo(void)
{
    tc_at((word)(NV50DSP + 4u)); /* TC DOWNFLAG; ADRES MRKNVFLG */
    continue_at(DOWNFLAG);
}

/* NV50DSP +6, where DOWNFLAG returns: NRMNVFLG down. */
void nv50dsp_after_downflag(void)
{
    tc_at((word)(NV50DSP + 6u)); /* TC DOWNFLAG; ADRES NRMNVFLG */
    continue_at(DOWNFLAG);
}

/* NV50DSP +010, where DOWNFLAG returns: PRONVFLG down. */
void nv50dsp_after_downflag2(void)
{
    tc_at((word)(NV50DSP + 010u)); /* TC DOWNFLAG; ADRES PRONVFLG */
    continue_at(DOWNFLAG);
}

/* BLANKCHK -- LM p. 1366, CM p. 1477: "BLANK BITS 1,2,3 IF SET"
 * (BLANKSUB). */
void blankchk(void)
{
    ca(ADDRESS_OF(TEMPOR2));
    tc_at((word)(BLANKCHK + 1u)); /* TC BLANKSUB */
    continue_at(BLANKSUB);
}

/* BLANKCHK +2, BLANKSUB's return when the DSKY is busy: NVDSP again. */
void blankchk_after_blanksub(void)
{
    nvdsp();
}

/* GOANIDLE +010: not a GODSPRET, flash or paste: back to the user at
 * the kind's CADRFLSH (ENDIT, by MPAC +3). */
void goanidle_plus8(void)
{
    ca(indexed(ADDRESS_OF(CADRFLSH), display_kind()));
    ts(ADDRESS_OF(MPAC) + 3u);
    continue_at(ENDIT);
}

/* ISITN00 -- LM p. 1366, CM p. 1477: "IS THIS A PASTE" (noun 00): "YES,
 * ASSUME PASTE ALWAYS ON FLASH"; if not, "NOT FLASH, NOT GOPERF,
 * THEREFORE EXIT". */
void isitn00(void)
{
    ca(indexed(ADDRESS_OF(NVWORD), display_kind()));
    mask(LOW7);
    if (A_is_zero()) {
        continue_at(FLASHSUB);
        return;
    }
    continue_at(ENDOFJOB);
}

/* GOANIDLE +4: "IS THIS A GODSPRET" (bit 6): if not ISITN00. */
void goanidle_plus4(void)
{
    cs(ADDRESS_OF(TEMPOR2));
    mask(BIT6);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        isitn00();
        return;
    }
    goanidle_plus8();
}

/* GOANIDLE -- LM p. 1366, CM p. 1477: a flashing display (bit 4) waits in
 * ENDIDLE: FLASHSUB. */
void goanidle(void)
{
    set_A(BIT4);
    mask_at(ADDRESS_OF(TEMPOR2));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        continue_at(FLASHSUB); /* "IT IS" */
        return;
    }
    goanidle_plus4();
}

/* V50PASTE -- LM p. 1367, CM p. 1478: "DISPLAY SECOND PART OF GOPERF",
 * V50. */
void v50paste(void)
{
    set_A(V50N00);
    nv50dsp();
}

/* 1STOR2ND +4: the first part done (bit 13 up in DSPFLG). "IS IT MARK"
 * (bit 15): "YES", MARKPERF. A V99PASTE (bit 12): "NVOWRD1= -0 IS V97.
 * NVWORD1= -400 IS V99" (a paste of V97 or V99); else V50PASTE. */
void first_or_second_plus4(void)
{
    word kind = display_kind();

    set_A(BIT13);
    ads(indexed(ADDRESS_OF(DSPFLG), kind));
    zl();
    if (A_is_zero_or_minus()) {
        continue_at(MARKPERF);
        return;
    }
    mask(BIT12);
    if (A_is_zero()) {
        v50paste();
        return;
    }
    cs(ADDRESS_OF(NVWORD1));
    ad(V97N00);
    nv50dsp();
}

/* 1STOR2ND -- LM p. 1366, CM p. 1477: the perform display's part: the
 * "SECOND" (bit 13) waits like any flash (GOANIDLE). */
void first_or_second(void)
{
    ca(ADDRESS_OF(TEMPOR2));
    mask(BIT13);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        goanidle();
        return;
    }
    first_or_second_plus4();
}

/* PERFCHEK -- LM p. 1366, CM p. 1477: "BIT5 FOR PERFORM". "IS THIS A
 * GOPERF DISPLAY": "YES", 1STOR2ND; if not GOANIDLE. */
void perfchek(void)
{
    set_A(BIT5);
    mask_at(ADDRESS_OF(TEMPOR2));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        first_or_second();
        return;
    }
    goanidle();
}

/* WITCHONE -- LM p. 1367, CM p. 1478: "TURN OFF KEY RELEASE LIGHT". Who
 * waits for the DSKY: "IS IT NVSUB" asleep (FLAGWRD4's NVBUSMSK bits),
 * CADRSTOR +1, else ENDIDLE's CADRSTOR. That CADR to A, and the word to
 * +0. */
void witchone(void)
{
    set_A(negate(BIT5));
    wand_dsalmout();
    ca(ADDRESS_OF(FLAGWRD4));
    mask(NVBUSMSK);
    if (ccs_register(A_ADDRESS) == POSITIVE) set_A(ONE);
    ts(L_ADDRESS);
    set_A(ZERO);
    xch(indexed(ADDRESS_OF(CADRSTOR), L));
    inhint();
    tc_through(Q_ADDRESS);
}

/* XCHNYLOC -- LM p. 1367, CM p. 1478: A to LOCCTR, where JOBWAKE left the
 * woken job's register set offset (-1 if none). "WAS THIS ADDRESS
 * SLEEPING": "YES": A becomes that job's LOC, where it goes on. RELINTQ. */
void xchnyloc(void)
{
    word job = fetch(ADDRESS_OF(LOCCTR));

    if (!(job & SIGN_BIT) && job > LAST_JOB) not_modelled();
    xch(ADDRESS_OF(LOCCTR));
    if (!A_is_zero_or_minus()) {
        xch(ADDRESS_OF(LOCCTR));
        ts(indexed(ADDRESS_OF(LOC), fetch(ADDRESS_OF(LOCCTR))));
    }
    relintq();
}

/* XCHTOEND -- LM p. 1367, CM p. 1478: "TC ENDOFJOB REPLACES GENADR IN LOC
 * FOR" the woken job: XCHNYLOC with ENDINST. */
void xchtoend(void)
{
    set_A(ENDINST);
    xchnyloc();
}
