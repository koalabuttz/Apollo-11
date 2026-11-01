/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc, part 4: the load verbs -- LM pp.
 * 425-431, CM pp. 343-349. The faithful C is the port's Pinball modules
 * (c64-port/src/lanec_pin_head_load.c, lanec_pin_gaps.c, lanec_pw_*.c).
 *
 * The verb fan (part 2b) sends a load verb here with the noun looked up.
 * Verbs 21-25 (ALOAD, BLOAD, CLOAD, ABLOAD, ABCLOAD) check that the noun
 * has the component asked for (COMPTEST) and can be loaded (NOUNTEST),
 * then ask for each word in turn: the verb flashes as 21, 22 or 23
 * (UPDATVB -1) and REQDATX, REQDATY or REQDATZ waits for the crew to key
 * it into R1, R2 or R3 (the keys leave it in XREG, YREG or ZREG, with
 * DECBRNCH saying how each was keyed). When all are in, "THE 3 DATA WORDS
 * LOADED ARE ALL DEC OR ALL OCT" (ALLDC/OC), the noun tables are read
 * again, and PUTCOM puts each word into the noun's erasable: as keyed for
 * octal, or through the scale factor routine's input routine for decimal
 * (PUTDECSF, SFINTABR: input_scale.c), which comes back to PUTCOM2. Noun
 * 07 sets or resets flag bits (XREG the flag word, YREG the bits, ZREG
 * whether to set). LOADLV ends the load and goes to RECALTST.
 *
 * Bank 41, the same code in both missions. Each function runs from a
 * label, or from the word a call returns to, to a call or jump to another
 * routine or to the next label the faithful port enters, which ends it
 * with Q and Z set. A call the faithful port runs within its step (SETNCADR,
 * DCTSTCYC, SFRUTMIX, DPTEST, and PUTCOM's own PUTNORM, PUTDPCOM and
 * PUTCOM2) is a C call. */
#include "pinball.h"
#include "../pinball_noun_tables/pinball_noun_tables.h"
#include "../agc/names.h"

/* ---- The load verbs (LM pp. 425-427, CM pp. 343-345) ---- */

/* ABCLOAD -- LM p. 425, CM p. 343. Verb 25, three words. Each call ends a
 * step of the faithful port, so each word a call returns to has its own
 * function: COMPTEST with minus the highest component (CS TWO), NOUNTEST,
 * then for each word the verb shown as 21, 22, 23 ("VB21 = ALOAD") and its
 * request. */
void abcload(void)
{
    set_A(negate(TWO));
    tc(ABCLOAD + 1u, COMPTEST);
}

void abcload_plus2(void)
{
    tc(ABCLOAD + 2u, NOUNTEST); /* "TEST IF NOUN CAN BE LOADED." */
}

void abcload_plus3(void)
{
    set_A(VBSP1LD);
    tc(ABCLOAD + 4u, UPDATVB - 1u);
}

void abcload_plus5(void)
{
    tc(ABCLOAD + 5u, REQDATX);
}

void abcload_plus6(void)
{
    set_A(VBSP2LD);
    tc(ABCLOAD + 7u, UPDATVB - 1u);
}

void abcload_plus8(void)
{
    tc(ABCLOAD + 8u, REQDATY);
}

void abcload_plus9(void)
{
    set_A(VBSP3LD);
    tc(ABCLOAD + 10u, UPDATVB - 1u);
}

void abcload_plus11(void)
{
    tc(ABCLOAD + 11u, REQDATZ);
}

static bool alldc_oc_returns(void);

/* PUTXYZ -- LM pp. 425-426, CM pp. 343-344. "TEST THAT THE 3 DATA WORDS
 * LOADED ARE ALL DEC OR ALL OCT.", the noun tables read again ("SWITCH
 * BANKS TO NOUN TABLE READING ROUTINE."), and the X, Y and Z components
 * put away in turn: PUTCOM leaves each in A, for the word NOUNADD points
 * at, +1 and +2. */
void putxyz(void)
{
    set_A(negate(SIX));
    tc_at(PUTXYZ + 1u);
    if (!alldc_oc_returns()) return;
    read_noun_tables(PUTXYZ + 4u);
    set_A(ZERO); /* "X COMP" */
    tc(PUTXYZ + 6u, PUTCOM);
}

void putxyz_plus7(void)
{
    /* INDEX NOUNADD; TS 0. An overflow (not from PUTCOM) would skip CAF ONE. */
    if (ts(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    set_A(ONE); /* "Y COMP" */
    tc(PUTXYZ + 10u, PUTCOM);
}

void putxyz_plus11(void)
{
    if (ts(indexed_erasable(1u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    set_A(TWO); /* "Z COMP" */
    tc(PUTXYZ + 14u, PUTCOM);
}

/* PUTXYZ +15, where PUTCOM comes back the third time. "IF NOUN 7 HAS JUST
 * BEEN LOADED, SET FLAG BITS AS SPECIFIED." */
void putxyz_plus15(void)
{
    if (ts(indexed_erasable(2u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    set_A(negate(SEVEN));
    ad_at(ADDRESS_OF(NOUNREG));
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        tc(PUTXYZ + 21u, LOADLV);
        return;
    }
    putxyz_plus22();
}

/* PUTXYZ +22, BITSOFF and BITSOFF1: noun 07. XREG is the "ECADR OF FLAG
 * WORD" (SETNCADR +1 sets EBANK and NOUNADD), YREG the "BITS TO BE
 * PROCESSED", ZREG "ZERO TO RESET BITS, NON-ZERO TO SET BITS". With
 * interrupts held off, the bits are set (ADS of the ones not yet set) or
 * reset (MASK with their complement). */
void putxyz_plus22(void)
{
    ca(ADDRESS_OF(XREG));
    tc_at(PUTXYZ + 23u);
    setncadr_plus1();
    ca(ADDRESS_OF(ZREG));
    inhint();
    if (A_is_zero()) { /* EXTEND; BZF BITSOFF */
        bitsoff();
        return;
    }
    cs(indexed_by(0u, fetch(ADDRESS_OF(NOUNADD))));
    mask_at(ADDRESS_OF(YREG));
    ads(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD)))); /* "SET BITS." */
    tc_at(PUTXYZ + 33u);
    bitsoff1();
}

void bitsoff(void)
{
    cs(ADDRESS_OF(YREG));
    mask_at(indexed_by(0u, fetch(ADDRESS_OF(NOUNADD))));
    ts(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD)))); /* "RESET BITS." */
    bitsoff1();
}

void bitsoff1(void)
{
    relint();
    tc(BITSOFF1 + 1u, LOADLV);
}

/* ABLOAD and PUTXY -- LM p. 426, CM p. 344. Verb 24, two words: as
 * ABCLOAD, without the third. */
void abload(void)
{
    set_A(negate(ONE));
    tc(ABLOAD + 1u, COMPTEST);
}

void abload_plus2(void)
{
    tc(ABLOAD + 2u, NOUNTEST);
}

void abload_plus3(void)
{
    set_A(VBSP1LD);
    tc(ABLOAD + 4u, UPDATVB - 1u);
}

void abload_plus5(void)
{
    tc(ABLOAD + 5u, REQDATX);
}

void abload_plus6(void)
{
    set_A(VBSP2LD);
    tc(ABLOAD + 7u, UPDATVB - 1u);
}

void abload_plus8(void)
{
    tc(ABLOAD + 8u, REQDATY);
}

void putxy(void)
{
    set_A(negate(FIVE));
    tc_at(PUTXY + 1u);
    if (!alldc_oc_returns()) return; /* "ALL DEC OR ALL OCT." */
    read_noun_tables(PUTXY + 4u);
    set_A(ZERO); /* "X COMP" */
    tc(PUTXY + 6u, PUTCOM);
}

void putxy_plus7(void)
{
    if (ts(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    set_A(ONE); /* "Y COMP" */
    tc(PUTXY + 10u, PUTCOM);
}

void putxy_plus11(void)
{
    if (ts(indexed_erasable(1u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    tc(PUTXY + 13u, LOADLV);
}

/* ALOAD -- LM p. 426, CM p. 344. Verb 21: the first word only. */
void aload(void)
{
    tc(ALOAD, REQDATX);
}

void aload_plus1(void)
{
    read_noun_tables(ALOAD + 3u);
    set_A(ZERO); /* "X COMP" */
    tc(ALOAD + 5u, PUTCOM);
}

void aload_plus6(void)
{
    if (ts(indexed_erasable(0u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    tc(ALOAD + 8u, LOADLV);
}

/* BLOAD and CLOAD -- LM pp. 426-427, CM pp. 344-345. Verbs 22 and 23: the
 * second or the third word only. "SET CLPASS FOR PASS0 ONLY" (BIT15). */
void bload(void)
{
    set_A(negate(ONE));
    tc(BLOAD + 1u, COMPTEST);
}

void bload_plus2(void)
{
    set_A(BIT15);
    ts(ADDRESS_OF(CLPASS));
    tc(BLOAD + 4u, REQDATY);
}

void bload_plus5(void)
{
    read_noun_tables(BLOAD + 7u);
    set_A(ONE);
    tc(BLOAD + 9u, PUTCOM);
}

void bload_plus10(void)
{
    if (ts(indexed_erasable(1u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    tc(BLOAD + 12u, LOADLV);
}

void cload(void)
{
    set_A(negate(TWO));
    tc(CLOAD + 1u, COMPTEST);
}

void cload_plus2(void)
{
    set_A(BIT15);
    ts(ADDRESS_OF(CLPASS));
    tc(CLOAD + 4u, REQDATZ);
}

void cload_plus5(void)
{
    read_noun_tables(CLOAD + 7u);
    set_A(TWO);
    tc(CLOAD + 9u, PUTCOM);
}

void cload_plus10(void)
{
    if (ts(indexed_erasable(2u, fetch(ADDRESS_OF(NOUNADD))))) not_modelled();
    tc(CLOAD + 12u, LOADLV);
}

/* LOADLV -- LM p. 427, CM p. 345. The load is done: DECBRNCH cleared,
 * LOADSTAT -0, the display released ("RELEASE FOR PRIORITY DISPLAY
 * PROBLEM."), then DSPCOUNT -VD1 "TO BLOCK NUMERICAL CHARACTERS AND CLEARS
 * AFTER A COMPLETED LOAD", and "AFTER COMPLETED LOAD, GO TO RECALTST TO
 * SEE IF THERE IS RECALL FROM ENDIDLE." */
void loadlv(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DECBRNCH));
    set_A(negate(ZERO));
    ts(ADDRESS_OF(LOADSTAT));
    tc(LOADLV + 4u, RELDSP);
}

void loadlv_plus5(void)
{
    set_A(negate(VD1));
    ts(ADDRESS_OF(DSPCOUNT));
    tc(LOADLV + 7u, POSTJUMP); /* CADR RECALTST */
}

/* ALLDC/OC and GOQ -- LM pp. 427-428, CM pp. 345-346. "TESTS THAT DATA
 * WORDS LOADED ARE EITHER ALL DEC OR ALL OCT. ALARMS IF NOT." Called with
 * minus the count's code (CS SIX for three words, CS FIVE for two),
 * kept in DECOUNT. DECBRNCH has a bit for each word keyed in decimal (bits
 * 3-5): shifted right twice through SR, "DEC COMP BITS IN LOW 3". All
 * zeros is "ALL OCTAL. OK"; else after the CCS they "MUST MATCH 6 FOR
 * 3COMP, 5 FOR 2COMP." True when it went back. */
static bool alldc_oc_returns(void)
{
    if (ts(ADDRESS_OF(DECOUNT))) not_modelled(); /* an overflow would skip CS DECBRNCH */
    cs(ADDRESS_OF(DECBRNCH));
    ts(ADDRESS_OF(SR));
    cs(ADDRESS_OF(SR));
    cs(ADDRESS_OF(SR)); /* "SHIFTED RIGHT 2" */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
    case NEGATIVE: ad_at(ADDRESS_OF(DECOUNT)); break;     /* "SOME ONES IN LOW 3" */
    case IS_PLUS_ZERO: tc_return(Q_ADDRESS); return true; /* "ALL ZEROS. ALL OCTAL. OK" */
    case IS_MINUS_ZERO: break;
    }
    if (!A_is_zero()) { /* EXTEND; BZF +2 */
        tc(ALLDC_OC + 11u, ALMCYCLE);
        return false;
    }
    goq(); /* "ALL REQUIRED ARE DEC. OK" */
    return true;
}

void alldc_oc(void)
{
    (void)alldc_oc_returns();
}

void goq(void)
{
    tc_return(Q_ADDRESS);
}

/* SFRET1, SFRET and CONUMNOR -- LM pp. 428-429, CM pp. 346-347: where the
 * scale factor code subroutines (SFRUTMIX, SFCONUM: subroutines.c) come
 * back through EXITEM, and the normal noun's constant number. The faithful
 * port enters them on their own. */
void sfret1(void)
{
    tc_return(ADDRESS_OF(EXITEM)); /* "SF ROUTINE NUMBER IN A" */
}

void sfret(void)
{
    double_A();
    tc_return(ADDRESS_OF(EXITEM)); /* "2X(SF CONSTANT NUMBER) IN A" */
}

void conumnor(void)
{
    set_A(LOW5); /* "NORMAL NOUN ALWAYS GETS LOW5 OF NNTYPTAB FOR SF CONUM." */
    mask_at(ADDRESS_OF(NNTYPTEM));
    double_A();
    tc_return(ADDRESS_OF(EXITEM));
}

/* ---- Putting a word away (LM pp. 429-431, CM pp. 347-349) ---- */

static void put_by_decbrnch(word at, bool mixed);

/* PUTCOM -- LM p. 429, CM p. 347. Component DECOUNT (from A): its word,
 * XREG +K and XREGLP +K (a decimal word comes as a double one), into MPAC,
 * MPAC +1, MPAC +6 cleared, and the return in DECRET. "IF MIXNOUN, PLACE
 * ADDRESS FOR COMPONENT K INTO NOUNADD, SET EBANK BITS": its IDADDTAB
 * ECADR (IDAD1TEM +K) by SETNCADR, less K, as NOUNADD "(ESUBK)-K", so that
 * the caller's INDEX NOUNADD; TS K lands on it. A normal noun has PUTNORM
 * set NOUNADD. */
void putcom(void)
{
    word decount;

    if (ts(ADDRESS_OF(DECOUNT))) not_modelled(); /* an overflow would skip XCH Q */
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(DECRET))) not_modelled(); /* an overflow in Q would skip CAF ZERO */
    set_A(ZERO);
    ts(ADDRESS_OF(MPAC) + 6u);
    decount = fetch(ADDRESS_OF(DECOUNT));
    xch(indexed_erasable(ADDRESS_OF(XREGLP), decount));
    ts(ADDRESS_OF(MPAC) + 1u);
    xch(indexed_erasable(ADDRESS_OF(XREG), decount));
    ts(ADDRESS_OF(MPAC));
    if (!mixed_noun(PUTCOM + 12u)) {
        tc_at(PUTCOM + 13u); /* "NORMAL NOUN" */
        putnorm();
        return;
    }
    ca(indexed_by(ADDRESS_OF(IDAD1TEM),
                  decount)); /* "GET IDADDTAB ENTRY FOR COMPONENT K OF NOUN." */
    mask(LOW11);             /* "(ECADR)SUBK FOR CURRENT COMP OF NOUN" */
    tc_at(PUTCOM + 17u);
    setncadr(); /* "ECADR INTO NOUNCADR. SETS EB, NOUNADD." */
    su_at(ADDRESS_OF(DECOUNT));
    ts(ADDRESS_OF(NOUNADD)); /* "PLACE (ESUBK)-K INTO NOUNADD" */
    put_by_decbrnch(PUTCOM + 21u, true);
}

/* PUTNORM -- LM pp. 429-430, CM pp. 347-348. A normal noun: "ECADR FROM
 * NOUNCADR. SETS EB, NOUNADD." */
void putnorm(void)
{
    tc_at(PUTNORM);
    setnadd();
    put_by_decbrnch(PUTNORM + 1u, false);
}

/* CCS DECBRNCH at at, in PUTCOM and in PUTNORM: a word keyed in decimal
 * goes to PUTDECSF; one keyed in octal is checked "IF DEC ONLY BIT = 1. IF
 * SO, ALARM AND RECYCLE." (DCTSTCYC), and for a double precision scale
 * (the scale routine by SFRUTMIX or SFRUTNOR, then DPTEST) "+0 INTO MAJOR
 * PART. SET NOUNADD FOR LOADING OCTAL WORD INTO MINOR PART." (PUTDPCOM).
 * A normal noun's word with no DP scale may be a channel's: "IF NNADTEM =
 * -1, CHANNEL TO BE SPECIFIED" (CHANLOAD). */
static void put_by_decbrnch(word at, bool mixed)
{
    switch (ccs_at(ADDRESS_OF(DECBRNCH))) {
    case POSITIVE: tc(at + 1u, PUTDECSF); return; /* "+ DEC" */
    case IS_PLUS_ZERO: break;                     /* "+0 OCTAL" */
    case NEGATIVE:
    case IS_MINUS_ZERO: not_modelled(); /* DECBRNCH holds bits 1-5 */
    }
    tc_at(at + 2u);
    if (!dctstcyc_returns()) return;
    tc_at(at + 3u);
    if (mixed)
        sfrutmix();
    else
        sfrutnor();
    tc_at(at + 4u);
    if (dptest()) {
        if (!mixed) {
            set_A(ZERO); /* "DP" */
            ts(ADDRESS_OF(DECOUNT));
            tc_at(PUTNORM + 9u);
        }
        putdpcom();
        return;
    }
    tc_at(at + 5u); /* "NO DP" */
    if (mixed) {
        putcom2();
        return;
    }
    /* PUTCOM2 -4 */
    ca(ADDRESS_OF(NNADTEM));
    ad(ONE);
    if (A_is_zero()) { /* EXTEND; BZF CHANLOAD */
        continue_at(CHANLOAD);
        return;
    }
    putcom2();
}

/* PUTDPCOM -- LM p. 429, CM p. 347. "DP (ESUBK)-K+1 OR E+1": NOUNADD
 * "NOW SET FOR MINOR PART", "(ESUBK)+1 OR E+1 INTO DECOUNT", and "ZERO
 * MAJOR PART(ESUBK OR E)". TS 0 -1 assembles as 053777, which INDEX
 * DECOUNT makes TS DECOUNT -1. */
void putdpcom(void)
{
    word at;

    incr(ADDRESS_OF(NOUNADD));
    ca(ADDRESS_OF(NOUNADD));
    ads(ADDRESS_OF(DECOUNT));
    set_A(ZERO);
    at = fetch(ADDRESS_OF(DECOUNT));
    if (at == 0u) not_modelled(); /* INDEX would leave it DXCH 1777 */
    ts(indexed_erasable(0u, at - 1u));
    tc_at(PUTDPCOM + 6u);
    putcom2();
}

/* PUTCOM2 -- LM p. 430, CM p. 348. The octal word (MPAC) into A, and back
 * through DECRET; the input routines end here too. */
void putcom2(void)
{
    xch(ADDRESS_OF(MPAC));
    tc_return(ADDRESS_OF(DECRET));
}

/* CHANLOAD -- LM p. 430, CM p. 348. A channel noun's word written to its
 * channel (NOUNCADR, low 9 bits): "DONT LOAD CHAN 7. (IT = SUPERBANK)." */
void chanload(void)
{
    set_A(negate(SEVEN));
    ad_at(ADDRESS_OF(NOUNCADR));
    if (A_is_zero()) { /* EXTEND; BZF LOADLV */
        continue_at(LOADLV);
        return;
    }
    ca(ADDRESS_OF(NOUNCADR));
    mask(LOW9);
    xch(ADDRESS_OF(MPAC));
    write_channel(fetch(ADDRESS_OF(MPAC))); /* EXTEND; INDEX MPAC; WRITE 0 */
    tc(CHANLOAD + 10u, LOADLV);
}

/* PUTDECSF -- LM p. 430, CM p. 348. "PUTDECSF FINDS MIXBR AND DECOUNT
 * STILL SET FROM PUTCOM": the scale factor constant into SFTEMP1, SFTEMP2
 * ("SWITCH BANKS TO SF CONSTANT TABLE READING ROUTINE.": GTSFIN, with
 * twice its number from SFCONUM), and the scale factor routine's number
 * (SFRUTNOR for a normal noun, SFRUTMIX for a mixed one) for PUTDCSF2. */
void putdecsf(void)
{
    tc_at(PUTDECSF);
    sfconum(); /* "2X(SF CON NUMB) IN A" */
    ts(ADDRESS_OF(SFTEMP1));
    dca(ADDRESS_OF(GTSFINLC));
    dtcb(PUTDECSF + 4u); /* "LOADS SFTEMP1, SFTEMP2." */
    gtsfin();
    if (mixed_noun(PUTDECSF + 6u)) {
        tc_at(PUTDECSF + 8u);
        sfrutmix();
        tc_at(PUTDECSF + 9u);
        putdcsf2();
        return;
    }
    tc_at(PUTDECSF + 7u);
    putsfnor();
}

void putsfnor(void)
{
    tc_at(PUTSFNOR);
    sfrutnor();
    putdcsf2();
}

/* PUTDCSF2 -- LM pp. 430-431, CM pp. 348-349. To the scale factor
 * routine's input routine through SFINTABR, by BANKJUMP ("SWITCH BANKS FOR
 * EXPANSION ROOM"): "ALARM AND RECYCLE IF DEC LOAD WITH OCTAL ONLY NOUN"
 * (GOALMCYC), DSPALARM for those that "CANT BE LOADED", or one of the
 * routines of input_scale.c, which end at PUTCOM2. */
void putdcsf2(void)
{
    if (a_word() > 13u) not_modelled(); /* past SFINTABR */
    ca(indexed_by(ADDRESS_OF(SFINTABR), a_word()));
    tc(PUTDCSF2 + 2u, BANKJUMP);
}
