/* SPDX-License-Identifier: MIT */
/* PINBALL_GAME_BUTTONS_AND_LIGHTS.agc: the small subroutines, most in
 * fixed-fixed memory -- LM pp. 428-429, 433-434, 446, 460-465, CM pp.
 * 346-347, 351-352, 364, 378-383. The faithful C is the port's Pinball modules
 * (c64-port/src/lanec_pw_*.c, lanec_pin_gaps.c, lanec_pin_head_*.c).
 * So far those parts 1 to 3 use: SLEFT5 and RELDSPON (the keys);
 * SETNCADR, SETNADD, SETEBANK, RIGHT5, LEFT5, FALTON, FLASHON, FLASHOFF,
 * RELDSP, RELDSP1, ALMCYCLE and SFRUTMIX (ENTER and the verb fan);
 * SFRUTNOR, SFCONUM, TPLEFTN, TPSL1 and PRSHRTMP (the display verbs).
 * SFRUTMIX, SFRUTNOR and SFCONUM are in bank 41, TPLEFTN in bank 40.
 *
 * Each goes back with TC Q (or through its return word), so a routine of
 * the file calls it as a C function, Q set first (tc_at); the ones other
 * files call, or the faithful port's steps stop at, are entries as
 * well. */
#include "pinball.h"
#include "../interpreter/interpreter.h"
#include "../agc/names.h"

/* SLEFT5 -- LM p. 461, CM p. 379. A five bits up by doubling it five
 * times (DOUBLE), which, unlike LEFT5's CYL, carries nothing round. */
void sleft5(void)
{
    double_A();
    double_A();
    double_A();
    double_A();
    double_A();
    tc_return(Q_ADDRESS);
}

/* RELDSPON -- LM p. 461, CM p. 379. "TURN ON KEY RELEASE LIGHT": "BIT 5 OF
 * CHANNEL 11". */
void reldspon(void)
{
    set_A(BIT5);
    wor_dsalmout();
    tc_return(Q_ADDRESS);
}

/* ---- The noun's address (LM p. 460, CM p. 378) ---- */

/* SETNCADR +2 on: the address within the bank, 1400-1777, into NOUNADD,
 * and back. */
static void put_noun_address(void)
{
    if (ts(ADDRESS_OF(NOUNADD))) not_modelled(); /* an overflow would skip TC Q */
    tc_return(Q_ADDRESS);
}

static void noun_address(void)
{
    mask(LOW8);
    ad(OCT1400);
    put_noun_address();
}

/* SETNCADR -- "E CADR ARRIVES IN A. IT IS STORED IN NOUNCADR. EBANK BITS
 * ARE SET. E ADRES IS DERIVED AND PUT INTO NOUNADD." The ECADR's low
 * eight bits are the address within the bank, 1400-1777. */
void setncadr(void)
{
    if (ts(ADDRESS_OF(NOUNCADR))) { /* an overflow skips TS EBANK */
        noun_address();
        return;
    }
    setncadr_plus1();
}

void setncadr_plus1(void)
{
    if (ts(EBANK_ADDRESS)) { /* an overflow skips MASK LOW8 */
        ad(OCT1400);
        put_noun_address();
        return;
    }
    noun_address();
}

/* SETNADD -- "GETS E CADR FROM NOUNCADR, SETS EBANK BITS, DERIVES E ADRES
 * AND PUTS IT INTO NOUNADD." */
void setnadd(void)
{
    ca(ADDRESS_OF(NOUNCADR));
    setncadr_plus1(); /* TCF SETNCADR +1 */
}

/* SETEBANK -- "E CADR ARRIVES IN A. EBANK BITS ARE SET. E ADRES IS DERIVED
 * AND LEFT IN A." */
void setebank(void)
{
    if (!ts(EBANK_ADDRESS)) mask(LOW8); /* an overflow skips the MASK */
    ad(OCT1400);
    tc_return(Q_ADDRESS);
}

/* RIGHT5 and LEFT5 -- LM pp. 460-461, CM pp. 378-379. A cycled five
 * places right (CYR) or left (CYL): the store and four CSs edit it five
 * times, the fourth CS leaving it uncomplemented; XCH takes it back. An
 * overflow in A skips the first CS (TS). */
void right5(void)
{
    if (!ts(ADDRESS_OF(CYR))) cs(ADDRESS_OF(CYR)); /* an overflow skips the first CS */
    cs(ADDRESS_OF(CYR));
    cs(ADDRESS_OF(CYR));
    cs(ADDRESS_OF(CYR));
    xch(ADDRESS_OF(CYR));
    tc_return(Q_ADDRESS);
}

void left5(void)
{
    if (!ts(ADDRESS_OF(CYL))) cs(ADDRESS_OF(CYL)); /* an overflow skips the first CS */
    cs(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    cs(ADDRESS_OF(CYL));
    xch(ADDRESS_OF(CYL));
    tc_return(Q_ADDRESS);
}

/* ---- The lights (LM pp. 461-462, CM pp. 379-380) ---- */

/* FALTON -- "TURN ON OPERATOR ERROR LIGHT": "BIT 7 OF CHANNEL 11". */
void falton(void)
{
    set_A(BIT7);
    wor_dsalmout();
    tc_return(Q_ADDRESS);
}

/* FLASHON -- "TURN ON V/N FLASH": "BIT 6 OF CHANNEL 11". */
void flashon(void)
{
    set_A(BIT6);
    wor_dsalmout();
    tc_return(Q_ADDRESS);
}

/* FLASHOFF -- "TURN OFF V/N FLASH". */
void flashoff(void)
{
    set_A(negate(BIT6));
    wand_dsalmout();
    tc_return(Q_ADDRESS);
}

/* ---- Releasing the display system (LM pp. 464-465, CM pp. 382-383) ---- */

/* RELDSP2: "TURN OFF KEY RELEASE LIGHT (BIT 5 OF CHANNEL 11)", DSPLOCK
 * +0 (the display system free), and back through RELRET. */
static void reldsp2(void)
{
    relint();
    set_A(negate(BIT5));
    wand_dsalmout();
    set_A(ZERO);
    ts(ADDRESS_OF(DSPLOCK));
    tc_return(ADDRESS_OF(RELRET));
}

/* RELDSP -- "SET DSPLOCK TO +0, TURN RELDSP LIGHT OFF, SEARCH DSPLIST".
 * "RELDSP IS USED BY VBPROC, VBTERM, VBRQEXEC, VBRQWAIT, VBRELDSP,
 * EXTENDED VERB DISPATCHER, VBRESEQ, RECALTST." "TURN OFF EXTERNAL
 * MONITOR BIT" (BIT14 of MONSAVE1), interrupts off. A job waiting for the
 * display system (DSPLIST) is woken (JOBWAKE): the faithful port takes
 * RELDSP only with DSPLIST empty, and so does the readable one. */
void reldsp(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(RELRET))) not_modelled(); /* an overflow in Q would skip CS BIT14 */
    set_A(negate(BIT14));
    inhint();
    mask_at(ADDRESS_OF(MONSAVE1));
    ts(ADDRESS_OF(MONSAVE1));
    if (ccs_at(ADDRESS_OF(DSPLIST)) != IS_PLUS_ZERO) not_modelled(); /* JOBWAKE */
    tc_at(RELDSP + 8u);                                              /* TC RELDSP2: "LIST EMPTY" */
    reldsp2();
}

/* RELDSP1 -- "SET DSPLOCK TO +0. NO DSPLIST SEARCH. TURN KEY RLSE LIGHT
 * OFF IF DSPLIST IS EMPTY. LEAVE KEY RLSE LIGHT ALONE IF DSPLIST IS NOT
 * EMPTY." "RELDSP1 IS USED BY MONITOR SET UP, VBRELDSP." */
void reldsp1(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(RELRET))) not_modelled(); /* an overflow in Q would skip the CCS */
    switch (ccs_at(ADDRESS_OF(DSPLIST))) {
    case IS_PLUS_ZERO:
        tc_at(RELDSP1 + 4u); /* TC RELDSP2: "+0 EMPTY" */
        reldsp2();
        return;
    case POSITIVE:
        tc_at(RELDSP1 + 3u); /* TC +2: "+ NOT EMPTY" */
        /* fall through */
    case NEGATIVE:
        set_A(ZERO); /* "- NOT EMPTY" */
        /* fall through */
    case IS_MINUS_ZERO: break;
    }
    ts(ADDRESS_OF(DSPLOCK));
    tc_return(ADDRESS_OF(RELRET));
}

/* ---- Alarm and recycle (LM p. 446, CM p. 364) ---- */

/* ALMCYCLE -- "ALMCYCLE TURNS ON CHECK FAIL LIGHT, REDISPLAYS THE ORIGINAL
 * VERB THAT WAS EXECUTED, AND RECYCLES TO EXECUTE THE ORIGINAL VERB/NOUN
 * COMBINATION THAT WAS LAST EXECUTED. USED FOR BAD DATA DURING LOAD VERBS
 * AND BY MCTBS. ALSO BY MMCHANG IF 2 NUMERICAL CHARACTERS WERE NOT
 * PUNCHED IN FOR MM CODE." The verb saved at ENTPAS0 (VERBSAVE, kept
 * complemented) goes into REQRET, positive for ENTER's pass 0; UPDATVB -1
 * puts it back in VERBREG and on the DSKY. */
void almcycle(void)
{
    tc_at(ALMCYCLE);
    falton(); /* "TURN ON CHECK FAIL LIGHT." */
    cs(ADDRESS_OF(VERBSAVE));
    ts(ADDRESS_OF(REQRET));
    tc(ALMCYCLE + 3u, BANKCALL); /* CADR UPDATVB -1 */
}

/* ALMCYCLE +5, where UPDATVB returns: to ENTER (ENDALM: CADR ENTER). */
void almcycle_after_updatvb(void)
{
    tc(ALMCYCLE + 5u, POSTJUMP);
}

/* ---- The scale routine of a mixed noun's component (LM p. 428, CM
 * p. 346) ---- */

/* SFRUTMIX -- "GETS SF ROUTINE NUMBER FOR MIXED CASE". RUTMXTEM holds the
 * noun's three components' scale routines, five bits each: DECOUNT 0
 * picks the low five (LOW5, and DISPLACE's TC GOQ, back as they are), 1
 * the middle (MID5, TC RIGHT5) and 2 the high (HI5, TC LEFT5: cycled left
 * five, the high five come round to the bottom). INDEX L; TC 0 runs the
 * TC in L, with Q SFRET1, which returns through EXITEM ("CAN'T USE L FOR
 * RETURN"). */
void sfrutmix(void)
{
    word decount, to;

    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(EXITEM))) not_modelled(); /* an overflow in Q would skip the INDEX */
    decount = fetch(ADDRESS_OF(DECOUNT));
    ca(indexed_by(ADDRESS_OF(DISPLACE), decount)); /* "PUT TC GOQ, TC RIGHT5, OR TC LEFT5 IN L" */
    ts(L_ADDRESS);
    ca(indexed_by(ADDRESS_OF(LOW5), decount)); /* "LOW5, MID5, OR HI5 IN A" */
    mask_at(ADDRESS_OF(RUTMXTEM));
    to = indexed_instruction(0u, L); /* INDEX L; TC 0 */
    tc_at(SFRUTMIX + 9u);
    if (to == GOQ)
        tc_return(Q_ADDRESS); /* GOQ: TC Q */
    else if (to == RIGHT5)
        right5();
    else if (to == LEFT5)
        left5();
    else
        not_modelled();
    tc_return(ADDRESS_OF(EXITEM)); /* SFRET1: "SF ROUTINE NUMBER IN A" */
}

/* SFRUTNOR -- LM p. 428, CM p. 346. "GETS SF ROUTINE NUMBER FOR NORMAL
 * CASE": the middle five bits of NNTYPTEM, moved down (RIGHT5). The return
 * in EXITEM: "CAN'T USE L FOR RETURN. TSTFORDP USES L." */
void sfrutnor(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(EXITEM))) not_modelled(); /* an overflow in Q would skip CAF MID5 */
    set_A(MID5);
    mask_at(ADDRESS_OF(NNTYPTEM));
    tc_at(SFRUTNOR + 4u);
    right5();
    tc_return(ADDRESS_OF(EXITEM)); /* "SF ROUTINE NUMBER IN A" */
}

/* SFCONUM, SFRET and CONUMNOR -- LM pp. 428-429, CM pp. 346-347. "GETS
 * 2X(SF CONSTANT NUMBER)", the scale factor constant's place in SFOUTAB
 * and SFINTAB (two words each). "NORMAL NOUN ALWAYS GETS LOW5 OF NNTYPTAB
 * FOR SF CONUM." A mixed noun's component DECOUNT has its own, in
 * NNTYPTEM five bits at a time, picked as SFRUTMIX picks its routine. */
void sfconum(void)
{
    word decount, to;

    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(EXITEM))) not_modelled(); /* an overflow in Q would skip the INDEX */
    if (!mixed_noun(SFCONUM + 3u)) {
        tc_at(SFCONUM + 4u); /* TC CONUMNOR: "NORMAL NOUN" */
        set_A(LOW5);
        mask_at(ADDRESS_OF(NNTYPTEM));
        double_A();
        tc_return(ADDRESS_OF(EXITEM)); /* "2X(SF CONSTANT NUMBER) IN A" */
        return;
    }
    decount = fetch(ADDRESS_OF(DECOUNT)); /* "MIXED NOUN" */
    ca(indexed_by(ADDRESS_OF(DISPLACE), decount));
    ts(L_ADDRESS); /* "PUT TC GOQ, TC RIGHT5, OR TC LEFT5 IN L" */
    ca(indexed_by(ADDRESS_OF(LOW5), decount));
    mask_at(ADDRESS_OF(NNTYPTEM));
    to = indexed_instruction(0u, L); /* INDEX L; TC 0 */
    tc_at(SFCONUM + 12u);
    if (to == GOQ)
        tc_return(Q_ADDRESS); /* GOQ: TC Q */
    else if (to == RIGHT5)
        right5();
    else if (to == LEFT5)
        left5();
    else
        not_modelled();
    /* SFRET */
    double_A(); /* "2X(SF CONSTANT NUMBER) IN A" */
    tc_return(ADDRESS_OF(EXITEM));
}

/* ---- Triple word shifts (LM pp. 433-434 and 462, CM pp. 351-352 and
 * 380) ---- */

/* TPSL1 -- LM p. 462, CM p. 380. "SHIFTS MPAC, +1, +2 LEFT 1 LEAVES
 * OVFIND SET TO +/- 1 FOR OF/UF": each word added to itself, the carries
 * passed up (DAS, ADS); an overflow out of MPAC sets MPAC +6 ("TS A DOES
 * NOT CHANGE A ON OF/UF.": TS into ZERO only tests it). */
void tpsl1(void)
{
    dca(ADDRESS_OF(MPAC) + 1u);
    das(ADDRESS_OF(MPAC) + 1u);
    ad_at(ADDRESS_OF(MPAC));
    ads(ADDRESS_OF(MPAC));
    if (ts(ZERO_ADDRESS)) ts(ADDRESS_OF(MPAC) + 6u); /* "MPAC +6 SET TO +/- 1 FOR OF/UF" */
    tc_return(Q_ADDRESS);
}

/* TPLEFTN and LEFTNCOM -- LM pp. 433-434, CM pp. 351-352. "SHIFTS MPAC,
 * +1, +2 LEFT N. SETS OVFIND TO +1 FOR OF, -1 FOR UF. CALL WITH N-1 IN
 * A.": TPSL1 N times ("LOOP TIME .37 MSEC."), the count in SFTEMP1 and
 * the return in SFTEMP2. */
void tpleftn(void)
{
    xch(Q_ADDRESS);
    if (ts(ADDRESS_OF(SFTEMP2))) not_modelled(); /* an overflow in Q would skip XCH Q */
    xch(Q_ADDRESS);
    leftncom();
}

void leftncom(void)
{
    for (;;) {
        ts(ADDRESS_OF(SFTEMP1));
        tc_at(LEFTNCOM + 1u);
        tpsl1();
        switch (ccs_at(ADDRESS_OF(SFTEMP1))) {
        case POSITIVE: tc_at(LEFTNCOM + 3u); continue; /* TC LEFTNCOM */
        case IS_PLUS_ZERO: tc_return(ADDRESS_OF(SFTEMP2)); return;
        case NEGATIVE:
        case IS_MINUS_ZERO: not_modelled(); /* the CCS would run 2ROUND */
        }
    }
}

/* PRSHRTMP -- LM p. 462, CM p. 380. "IF MPAC, +1 ARE EACH +NZ OR +0 AND
 * C(A)=-0, SHORTMP WRONGLY GIVES +0." "PRSHRTMP FIXES FORST CASE ONLY, BY
 * MERELY TESTING C(A) AND IF IT = -0, SETTING RESULT TO -0." "(DO NOT USE
 * PRSHRTMP UNLESS MPAC, +1 ARE EACH +NZ OR +0, AS THEY ARE WHEN THEY
 * CONTAIN THE SF CONSTANTS.)" */
void prshrtmp(void)
{
    if (ts(ADDRESS_OF(MPTEMP))) not_modelled(); /* an overflow would skip the CCS */
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE:
    case NEGATIVE: ca(ADDRESS_OF(MPTEMP));      /* fall through */
    case IS_PLUS_ZERO: shortmp_plus1(); return; /* "DO REGULAR SHORTMP" */
    case IS_MINUS_ZERO: break;
    }
    set_A(negate(ZERO)); /* CS ZERO: "FORCE RESULT TO -0 AND RETURN." */
    ts(ADDRESS_OF(MPAC));
    ts(ADDRESS_OF(MPAC) + 1u);
    ts(ADDRESS_OF(MPAC) + 2u);
    tc_return(Q_ADDRESS);
}
