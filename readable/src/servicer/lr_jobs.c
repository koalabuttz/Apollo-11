/* SPDX-License-Identifier: MIT */
/* SERVICER.agc, pp. 892-897 (Luminary099 only, banks 33 and 34): the
 * landing radar's jobs and task. LRVJOB, which reads a velocity beam, and
 * LRHJOB, the altitude, each asleep in RADSTALL while the radar samples;
 * RDGIMS, the task that reads the CDUs and PIPAs midway through a
 * velocity reading; HIGATJOB, which moves the antenna to position 2 before
 * high gate; and SETPOS1, SETPOS2 and SETPOS, which set up the beams'
 * vectors for the antenna's position. The faithful C is
 * c64-port/src/lm_servicer.c. */
#include "servicer.h"
#include "../interpreter/interpreter.h"

#if LUMINARY099

/* ---- LRVJOB, ENDLRV, VBAD -- pp. 892-893 ---- */

/* LRVJOB: "SET TASK TO READ CDUS + PIPAS" (RDGIMS, 170MS on); ... */
void lrvjob(void)
{
    set_A(K170MS);
    tc_to(LRVJOB + 1u, WAITLIST); /* 2CADR RDGIMS */
}

/* ... "SEQUENCE LR VEL BEAM SELECTOR": VSELECT less one, two in place of
 * +0, "2XVSELECT USED FOR VBEAM INDEX IN LRVEL" (a -0 VSELECT, which the
 * program never holds, leaves +0); "GO INITIALIZE LR VEL READ ROUTINE"
 * (LRVEL, through BANKCALL); ... */
void lrvjob_after_waitlist(void)
{
    switch (ccs_at(ADDRESS_OF(VSELECT))) {
    case POSITIVE:
    case NEGATIVE: double_A(); break;
    case IS_PLUS_ZERO:
        set_A(TWO); /* "IF ZERO, RESET TO TWO" */
        double_A();
        break;
    case IS_MINUS_ZERO: break;
    }
    tc_to(LRVJOB + 010u, BANKCALL); /* CADR LRVEL */
}

/* ... "PUT LRVJOB TO SLEEP ABOUT 500 MS" (RADSTALL), ... */
void lrvjob_after_lrvel(void)
{
    tc_to(LRVJOB + 012u, BANKCALL); /* CADR RADSTALL */
}

/* ... the bad return: VBAD; ... */
void lrvjob_bad_exit(void)
{
    vbad();
}

/* ... the good one: "IS DATA GOOD JUST PRESENT?" (STILBADV): "JUST GOOD --
 * MUST WAIT 4 SECONDS." (VSTILBAD, with it less one). Else, interrupts
 * held, "GOOD RETURN -- STOW AWAY VMEAS" (SAMPLSUM); "FOR DOWNLINK" (E
 * bank 4) the reading's time and CDUs (LRVTIMDL, LRXCDUDL, LRZCDUDL);
 * "SET BIT TO INDICATE VELOCITY" "MEASUREMENT MADE" (VELDABIT), and
 * ENDLRV. (A negative STILBADV skips the INHINT; a -0 one, onto the DCA
 * after its EXTEND, is not modelled.) */
void lrvjob_good_exit(void)
{
    switch (ccs_at(ADDRESS_OF(STILBADV))) {
    case POSITIVE: vstilbad(); return;
    case IS_PLUS_ZERO: inhint(); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    dca(ADDRESS_OF(SAMPLSUM));
    dxch(ADDRESS_OF(VMEAS));
    set_A(EBANK4); /* "FOR DOWNLINK" */
    ts(EBANK_ADDRESS);
    dca(ADDRESS_OF(LRVTIME));
    dxch(ADDRESS_OF(LRVTIMDL));
    dca(ADDRESS_OF(LRXCDU));
    dxch(ADDRESS_OF(LRXCDUDL));
    ca(ADDRESS_OF(LRZCDU));
    ts(ADDRESS_OF(LRZCDUDL));
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    cs(ADDRESS_OF(FLGWRD11)); /* "SET BIT TO INDICATE VELOCITY" */
    mask(VELDABIT);           /* "MEASUREMENT MADE" */
    ads(ADDRESS_OF(FLGWRD11));
    endlrv();
}

/* ENDLRV: "UPDATE VSELECT": less one, two in place of +0 (a -0 VSELECT
 * stays), and the job ends. */
void endlrv(void)
{
    switch (ccs_at(ADDRESS_OF(VSELECT))) {
    case POSITIVE:
    case NEGATIVE: ts(ADDRESS_OF(VSELECT)); break;
    case IS_PLUS_ZERO:
        set_A(TWO);
        ts(ADDRESS_OF(VSELECT));
        break;
    case IS_MINUS_ZERO: break;
    }
    continue_at(ENDOFJOB);
}

/* VBAD: "SET STILBAD TO WAIT 4 SECONDS" (two passes); ... */
void vbad(void)
{
    set_A(TWO);
    vstilbad();
}

/* ... VSTILBAD: in STILBADV, and ENDLRV. (A TS that overflows, entered
 * here with A so, skips the TCF: on to HIGATJOB.) */
void vstilbad(void)
{
    if (ts(ADDRESS_OF(STILBADV))) {
        higatjob();
        return;
    }
    endlrv();
}

/* ---- LRHJOB, HBAD, RDGIMS -- pp. 893-894 (bank 34) ---- */

/* LRHJOB: "INITIATE LR ALT MEASUREMENT" (LRALT, through BANKCALL); ... */
void lrhjob(void)
{
    tc_to(LRHJOB, BANKCALL); /* CADR LRALT */
}

/* ... "LRHJOB TO SLEEP ABOUT 95MS" (RADSTALL), ... */
void lrhjob_after_lralt(void)
{
    tc_to(LRHJOB + 2u, BANKCALL); /* CADR RADSTALL */
}

/* ... the bad return: HBAD; ... */
void lrhjob_bad_exit(void)
{
    hbad();
}

/* ... the good one: "IS DATA GOOD JUST PRESENT?" (STILBADH): "JUST GOOD --
 * MUST WAIT 4 SECONDS." (HSTILBAD, with it less one). Else, interrupts
 * held, "GOOD RETURN -- STORE AWAY LRH DATA" (HMEAS, "LRH DATA 1.079
 * FT/BIT"), and "FOR DOWNLINK" its time (MKTIME) and the CDUs, "CDUY,Z =
 * AIG,AMG" and "CDUX = AOG"; "SET BIT TO INDICATE RANGE" "MEASUREMENT
 * MADE." (RNGEDBIT), and ENDLRH. (A negative STILBADH skips the INHINT; a
 * -0 one, onto the DCA after its EXTEND, is not modelled.) */
void lrhjob_good_exit(void)
{
    switch (ccs_at(ADDRESS_OF(STILBADH))) {
    case POSITIVE: hstilbad(); return;
    case IS_PLUS_ZERO: inhint(); break;
    case NEGATIVE: break;
    case IS_MINUS_ZERO: not_modelled();
    }
    dca(ADDRESS_OF(SAMPLSUM)); /* "GOOD RETURN -- STORE AWAY LRH DATA" */
    dxch(ADDRESS_OF(HMEAS));   /* "LRH DATA 1.079 FT/BIT" */
    dca(ADDRESS_OF(PIPTIME1));
    dxch(ADDRESS_OF(MKTIME));
    dca(ADDRESS_OF(CDUTEMPY)); /* "CDUY,Z = AIG,AMG" */
    dxch(ADDRESS_OF(AIG));
    ca(ADDRESS_OF(CDUTEMPX)); /* "CDUX = AOG" */
    ts(ADDRESS_OF(AOG));
    cs(ADDRESS_OF(FLGWRD11)); /* "SET BIT TO INDICATE RANGE" */
    mask(RNGEDBIT);           /* "MEASUREMENT MADE." */
    ads(ADDRESS_OF(FLGWRD11));
    endlrh();
}

/* ENDLRH: "TERMATE LRHJOB". */
void endlrh(void)
{
    tc_to(ENDLRH, ENDOFJOB);
}

/* HBAD: "IS BAD RETURN DUE TO SCALE CHANGE?" (RNGSCBIT of FLAGWRD5): "NO
 * RESET HSTILBAD". "YES  RESET SCALE CHANGE BIT AND IGNORE" (RNGSCFLG), ... */
void hbad(void)
{
    ca(ADDRESS_OF(FLAGWRD5));
    mask(RNGSCBIT);
    if (A_is_zero()) {
        hbad_reset();
        return;
    }
    tc_to(HBAD + 4u, DOWNFLAG); /* ADRES RNGSCFLG */
}

/* ... and the job ends. */
void hbad_after_rngscflg(void)
{
    tc_to(HBAD + 6u, ENDOFJOB);
}

/* HSTILBAD -1: "SET STILBAD TO WAIT 4 SECONDS" (two passes); ... */
void hbad_reset(void)
{
    set_A(TWO);
    hstilbad();
}

/* ... HSTILBAD: in STILBADH, and the job ends. (A TS that overflows,
 * entered here with A so, skips the TC: on to RDGIMS.) */
void hstilbad(void)
{
    if (ts(ADDRESS_OF(STILBADH))) {
        rdgims();
        return;
    }
    tc_to(HSTILBAD + 1u, ENDOFJOB);
}

/* RDGIMS, "A TASK SET UP BY LRVJOB TO PICK UP THE IMU CDUS AND TIME": "PICK
 * UP TIME2, TIME1" "AND SAVE IN LRVTIME", the CDUs in LRXCDU, LRYCDU and
 * LRZCDU, and the PIPAs in PIPTEM, PIPTEM +1 and PIPTEM +2; the task
 * ends. */
void rdgims(void)
{
    dca(ADDRESS_OF(TIME2)); /* "PICK UP TIME2, TIME1" */
    dxch(ADDRESS_OF(LRVTIME));
    dca(ADDRESS_OF(CDUX)); /* "PICK UP CDUX AND CDUY" */
    dxch(ADDRESS_OF(LRXCDU));
    ca(ADDRESS_OF(CDUZ));
    ts(ADDRESS_OF(LRZCDU)); /* "SAVE CDUZ IN LRZCDU" */
    ca(ADDRESS_OF(PIPAX));
    ts(ADDRESS_OF(PIPTEM)); /* "SAVE PIPAX IN PIPTEM" */
    dca(ADDRESS_OF(PIPAY)); /* "PICK UP PIPAY AND PIPAZ" */
    dxch(ADDRESS_OF(PIPTEM) + 1u);
    tc_to(RDGIMS + 015u, TASKOVER);
}

/* ---- HIGATJOB, POSGOOD, POSALARM -- p. 895 ---- */

/* HIGATJOB: "START LRPOS2 JOB" (through BANKCALL); ... */
void higatjob(void)
{
    tc_to(HIGATJOB, BANKCALL); /* CADR LRPOS2 */
}

/* ... "PUT HIGATJOB TO SLEEP UNTIL JOB IS DONE" (RADSTALL); ... */
void higatjob_after_lrpos2(void)
{
    tc_to(HIGATJOB + 2u, BANKCALL); /* CADR RADSTALL */
}

/* ... "BAD END    ALARM" (POSALARM); the good end is POSGOOD. */
void higatjob_bad_exit(void)
{
    posalarm();
}

/* POSGOOD: "REDUCE PRIORITY FOR INTERPRETIVE COMPS." (PRIO23, PRIOCHNG);
 * ... */
void posgood(void)
{
    set_A(PRIO23);
    tc_to(POSGOOD + 1u, PRIOCHNG);
}

/* ... "LR IN POS2 -- SET UP TRANSFORMATIONS" (SETPOS2); ... */
void posgood_after_priochng(void)
{
    tc_at(POSGOOD + 2u);
    setpos2();
}

/* ... "RESET NOLRREAD FLAG TO ENABLE LR READING"; ... */
void posgood_after_setpos2(void)
{
    tc_to(POSGOOD + 3u, DOWNFLAG); /* ADRES NOLRREAD */
}

/* ... and the job ends. */
void posgood_after_nolrread(void)
{
    tc_to(POSGOOD + 5u, ENDOFJOB);
}

/* POSALARM: "FLASH ALARM CODE" 523 (PRIOLARM); ... */
void posalarm(void)
{
    set_A(OCT523);
    tc_to(POSALARM + 1u, BANKCALL); /* CADR PRIOLARM */
}

/* ... "TERMINATE" (GOTOPOOH); ... */
void posalarm_terminate(void)
{
    continue_at(GOTOPOOH);
}

/* ... "PROCEED -- TRY AGAIN"; ... */
void posalarm_proceed(void)
{
    posalarm_retry();
}

/* ... "V 32 E    TERMINATE R12"; ... */
void posalarm_enter(void)
{
    continue_at(ENDOFJOB);
}

/* ... PRIOLARM's immediate return: the job ends. */
void posalarm_immediate_return(void)
{
    tc_to(POSALARM + 6u, ENDOFJOB);
}

/* POSALARM +7 (the listing's "+3"): "SEE IF IN POS2 YET" (bit 7 of
 * channel 33 +0): "POS2 ACHIEVED   SET UP ANTENNA BEAMS" (POSGOOD); else
 * "STILL DIDN'T MAKE IT   REALARM". */
void posalarm_retry(void)
{
    set_A(BIT7);
    rand_channel(033u);
    if (A_is_zero()) {
        posgood();
        return;
    }
    posalarm();
}

/* ---- SETPOS1, SETPOS2, SETPOS -- pp. 895-897 ---- */

/* SETPOS1 ("MUST BE CALLED BY BANKCALL"): the caller's CADR (MAKECADR);
 * ... */
void setpos1(void)
{
    tc_to(SETPOS1, MAKECADR);
}

/* ... "SAVE RETURN CADR.  SINCE BUP2 CLOBBERED" (LRADRET1); STILBADH and
 * STILBADV two ("INITIALIZE STILBAD"); "SET L,M,R, ANS S TO ZERO", and
 * "INITIALIZE VSELECT"; "CONTINUE WITH COMPUTATIONS." with the index +0
 * ("INDEX FOR LRALPHA, LRBETA IN POS 1.") in A (SETPOS). (A TS that
 * overflows, entered here with A so, skips the CAF TWO.) */
void setpos1_after_makecadr(void)
{
    if (!ts(ADDRESS_OF(LRADRET1))) set_A(TWO);
    ts(ADDRESS_OF(STILBADH));
    ts(ADDRESS_OF(STILBADV));
    set_A(ZERO); /* "INDEX FOR LRALPHA, LRBETA IN POS 1." */
    ts(ADDRESS_OF(LRLCTR));
    ts(ADDRESS_OF(LRMCTR));
    ts(ADDRESS_OF(LRRCTR));
    ts(ADDRESS_OF(LRSCTR));
    ts(ADDRESS_OF(VSELECT));
    tc_at(SETPOS1 + 013u);
    setpos();
}

/* ... and "RETURN TO CALLER" (BANKJUMP, the CADR). */
void setpos1_after_setpos(void)
{
    ca(ADDRESS_OF(LRADRET1));
    tc_to(SETPOS1 + 015u, BANKJUMP);
}

/* SETPOS2: "INDEX FOR POS2" (two), and SETPOS. */
void setpos2(void)
{
    set_A(TWO);
    setpos();
}

/* SETPOS: "SAVE INDEX IN Q", "SAVE RETURN" (LRADRET); from E bank 5,
 * "LRALPHA IN A, LRBETA IN L" for the position (LRALPHA2 and LRBETA2 two
 * on), the rotations "ABOUT X" (CDUSPOT +4) and "ABOUT Y" (CDUSPOT), "ZERO
 * ROTATION ABOUT Z."; back to E bank 7, and to the interpreter. (A TS that
 * overflows, entered here with Q so, skips the CA EBANK5. Not modelled:
 * an index other than 0 and 2.) */
void setpos(void)
{
    xch(Q_ADDRESS); /* "SAVE INDEX IN Q" */
    if (!ts(ADDRESS_OF(LRADRET))) set_A(EBANK5);
    ts(EBANK_ADDRESS);
    if (Q != 0u && Q != 2u) not_modelled();
    dca(indexed(ADDRESS_OF(LRALPHA), (word)Q)); /* "LRALPHA IN A, LRBETA IN L" */
    ts(ADDRESS_OF(CDUSPOT) + 4u);               /* "ROTATION ABOUT X" */
    lxch(ADDRESS_OF(CDUSPOT));                  /* "ROTATION ABOUT Y" */
    set_A(ZERO);
    ts(ADDRESS_OF(CDUSPOT) + 2u); /* "ZERO ROTATION ABOUT Z." */
    set_A(EBANK7);
    ts(EBANK_ADDRESS);
    tc_at(SETPOS + 015u);
    intpret();
}

/* The beams in the navigation base's frame: UNITY and UNITX of the
 * antenna (TRG*SMNB, *SMNB*, with the rotations in CDUSPOT) the Y and X
 * beams, "Z = X * Y", and the altitude beam HBEAMANT "CONVERT TO NB". */
static const word setpos_words[] = {
    /* SETPOS +16 */ ORDERS(VLOAD, CALL),
    ADDRESS_PLUS_ONE(UNITY), /* "CONVERT UNITY(ANTENNA) TO NB" */
    ADDRESS(TRG_STAR_SMNB),
    STOVL(VYBEAMNB),
    ADDRESS_PLUS_ONE(UNITX), /* "CONVERT UNITX(ANTENNA) TO NB" */
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR),
    STORE(VXBEAMNB),
    ORDERS(VXV, VSL1),
    ADDRESS_PLUS_ONE(VYBEAMNB),
    STOVL(VZBEAMNB), /* "Z = X * Y" */
    ADDRESS_PLUS_ONE(HBEAMANT),
    ORDER(CALL),
    ADDRESS(STAR_SMNB_STAR), /* "CONVERT TO NB" */
    STORE(HBEAMNB),
    ORDER(EXIT),
};

const struct interpretive_code SETPOS_CODE = {033u, SETPOS + 016u, COUNT(setpos_words),
                                              setpos_words};

/* After it, back to the caller (TC LRADRET). */
void setpos_after_exit(void)
{
    tc_through_computed(ADDRESS_OF(LRADRET));
}

#endif
