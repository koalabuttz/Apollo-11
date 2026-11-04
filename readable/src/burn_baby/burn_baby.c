/* SPDX-License-Identifier: MIT */
/* BURN,_BABY,_BURN_--_MASTER_IGNITION_ROUTINE.agc, pp. 731-743 (Luminary099
 * only): the ignition tables, BURNBABY and its countdown tasks TIG-35 ..
 * TIG-0, the program-specific ignition paths, the zoom tasks, the COMFAIL
 * family, INVFLAG, NOULLAGE and ONULLAGE. The faithful C is
 * c64-port/src/lm_burn_baby.c.
 *
 * One routine serves every burn program. The program names its ignition
 * table in WHICH (P12TABLE, P40TABLE, P41TABLE -5, P42TABLE, P63TABLE or
 * ABRTABLE), and the routine reads it with INDEX WHICH: word 0 the
 * countdown display's verb and noun, word 6 the ullage time, words 7 and 8
 * AVEGEXIT's 2CADR, and the others TCFs to the program's own steps. */
#include "burn_baby.h"
#include "../agc/names.h"

#if LUMINARY099
#include "../interpreter/interpreter.h"

#define COUNT(array)   (sizeof(array) / sizeof((array)[0]))
#define BURN_BABY_BANK 036u

/* TC at, to a routine out of the file or to INTPRET: Q gets the return
 * and the routine runs next. */
static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* EXTEND; DCA NEG0; DXCH -PHASEn: restart group n ends (-PHASEn -0, PHASEn
 * +0, from NEG0 and the word after it). */
static void end_group(unsigned minus_phase)
{
    dca(ADDRESS_OF(NEG0));
    dxch(minus_phase);
}

/* ---- The ignition tables -- p. 731-733 ---- */

/* INDEX WHICH: the program's table. Not modelled: a WHICH that is not one
 * of the six addresses the burn programs store there (P41's is P41TABLE
 * -5, which shares P40TABLE's first five words). */
static word which_table(void)
{
    word table = ERASABLE(ADDRESS_OF(WHICH));

    if (table != P12TABLE && table != P40TABLE && table != P41TABLE - 5u && table != P42TABLE &&
        table != P63TABLE && table != ABRTABLE)
        not_modelled();
    return table;
}

/* INDEX WHICH; TCF n: on to the table's word n, which runs as an
 * instruction. It is a TCF to the program's step (or out of the file, to
 * TASKOVER or P42STAGE); ABRTABLE's NOOPs (TCF to the next word) lead on
 * to its next TCF, and the word after its last (word 14) is BURNBABY's
 * first. Not modelled: a data word (VN, DEC or 2CADR). */
static void tcf_into_table(word table, word n)
{
    word at = (word)(table + n);
    word instruction;

    while (at >= P12TABLE && at < BURNBABY) {
        instruction = fetch_fixed(at);
        if (OPERATION_CODE(instruction) != 1u || ADDRESS_FIELD(instruction) < 02000u)
            not_modelled();
        at = ADDRESS_FIELD(instruction);
    }
    continue_at(at);
}

/* P41TABLE (5): P41's SPOT, TCF P41SPOT -- p. 732 */
void p41table(void)
{
    p41spot();
}

/* ---- BURNBABY -- p. 733 ---- */

/* BURNBABY: group 4 restarts here (TC PHASCHNG, OCT 04024), */
void burnbaby(void)
{
    tc_to(BURNBABY, PHASCHNG);
}

/* DVTOTAL cleared ("EXTIRPATE JUNK LEFT IN DVTOTAL"), and P40AUTO, which
 * "MUST BE BANKCALLED EVEN FROM ITS" own bank. */
void burnbaby_after_phaschng(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(DVTOTAL));
    ts(ADDRESS_OF(DVTOTAL) + 1u);
    tc_to(BURNBABY + 5u, BANKCALL); /* CADR P40AUTO */
}

/* B*RNB*B*: TIG kept in GOBLTIME, the engine commanded off (ENGINOF3), */
void b_star_rnb_star_b_star(void)
{
    dca(ADDRESS_OF(TIG)); /* EXTEND; DCA TIG: "STORE NOMINAL TIG FOR OBLATENESS COMP." */
    dxch(ADDRESS_OF(GOBLTIME));
    inhint();
    tc_to(B_STAR_RNB_STAR_B_STAR + 4u, IBNKCALL); /* CADR ENGINOF3 */
}

/* and on to the program's SPOT, table word 5. */
void b_star_rnb_star_b_star_after_enginof3(void)
{
    relint();
    tcf_into_table(which_table(), 5u);
}

/* P40SPOT (P42SPOT, P12SPOT): the countdown clock (STCLOK2 with
 * -CNTDNDEX), which "MUST BE BANKCALLED FOR GENERALIZED" return. -- p. 733 */
void p40spot(void)
{
    set_A(negate(CNTDNDEX));
    tc_to(P40SPOT + 1u, BANKCALL); /* CADR STCLOK2 */
}

/* ---- P41SPOT (P63SPOT) -- p. 734 ---- */

/* P41SPOT: to the interpreter, for P41SPOT_CODE: the CSM's state vector
 * integrated to TIG -29.9 seconds, then the LM's (MIDTOAV1). */
void p41spot(void)
{
    tc_to(P41SPOT, INTPRET);
}

/* TDEC1 is TIG -29.9 seconds; INITCDUW; the CSM's state (CSMPREC, unless
 * MUNFLAG is set) in V(CSM), R(CSM) and its gravity in G(CSM); TDEC1
 * again; and MIDTOAV1 integrates the LM's state to it. */
static const word p41spot_words[] = {
    ORDERS(DLOAD, DSU),
    ADDRESS_PLUS_ONE(TIG),
    ADDRESS_PLUS_ONE(D29_9SEC),
    STCALL(TDEC1),
    ADDRESS(INITCDUW),
    ORDERS(BOFF, CALL),
    SWITCH(BOF, MUNFLAG),
    ADDRESS(GOMIDAV),
    ADDRESS(CSMPREC),
    ORDERS(VLOAD, MXV),
    ADDRESS_PLUS_ONE(VATT1),
    ADDRESS_PLUS_ONE(REFSMMAT),
    ORDER(VSR1),
    STOVL(V_CSM), /* "CSM VELOCITY - M/CS*2(7)" */
    ADDRESS_PLUS_ONE(RATT1),
    ORDERS(VSL4, MXV),
    ADDRESS_PLUS_ONE(REFSMMAT),
    STCALL(R_CSM), /* "CSM POSITION - M*2(24)" */
    ADDRESS(MUNGRAV),
    STODL(G_CSM), /* "CSM GRAVITY VEC. - M/CS*2(7)" */
    ADDRESS_PLUS_ONE(TAT),
    STORE(TDEC1),              /* "RELOAD TDEC1 FOR MIDTOAV." */
    /* GOMIDAV */ ORDER(CALL), /* CALRB, which assembles as CALL */
    ADDRESS(MIDTOAV1),
};

/* MIDTOAV1's return: "MADE IT IN TIME." */
void gomidav_after_midtoav1(void)
{
    callt_35();
}

/* Its other return: "TIG WAS SLIPPED", so TIG becomes 29.9 seconds after
 * the time the integration reached (PIPTIME1, the low word of PIPTIME). */
void gomidav_after_midtoav1_slipped(void)
{
    dca(ADDRESS_OF(PIPTIME1));
    dxch(ADDRESS_OF(TIG));
    dca(D29_9SEC);
    das(ADDRESS_OF(TIG));
    callt_35();
}

/* CALLT-35: the time until TIG -30 (MPAC, kept in SAVET-30), less five
 * seconds, until TIG -35, when TIG-35 runs (LONGCALL); */
void callt_35(void)
{
    dxch(ADDRESS_OF(MPAC));
    dxch(ADDRESS_OF(SAVET_30)); /* "DELTA-T UNTIL TIG-30" */
    dcs(FIVE_SECDP);
    das(ADDRESS_OF(SAVET_30)); /* "DELTA-T UNTIL TIG-35" */
    dca(ADDRESS_OF(SAVET_30));
    tc_to(CALLT_35 + 7u, LONGCALL); /* 2CADR TIG-35 */
}

/* group 4's restart at TIG-35 (OCT 20254), */
void callt_35_after_longcall(void)
{
    tc_to(CALLT_35 + 012u, PHASCHNG);
}

/* and P63 (CHECKMM, DEC 63) shows its countdown now. */
void callt_35_after_phaschng(void)
{
    tc_to(CALLT_35 + 014u, CHECKMM);
}

void callt_35_not_p63(void)
{
    continue_at(ENDOFJOB);
}

/* "P63 CAN START DISPLAYING NOW.": DISPDEX -CNTDNDEX, and to the
 * interpreter for ABVEL_CODE. */
void callt_35_p63(void)
{
    set_A(negate(CNTDNDEX));
    ts(ADDRESS_OF(DISPDEX));
    tc_to(CALLT_35 + 021u, INTPRET);
}

/* ABVEL, the length of VN1 ("INITIALIZE ABVEL FOR P63 DISPLAY"). */
static const word abvel_words[] = {
    ORDERS(VLOAD, ABVAL),
    ADDRESS_PLUS_ONE(VN1),
    STORE(ABVEL),
    ORDER(EXIT),
};

/* After the EXIT, the job ends. */
void callt_35_after_exit(void)
{
    continue_at(ENDOFJOB);
}

/* ---- The countdown tasks -- pp. 735-741 ---- */

/* TIG-35: TIG-30 in five seconds (TWIDDLE), */
void tig_35(void)
{
    set_A(FIVE_SEC);
    tc_to(TIG_35 + 1u, TWIDDLE); /* ADRES TIG-30 */
}

/* group 4's restart at TIG-30 (OCT 40154), */
void tig_35_after_twiddle(void)
{
    tc_to(TIG_35 + 3u, PHASCHNG);
}

/* the DSKY blank for five seconds (DISPDEX -BLANKDEX). The table's ullage
 * time (word 6) is negative only for P41, which has no countdown clock: it
 * also gets TIG-30.1 in 4.9 seconds, to put the display back. */
void tig_35_after_phaschng(void)
{
    set_A(negate(BLANKDEX));
    ts(ADDRESS_OF(DISPDEX));
    cs(which_table() + 6u);     /* INDEX WHICH; CS 6: "CHECK ULLAGE TIME." */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF TASKOVER */
        continue_at(TASKOVER);
        return;
    }
    set_A(FOUR_9SEC);
    tc_to(TIG_35 + 014u, TWIDDLE); /* ADRES TIG-30.1 */
}

/* and P41BLANK as a job (NOVAC), to blank the DSKY. */
void tig_35_after_second_twiddle(void)
{
    set_A(PRIO17);
    tc_to(TIG_35 + 017u, NOVAC); /* 2CADR P41BLANK */
}

void tig_35_after_novac(void)
{
    continue_at(TASKOVER);
}

/* P41BLANK: "BLANK DSKY." (CLEANDSP), */
void p41blank(void)
{
    tc_to(P41BLANK, BANKCALL); /* CADR CLEANDSP */
}

void p41blank_after_cleandsp(void)
{
    continue_at(ENDOFJOB);
}

/* TIG-30.1: TIG-30A as a job, to put the display back at TIG -30, */
void tig_30_1(void)
{
    set_A(PRIO17);
    tc_to(TIG_30_1 + 1u, NOVAC); /* 2CADR TIG-30A */
}

void tig_30_1_after_novac(void)
{
    continue_at(TASKOVER);
}

/* TIG-30A: verb 16 noun 85 again (REGODSP, which ends the job). */
void tig_30a(void)
{
    set_A(V16N85B);
    tc_to(TIG_30A + 1u, BANKCALL); /* CADR REGODSP */
}

/* TIG-30: TIG-5 in 24.9 seconds (TWIDDLE), */
void tig_30(void)
{
    set_A(S24_9SEC);
    tc_to(TIG_30 + 1u, TWIDDLE); /* ADRES TIG-5 */
}

/* the countdown clock again (DISPDEX -CNTDNDEX). With a positive ullage
 * time (table word 6, kept in SAVET-30 for a restart) ULLGTASK runs then;
 * with none, on to ULLGNOT. */
void tig_30_after_twiddle(void)
{
    set_A(negate(CNTDNDEX));
    ts(ADDRESS_OF(DISPDEX));
    ca(which_table() + 6u);     /* INDEX WHICH; CAF 6 */
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF ULLGNOT */
        ullgnot();
        return;
    }
    ts(ADDRESS_OF(SAVET_30));
    tc_to(TIG_30 + 012u, TWIDDLE); /* ADRES ULLGTASK */
}

/* ULLGTASK's restart (group 1 at its third option, TBASE1 now), then the
 * table's word 1. */
void tig_30_after_second_twiddle(void)
{
    set_A(THREE);
    ts(L_ADDRESS);
    set_A(negate(THREE));
    dxch(ADDRESS_OF(MINUS_PHASE1));
    cs(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TBASE1));
    tcf_into_table(which_table(), 1u);
}

/* WANTAPS: P42's word 1. "FOR P42 ENSURE APSFLAG IS SET." */
void wantaps(void)
{
    cs(ADDRESS_OF(FLGWRD10));
    mask(APSFLBIT);
    ads(ADDRESS_OF(FLGWRD10));
    ullgnot();
}

/* ULLGNOT: the other programs' word 1: AVEGEXIT gets the table's 2CADR
 * (words 7 and 8), and group 4 restarts at REDO4.2 and at TIG-5 (its
 * second option, TBASE4 now); */
void ullgnot(void)
{
    dca(which_table() + 7u); /* EXTEND; INDEX WHICH; DCA 7 */
    dxch(ADDRESS_OF(AVEGEXIT));
    set_A(TWO);
    ts(L_ADDRESS);
    set_A(negate(TWO));
    dxch(ADDRESS_OF(MINUS_PHASE4));
    cs(ADDRESS_OF(TIME1));
    ts(ADDRESS_OF(TBASE4)); /* "SET TBASE4 FOR TIG-5 RESTART" */
    redo2_17();
}

/* REDO2.17: group 2 ends ("CLEAR OUT GROUP 2 SO LAMBERT CAN START"); */
void redo2_17(void)
{
    end_group(ADDRESS_OF(MINUS_PHASE2));
    redo4_2();
}

/* REDO4.2: "IS SERVICER GOING?" (PHASE5). If not, PREREAD starts it and
 * ends the task. Not modelled: a negative PHASE5, whose CCS would run the
 * CADR word; a -0 one runs on into ULLGTASK. */
void redo4_2(void)
{
    switch (ccs_at(ADDRESS_OF(PHASE5))) {
    case POSITIVE: continue_at(TASKOVER); break;
    case IS_PLUS_ZERO: tc_to(REDO4_2 + 2u, POSTJUMP); break; /* CADR PREREAD */
    case NEGATIVE: not_modelled(); break;
    case IS_MINUS_ZERO: ullgtask(); break;
    }
}

/* ULLGTASK: "THIS COMES AT TIG-7.5 OR TIG-3.5": ullage on (ONULLAGE), */
void ullgtask(void)
{
    tc_at(ULLGTASK);
    onullage();
}

/* and group 1 ends (OCT 1). */
void ullgtask_after_onullage(void)
{
    tc_to(ULLGTASK + 1u, PHASCHNG);
}

void ullgtask_after_phaschng(void)
{
    continue_at(TASKOVER);
}

/* TIG-5: group 3 ends ("INSURE THAT GROUP 3 IS INACTIVE."), TIG-0 in five
 * seconds, */
void tig_5(void)
{
    end_group(ADDRESS_OF(MINUS_PHASE3));
    set_A(FIVE_SEC);
    tc_to(TIG_5 + 4u, TWIDDLE); /* ADRES TIG-0 */
}

/* IGNFLAG and ASTNFLAG reset "FOR LIGHT-UP LOGIC" (DOWNFLAG), */
void tig_5_after_twiddle(void)
{
    tc_to(TIG_5 + 6u, DOWNFLAG); /* ADRES IGNFLAG */
}

void tig_5_after_downflag(void)
{
    tc_to(TIG_5 + 010u, DOWNFLAG); /* ADRES ASTNFLAG */
}

/* and the table's word 11. */
void tig_5_after_second_downflag(void)
{
    tcf_into_table(which_table(), 011u);
}

/* P40SJUNK: P40's and P42's word 11. "S40.13 IN PROGRESS?" (PHASE3): if
 * not, S40.13 as a job (FINDVAC). Not modelled: a negative or -0 PHASE3,
 * whose CCS would run the FINDVAC with A not a priority, or a 2CADR word. */
void p40sjunk(void)
{
    switch (ccs_at(ADDRESS_OF(PHASE3))) {
    case POSITIVE: dispchng(); break;
    case IS_PLUS_ZERO:
        set_A(PRIO20);
        tc_to(P40SJUNK + 3u, FINDVAC); /* 2CADR S40.13 */
        break;
    default: not_modelled();
    }
}

/* and group 3's restart for S40.13 (OCT 00053). */
void p40sjunk_after_findvac(void)
{
    tc_to(P40SJUNK + 6u, PHASCHNG);
}

/* DISPCHNG: the other programs' word 11: the verb 99 display next
 * (DISPDEX -VB99DEX), */
void dispchng(void)
{
    set_A(negate(VB99DEX));
    ts(ADDRESS_OF(DISPDEX));
    common();
}

/* COMMON (P41's word 11): "RESTART TIG-0 (4.7SPOT)" (OCT 40074). */
void common(void)
{
    tc_to(COMMON, PHASCHNG);
}

void common_after_phaschng(void)
{
    continue_at(TASKOVER);
}

/* TIG-0: "SET IGNFLAG SINCE TIG HAS ARRIVED"; for P63 (CHECKMM) the
 * throttle-up (P63ZOOM) is ZOOMTIME after nominal ignition. */
void tig_0(void)
{
    cs(ADDRESS_OF(FLAGWRD7));
    mask(IGNFLBIT);
    ads(ADDRESS_OF(FLAGWRD7));
    tc_to(TIG_0 + 3u, CHECKMM); /* DEC 63 */
}

void tig_0_not_p63(void)
{
    ignyet_q();
}

void tig_0_p63(void)
{
    ca(ADDRESS_OF(ZOOMTIME));
    tc_to(TIG_0 + 7u, WAITLIST); /* 2CADR P63ZOOM */
}

/* Its restart: groups 3 and 4 (OCT 40033, 05014, 77777). */
void tig_0_after_waitlist(void)
{
    tc_to(TIG_0 + 012u, TWO_PHSCHNG);
}

/* IGNYET?: "CHECK ASTNFLAG: HAS ASTRONAUT RESPONDED" to the engine enable
 * request? If not, the table's word 12. */
void ignyet_q(void)
{
    word table;

    set_A(ASTNBIT);
    mask_at(ADDRESS_OF(FLAGWRD7));
    table = which_table();
    if (A_is_zero()) { /* EXTEND; INDEX WHICH; BZF 12 */
        tcf_into_table(table, 012u);
        return;
    }
    ignition();
}

/* IGNITION: ENGONFLG set, the engine on (DSALMOUT bit 13), TEVENT the
 * time for the downlink, TIG brought up to date from TGO ("UPDATE TIG
 * USING TGO FROM S40.13"), FLUNDISP clear ("PERMIT GUIDANCE LOOP
 * DISPLAYS"), and the table's word 13. -- p. 738 */
void ignition(void)
{
    cs(ADDRESS_OF(FLAGWRD5));
    mask(ENGONBIT);
    ads(ADDRESS_OF(FLAGWRD5));
    set_A(negate(PRIO30)); /* "TURN ON THE ENGINE." */
    rand_channel(ADDRESS_OF(DSALMOUT));
    ad(BIT13);
    write_channel(ADDRESS_OF(DSALMOUT));
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(TEVENT));
    dca(ADDRESS_OF(TGO));
    dxch(ADDRESS_OF(TIG));
    dca(ADDRESS_OF(TIME2));
    das(ADDRESS_OF(TIG));
    set_A(negate(FLUNDBIT));
    mask_at(ADDRESS_OF(FLAGWRD8));
    ts(ADDRESS_OF(FLAGWRD8));
    tcf_into_table(which_table(), 013u);
}

/* P63IGN: P63's word 13: "INITIATE BURN DISPLAYS" (AVGEXIT P63DISPS -2),
 * the countdown clock killed (DISPDEX positive, CA Z), LETABORT set "FOR
 * P70-P71", SWANDISP set "TO ENABLE R10.", the DAP out of minimum impulse
 * (PULSES clear), TIG now "FOR P70 AND P71", WCHPHASE and WCHPHOLD +0 and
 * FLPASS0 2; then P42IGN. -- p. 739 */
void p63ign(void)
{
    dca(DSP2CADR);
    dxch(ADDRESS_OF(AVGEXIT));
    set_A(P63IGN + 4u); /* CA Z: the address of the next instruction */
    ts(ADDRESS_OF(DISPDEX));
    cs(ADDRESS_OF(FLAGWRD9));
    mask(LETABBIT);
    ads(ADDRESS_OF(FLAGWRD9));
    cs(ADDRESS_OF(FLAGWRD7));
    mask(SWANDBIT);
    ads(ADDRESS_OF(FLAGWRD7));
    set_A(negate(PULSES));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    dca(ADDRESS_OF(TIME2));
    dxch(ADDRESS_OF(TIG));
    set_A(ZERO);
    ts(ADDRESS_OF(WCHPHASE));
    ts(ADDRESS_OF(WCHPHOLD));
    set_A(TWO);
    ts(ADDRESS_OF(FLPASS0));
    p42ign();
}

/* P40IGN: P40's word 13. With NOTHRFLG set, P40ZOOM at ZOOMTIME
 * (WAITLIST), */
void p40ign(void)
{
    cs(ADDRESS_OF(FLAGWRD5));
    mask(NOTHRBIT);
    if (A_is_zero()) { /* EXTEND; BZF P42IGN */
        p42ign();
        return;
    }
    ca(ADDRESS_OF(ZOOMTIME));
    tc_to(P40IGN + 5u, WAITLIST); /* 2CADR P40ZOOM */
}

/* P63IGN1: and the zoom's restart, groups 3 and 4 ("3.3SPOT FOR ZOOM
 * RESTART."; OCT 40033, 05014, 77777); */
void p63ign1(void)
{
    tc_to(P63IGN1, TWO_PHSCHNG);
}

/* then P42IGN. */
void p63ign1_after_2phschng(void)
{
    p42ign();
}

/* P12IGN: P12's word 13: the DAP's bias acceleration estimates ("AT P12
 * IGNITION", IGNAOSQ and IGNAOSR to AOSQ and AOSR, under E-bank 6); then
 * ABRTIGN. -- p. 740 */
void p12ign(void)
{
    set_A(EBANK6);
    ts(EBANK_ADDRESS);
    ca(ADDRESS_OF(IGNAOSQ));
    ts(ADDRESS_OF(AOSQ));
    ca(ADDRESS_OF(IGNAOSR));
    ts(ADDRESS_OF(AOSR));
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    abrtign();
}

/* ABRTIGN: the aborts' word 13: the countdown clock killed (CA Z), the
 * ascent guidance connected to SERVICER (AVGEXIT ATMAG), SWANDISP set;
 * then P42IGN. */
void abrtign(void)
{
    set_A(ABRTIGN + 1u); /* CA Z */
    ts(ADDRESS_OF(DISPDEX));
    dca(ATMAGADR);
    dxch(ADDRESS_OF(AVGEXIT));
    cs(ADDRESS_OF(FLAGWRD7));
    mask(SWANDBIT);
    ads(ADDRESS_OF(FLAGWRD7));
    p42ign();
}

/* P42IGN: P42's word 13: DRIFTDFL clear ("ENSURE THAT POWERED-FLIGHT
 * SWITCHING CURVES ARE USED."), then the impulse switch (IMPULSW): set,
 * IMPLBURN (P40-P47); clear, DVMONCON. (A MASK leaves no negative word.) */
void p42ign(void)
{
    set_A(negate(DRIFTBIT));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    set_A(IMPULBIT);
    mask_at(ADDRESS_OF(FLAGWRD2));
    switch (ccs_at(A_ADDRESS)) {
    case POSITIVE: continue_at(IMPLBURN); break;
    case IS_PLUS_ZERO: dvmoncon(); break;
    default: not_modelled();
    }
}

/* DVMONCON: IGNFLAG, ASTNFLAG and IDLEFLAG reset ("CONNECT DVMON"), */
void dvmoncon(void)
{
    tc_to(DVMONCON, DOWNFLAG); /* ADRES IGNFLAG */
}

void dvmoncon_after_downflag(void)
{
    tc_to(DVMONCON + 2u, DOWNFLAG); /* ADRES ASTNFLAG */
}

void dvmoncon_after_second_downflag(void)
{
    tc_to(DVMONCON + 4u, DOWNFLAG); /* ADRES IDLEFLAG */
}

/* group 4's restart (OCT 40054), */
void dvmoncon_after_third_downflag(void)
{
    tc_to(DVMONCON + 6u, PHASCHNG);
}

/* and half a second later (FIXDELAY, DEC 50) ULLAGOFF. */
void dvmoncon_after_phaschng(void)
{
    tc_to(DVMONCON + 010u, FIXDELAY);
}

/* ULLAGOFF: ullage off (NOULLAGE), back to WAITABIT. */
void ullagoff(void)
{
    tc_at(ULLAGOFF);
    noullage();
}

/* WAITABIT: the tables' word 12: "KILL GROUP 4". */
void waitabit(void)
{
    end_group(ADDRESS_OF(MINUS_PHASE4));
    continue_at(TASKOVER);
}

/* TIGTASK: P41's word 12, on in bank 17 (POSTJUMP). -- p. 741 */
void tigtask(void)
{
    tc_to(TIGTASK, POSTJUMP); /* CADR TIGTASK1 */
}

/* TIGTASK1: TIGNOW as a job (NOVAC), */
void tigtask1(void)
{
    set_A(PRIO16);
    tc_to(TIGTASK1 + 1u, NOVAC); /* 2CADR TIGNOW */
}

/* and "KILL GROUP 6." (OCT 6). */
void tigtask1_after_novac(void)
{
    tc_to(TIGTASK1 + 4u, PHASCHNG);
}

void tigtask1_after_phaschng(void)
{
    continue_at(TASKOVER);
}

/* P63ZOOM: AVEGEXIT LUNLAND (LUNLANAD), the throttle to full (FLATOUT), */
void p63zoom(void)
{
    dca(LUNLANAD);
    dxch(ADDRESS_OF(AVEGEXIT));
    tc_to(P63ZOOM + 3u, IBNKCALL); /* CADR FLATOUT */
}

void p63zoom_after_flatout(void)
{
    p40zooma();
}

/* P40ZOOM: THRUST BIT13 and channel 14 bit 4 (the thrust drive), */
void p40zoom(void)
{
    set_A(BIT13);
    ts(ADDRESS_OF(THRUST));
    set_A(BIT4);
    wor_channel(ADDRESS_OF(CHAN14));
    p40zooma();
}

/* P40ZOOMA (ZOOM): group 3 ends (OCT 3). */
void p40zooma(void)
{
    tc_to(P40ZOOMA, PHASCHNG);
}

void p40zooma_after_phaschng(void)
{
    continue_at(TASKOVER);
}

/* ---- COMFAIL -- p. 742 ---- */

/* COMFAIL: IDLEFLAG set, FLUNDISP set ("TO SUPPRESS CONFLICTING DISPLAY"), */
void comfail(void)
{
    tc_to(COMFAIL, UPFLAG); /* ADRES IDLEFLAG */
}

void comfail_after_upflag(void)
{
    tc_to(COMFAIL + 2u, UPFLAG); /* ADRES FLUNDISP */
}

/* DVCNTR 4 ("RESET DVMON"), and the countdown clock started (STCLOK1)
 * unless it is running (PHASE6). Not modelled: a negative PHASE6, whose
 * CCS would run the CADR word; a -0 one goes on as a positive one does. */
void comfail_after_second_upflag(void)
{
    set_A(FOUR);
    ts(ADDRESS_OF(DVCNTR));
    switch (ccs_at(ADDRESS_OF(PHASE6))) { /* "CLOCKTASK ACTIVE?" */
    case POSITIVE:
    case IS_MINUS_ZERO: comfail_after_stclok1(); break;        /* TCF +3 */
    case IS_PLUS_ZERO: tc_to(COMFAIL + 010u, BANKCALL); break; /* CADR STCLOK1 */
    case NEGATIVE: not_modelled(); break;
    }
}

/* The verb 97 display next (DISPDEX -VB97DEX), and "TURN OFF GROUP 4."
 * (OCT 00004). */
void comfail_after_stclok1(void)
{
    set_A(negate(VB97DEX));
    ts(ADDRESS_OF(DISPDEX));
    tc_to(COMFAIL + 014u, PHASCHNG);
}

void comfail_after_phaschng(void)
{
    continue_at(ENDOFJOB);
}

/* COMFAIL1: the table's word 2 (COMFAIL3 or COMFAIL4). */
void comfail1(void)
{
    tcf_into_table(which_table(), 2u);
}

/* DISPDEX from A, then IDLEFLAG reset ("RECONNECT DV MONITOR"). */
static void comfail_display(void)
{
    ts(ADDRESS_OF(DISPDEX));
    tc_to(COMFAIL4 + 2u, DOWNFLAG); /* ADRES IDLEFLAG */
}

/* COMFAIL3: "KILL CLOKTASK USING Z" (CA Z: DISPDEX positive), */
void comfail3(void)
{
    set_A(COMFAIL3 + 1u);
    comfail_display(); /* TCF +2 */
}

/* COMFAIL4: or the countdown clock again (-CNTDNDEX); */
void comfail4(void)
{
    set_A(negate(CNTDNDEX));
    comfail_display();
}

/* FLUNDISP reset ("PERMIT GUIDANCE LOOP DISPLAYS"). */
void comfail4_after_downflag(void)
{
    tc_to(COMFAIL4 + 4u, DOWNFLAG); /* ADRES FLUNDISP */
}

void comfail4_after_second_downflag(void)
{
    continue_at(ENDOFJOB);
}

/* COMFAIL2: "KILL ZOOM RESTART PROTECTION" (OCT 00003), */
void comfail2(void)
{
    tc_to(COMFAIL2, PHASCHNG);
}

/* "KILL ZOOM, IN CASE IT'S STILL TO COME" (KILLTASK), */
void comfail2_after_phaschng(void)
{
    inhint();
    tc_at(COMFAIL2 + 3u);
    killtask(); /* CADR ZOOM */
}

/* "COMMAND ENGINE OFF" (ENGINOF4), */
void comfail2_after_killtask(void)
{
    tc_to(COMFAIL2 + 5u, IBNKCALL); /* CADR ENGINOF4 */
}

/* DRIFTDFL set, "USE OTHER RCS SYSTEM" (AORBTFLG inverted), ullage on
 * (ULLAGFLG), */
void comfail2_after_enginof4(void)
{
    tc_to(COMFAIL2 + 7u, UPFLAG); /* ADRES DRIFTDFL */
}

void comfail2_after_upflag(void)
{
    tc_at(COMFAIL2 + 011u);
    invflag(); /* ADRES AORBTFLG */
}

void comfail2_after_invflag(void)
{
    tc_to(COMFAIL2 + 013u, UPFLAG); /* ADRES ULLAGFLG */
}

/* and TIG-5 at once (TWIDDLE, BIT1). */
void comfail2_after_second_upflag(void)
{
    set_A(BIT1);
    inhint();
    tc_to(COMFAIL2 + 017u, TWIDDLE); /* ADRES TIG-5 */
}

void comfail2_after_twiddle(void)
{
    continue_at(ENDOFJOB);
}

/* ---- INVFLAG, NOULLAGE and ONULLAGE -- p. 743 ---- */

/* INVFLAG: TC INVFLAG; ADRES flag inverts the flag. DEBIT (with Q in A)
 * gives the flag's bit in A and its flag word's address in L; */
void invflag(void)
{
    ca(Q_ADDRESS);
    tc_to(INVFLAG + 1u, DEBIT);
}

/* the flag word with the bit inverted (COM; RXOR LCHAN) goes back
 * (COMFLAG). */
void invflag_after_debit(void)
{
    com();
    rxor_lchan();
    continue_at(COMFLAG);
}

/* TC Q: back to the TC in fixed memory that called. Not modelled: a Q
 * that is not such a return (02000-07777). */
static void return_to_q(void)
{
    word at = (word)Q;

    if (Q < 02000u || Q > 07777u) not_modelled();
    tc_at(Q_ADDRESS);
    continue_at(at);
}

/* NOULLAGE: ullage off (DAPBOOLS's ULLAGER clear). It "MUST BE CALLED IN
 * A TASK OR UNDER INHINT". */
void noullage(void)
{
    set_A(negate(ULLAGER));
    mask_at(ADDRESS_OF(DAPBOOLS));
    ts(ADDRESS_OF(DAPBOOLS));
    return_to_q();
}

/* ONULLAGE: "TURN ON ULLAGE." (ULLAGER set). */
void onullage(void)
{
    cs(ADDRESS_OF(DAPBOOLS));
    mask(ULLAGER);
    ads(ADDRESS_OF(DAPBOOLS));
    return_to_q();
}

const struct interpretive_code P41SPOT_CODE = {BURN_BABY_BANK, P41SPOT + 1u, COUNT(p41spot_words),
                                               p41spot_words};
const struct interpretive_code ABVEL_CODE = {BURN_BABY_BANK, CALLT_35 + 022u, COUNT(abvel_words),
                                             abvel_words};

#endif
