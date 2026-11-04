/* SPDX-License-Identifier: MIT */
/* P40-P47.agc, part 2 (Comanche055 pp. 694-699, bank 24): the burn
 * itself, from five seconds before ignition to the engine's shutdown. The
 * faithful C is c64-port/src/cm_p40_p47.c.
 *
 * TIG-5, a task, sets up TIG-0 for ignition and starts S40.13, which
 * keeps the time to go (TGO). TIG-0 sets IGNFLAG and waits for the
 * crew's go (verb 99, ASTNFLAG); IGNITION then notes the time (TEVENT),
 * sets ENGONFLG and turns the SPS engine on (SPSON). A burn short enough
 * to be timed (IMPULSW, "IMPULSIVE") takes IMPLBURN, which sets
 * ENGINOFF for the end of the time to go; a longer one is steered
 * (STRULLSW, then STEERSW). Either way PREPTVC ends the RCS autopilot,
 * and 0.4 seconds later DOTVCON starts the thrust vector control
 * autopilot (TVCDAPON); 1.6 seconds after that DOSTRULL turns the
 * ullage jets off. At the end ENGINOFF (a task) or the steering's cutoff
 * shuts the engine down (SPSOFF, which also takes back part of the mass
 * the autopilot took off ahead of time, MASSBACK, and keeps the trim it
 * found, BESTTRIM); 2.5 seconds on DOTVCRCS gives the attitude back to
 * the RCS autopilot (RCSDAPON), wipes out the TVC (TVCZAP) and starts
 * POSTBURN.
 *
 * Each call out of the file (WAITLIST, NEWPHASE, FIXDELAY, ...) leaves Z
 * at the routine and Q at the word after the TC; a TCF out of it
 * (TASKOVER) leaves Z there. */
#include "p40_p47_cm.h"
#include "../agc/names.h"

#if COMANCHE055

static void tc_to(word at, word routine)
{
    tc_at(at);
    continue_at(routine);
}

/* TC Q, a subroutine's return (as in programs.c): Z gets the address in
 * Q, which becomes 3. Not modelled: a Q that is no such address. */
static void tc_q(void)
{
    word at = overflow_corrected(Q);

    if (at < 010u || at > 07777u) not_modelled();
    Q = 3u;
    Z = at;
}

/* TCR E7SETTER and TCR E6SETTER at a word: EBANK 7 (or 6), and TC Q back
 * to the word after the TCR. */
static void tcr_e7setter(word at)
{
    tc_at(at);
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_q();
}

static void tcr_e6setter(word at)
{
    tc_at(at);
    set_A(EBANK6); /* "SET UP EBANK6" */
    ts(EBANK_ADDRESS);
    tc_q();
}

/* ---- TIG-5 -- p. 694 ---- */

/* TIG-5, a task: TIG-0 in five seconds (WAITLIST). */
void tig_5(void)
{
    set_A(FIVE_SEC);
    tc_to(TIG_5 + 1u, WAITLIST); /* 2CADR TIG-0 */
}

/* TIG-5 +4: NVWORD1 -BIT9, which "WILL CAUSE V99 FLASH"; restart groups
 * 4 and 3 (2PHSCHNG, OCT 40074, "4.7 = TIG-0 (500CS)", and OCT 00033,
 * "3.3 = S40.13 (PRIO20)"). */
void tig_5_after_waitlist(void)
{
    set_A(negate(BIT9)); /* "WILL CAUSE V99 FLASH" */
    ts(ADDRESS_OF(NVWORD1));
    tc_to(TIG_5 + 6u, TWO_PHSCHNG);
}

/* TIG-5 +011: S40.13 as a job (FINDVAC, PRIO20), */
void tig_5_after_2phschng(void)
{
    set_A(PRIO20);
    tc_to(TIG_5 + 012u, FINDVAC); /* 2CADR S40.13 */
}

/* and the task ends (TCF TASKOVER). */
void tig_5_after_findvac(void)
{
    continue_at(TASKOVER);
}

/* ---- TIG-0 and IGNITION -- pp. 695-697 ---- */

static void implburn(void);
static void preptvc(void);

/* TIG-0, a task at ignition: "SET IGN FLAG" (IGNFLAG, bit 13 of
 * FLAGWRD7); "CHECK ASTN FLAG FOR V99 RESPONSE" (bit 12): not yet, "WAIT
 * FOR V99P" (TASKOVER). Otherwise "CLEAR THE V99" (NVWORD1, verb 06 noun
 * 40), and "V99P HAS COME ALREADY, DO IGNITION NOW" (PHASCHNG, OCT 00614,
 * "4.61 = IGNITION (-0CS)"). */
void tig_0(void)
{
    cs(ADDRESS_OF(FLAGWRD7)); /* "SET IGN FLAG" */
    mask(BIT13);
    ads(ADDRESS_OF(FLAGWRD7));
    ca(ADDRESS_OF(FLAGWRD7)); /* "CHECK ASTN FLAG FOR V99 RESPONSE" */
    mask(BIT12);
    if (A_is_zero()) { /* EXTEND; BZF TASKOVER */
        continue_at(TASKOVER);
        return;
    }
    set_A(V06N40);
    ts(ADDRESS_OF(NVWORD1));
    tc_to(TIG_0 + 011u, PHASCHNG);
}

/* IGNITION: CDUX "FOR ROLL DAP REFERENCE" (OGAD); the time (TIME2) to
 * TEVENT; "SET ENGONFLG" (bit 7 of FLAGWRD5). SPSON: "TURN ON SPS ENGINE"
 * (bit 13 of DSALMOUT). IMPULCHK: "CHECK FOR IMPULSIVE BURN" (IMPULSW,
 * bit 9 of FLAGWRD2, by CCS A): IMPLBURN; or "NON-IMPULSIVE, SET STRULLSW
 * FOR STEERULL" (bit 13 of FLAGWRD6), and PREPTVC. */
void ignition(void)
{
    ca(ADDRESS_OF(CDUX));
    ts(ADDRESS_OF(OGAD));
    dca(ADDRESS_OF(TIME2)); /* EXTEND */
    dxch(ADDRESS_OF(TEVENT));
    cs(ADDRESS_OF(FLAGWRD5)); /* "SET ENGONFLG" */
    mask(BIT7);
    ads(ADDRESS_OF(FLAGWRD5));
    set_A(BIT13);   /* SPSON: "TURN ON SPS ENGINE" */
    wor_dsalmout(); /* EXTEND */
    set_A(BIT9);    /* IMPULCHK: "CHECK FOR IMPULSIVE BURN" */
    mask_at(ADDRESS_OF(FLAGWRD2));
    if (!A_is_zero()) { /* CCS A */
        implburn();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD6));
    mask(BIT13);
    ads(ADDRESS_OF(FLAGWRD6));
    preptvc();
}

/* PREPTVC: "RESET T5 BITS" (bits 15 and 14 of FLAGWRD6); "KILL RCS"
 * (T5LOC gets T5IDL24's 2CADR, T5IDLOC); "4.3 = DOTVCON (40CS)" (CS
 * THREE), restart group 4 (NEWPHASE). */
static void preptvc(void)
{
    set_A(negate(OCT60000)); /* "RESET T5 BITS" */
    mask_at(ADDRESS_OF(FLAGWRD6));
    ts(ADDRESS_OF(FLAGWRD6));
    dca(T5IDL24); /* EXTEND; "KILL RCS" */
    dxch(ADDRESS_OF(T5LOC));
    set_A(negate(THREE));
    tc_to(PREPTVC + 7u, NEWPHASE);
}

/* PREPTVC +011: "0.4 SECOND DELAY FOR THRUST BUILDUP" (FIXDELAY, DEC
 * 40), then DOTVCON. */
void preptvc_after_newphase(void)
{
    tc_to(PREPTVC + 011u, FIXDELAY);
}

/* DOTVCON: "SET TVCPHASE = TVCDAPON CALL (FRESHDAP)" (-1); TVCEXPHS +0,
 * "SET TVCEXECUTIVE PHASE"; "SET T5 BITS TO INDICATE TVC TAKEOVER" (bits
 * 15,14 = 10). Restart group 6: "6.3 = CLOKTASK (100CS), DROPPING
 * PRE40.6" (-PHASE6 and PHASE6 from A and L, -3 and 3). "STORE RCS
 * ATTITUDE ERRORS FOR USE IN INITIALIZING TVC ATTITUDE ERRORS" (ERRBTMP
 * gets ERRORY's pair, complemented). "4.5 = DOSTRULL (160 CS)" (CS FIVE),
 * restart group 4 (NEWPHASE). */
void dotvcon(void)
{
    set_A(negate(BIT1));
    ts(ADDRESS_OF(TVCPHASE));
    set_A(ZERO); /* "SET TVCEXECUTIVE PHASE" */
    ts(ADDRESS_OF(TVCEXPHS));
    set_A(negate(OCT60000)); /* "SET T5 BITS TO INDICATE TVC TAKEOVER ...." */
    mask_at(ADDRESS_OF(FLAGWRD6));
    ad(BIT15);
    if (!ts(ADDRESS_OF(FLAGWRD6))) set_A(THREE);
    ts(L_ADDRESS);
    com();
    dxch(ADDRESS_OF(MINUS_PHASE6));
    dcs(ADDRESS_OF(ERRORY)); /* EXTEND */
    dxch(ADDRESS_OF(ERRBTMP));
    set_A(negate(FIVE));
    tc_to(DOTVCON + 020u, NEWPHASE);
}

/* DOTVCON +022: "SET TIME5 FOR STARTING RIGHT AWAY" (POSMAX); T5LOC gets
 * TVCON2C's 2CADR, TVCDAPON, "(KILLS RCS DAP)"; "0.4 + 1.6 = 2.0 SEC FOR
 * ULLAGE-OFF AND STEERING (IF NON-IMPULSIVE)" (FIXDELAY, DEC 160), then
 * DOSTRULL. */
void dotvcon_after_newphase(void)
{
    set_A(POSMAX); /* "SET TIME5 FOR STARTING RIGHT AWAY" */
    ts(ADDRESS_OF(TIME5));
    dca(TVCON2C); /* EXTEND */
    dxch(ADDRESS_OF(T5LOC));
    tc_to(DOTVCON + 027u, FIXDELAY);
}

/* ULAGEOFF: "ZERO CHANNEL 5" (the RCS jets, the ullage among them); TC
 * Q. */
static void ulageoff(void)
{
    set_A(ZERO);
    write_channel(CHAN5); /* EXTEND */
    tc_q();
}

/* STEERULL: "SET STEERSW" (bit 11 of FLAGWRD2), and on into ULAGEOFF. */
static void steerull(void)
{
    cs(ADDRESS_OF(FLAGWRD2));
    mask(BIT11);
    ads(ADDRESS_OF(FLAGWRD2));
    ulageoff();
}

/* DOSTRULL: "CHECK STRULLSW FOR IMPULSIVE BURN" (bit 13 of FLAGWRD6, by
 * CCS A): "NON-IMPULSIVE, STEERING AND ULLAGE OFF" (TCR STEERULL); then
 * "ULLAGE OFF (ONLY, OR AGAIN)" (TCR ULAGEOFF). */
void dostrull(void)
{
    set_A(BIT13);
    mask_at(ADDRESS_OF(FLAGWRD6));
    if (!A_is_zero()) { /* CCS A */
        tc_at(DOSTRULL + 3u);
        steerull();
    }
    dostrull_after_steerull();
}

/* DOSTRULL +4, STEERULL's return: TCR ULAGEOFF. */
void dostrull_after_steerull(void)
{
    tc_at(DOSTRULL + 4u);
    ulageoff();
    dostrull_after_ulageoff();
}

/* DOSTRULL +5, ULAGEOFF's return: "KILL GROUP 4" (-PHASE4 and PHASE4 from
 * NEG0, "DP NEG0 = -0,+0"); ENDIGN, the task ends (TCF TASKOVER). */
void dostrull_after_ulageoff(void)
{
    dca(ADDRESS_OF(NEG0)); /* EXTEND */
    dxch(ADDRESS_OF(MINUS_PHASE4));
    continue_at(TASKOVER);
}

/* IMPLBURN: "RESET STRULLSW (COULD BE AN IMPULSIVE ENGINE FAIL)"; EBANK 7
 * (TCR E7SETTER). */
static void implburn(void)
{
    set_A(negate(BIT13));
    mask_at(ADDRESS_OF(FLAGWRD6));
    ts(ADDRESS_OF(FLAGWRD6));
    tcr_e7setter(IMPLBURN + 3u);
    implburn_after_e7setter();
}

/* IMPLBURN +4, E7SETTER's return: "PREPARE FOR R1 OF V06N40 (CLOCKTASK)"
 * (TIG gets TGO plus the time); restart groups 3 and 4 (2PHSCHNG, OCT
 * 40153, "3.15 = ENGINOFF (TGO+1)", and OCT 07014, IMPLCONT at once). */
void implburn_after_e7setter(void)
{
    dca(ADDRESS_OF(TGO)); /* EXTEND */
    dxch(ADDRESS_OF(TIG));
    dca(ADDRESS_OF(TIME2)); /* EXTEND */
    das(ADDRESS_OF(TIG));
    tc_to(IMPLBURN + 012u, TWO_PHSCHNG);
}

/* IMPLBURN +020: ENGINOFF after the time to go, TGO +1 (WAITLIST), which
 * returns to IMPLCONT. */
void implburn_after_2phschng(void)
{
    ca(ADDRESS_OF(TGO) + 1u);
    tc_to(IMPLBURN + 021u, WAITLIST); /* 2CADR ENGINOFF */
}

/* IMPLCONT: "RESET IMPULSW, ENGINOFF IS NOW SET UP" (bit 9 of FLAGWRD2);
 * EBANK 6 (TCR E6SETTER). */
void implcont(void)
{
    set_A(negate(BIT9));
    mask_at(ADDRESS_OF(FLAGWRD2));
    ts(ADDRESS_OF(FLAGWRD2));
    tcr_e6setter(IMPLCONT + 3u);
    implcont_after_e6setter();
}

/* IMPLCONT +4, E6SETTER's return: V97VCNTR +0, "SET UP V97VCNTR IN CASE
 * ENGINOFF" (its mass-back) "ARRIVES BEFORE TVCDAPON"; and PREPTVC. */
void implcont_after_e6setter(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(V97VCNTR));
    preptvc();
}

/* ---- ENGINOFF, DOTVCRCS, SPSOFF and TVCZAP -- pp. 697-699 ---- */

/* ENGINOFF, IMPLBURN's task at the end of the time to go: EBANK 6 (TCR
 * E6SETTER). */
void enginoff(void)
{
    tcr_e6setter(ENGINOFF);
    enginoff_after_e6setter();
}

/* ENGINOFF +1, E6SETTER's return: CSMMASS to MASSTMP, "COPYCYCLE FOR
 * MASSBACK"; "KILL GROUP 3 PROTECTION OF ENGINOFF" and "4.63 = DOSPSOFF
 * (-0CS)" (2PHSCHNG, OCT 00003, OCT 40634), which runs on into DOSPSOFF. */
void enginoff_after_e6setter(void)
{
    ca(ADDRESS_OF(CSMMASS));
    ts(ADDRESS_OF(MASSTMP)); /* "COPYCYCLE FOR MASSBACK" */
    tc_to(ENGINOFF + 3u, TWO_PHSCHNG);
}

/* DOSPSOFF: "SHUTDOWN SPS, MASS UPDATES, ETC." (TCR SPSOFF). */
void dospsoff(void)
{
    tc_at(DOSPSOFF);
    spsoff();
    dospsoff_after_spsoff();
}

/* DOSPSOFF +1, SPSOFF's return: "4.27 = DOTVCRCS (250 CS)" (CS OCT27/24),
 * restart group 4 (NEWPHASE). */
void dospsoff_after_spsoff(void)
{
    set_A(negate(OCT27_24));
    tc_to(DOSPSOFF + 2u, NEWPHASE);
}

/* DOSPSOFF +4: "2.5 SECOND DELAY FOR SPS TAILOFF" (FIXDELAY, DEC 250),
 * then DOTVCRCS. */
void dospsoff_after_newphase(void)
{
    tc_to(DOSPSOFF + 4u, FIXDELAY);
}

/* DOTVCRCS: "WIDE DEADBAND FOR CUTOFF TRANSIENT" (TCR SETMAXDB). */
void dotvcrcs(void)
{
    tc_at(DOTVCRCS);
    setmaxdb();
    dotvcrcs_after_setmaxdb();
}

/* DOTVCRCS +1, SETMAXDB's return: "SET UP RCS DAP" (IBNKCALL, CADR
 * RCSDAPON), */
void dotvcrcs_after_setmaxdb(void)
{
    tc_to(DOTVCRCS + 1u, IBNKCALL); /* CADR RCSDAPON */
}

/* "UPDATE WEIGHT/G AND MASS-PROPERTIES FOR RCS DAP STARTUP" (IBNKCALL,
 * CADR MASSPROP), */
void dotvcrcs_after_rcsdapon(void)
{
    tc_to(DOTVCRCS + 3u, IBNKCALL); /* CADR MASSPROP */
}

/* "WIPE OUT TVC, TURN OFF CLOKTASK" (TCR TVCZAP), */
void dotvcrcs_after_massprop(void)
{
    tc_at(DOTVCRCS + 5u);
    tvczap();
    dotvcrcs_after_tvczap();
}

/* restart group 4 (PHASCHNG, OCT 00354, "4.35 = POSTBURN (NOVAC,
 * PRIO12)"), */
void dotvcrcs_after_tvczap(void)
{
    tc_to(DOTVCRCS + 6u, PHASCHNG);
}

/* "SET UP POSTBURN V16N40 JOB" (NOVAC, PRIO12), */
void dotvcrcs_after_phaschng(void)
{
    set_A(PRIO12_WORD);
    tc_to(DOTVCRCS + 011u, NOVAC); /* 2CADR POSTBURN */
}

/* and the task ends (TCF TASKOVER). */
void dotvcrcs_after_novac(void)
{
    continue_at(TASKOVER);
}

/* SPSOFF: "ESTABLISH SPSOFF TEVENT" (TIME2); "RESET ENGONFLG" (bit 7 of
 * FLAGWRD5), "RESTARTS WILL SHUT DOWN SPS NOW"; "SHUT DOWN SPS ENGINE"
 * (bit 13 of DSALMOUT); "ISSUE SIV CUTOFF COMMAND" (bit 14 of channel
 * 12). MASSBACK: "RESTORE PART OF PRE-DECREMENTED MASS": V97VCNTR's
 * half-seconds of flow (EMDOT, MP 1SEC) back on MASSTMP, to CSMMASS (an
 * overflow there skips the CA TVCPHASE). BESTTRIM, "UPDATE TRIMS WITH
 * DELFILTER VALUES" (PACTOFF from DELPBAR, YACTOFF from DELYBAR), only
 * once the TVC is set up: TVCPHASE not -1 ("INITIALIZATION NOT
 * COMPLETE"), and bit 15 of FLAGWRD6 set, bit 14 clear ("NOT TVC YET").
 * TC Q. */
void spsoff(void)
{
    dca(ADDRESS_OF(TIME2)); /* EXTEND; "ESTABLISH SPSOFF TEVENT" */
    dxch(ADDRESS_OF(TEVENT));
    set_A(negate(BIT7)); /* "RESET ENGONFLG" */
    mask_at(ADDRESS_OF(FLAGWRD5));
    ts(ADDRESS_OF(FLAGWRD5));
    set_A(negate(BIT13));     /* "SHUT DOWN SPS ENGINE" */
    wand_dsalmout();          /* EXTEND */
    set_A(BIT14);             /* "ISSUE SIV CUTOFF COMMAND" */
    wor_channel(CHAN12);      /* EXTEND */
    ca(ADDRESS_OF(V97VCNTR)); /* MASSBACK */
    mp_at(ADDRESS_OF(EMDOT)); /* EXTEND */
    lxch(A_ADDRESS);
    mp(ONE_SEC); /* EXTEND */
    ad_at(ADDRESS_OF(MASSTMP));
    if (!ts(ADDRESS_OF(CSMMASS))) ca(ADDRESS_OF(TVCPHASE));
    ad(ONE);           /* "CHECK IF OK FOR TRIM UPDATE" */
    if (A_is_zero()) { /* EXTEND; BZF BTRIMR */
        tc_q();
        return;
    }
    cs(ADDRESS_OF(FLAGWRD6)); /* "YES, CHECK IF TVC" */
    mask(OCT60000);
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF BTRIMR */
        tc_q();
        return;
    }
    ca(ADDRESS_OF(DELPBAR)); /* BESTTRIM */
    ts(ADDRESS_OF(PACTOFF));
    ca(ADDRESS_OF(DELYBAR));
    ts(ADDRESS_OF(YACTOFF));
    tc_q(); /* BTRIMR */
}

/* TVCZAP: "DISABLE TVC AND OPT ERR CNTRLS, REENGAGE OPTICS DAC" (bits 2,
 * 8 and 11 of channel 12 clear, OCT02202); OPTIND -1, "ENABLE T4RUPT
 * OPTICS MONITOR"; "CLEAR NVWORD1 IN CASE CLOCKJOB WAITING"; "CLEAR
 * TIMRFLAG TO STOP CLOKTASK" (bit 11 of FLAGWRD7). TC Q. */
void tvczap(void)
{
    set_A(negate(OCT02202));
    wand_channel(CHAN12); /* EXTEND */
    set_A(negate(BIT1));  /* "ENABLE T4RUPT OPTICS MONITOR .... PERMIT" */
    ts(ADDRESS_OF(OPTIND));
    set_A(ZERO); /* "CLEAR NVWORD1 IN CASE CLOCKJOB WAITING" */
    ts(ADDRESS_OF(NVWORD1));
    set_A(negate(BIT11)); /* "CLEAR TIMRFLAG TO STOP CLOKTASK" */
    mask_at(ADDRESS_OF(FLAGWRD7));
    ts(ADDRESS_OF(FLAGWRD7));
    tc_q();
}

#endif
