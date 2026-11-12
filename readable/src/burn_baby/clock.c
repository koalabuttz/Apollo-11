/* SPDX-License-Identifier: MIT */
/* BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, pp. 743-748 (Luminary099
 * only): the countdown clock (STCLOK1-3, CLOKTASK and CLOKJOB with its
 * displays), STOPCLOK and NULLCLOK, the crew's responses (ASTNRETN,
 * *PROCEED, *ENTER), GOPOST, GOCUTOFF, IGNITE, P40ALM and REP40ALM, and
 * P40AUTO in bank 35. The faithful C is c64-port/src/lm_burn_baby.c.
 *
 * DISPDEX drives the clock. Positive kills it; minus a display's index
 * picks the display CLOKJOB shows each second: -CNTDNDEX the countdown
 * (verb 16 noun 85 or the table's), -VB97DEX verb 97, -VB99DEX the "PLEASE
 * ENABLE ENGINE" verb 99, -BLANKDEX a blank DSKY. */
#include "burn_baby.h"
#include "../agc/names.h"

#if LUMINARY099

/* EXTEND; DCA NEG0; DXCH -PHASEn: restart group n ends. */
static void end_group(unsigned minus_phase)
{
    dca(ADDRESS_OF(NEG0));
    dxch(minus_phase);
}

/* INDEX WHICH: the program's ignition table (burn_baby.c's which_table()).
 * Not modelled: a WHICH that is not one of the six. */
static word which_table(void)
{
    word table = ERASABLE(ADDRESS_OF(WHICH));

    if (table != P12TABLE && table != P40TABLE && table != P41TABLE - 5u && table != P42TABLE &&
        table != P63TABLE && table != ABRTABLE)
        not_modelled();
    return table;
}

/* INDEX WHICH; TCF n: the table's word n, a TCF (burn_baby.c's
 * tcf_into_table(), for *ENTER's and REP40ALM's words 3 and 14). */
static void tcf_into_table(word n)
{
    word at = (word)(which_table() + n);
    word instruction;

    while (at >= P12TABLE && at < BURNBABY) {
        instruction = fetch_fixed(at);
        if (OPERATION_CODE(instruction) != 1u || ADDRESS_FIELD(instruction) < 02000u)
            not_modelled();
        at = ADDRESS_FIELD(instruction);
    }
    continue_at(at);
}

/* ---- The countdown clock -- pp. 743-744 ---- */

/* STCLOK1 -- p. 743: "THIS ROUTINE STARTS THE COUNT-DOWN" with DISPDEX
 * +0, */
void stclok1(void)
{
    set_A(ZERO);
    stclok2();
}

/* STCLOK2 -- p. 743: or with DISPDEX from A ("SETTING DISPDEX POSITIVE KILLS
 * IT"; an overflow in A skips the TC MAKECADR), */
void stclok2(void)
{
    if (ts(ADDRESS_OF(DISPDEX))) {
        stclok3_after_makecadr();
        return;
    }
    stclok3();
}

/* STCLOK3 -- p. 743: or as it is. The caller's return as a CADR
 * (MAKECADR), */
void stclok3(void)
{
    tc_to(STCLOK3, MAKECADR);
}

/* STCLOK3 +1 -- p. 743: kept in TBASE4 ("RETURN SAVE (NOT FOR RESTARTS)"),
 * and TIG - TIME2 in MPAC, sign-agreed (TPAGREE). Not modelled: an overflow
 * at the TS, which would skip the EXTEND (a CADR never overflows). */
void stclok3_after_makecadr(void)
{
    if (ts(ADDRESS_OF(TBASE4))) not_modelled();
    dca(ADDRESS_OF(TIG));
    dxch(ADDRESS_OF(MPAC));
    dcs(ADDRESS_OF(TIME2));
    das(ADDRESS_OF(MPAC)); /* "HAVE TIG - TIME2, UNDOUBTEDLY A + NUMBER" */
    tc_to(STCLOK3 + 010u, TPAGREE);
}

/* STCLOK3 +011 -- p. 744: CLOKTASK runs at the next whole second of the
 * countdown: in the remainder of (TIG - TIME2) / 1 second, plus 2
 * centiseconds (TWIDDLE). */
void stclok3_after_tpagree(void)
{
    set_A(ONE_SEC);
    ts(Q_ADDRESS);
    dxch(ADDRESS_OF(MPAC));
    mask(LOW5); /* "RESTRICT MAGNITUDE OF NUMBER IN A" */
    dv_at(Q_ADDRESS);
    ca(L_ADDRESS); /* "GET REMAINDER" */
    ad(TWO);
    inhint();
    tc_to(STCLOK3 + 022u, TWIDDLE); /* ADRES CLOKTASK */
}

/* STCLOK3 +024 -- p. 744: "6.3SPOT FOR CLOKTASK" (OCT 40036, 05024,
 * 13000), */
void stclok3_after_twiddle(void)
{
    tc_to(STCLOK3 + 024u, TWO_PHSCHNG);
}

/* STCLOK3 +030 -- p. 744: and back to the caller (BANKJUMP to the CADR in
 * TBASE4). */
void stclok3_after_2phschng(void)
{
    ca(ADDRESS_OF(TBASE4));
    tc_to(STCLOK3 + 031u, BANKJUMP);
}

/* CLOKTASK -- p. 744: "SET TBASE6 FOR GROUP 6 RESTART". A positive DISPDEX
 * kills the clock; otherwise CLOKJOB as a job (NOVAC; after a -0 DISPDEX,
 * whose CCS skips the CAF PRIO27, with A +0) and CLOKTASK again in a
 * second. */
void cloktask(void)
{
    cs(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TBASE6));
    switch (ccs_at(ADDRESS_OF(DISPDEX))) {
    case POSITIVE: killclok(); return;
    case IS_PLUS_ZERO: /* NOOP */
    case NEGATIVE: set_A(PRIO27); break;
    case IS_MINUS_ZERO: break;
    }
    tc_to(CLOKTASK + 6u, NOVAC); /* 2CADR CLOKJOB */
}

/* CLOKTASK +011 -- p. 744: "WAIT A SECOND BEFORE STARTING OVER" (FIXDELAY,
 * DEC 100), */
void cloktask_after_novac(void)
{
    tc_to(CLOKTASK + 011u, FIXDELAY);
}

void cloktask_after_fixdelay(void)
{
    cloktask();
}

/* KILLCLOK -- p. 744: "KILL RESTART" (group 6 ends). */
void killclok(void)
{
    end_group(ADDRESS_OF(MINUS_PHASE6));
    continue_at(TASKOVER);
}

/* CLOKJOB: TTOGO, TIME2 - TIG, then the display DISPDEX picks: DISPNOT
 * -n for a DISPDEX of -n (INDEX A; TCF DISPNOT -1 after CCS and COM). A
 * positive or +0 DISPDEX, which a task or a higher priority job set since
 * the last CLOKTASK, ends the job. Not modelled: a -0 DISPDEX ("DISPDEX
 * MUST NEVER BE -0") and a negative one that picks no display (the file
 * stores -CNTDNDEX, -VB97DEX, -VB99DEX and -BLANKDEX; -25 is a display's
 * too). -- p. 744 */
void clokjob(void)
{
    word display;

    dcs(ADDRESS_OF(TIG));
    dxch(ADDRESS_OF(TTOGO));
    dca(ADDRESS_OF(TIME2));
    das(ADDRESS_OF(TTOGO));
    inhint();
    switch (ccs_at(ADDRESS_OF(DISPDEX))) {
    case POSITIVE:
    case IS_PLUS_ZERO: continue_at(ENDOFJOB); return;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled(); break;
    }
    com();
    relint();
    display = (word)indexed(DISPNOT - 1u, a_word());
    if (display != DISPNOT - 035u && display != DISPNOT - 025u && display != DISPNOT - 017u &&
        display != DISPNOT - 013u && display != DISPNOT - 2u)
        not_modelled();
    continue_at(display);
}

/* ---- The displays -- pp. 745-746 ---- */

/* DISPNOT -35 -- p. 745 (VB97DEX): "INDICATE VERB 97 PASTE" (NVWORD1 -0) and
 * flash the noun in NVWORD +2 with it (CLOCPLAY). */
void dispnot_minus35(void)
{
    set_A(MINUS_ZERO); /* CS ZERO */
    ts(ADDRESS_OF(NVWORD1));
    ca(ADDRESS_OF(NVWORD) + 2u);          /* "NVWROD+2 CONTAINS V06 & APPROPRIATE NOUN" */
    tc_to(DISPNOT - 035u + 3u, BANKCALL); /* CADR CLOCPLAY */
}

/* DISPNOT -030 -- p. 745: Terminate: "TERMINATE CLOKTASK ON THE WAY TO P00H"
 * (STOPCLOK); */
void dispnot_minus35_terminate(void)
{
    stopclok();
}

/* DISPNOT -027 -- p. 745: proceed: COMFAIL1; enter: COMFAIL2. */
void dispnot_minus35_proceed(void)
{
    comfail1();
}

void dispnot_minus35_enter(void)
{
    comfail2();
}

/* DISPNOT -25 -- p. 745: verb 06 noun 61 (REFLASH), "PRIMARILY USED BY THE
 * CREW IN P63" to reset the event timer to agree with TIG. */
void dispnot_minus25(void)
{
    set_A(V06N61);
    tc_to(DISPNOT - 025u + 1u, BANKCALL); /* CADR REFLASH */
}

/* DISPNOT -022 -- p. 745: Terminate: STOPCLOK; proceed: ASTNRETN; enter: TCF
 * -6, COMFAIL2. */
void dispnot_minus25_terminate(void)
{
    stopclok();
}

void dispnot_minus25_proceed(void)
{
    astnretn();
}

void dispnot_minus25_enter(void)
{
    dispnot_minus35_enter();
}

/* DISPNOT -17 -- p. 745 (CNTDNDEX): the countdown, each second from TIG -30
 * to TIG -5: the table's verb and noun (word 0) by REGODSP, which ends the
 * job. */
void dispnot_minus17(void)
{
    ca(which_table());                    /* INDEX WHICH; CAF 0 */
    tc_to(DISPNOT - 017u + 2u, BANKCALL); /* CADR REGODSP */
}

/* DISPNOT -13 -- p. 745 (VB99DEX, V99RECYC): "INDICATE VERB 99 PASTE"
 * (NVWORD1 -BIT9) with the table's noun: "THIS IS THE "PLEASE ENABLE
 * ENGINE"" display, from TIG -5 (CLOCPLAY). */
void dispnot_minus13(void)
{
    set_A(negate(BIT9));
    ts(ADDRESS_OF(NVWORD1));
    ca(which_table());                    /* INDEX WHICH; CAF 0 */
    tc_to(DISPNOT - 013u + 4u, BANKCALL); /* CADR CLOCPLAY */
}

/* DISPNOT -5 -- p. 745: Terminate: STOPCLOK; proceed: *PROCEED; enter:
 * *ENTER. */
void dispnot_minus13_terminate(void)
{
    stopclok();
}

void dispnot_minus13_proceed(void)
{
    star_proceed();
}

void dispnot_minus13_enter(void)
{
    star_enter();
}

/* DISPNOT -2 -- p. 746 (BLANKDEX): "BLANK DSKY." for five seconds at TIG -35
 * (CLEANDSP), */
void dispnot_minus2(void)
{
    tc_to(DISPNOT - 2u, BANKCALL); /* CADR CLEANDSP */
}

/* DISPNOT -- p. 746: and the job ends. */
void dispnot(void)
{
    continue_at(ENDOFJOB);
}

/* ---- Stopping the clock -- p. 746 ---- */

/* STOPCLOK -- p. 746: "STOP CLOKTASK & TURN OFF ULLAGE ON THE" way to P00
 * (NULLCLOK, then GOTOPOOH). */
void stopclok(void)
{
    tc_at(STOPCLOK);
    nullclok();
}

void stopclok_after_nullclok(void)
{
    continue_at(GOTOPOOH);
}

/* NULLCLOK -- p. 746: the return kept in P40/RET; "TURN OFF ULLAGE ..."
 * (NOULLAGE), */
void nullclok(void)
{
    inhint();
    qxch(ADDRESS_OF(P40_RET));
    tc_at(NULLCLOK + 3u);
    noullage();
}

/* NULLCLOK +4 -- p. 746: ULLGTASK killed (KILLTASK: "DON'T LET IT COME ON,
 * EITHER ..."), */
void nullclok_after_noullage(void)
{
    tc_at(NULLCLOK + 4u);
    killtask(); /* CADR ULLGTASK */
}

/* NULLCLOK +6 -- p. 746: group 1 ended ("NOT EVEN IF THERE'S A RESTART.";
 * OCT 1), */
void nullclok_after_killtask(void)
{
    tc_to(NULLCLOK + 6u, PHASCHNG);
}

/* NULLCLOK +010 -- p. 746: and "KILL CLOKTASK" (CA Z: DISPDEX positive).
 * Back by TC P40/RET: the word there runs as TC. Not modelled: a word there
 * that is not one of the three returns its QXCH stores (STOPCLOK's, GOPOST's
 * and GOCUTOFF's). */
void nullclok_after_phaschng(void)
{
    word back = ERASABLE(ADDRESS_OF(P40_RET));

    set_A(NULLCLOK + 011u);
    ts(ADDRESS_OF(DISPDEX));
    if (back != STOPCLOK + 1u && back != GOPOST + 010u && back != GOCUTOFF + 012u) not_modelled();
    tc_through(ADDRESS_OF(P40_RET));
}

/* ---- The crew's responses -- pp. 746-747 ---- */

/* ASTNRETN -- p. 746: group 4's restart (OCT 04024), */
void astnretn(void)
{
    tc_to(ASTNRETN, PHASCHNG);
}

/* ASTNRETN +2 -- p. 746: "STOP DISPLAYING BUT KEEP RUNNING." (DISPDEX +0),
 * and ASTNRET as a job (FINDVAC). */
void astnretn_after_phaschng(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DISPDEX));
    set_A(PRIO13);
    tc_to(ASTNRETN + 5u, FINDVAC); /* 2CADR ASTNRET */
}

void astnretn_after_findvac(void)
{
    continue_at(ENDOFJOB);
}

/* *PROCEED -- p. 746: ASTNFLAG set (the crew enabled the engine), then
 * IGNITE. */
void star_proceed(void)
{
    tc_to(STAR_PROCEED, UPFLAG); /* ADRES ASTNFLAG */
}

void star_proceed_after_upflag(void)
{
    ignite();
}

/* *ENTER -- p. 746: the table's word 3 (GOPOST, GOCUTOFF or V99RECYC). */
void star_enter(void)
{
    inhint();
    tcf_into_table(3u);
}

/* GOPOST -- p. 746: POSTBURN as a job (FINDVAC; it "MUST BE LOWER PRIORITY
 * THAN CLOKJOB"), */
void gopost(void)
{
    set_A(PRIO12_WORD);
    tc_to(GOPOST + 1u, FINDVAC); /* 2CADR POSTBURN */
}

/* GOPOST +4 -- p. 747: "SET UP THE DAP FOR COASTING FLIGHT." (ALLCOAST), */
void gopost_after_findvac(void)
{
    inhint();
    tc_to(GOPOST + 5u, IBNKCALL); /* CADR ALLCOAST */
}

/* GOPOST +7 -- p. 747: the clock stopped (NULLCLOK), */
void gopost_after_allcoast(void)
{
    tc_at(GOPOST + 7u);
    nullclok();
}

/* GOPOST +010 -- p. 747: and "4.13 RESTART FOR POSTBURN" (OCT 00134). */
void gopost_after_nullclok(void)
{
    tc_to(GOPOST + 010u, PHASCHNG);
}

void gopost_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* GOCUTOFF -- p. 747: CUTOFF as a job (FINDVAC), FLUNDISP reset, */
void gocutoff(void)
{
    set_A(PRIO17);
    tc_to(GOCUTOFF + 1u, FINDVAC); /* 2CADR CUTOFF */
}

void gocutoff_after_findvac(void)
{
    tc_to(GOCUTOFF + 4u, DOWNFLAG); /* ADRES FLUNDISP */
}

/* GOCUTOFF +6 -- p. 747: the DAP set for coasting flight (ALLCOAST), */
void gocutoff_after_downflag(void)
{
    inhint();
    tc_to(GOCUTOFF + 7u, IBNKCALL); /* CADR ALLCOAST */
}

/* GOCUTOFF +011 -- p. 747: the clock stopped (NULLCLOK), */
void gocutoff_after_allcoast(void)
{
    tc_at(GOCUTOFF + 011u);
    nullclok();
}

/* GOCUTOFF +012 -- p. 747: and CUTOFF's restart (OCT 07024, OCT 17000, 2CADR
 * CUTOFF). */
void gocutoff_after_nullclok(void)
{
    tc_to(GOCUTOFF + 012u, PHASCHNG);
}

void gocutoff_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* IGNITE: with IGNFLAG still clear (before TIG: CS FLAGWRD7 leaves its
 * bit set, CCS positive), only IGNITE1; once it is set (TIG has passed,
 * +0), IGNITION at once (TWIDDLE, BIT1). (A MASK leaves no negative
 * word.) -- p. 747 */
void ignite(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(IGNFLBIT);
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: ignite1(); break;
    case IS_PLUS_ZERO:
        set_A(BIT1);
        inhint();
        tc_to(IGNITE + 6u, TWIDDLE); /* ADRES IGNITION */
        break;
    default: not_modelled();
    }
}

/* IGNITE +010 -- p. 747: "IMMEDIATE RESTART AT IGNITION." (group 4: OCT23
 * and its complement), */
void ignite_after_twiddle(void)
{
    set_A(OCT23);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE4));
    ignite1();
}

/* IGNITE1 -- p. 747: "RESTORE OLD DISPLAY." (DISPDEX -CNTDNDEX). */
void ignite1(void)
{
    set_A(negate(CNTDNDEX));
    ts(ADDRESS_OF(DISPDEX));
    continue_at(ENDOFJOB);
}

/* ---- P40ALM and REP40ALM -- p. 748 ---- */

/* P40ALM -- p. 748: alarm 1706, "PROGRAM SELECTION NOT CONSISTENT WITH" the
 * vehicle's configuration, */
void p40alm(void)
{
    tc_to(P40ALM, ALARM); /* OCT 1706 */
}

/* REP40ALM -- p. 748: P40's and P42's word 14: verb 05 noun 09 (GOFLASH). */
void rep40alm(void)
{
    set_A(V05N09);
    tc_to(REP40ALM + 1u, BANKCALL); /* CADR GOFLASH */
}

/* REP40ALM +3 -- p. 748: V34E: terminate; */
void rep40alm_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* REP40ALM +4 -- p. 748: proceed: the table's word 14 (P42STAGE for P42,
 * "EVEN THOUGH VEHICLE IS UNSTAGED"); */
void rep40alm_proceed(void)
{
    tcf_into_table(014u);
}

/* REP40ALM +5 -- p. 748: V32E: "REDISPLAY ALARM". */
void rep40alm_recycle(void)
{
    rep40alm();
}

/* ---- P40AUTO -- p. 748 (bank 35) ---- */

/* P40AUTO -- p. 748: "HELLO THERE." The caller's return as a CADR
 * (MAKECADR), */
void p40auto(void)
{
    tc_to(P40AUTO, MAKECADR);
}

/* P40AUTO +1 -- p. 748: kept in TEMPR60 "FOR GENERALIZED RETURN TO OTHER
 * BANKS." Not modelled: an overflow at the TS (a CADR never overflows). */
void p40auto_after_makecadr(void)
{
    if (ts(ADDRESS_OF(TEMPR60))) not_modelled();
    p40a_p();
}

/* P40A/P -- p. 748: "SUBROUTINE TO CHECK PGNCS CONTROL" and auto
 * stabilization (G+N,AUTO), */
void p40a_p(void)
{
    tc_to(P40A_P, BANKCALL); /* CADR G+N,AUTO */
}

/* P40A/P +2 -- p. 748: which leaves A +0 in PGNCS and auto, positive if not.
 * Then "ARE WE ON THE DESCENT STAGE?" (APSFLAG clear), and if so the
 * autothrottle (channel 30 bit 5 +0). Either way GOBACK; else the checklist
 * (TURNITON). The CCSs skip as the AGC's do: a negative A goes on at the
 * MASK, -0 at the second CCS (with A +0). (A MASK leaves no negative
 * word.) */
void p40a_p_after_gpn_auto(void)
{
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: turniton(); return;
    case IS_PLUS_ZERO: set_A(APSFLBIT);           /* fall through */
    case NEGATIVE: mask_at(ADDRESS_OF(FLGWRD10)); /* fall through */
    case IS_MINUS_ZERO: break;
    }
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: goback(); return;
    case IS_PLUS_ZERO: break;
    default: not_modelled();
    }
    set_A(BIT5); /* "YES, CHECK FOR AUTOTHROTTLE MODE" */
    rand_channel(ADDRESS_OF(CHAN30));
    if (A_is_zero()) { /* EXTEND; BZF GOBACK */
        goback();
        return;
    }
    turniton();
}

/* TURNITON -- p. 748: verb 50 noun 25, checklist 203, "TURN ON PGNCS ETC."
 * (GOPERF1), */
void turniton(void)
{
    set_A(P40A_PMD);
    tc_to(TURNITON + 1u, BANKCALL); /* CADR GOPERF1 */
}

/* TURNITON +3 -- p. 748: V34E: terminate; */
void turniton_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* TURNITON +4 -- p. 748: recycle: check again. */
void turniton_recycle(void)
{
    p40a_p();
}

/* GOBACK -- p. 748: to the caller (BANKJUMP to the CADR in TEMPR60). */
void goback(void)
{
    ca(ADDRESS_OF(TEMPR60));
    tc_to(GOBACK + 1u, BANKJUMP);
}

#endif
