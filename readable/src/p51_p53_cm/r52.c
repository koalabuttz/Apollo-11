/* SPDX-License-Identifier: MIT */
/* P51-P53.agc of the CM, part 2 (Comanche055 pp. 743-747, bank 14): R52,
 * the automatic optics positioning routine, and ADVORB.
 *
 * R52 "TO POINT THE STAR LOS OF THE OPTICS AT A STAR OR LANDMARK DEFINED
 * BY THE PROGRAM OR BY DSKY INPUT", or at the LM while tracking it. Its
 * loop: SR52.1 finds where the optics must point; R52 drives them there
 * (DESOPTT, DESOPTS) while the optics mode switch (SWSAMPLE) is in CMC,
 * every half second (DELAYJOB), until TERMIFLG is set; then back to the
 * caller (SAVQR52). Past 90 degrees of trunnion, alarm 404 (PRIOLARM); in
 * manual mode, R53, the sighting mark routine, as a job of its own.
 *
 * Each call out of the file (BANKCALL, UPFLAG, DOWNFLAG, ALARM, FINDVAC,
 * CLEARMRK) leaves Z at the routine and Q at the word after the TC, where
 * the routine comes back to; ENDOFJOB and GOTOPOOH don't come back. The
 * faithful port keys a routine at each such return, at each TC's return
 * in the file, and at each TC INTPRET's NEWOPS. */
#include "p51_p53_cm.h"
#include "../agc/names.h"
#include "../interpreter/interpreter.h"

#if COMANCHE055

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void r52d(void);
static void r52f(void);
static void r52f_plus1(void);
static void r52q(void);
static void r52l(void);

/* ---- R52 -- pp. 744-745 ---- */

/* R52: the caller's return to SAVQR52, ADVTRK cleared; R52VRB, where
 * ADVORB comes in too. */
static const word r52_words[] = {
    /* R52 */ ORDERS(STQ, CLEAR),
    ADDRESS(SAVQR52),
    SWITCH(CLEAR, ADVTRK),
    /* R52VRB */ ORDER(EXIT),
};

const struct interpretive_code R52_CODE = {014u, R52, COUNT(r52_words), r52_words};

/* R52VRB +1: the optics to stay where they are for now (CDUT and CDUS to
 * DESOPTT and DESOPTS). */
void r52vrb_after_exit(void)
{
    dca(ADDRESS_OF(CDUT));
    dxch(ADDRESS_OF(DESOPTT));
    tc_at(R52VRB + 4u);
    intpret();
}

/* OPTIND +0 and R53FLAG cleared. */
static const word r52vrb_words[] = {
    /* R52VRB +5 */ ORDERS(SSP, CLEAR),
    ADDRESS_PLUS_ONE(OPTIND),
    NUMBER(0),
    SWITCH(CLEAR, R53FLAG),
    ORDER(EXIT),
};

const struct interpretive_code R52VRB_CODE = {014u, R52VRB + 5u, COUNT(r52vrb_words), r52vrb_words};

/* R52A: TRUNFLAG set; with TARG1FLG set, the LM's (R52H); else TERMIFLG
 * cleared. */
void r52a(void)
{
    tc_at(R52A);
    intpret();
}

static const word r52a_words[] = {
    /* R52A +1 */ ORDERS(SET, BON),
    SWITCH(SET, TRUNFLAG),
    SWITCH(BON, TARG1FLG),
    ADDRESS(R52H),
    ORDERS(CLEAR, EXIT),
    SWITCH(CLEAR, TERMIFLG),
};

const struct interpretive_code R52A_CODE = {014u, R52A + 1u, COUNT(r52a_words), r52a_words};

/* R52C: "IS OPTICS MODE IN AGC" (SWSAMPLE, its sign bit): no, "MANUAL"
 * (R52M); yes, SR52.1 (R52D). */
void r52c(void)
{
    ca(ADDRESS_OF(SWSAMPLE));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF R52M */
        r52m();
        return;
    }
    r52d();
}

/* R52D: SR52.1, where the optics must point. It returns to one of three
 * words, by the trunnion angle. */
static void r52d(void)
{
    tc_to(R52D, BANKCALL); /* CADR SR52.1 */
}

/* "GR 90 DEGREES" (R52L). */
void r52d_over_90(void)
{
    r52l(); /* TCF R52L */
}

/* "GR 50 DEGREES" (R52J). */
void r52d_over_50(void)
{
    r52j(); /* TCF R52J */
}

/* "LS 50 DEGREES": "SET TRUNFLAG BIT 4 FLAG 0" (UPFLAG), then R52JA. */
void r52d_under_50(void)
{
    tc_to(R52D + 4u, UPFLAG); /* ADRES TRUNFLAG */
}

/* R52JA: "IS THIS A LEM" (STATE +1, bit 10): yes, R52E. (After each MASK,
 * A is +0 or the bit: CCS takes its first two ways only.) */
void r52ja(void)
{
    set_A(BIT10);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(R52JA + 3u); /* TC R52E */
        r52e();
        return;
    }
    r52ja_not_lem();
}

/* R52JA +4: "NO, IS R53FLAG SET" (STATE, bit 6): yes, R52E; no, the angles
 * shown (V06N92, GODSPR). */
void r52ja_not_lem(void)
{
    set_A(BIT6);
    mask_at(ADDRESS_OF(STATE));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        r52e(); /* TCF R52E */
        return;
    }
    set_A(V06N92);
    tc_to(R52JA + 011u, BANKCALL); /* CADR GODSPR */
}

/* R52E: "IS OSS IN CMC MODE" (SWSAMPLE): no, R52F; yes, "IS TRUNFLAG SET"
 * (STATE, bit 4, through CS: +0 when it is): no, only the shaft (SAC to
 * DESOPTS); yes, the trunnion too (PAC to DESOPTT). */
void r52e(void)
{
    ca(ADDRESS_OF(SWSAMPLE));
    if (A_is_zero_or_minus()) { /* EXTEND; BZMF R52F */
        r52f();
        return;
    }
    cs(ADDRESS_OF(STATE));
    mask(BIT4);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(R52E + 6u); /* TC +3 */
        ca(ADDRESS_OF(SAC));
        ts(ADDRESS_OF(DESOPTS));
        r52f();
        return;
    }
    r52e_trunnion();
}

/* R52E +7: PAC to DESOPTT and SAC to DESOPTS. (CA leaves A without
 * overflow: the TSs do not skip.) */
void r52e_trunnion(void)
{
    ca(ADDRESS_OF(PAC));
    ts(ADDRESS_OF(DESOPTT));
    ca(ADDRESS_OF(SAC));
    ts(ADDRESS_OF(DESOPTS));
    r52f();
}

/* R52F: "WAIT 1/2 SEC" (DELAYJOB). */
static void r52f(void)
{
    set_A(HALF_SEC);
    r52f_plus1();
}

static void r52f_plus1(void)
{
    tc_to(R52F + 1u, BANKCALL); /* CADR DELAYJOB */
}

/* R52F +3: a LM (STATE +1, bit 10), R52HA; else "IS TERMIFLG SET" (STATE
 * +7, bit 15): no, round again (R52C); yes, back to the caller (R52Q). */
void r52f_after_delayjob(void)
{
    set_A(BIT10);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        r52ha(); /* TCF R52HA */
        return;
    }
    set_A(BIT15);
    mask_at(ADDRESS_OF(STATE) + 7u);
    if (A_is_zero()) { /* EXTEND; BZF R52C */
        r52c();
        return;
    }
    r52q();
}

/* R52Q: back to the caller (GOTO SAVQR52). */
static void r52q(void)
{
    tc_at(R52Q);
    intpret();
}

static const word r52q_words[] = {
    /* R52Q +1 */ ORDER(GOTO),
    ADDRESS(SAVQR52),
    /* R52H */ ORDER(EXIT),
};

const struct interpretive_code R52Q_CODE = {014u, R52Q + 1u, COUNT(r52q_words), r52q_words};

/* R52HA: the LM's: R61CSM points the CM at it. */
void r52ha(void)
{
    tc_to(R52HA, BANKCALL); /* CADR R61CSM */
}

/* R52HA +2: without TRACKFLG (STATE +1, bit 5), back to the caller
 * (R52Q). Without UPDATFLG (bit 7, through CS: +0 when it is set),
 * R52SYNC; with it, R52I: with PRFTRKAT (STATE +5, bit 10), SR52.1 again
 * (TC R52D); else R52SYNC. */
void r52ha_after_r61csm(void)
{
    ca(ADDRESS_OF(STATE) + 1u);
    mask(BIT5);
    if (A_is_zero()) { /* EXTEND; BZF R52Q */
        r52q();
        return;
    }
    cs(ADDRESS_OF(STATE) + 1u);
    mask(BIT7);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        r52sync(); /* TCF R52SYNC */
        return;
    }
    ca(ADDRESS_OF(STATE) + 5u);
    mask(BIT10);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(R52I + 3u); /* TC R52D */
        r52d();
        return;
    }
    r52sync();
}

/* R52SYNC: "MAKE UP FOR LOST TIME": 1.8 seconds, then DELAYJOB (TCF R52F
 * +1). */
void r52sync(void)
{
    set_A(ONE_8SEC);
    r52f_plus1();
}

/* R52J: "CLEAR TRUNFLAG" (DOWNFLAG), "SET 407 ALARM", then R52JA. */
void r52j(void)
{
    tc_to(R52J, DOWNFLAG); /* ADRES TRUNFLAG */
}

void r52j_after_downflag(void)
{
    tc_to(R52J + 2u, ALARM); /* OCT 407 */
}

void r52j_after_alarm(void)
{
    tc_at(R52J + 4u); /* TC R52JA */
    r52ja();
}

/* R52M: "IS R53FLAG SET": yes, R52F; no, R53JOB as a job (FINDVAC, PRIO24)
 * with interrupts inhibited. */
void r52m(void)
{
    set_A(BIT6);
    mask_at(ADDRESS_OF(STATE));
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(R52M + 3u); /* TC R52F */
        r52f();
        return;
    }
    r52m_no_r53();
}

void r52m_no_r53(void)
{
    inhint();
    set_A(PRIO24);
    tc_to(R52M + 6u, FINDVAC); /* 2CADR R53JOB */
}

void r52m_after_findvac(void)
{
    relint();
    r52f(); /* TCF R52F */
}

/* R53JOB: R53, the sighting marks; ENDPLAC, "INTERPRETER RETURN TO
 * ENDOFJOB (R22 USES)". */
void r53job(void)
{
    tc_at(R53JOB);
    intpret();
}

static const word r53job_words[] = {
    /* R53JOB +1 */ ORDER(CALL),
    ADDRESS(R53),
    /* ENDPLAC */ ORDER(EXIT),
};

const struct interpretive_code R53JOB_CODE = {014u, R53JOB + 1u, COUNT(r53job_words), r53job_words};

void endplac_after_exit(void)
{
    tc_to(ENDPLAC + 1u, ENDOFJOB);
}

/* ---- R52L and TERM52 -- pp. 746-747 ---- */

/* R52L: "IS THIS A LEM": yes, R52J; no, alarm 404 (PRIOLARM): terminate
 * (TERM52), proceed (R52F), "NO PROVISION FOR NEW DATA" (R52F), or the
 * immediate return (ENDOFJOB). */
static void r52l(void)
{
    set_A(BIT10);
    mask_at(ADDRESS_OF(STATE) + 1u);
    if (ccs_register(A_ADDRESS) == POSITIVE) {
        tc_at(R52L + 3u); /* TC R52J */
        r52j();
        return;
    }
    r52l_not_lem();
}

void r52l_not_lem(void)
{
    set_A(OCT404);
    tc_to(R52L + 5u, BANKCALL); /* CADR PRIOLARM */
}

void r52l_terminate(void)
{
    term52(); /* TCF TERM52 */
}

void r52l_proceed(void)
{
    r52f(); /* TCF R52F */
}

void r52l_enter(void)
{
    r52f(); /* TCF R52F */
}

void r52l_immediate_return(void)
{
    continue_at(ENDOFJOB); /* TCF ENDOFJOB */
}

/* TERM52: the marks cleared (CLEARMRK), "KILL MARK SYSTEM" (MKRELEAS),
 * OPTCADR +0, "CLEAR OUT EXTENDED VERBS" (KLEENEX), "NO GO TO P00". */
void term52(void)
{
    tc_to(TERM52, CLEARMRK);
}

void term52_after_clearmrk(void)
{
    tc_to(TERM52 + 1u, BANKCALL); /* CADR MKRELEAS */
}

/* (A is +0: TS does not skip.) */
void term52_after_mkreleas(void)
{
    set_A(ZERO);
    ts(ADDRESS_OF(OPTCADR));
    tc_to(TERM52 + 5u, BANKCALL); /* CADR KLEENEX */
}

void term52_after_kleenex(void)
{
    tc_to(TERM52 + 7u, GOTOPOOH);
}

/* ADVORB: "SETS UP ADVANCED ORBIT TRACKING": the caller's return to
 * SAVQR52, ADVTRK, LUNAFLAG and ERADFLAG set, then R52VRB. */
static const word advorb_words[] = {
    /* ADVORB */ ORDERS(STQ, SET),
    ADDRESS(SAVQR52),
    SWITCH(SET, ADVTRK),
    ORDERS(SET, SET),
    SWITCH(SET, LUNAFLAG),
    SWITCH(SET, ERADFLAG),
    ORDER(GOTO),
    ADDRESS(R52VRB),
};

const struct interpretive_code ADVORB_CODE = {014u, ADVORB, COUNT(advorb_words), advorb_words};

#endif
